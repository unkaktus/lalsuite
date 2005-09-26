/*----------------------------------- <lalVerbatim file="SegmentsIOTestCV">
 * Author: Peter Shawhan
 * Revision: $Id$
 *----------------------------------- </lalVerbatim> */

#if 0
<lalLaTeX>

\subsection{Program \texttt{SegmentsIOTest.c}}
\label{s:SegmentsIOTest.c}

Tests the routines in \verb@SegmentsIO.c@.

\subsubsection*{Usage}
\begin{verbatim}
SegmentsIOTest [ lalDebugLevel ]
\end{verbatim}

The default value of \texttt{lalDebugLevel} is 4.

If the \texttt{lalDebugLevel} argument is omitted, the test program sets it
to 4 to turn on info messages only.
Note that this default value does not enable LAL/XLAL error messages,
since many of the tests intentionally create error conditions and verify that
the proper error codes are generated.  If you want to turn on the LAL/XLAL
error and warning messages, specify a \texttt{lalDebugLevel} value of 7,
or 23 if you also want informational messages related to memory checking.
If you specify 0, then no messages will be printed under any circumstances.
However, in all cases, the return status of the program will be 0 if all
tests passed, 1 if one or more tests failed.

\subsubsection*{Description}

NOTE: This test program does not currently do an exhaustive test of
functionality and failure modes; it is more like a starting point for
spot-checking by modifying, recompiling and running this test program
and inspecting the output.

\subsubsection*{Exit codes}

0 if all tests passed.

1 if one or more tests failed.

\subsubsection*{Uses}

\begin{verbatim}
LALSegListRead()
LALSegListWrite()
\end{verbatim}

\subsubsection*{Notes}

\vfill{\footnotesize\input{SegmentsIOTestCV}}

</lalLaTeX>
#endif

#include <lal/LALStatusMacros.h>
#include <lal/SegmentsIO.h>

NRCSID( SEGMENTSIOTESTC, "$Id$" );

/*-- Macros for this test program --*/

#define RETPASS(testname,status) XLALPrintInfo( \
    "Pass return check for %s: return=%d, xlalErrno=%d\n", \
    testname, status, xlalErrno );

#define RETFAIL(testname,status) XLALPrintInfo( \
    "*FAIL* return check for %s: return=%d, xlalErrno=%d\n", \
    testname, status, xlalErrno ); nfailures++;

#define FUNCPASS(testname) XLALPrintInfo( \
    "Pass functional check for %s\n", testname );

#define FUNCFAIL(testname,reason) XLALPrintInfo( \
    "*FAIL* functional check for %s: %s\n", testname, reason ); nfailures++;

/*-- Default debug level includes info messages (4), but not
     memory checking (16), error messages (1), or warning messages (2) --*/
int lalDebugLevel = 4;

/*===========================================================================*/


/*===========================================================================*/
int main( int argc, char *argv[] )
{
  INT4 nfailures = 0;
  static LALStatus status;
  INT4 xstatus;
  LALSegList seglist1, seglist2;
  LALSeg seg, *segp;
  LIGOTimeGPS segstart1 = {710000000, 123456789};
  LIGOTimeGPS segend1 =   {710000234, 555555555};

  /*------ Parse input line. ------*/
  if ( argc == 2 )
    lalDebugLevel = atoi( argv[1] );
  else if ( argc != 1 )
    {
      fprintf( stderr, "Usage: %s [ lalDebugLevel ]\n", argv[0] );
      return 0; /* so that test script won't fail */
    }

  /*-------------------------------------------------------------------------*/
  XLALPrintInfo("\n========== Initial setup \n");
  /*-------------------------------------------------------------------------*/

  /*-------------------------------------------------------------------------*/
  /* Initialize the segment list */
  xstatus = XLALSegListInit( &seglist1 );
  if ( xstatus ) {
    RETFAIL( "Initializing segment list", xstatus );
    XLALClearErrno();
  }

  /* Add one segment to the segment list */
  xstatus = XLALSegSet( &seg, &segstart1, &segend1, 29 );
  if ( xstatus ) {
    RETFAIL( "Creating segment from GPS times", xstatus );
    XLALClearErrno();
  } else {
    xstatus = XLALSegListAppend( &seglist1, &seg );
    if ( xstatus ) {
      RETFAIL( "Appending initial segment to segment list", xstatus );
      XLALClearErrno();
    }
  }


  /*-------------------------------------------------------------------------*/
  XLALPrintInfo("\n========== LALSegListRead tests \n");
  /*-------------------------------------------------------------------------*/

  /*------------------------------*/
  LALSegListRead( &status, &seglist1, "SegmentsInput1.data", "" );
  if ( status.statusCode ) {
    RETFAIL( "LALSegListRead with standard segment list file",
	     status.statusCode );
    REPORTSTATUS( &status );
  }

#if 0
  /*-- Dump out list of segments that was read --*/
  printf( "Segments:\n" );
  for ( segp=seglist1.segs; segp<seglist1.segs+seglist1.length; segp++ ) {
    printf( "  %10d.%09d - %10d.%09d  %d\n",
	    segp->start.gpsSeconds, segp->start.gpsNanoSeconds,
	    segp->end.gpsSeconds, segp->end.gpsNanoSeconds,
	    segp->id );
  }
#endif

  /*-------------------------------------------------------------------------*/
  XLALPrintInfo("\n========== LALSegListWrite tests \n");
  /*-------------------------------------------------------------------------*/
  
  LALSegListWrite( &status, &seglist1, "SegmentsOutput1.data", "adi" );
  if ( status.statusCode ) {
    RETFAIL( "LALSegListWrite with standard segment list", status.statusCode );
    REPORTSTATUS( &status );
    nfailures++;
  } else {
    XLALPrintInfo( "Wrote segment list file SegmentsOutput1.data\n" );
  }

  /*-------------------------------------------------------------------------*/
  /* Clean up leftover seg lists */
  if ( seglist1.segs ) { XLALSegListClear( &seglist1 ); }

  /*-------------------------------------------------------------------------*/
  LALCheckMemoryLeaks();

  /*-------------------------------------------------------------------------*/
  if ( nfailures == 0 ) {
    XLALPrintInfo("\n= = = = All tests passed = = = =\n\n");
    return 0;
  } else {
    XLALPrintInfo("\n= = = = %d tests FAILED = = = =\n\n", nfailures);
    return 1;
  }
}
