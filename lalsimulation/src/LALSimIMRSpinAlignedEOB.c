/*
*  Copyright (C) 2011 Craig Robinson, Enrico Barausse
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with with program; see the file COPYING. If not, write to the
*  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*/

/**
 * \author Craig Robinson
 *
 * \file 
 *
 * \brief Functions for producing EOB waveforms for 
 * spinning binaries, as described in Barausse and Buonanno ( arXiv 0912.3517 ).
 */

#include <lal/LALSimInspiral.h>
#include <lal/LALSimIMR.h>
#include <lal/TimeSeries.h>
#include <lal/Units.h>
#include <lal/LALAdaptiveRungeKutta4.h>
#include <lal/NRWaveInject.h>
#include <gsl/gsl_sf_gamma.h>

#include "LALSimIMREOBNRv2.h"
#include "LALSimIMRSpinEOB.h"

/* Include all the static function files we need */
#include "LALSimIMREOBHybridRingdown.c"
#include "LALSimIMREOBFactorizedWaveform.c"
#include "LALSimIMREOBNewtonianMultipole.c"
#include "LALSimIMREOBNQCCorrection.c"
#include "LALSimIMRSpinEOBInitialConditions.c"
#include "LALSimIMRSpinEOBAuxFuncs.c"
#include "LALSimIMRSpinAlignedEOBHcapDerivative.c"
#include "LALSimIMRSpinEOBHamiltonian.c"
#include "LALSimIMRSpinEOBFactorizedWaveform.c"
#include "LALSimIMRSpinEOBFactorizedFlux.c"

#ifdef __GNUC__
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

static int
XLALEOBSpinAlignedStopCondition(double UNUSED t,
                           const double values[],
                           double dvalues[],
                           void *funcParams
                          )
{

  REAL8 omega, r;
  SpinEOBParams *params = (SpinEOBParams *)funcParams;

  r     = values[0];
  omega = dvalues[1];

  //if ( omega < params->eobParams->omega )
  if ( r < 6. && omega < params->eobParams->omega )
  {
    return 1;
  }

  params->eobParams->omega = omega;
  return GSL_SUCCESS;
}

static int
XLALSpinAlignedHiSRStopCondition(double UNUSED t,
                           const double UNUSED values[],
                           double dvalues[],
                           void UNUSED *funcParams
                          )
{

  if ( values[0] <= 1.8 || isnan( dvalues[3] ) || isnan (dvalues[2]) || isnan (dvalues[1]) || isnan (dvalues[0]) )
  {
    return 1;
  }
  return GSL_SUCCESS;
}

int XLALSimIMRSpinAlignedEOBWaveform(
        REAL8TimeSeries **hplus,
        REAL8TimeSeries **hcross,
        const REAL8     UNUSED phiC,
        REAL8           deltaT,
        const REAL8     m1,
        const REAL8     m2,
        const REAL8     fMin,
        const REAL8     r,
        const REAL8     inc,
        const REAL8     spin1z,
        const REAL8     spin2z
     )
{

  INT4 i;

  REAL8Vector *values = NULL;

  /* EOB spin vectors used in the Hamiltonian */
  REAL8Vector *sigmaStar = NULL;
  REAL8Vector *sigmaKerr = NULL;
  REAL8       a;
  REAL8       chiS, chiA;

  /* Wrapper spin vectors used to calculate sigmas */
  REAL8Vector s1Vec;
  REAL8Vector s2Vec;
  REAL8       spin1[3] = {0, 0, spin1z};
  REAL8       spin2[3] = {0, 0, spin2z};
  REAL8       s1Data[3], s2Data[3];


  /* Parameters of the system */
  REAL8 mTotal, eta, mTScaled;
  REAL8 amp0;
  LIGOTimeGPS tc = LIGOTIMEGPSZERO;

  /* Dynamics of the system */
  REAL8Vector rVec, phiVec, prVec, pPhiVec;
  REAL8       omega, v, ham;

  /* Cartesian vectors needed to calculate Hamiltonian */
  REAL8Vector cartPosVec, cartMomVec;
  REAL8       cartPosData[3], cartMomData[3];

  /* Signal mode */
  COMPLEX16   hLM;
  REAL8Vector *sigReVec = NULL, *sigImVec = NULL;

  /* Non-quasicircular correction */
  EOBNonQCCoeffs nqcCoeffs;
  COMPLEX16      hNQC;
  REAL8Vector    *ampNQC = NULL, *phaseNQC = NULL;

  /* Ringdown freq used to check the sample rate */
  COMPLEX16Vector modefreqVec;
  COMPLEX16      modeFreq;

  /* Spin-weighted spherical harmonics */
  COMPLEX16  MultSphHarmP;
  COMPLEX16  MultSphHarmM;

  /* We will have to switch to a high sample rate for ringdown attachment */
  UINT4 resampFac;
  UINT4 resampPwr;
  REAL8 resampEstimate;

  /* How far will we have to step back to attach the ringdown? */
  REAL8 tStepBack;
  INT4  nStepBack;

  /* Dynamics and details of the high sample rate part used to attach the ringdown */
  UINT4 hiSRndx;
  REAL8Vector timeHi, rHi, phiHi, prHi, pPhiHi;
  REAL8Vector *sigReHi = NULL, *sigImHi = NULL;
  REAL8Vector *omegaHi = NULL;

  /* Indices of peak frequency and final point */
  /* Needed to attach ringdown at the appropriate point */
  UINT4 peakIdx = 0, finalIdx = 0;

  /* (2,2) and (2,-2) spherical harmonics needed in (h+,hx) */
  REAL8 y_1, y_2, z1, z2;

  /* Variables for the integrator */
  ark4GSLIntegrator       *integrator = NULL;
  REAL8Array              *dynamics   = NULL;
  REAL8Array              *dynamicsHi = NULL;
  INT4                    retLen;
  REAL8  UNUSED           tMax;

  /* Accuracies of adaptive Runge-Kutta integrator */
  const REAL8 EPS_ABS = 1.0e-10;
  const REAL8 EPS_REL = 1.0e-9;

  /* Parameter structures containing important parameters for the model */
  SpinEOBParams           seobParams;
  SpinEOBHCoeffs          seobCoeffs;
  EOBParams               eobParams;
  FacWaveformCoeffs       hCoeffs;
  NewtonMultipolePrefixes prefixes;

  /* Initialize parameters */
  mTotal = m1 + m2;
  mTScaled = mTotal * LAL_MTSUN_SI;
  eta    = m1 * m2 / (mTotal*mTotal);

  amp0 = mTotal * LAL_MRSUN_SI / r;

  /* TODO: Insert potentially necessary checks on the arguments */

  /* Calculate the time we will need to step back for ringdown */
  tStepBack = 50. * mTScaled;
  nStepBack = ceil( tStepBack / deltaT );

  /* Calculate the resample factor for attaching the ringdown */
  /* We want it to be a power of 2 */
  /* Of course, we only want to do this if the required SR > current SR... */
  /* The form chosen for the resampleEstimate will essentially set */
  /* deltaT = M / 20. ( or less taking into account the power of 2 stuff */
  resampEstimate = 50. * deltaT / mTScaled;
  resampFac = 1;

  if ( resampEstimate > 1. )
  {
    resampPwr = (UINT4)ceil( log2( resampEstimate ) );
    while ( resampPwr-- )
    {
      resampFac *= 2u;
    }
  }

  /* Allocate the values vector to contain the ICs */
  /* Since we have aligned spins, we can use the 4-d vector as in the non-spin case */
  if ( !(values = XLALCreateREAL8Vector( 4 )) )
  {
    XLAL_ERROR( XLAL_ENOMEM );
  }
  memset ( values->data, 0, values->length * sizeof( REAL8 ));

  /* Set up structures and calculate necessary PN parameters */
  /* Unlike the general case, we only need to calculate these once */
  memset( &seobParams, 0, sizeof(seobParams) );
  memset( &seobCoeffs, 0, sizeof(seobCoeffs) );
  memset( &eobParams, 0, sizeof(eobParams) );
  memset( &hCoeffs, 0, sizeof( hCoeffs ) );
  memset( &prefixes, 0, sizeof( prefixes ) );

  /* Before calculating everything else, check sample freq is high enough */
  modefreqVec.length = 1;
  modefreqVec.data   = &modeFreq;

  if ( XLALSimIMREOBGenerateQNMFreqV2( &modefreqVec, m1, m2, spin1, spin2, 2, 2, 1, SEOBNRv1 ) == XLAL_FAILURE )
  {
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_EFUNC );
  }

  /* If Nyquist freq < 220 QNM freq, exit */
  if ( deltaT > LAL_PI / modeFreq.re )
  {
    XLALPrintError( "XLAL Error - %s: Ringdown frequency > Nyquist frequency!\nAt present this situation is not supported.\n", __func__);
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_EINVAL );
  }

  if ( !(sigmaStar = XLALCreateREAL8Vector( 3 )) )
  {
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_ENOMEM );
  }

  if ( !(sigmaKerr = XLALCreateREAL8Vector( 3 )) )
  {
    XLALDestroyREAL8Vector( sigmaStar );
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_ENOMEM );
  }

  seobParams.alignedSpins = 1;
  seobParams.tortoise     = 1;
  seobParams.sigmaStar    = sigmaStar;
  seobParams.sigmaKerr    = sigmaKerr;
  seobParams.seobCoeffs   = &seobCoeffs;
  seobParams.eobParams    = &eobParams;
  eobParams.hCoeffs       = &hCoeffs;
  eobParams.prefixes      = &prefixes;

  eobParams.m1  = m1;
  eobParams.m2  = m2;
  eobParams.eta = eta;

  s1Vec.length = s2Vec.length = 3;
  s1Vec.data   = s1Data;
  s2Vec.data   = s2Data;

  /* copy the spins into the appropriate vectors, and scale them by the mass */
  memcpy( s1Data, spin1, sizeof( s1Data ) );
  memcpy( s2Data, spin2, sizeof( s2Data ) );

  /* Calculate chiS and chiA */
  /* XXX I am assuming that, since spins are aligned, it is okay to just use the z component XXX */
  /* TODO: Check this is actually the way it works in LAL */
  chiS = 0.5 * (spin1[2] + spin2[2]);
  chiA = 0.5 * (spin1[2] - spin2[2]);

  for( i = 0; i < 3; i++ )
  {
    s1Data[i] *= m1*m1;
    s2Data[i] *= m2*m2;
  }

  cartPosVec.length = cartMomVec.length = 3;
  cartPosVec.data = cartPosData;
  cartMomVec.data = cartMomData;
  memset( cartPosData, 0, sizeof( cartPosData ) );
  memset( cartMomData, 0, sizeof( cartMomData ) );

  /* Populate the initial structures */
  if ( XLALSimIMRSpinEOBCalculateSigmaStar( sigmaStar, m1, m2, &s1Vec, &s2Vec ) == XLAL_FAILURE )
  {
    XLALDestroyREAL8Vector( sigmaKerr );
    XLALDestroyREAL8Vector( sigmaStar );
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_EFUNC );
  }

  if ( XLALSimIMRSpinEOBCalculateSigmaKerr( sigmaKerr, m1, m2, &s1Vec, &s2Vec ) == XLAL_FAILURE )
  {
    XLALDestroyREAL8Vector( sigmaKerr );
    XLALDestroyREAL8Vector( sigmaStar );
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_EFUNC );
  }

  /* Calculate the value of a */
  /* XXX I am assuming that, since spins are aligned, it is okay to just use the z component XXX */
  /* TODO: Check this is actually the way it works in LAL */
  a = 0.0;
  /*for ( i = 0; i < 3; i++ )
  {
    a += sigmaKerr->data[i]*sigmaKerr->data[i];
  }
  a = sqrt( a );*/
  seobParams.a = a = sigmaKerr->data[2];

  if ( XLALSimIMREOBCalcSpinFacWaveformCoefficients( &hCoeffs, eta, /*a*/0.0, chiS, chiA ) == XLAL_FAILURE )
  {
    XLALDestroyREAL8Vector( sigmaKerr );
    XLALDestroyREAL8Vector( sigmaStar );
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_EFUNC );
  }

  if ( XLALSimIMREOBComputeNewtonMultipolePrefixes( &prefixes, eobParams.m1, eobParams.m2 )
         == XLAL_FAILURE )
  {
    XLALDestroyREAL8Vector( sigmaKerr );
    XLALDestroyREAL8Vector( sigmaStar );
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_EFUNC );
  }

  
  /* Set the initial conditions. For now we use the generic case */
  /* TODO: Simplify this if need be */
  REAL8Vector *tmpValues = XLALCreateREAL8Vector( 14 );
  if ( !tmpValues )
  {
    XLALDestroyREAL8Vector( sigmaKerr );
    XLALDestroyREAL8Vector( sigmaStar );
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_ENOMEM );
  }

  memset( tmpValues->data, 0, tmpValues->length * sizeof( REAL8 ) );

  /* We set inc zero here to make it easier to go from Cartesian to spherical coords */
  if ( XLALSimIMRSpinEOBInitialConditions( tmpValues, m1, m2, fMin, 0, s1Data, s2Data, &seobParams ) == XLAL_FAILURE )
  {
    XLALDestroyREAL8Vector( tmpValues );
    XLALDestroyREAL8Vector( sigmaKerr );
    XLALDestroyREAL8Vector( sigmaStar );
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_EFUNC );
  }

  fprintf( stderr, "ICs = %.16e %.16e %.16e %.16e %.16e %.16e %.16e %.16e %.16e %.16e %.16e %.16e\n", tmpValues->data[0], tmpValues->data[1], tmpValues->data[2],
      tmpValues->data[3], tmpValues->data[4], tmpValues->data[5], tmpValues->data[6], tmpValues->data[7], tmpValues->data[8],
      tmpValues->data[9], tmpValues->data[10], tmpValues->data[11] );

  /* Taken from Andrea's code */
/*  memset( tmpValues->data, 0, tmpValues->length*sizeof(tmpValues->data[0]));*/
/*
  tmpValues->data[0] = 12.983599142327673;
  tmpValues->data[3] = -0.002383249720459786;
  tmpValues->data[4] = 4.3204065947459735/tmpValues->data[0];
*/
  /* Now convert to Spherical */
  values->data[0] = tmpValues->data[0];
  values->data[1] = 0.;
  values->data[2] = tmpValues->data[3];
  values->data[3] = tmpValues->data[0] * tmpValues->data[4];

  fprintf( stderr, "Spherical initial conditions: %e %e %e %e\n", values->data[0], values->data[1], values->data[2], values->data[3] );

  XLALDestroyREAL8Vector( tmpValues );

  /* Now compute the spinning H coefficients, just in case the don't have the right values */
  if ( XLALSimIMRCalculateSpinEOBHCoeffs( &seobCoeffs, eta, a ) == XLAL_FAILURE )
  {    
    XLALDestroyREAL8Vector( tmpValues );
    XLALDestroyREAL8Vector( sigmaKerr );
    XLALDestroyREAL8Vector( sigmaStar );
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_EFUNC );
  }

  /* Now we have the initial conditions, we can initialize the adaptive integrator */
  if (!(integrator = XLALAdaptiveRungeKutta4Init(4, XLALSpinAlignedHcapDerivative, XLALEOBSpinAlignedStopCondition, EPS_ABS, EPS_REL)))
  {
    XLALDestroyREAL8Vector( values );
    XLAL_ERROR( XLAL_EFUNC );
  }

  integrator->stopontestonly = 1;
  integrator->retries = 1;

  retLen = XLALAdaptiveRungeKutta4( integrator, &seobParams, values->data, 0., 20./mTScaled, deltaT/mTScaled, &dynamics );
  if ( retLen == XLAL_FAILURE )
  {
    XLAL_ERROR( XLAL_EFUNC );
  }

  /* Set up pointers to the dynamics */
  rVec.length = phiVec.length = prVec.length = pPhiVec.length = retLen;
  rVec.data    = dynamics->data+retLen;
  phiVec.data  = dynamics->data+2*retLen;
  prVec.data   = dynamics->data+3*retLen;
  pPhiVec.data = dynamics->data+4*retLen;

  printf( "We think we hit the peak at time %e\n", dynamics->data[retLen-1] );

  /* TODO : Insert high sampling rate / ringdown here */
  FILE *out = fopen( "saDynamics.dat", "w" );
  for ( i = 0; i < retLen; i++ )
  {
    fprintf( out, "%.16e %.16e %.16e %.16e %.16e\n", dynamics->data[i], rVec.data[i], phiVec.data[i], prVec.data[i], pPhiVec.data[i] );
  }
  fclose( out );

  /* Set up the high sample rate integration */
  hiSRndx = retLen - nStepBack;
  deltaT /= (REAL8)resampFac;

  fprintf( stderr, "Stepping back %d points - we expect %d points at high SR\n", nStepBack, nStepBack*resampFac );
  fprintf( stderr, "Commencing high SR integration... from %.16e %.16e %.16e %.16e %.16e\n",
     (dynamics->data)[hiSRndx],rVec.data[hiSRndx], phiVec.data[hiSRndx], prVec.data[hiSRndx], pPhiVec.data[hiSRndx] );

  values->data[0] = rVec.data[hiSRndx];
  values->data[1] = phiVec.data[hiSRndx];
  values->data[2] = prVec.data[hiSRndx];
  values->data[3] = pPhiVec.data[hiSRndx];

  integrator->stop = XLALSpinAlignedHiSRStopCondition;

  retLen = XLALAdaptiveRungeKutta4( integrator, &seobParams, values->data, 0., 20./mTScaled, deltaT/mTScaled, &dynamicsHi );
  if ( retLen == XLAL_FAILURE )
  {
    XLAL_ERROR( XLAL_EFUNC );
  }

  fprintf( stderr, "We got %d points at high SR\n", retLen );

  /* Set up pointers to the dynamics */
  rHi.length = phiHi.length = prHi.length = pPhiHi.length = timeHi.length = retLen;
  timeHi.data = dynamicsHi->data;
  rHi.data    = dynamicsHi->data+retLen;
  phiHi.data  = dynamicsHi->data+2*retLen;
  prHi.data   = dynamicsHi->data+3*retLen;
  pPhiHi.data = dynamicsHi->data+4*retLen;

  out = fopen( "saDynamicsHi.dat", "w" );
  for ( i = 0; i < retLen; i++ )
  {
    fprintf( out, "%.16e %.16e %.16e %.16e %.16e\n", timeHi.data[i], rHi.data[i], phiHi.data[i], prHi.data[i], pPhiHi.data[i] );
  }
  fclose( out );

  /* Allocate the high sample rate vectors */
  sigReHi  = XLALCreateREAL8Vector( retLen + (UINT4)ceil( 20 / ( modeFreq.im * deltaT )) );
  sigImHi  = XLALCreateREAL8Vector( retLen + (UINT4)ceil( 20 / ( modeFreq.im * deltaT )) );
  omegaHi  = XLALCreateREAL8Vector( retLen + (UINT4)ceil( 20 / ( modeFreq.im * deltaT )) );
  ampNQC   = XLALCreateREAL8Vector( retLen );
  phaseNQC = XLALCreateREAL8Vector( retLen );

  if ( !sigReHi || !sigImHi || !omegaHi || !ampNQC || !phaseNQC )
  {
    XLAL_ERROR( XLAL_ENOMEM );
  }

  memset( sigReHi->data, 0, sigReHi->length * sizeof( sigReHi->data[0] ));
  memset( sigImHi->data, 0, sigImHi->length * sizeof( sigImHi->data[0] ));

  /* Populate the high SR waveform */
  REAL8 omegaOld = 0.0;
  INT4  phaseCounter = 0;
  for ( i = 0; i < retLen; i++ )
  {
    values->data[0] = rHi.data[i];
    values->data[1] = phiHi.data[i];
    values->data[2] = prHi.data[i];
    values->data[3] = pPhiHi.data[i];

    omegaHi->data[i] = omega = XLALSimIMRSpinAlignedEOBCalcOmega( values->data, &seobParams );
    v = cbrt( omega );

    /* Calculate the value of the Hamiltonian */
    cartPosVec.data[0] = values->data[0];
    cartMomVec.data[0] = values->data[2];
    cartMomVec.data[1] = values->data[3] / values->data[0];

    ham = XLALSimIMRSpinEOBHamiltonian( eta, &cartPosVec, &cartMomVec, sigmaKerr, sigmaStar, seobParams.tortoise, &seobCoeffs );

    if ( XLALSimIMRSpinEOBGetSpinFactorizedWaveform( &hLM, values, v, ham, 2, 2, &seobParams )
           == XLAL_FAILURE )
    {
      /* TODO: Clean-up */
      XLAL_ERROR( XLAL_EFUNC );
    }

    ampNQC->data[i]  = XLALCOMPLEX16Abs( hLM );
    sigReHi->data[i] = (REAL4)(amp0 * hLM.re);
    sigImHi->data[i] = (REAL4)(amp0 * hLM.im);
    phaseNQC->data[i]= XLALCOMPLEX16Arg( hLM ) + phaseCounter * LAL_TWOPI;

    if ( i && phaseNQC->data[i] > phaseNQC->data[i-1] )
    {
      phaseCounter--;
      phaseNQC->data[i] -= LAL_TWOPI;
    }

    if ( omega <= omegaOld && !peakIdx )
    {
      printf( "Have we got the peak? omegaOld = %.16e, omega = %.16e\n", omegaOld, omega );
      peakIdx = i;
    }
    omegaOld = omega;
  }
  printf( "We now think the peak is at %d\n", peakIdx );
  finalIdx = retLen - 1;

  /* Stuff to find the actual peak time */
  gsl_spline    *spline = NULL;
  gsl_interp_accel *acc = NULL;
  REAL8 omegaDeriv1, omegaDeriv2;
  REAL8 time1, time2;
  REAL8 timePeak, timewavePeak = 0., omegaDerivMid;
  REAL8 sigAmpSqHi = 0., oldsigAmpSqHi = 0.;
  INT4  peakCount = 0;

  spline = gsl_spline_alloc( gsl_interp_cspline, retLen );
  acc    = gsl_interp_accel_alloc();

  time1 = dynamicsHi->data[peakIdx];

  gsl_spline_init( spline, dynamicsHi->data, omegaHi->data, retLen );
  omegaDeriv1 = gsl_spline_eval_deriv( spline, time1, acc );
  if ( omegaDeriv1 > 0. )
  {
    time2 = dynamicsHi->data[peakIdx+1];
    omegaDeriv2 = gsl_spline_eval_deriv( spline, time2, acc );
  }
  else
  {
    omegaDeriv2 = omegaDeriv1;
    time2 = time1;
    time1 = dynamicsHi->data[peakIdx-1];
    peakIdx--;
    omegaDeriv1 = gsl_spline_eval_deriv( spline, time1, acc );
  }

  do
  {
    timePeak = ( time1 + time2 ) / 2.;
    omegaDerivMid = gsl_spline_eval_deriv( spline, timePeak, acc );

    if ( omegaDerivMid * omegaDeriv1 < 0.0 )
    {
      omegaDeriv2 = omegaDerivMid;
      time2 = timePeak;
    }
    else
    {
      omegaDeriv1 = omegaDerivMid;
      time1 = timePeak;
    }
  }
  while ( time2 - time1 > 1.0e-5 );

  gsl_spline_free( spline );
  gsl_interp_accel_free( acc );

  XLALPrintInfo( "Estimation of the peak is now at time %.16e\n", timePeak );


  if ( XLALSimIMRGetEOBCalibratedSpinNQC( &nqcCoeffs, 2, 2, eta, a ) == XLAL_FAILURE )
  {
    XLAL_ERROR( XLAL_EFUNC );
  }

  /* Calculate non-quasicircular coefficients and apply to hi-sampled waveform */
  if ( XLALSimIMRSpinEOBCalculateNQCCoefficients( ampNQC, phaseNQC, &rHi, &prHi, omegaHi,
          2, 2, timePeak, deltaT/mTScaled, eta, a, &nqcCoeffs ) == XLAL_FAILURE )
  {
    XLAL_ERROR( XLAL_EFUNC );
  }

  /* Apply to the high sampled part */
  out = fopen( "saWavesHi.dat", "w" );
  for ( i = 0; i < retLen; i++ )
  {
    values->data[0] = rHi.data[i];
    values->data[1] = phiHi.data[i];
    values->data[2] = prHi.data[i];
    values->data[3] = pPhiHi.data[i];

    //printf("NQCs entering hNQC: %.16e, %.16e, %.16e, %.16e, %.16e, %.16e\n", nqcCoeffs.a1, nqcCoeffs.a2,nqcCoeffs.a3, nqcCoeffs.a3S, nqcCoeffs.a4, nqcCoeffs.a5 );
    if ( XLALSimIMREOBNonQCCorrection( &hNQC, values, omegaHi->data[i], &nqcCoeffs ) == XLAL_FAILURE )
    {
      XLAL_ERROR( XLAL_EFUNC );
    }

    hLM.re = sigReHi->data[i];
    hLM.im = sigImHi->data[i];
    fprintf( out, "%.16e %.16e %.16e %.16e %.16e\n", timeHi.data[i], hLM.re, hLM.im, hNQC.re, hNQC.im );

    hLM = XLALCOMPLEX16Mul( hNQC, hLM );
    sigReHi->data[i] = (REAL4) hLM.re;
    sigImHi->data[i] = (REAL4) hLM.im;
    sigAmpSqHi = hLM.re*hLM.re+hLM.im*hLM.im;
    if (sigAmpSqHi < oldsigAmpSqHi && peakCount == 0) 
    {
      timewavePeak = (i-1)*deltaT/mTScaled;
      peakCount += 1;
    }
    oldsigAmpSqHi = sigAmpSqHi;
  }
  fclose(out);
  if (timewavePeak < 1.0e-16)
  {
    printf("YP::warning: could not locate mode peak.\n");
  }
  /* Failed to locate mode peak, use calibrated timeshiftPeak instead */
  printf( "eta: %.16e  a: %.16e\n", eta, a);
  timewavePeak = XLALSimIMREOBGetNRSpinPeakDeltaT(2, 2, eta,  a);
  timewavePeak = timePeak - timewavePeak;

  out = fopen( "saInspWaveHi.dat", "w" );
  for ( i = 0; i < retLen; i++ )
  {
    fprintf( out, "%.16e %.16e %.16e\n", timeHi.data[i], sigReHi->data[i], sigImHi->data[i] );
  }
  fclose( out );
  

  /* Attach the ringdown */
  /* XXX For now just hard-code the comb size, etc. We can drop it in properly XXX */
  /* XXX once we get the information from Andrea's calibration.                XXX */
  REAL8 combSize = 7.5;
  REAL8 timeshiftPeak;
  timeshiftPeak = timePeak - timewavePeak;

  printf("YP::timePeak and timewavePeak: %.16e and %.16e\n",timePeak,timewavePeak);
 
  REAL8Vector *rdMatchPoint = XLALCreateREAL8Vector( 3 );
  if ( !rdMatchPoint )
  {
    XLAL_ERROR( XLAL_ENOMEM );
  }

  if ( combSize > timePeak - timeshiftPeak )
  {
    XLALPrintError( "The comb size looks to be too big!!!\n" );
  }

  rdMatchPoint->data[0] = combSize < timePeak - timeshiftPeak ? timePeak - timeshiftPeak - combSize : 0;
  rdMatchPoint->data[1] = timePeak - timeshiftPeak;
  rdMatchPoint->data[2] = dynamicsHi->data[finalIdx];

  if ( XLALSimIMREOBHybridAttachRingdown( sigReHi, sigImHi, 2, 2,
              deltaT, m1, m2, spin1, spin2, &timeHi, rdMatchPoint, SEOBNRv1)
          == XLAL_FAILURE ) 
  {
    XLAL_ERROR( XLAL_EFUNC );
  }

  /* Now create vectors at the correct sample rate, and compile the complete waveform */
  sigReVec = XLALCreateREAL8Vector( rVec.length + ceil( sigReHi->length / resampFac ) );
  sigImVec = XLALCreateREAL8Vector( sigReVec->length );

  memset( sigReVec->data, 0, sigReVec->length * sizeof( REAL8 ) );
  memset( sigImVec->data, 0, sigImVec->length * sizeof( REAL8 ) );

/*  for ( i = 0; i < 20; i++ )
  {
    printf( "\n\n\n\n\n******************************************************************************\n" );
  }*/
 
  /* TODO - Check vectors were allocated */
  for ( i = 0; i < (INT4)rVec.length; i++ )
  {
    values->data[0] = rVec.data[i];
    values->data[1] = phiVec.data[i];
    values->data[2] = prVec.data[i];
    values->data[3] = pPhiVec.data[i];

    omega = XLALSimIMRSpinAlignedEOBCalcOmega( values->data, &seobParams );
    v = cbrt( omega );

    /* Calculate the value of the Hamiltonian */
    cartPosVec.data[0] = values->data[0];
    cartMomVec.data[0] = values->data[2];
    cartMomVec.data[1] = values->data[3] / values->data[0];

    ham = XLALSimIMRSpinEOBHamiltonian( eta, &cartPosVec, &cartMomVec, sigmaKerr, sigmaStar, seobParams.tortoise, &seobCoeffs );

    if ( XLALSimIMRSpinEOBGetSpinFactorizedWaveform( &hLM, values, v, ham, 2, 2, &seobParams )
           == XLAL_FAILURE )
    {
      /* TODO: Clean-up */
      XLAL_ERROR( XLAL_EFUNC );
    }

    if ( XLALSimIMREOBNonQCCorrection( &hNQC, values, omega, &nqcCoeffs ) == XLAL_FAILURE )
    {
      XLAL_ERROR( XLAL_EFUNC );
    }

    hLM = XLALCOMPLEX16Mul( hNQC, hLM );

    sigReVec->data[i] = amp0 * hLM.re;
    sigImVec->data[i] = amp0 * hLM.im;
  }

  /* Attach the ringdown part to the inspiral */
  for ( i = 0; i < (INT4)(sigReHi->length / resampFac); i++ )
  {
    sigReVec->data[i+hiSRndx] = sigReHi->data[i*resampFac];
    sigImVec->data[i+hiSRndx] = sigImHi->data[i*resampFac];
  }

  /* For now, let us just try to create a waveform */
  REAL8TimeSeries *hPlusTS  = XLALCreateREAL8TimeSeries( "H_PLUS", &tc, 0.0, deltaT, &lalStrainUnit, sigReVec->length );
  REAL8TimeSeries *hCrossTS = XLALCreateREAL8TimeSeries( "H_CROSS", &tc, 0.0, deltaT, &lalStrainUnit, sigImVec->length );

  /* Compute spin-weighted spherical harmonics and generate waveform */
  REAL8 coa_phase = 0.0;

  if ( XLALSphHarm( &MultSphHarmP, 2, 2, inc, coa_phase ) == XLAL_FAILURE )
  {
    /* TODO: Clean-up */
    XLAL_ERROR( XLAL_EFUNC );
  }

  if ( XLALSphHarm( &MultSphHarmM, 2, -2, inc, coa_phase ) == XLAL_FAILURE )
  {
    /* TODO: Clean-up */
    XLAL_ERROR( XLAL_EFUNC );
  }

  y_1 =   MultSphHarmP.re + MultSphHarmM.re;
  y_2 =   MultSphHarmM.im - MultSphHarmP.im;
  z1 = - MultSphHarmM.im - MultSphHarmP.im;
  z2 =   MultSphHarmM.re - MultSphHarmP.re;

  for ( i = 0; i < (INT4)sigReVec->length; i++ )
  {
    REAL8 x1 = sigReVec->data[i];
    REAL8 x2 = sigImVec->data[i];

    hPlusTS->data->data[i]  = (x1 * y_1) + (x2 * y_2);
    hCrossTS->data->data[i] = (x1 * z1) + (x2 * z2);
  }

  /* Point the output pointers to the relevant time series and return */
  (*hplus)  = hPlusTS;
  (*hcross) = hCrossTS;


  return XLAL_SUCCESS;
}