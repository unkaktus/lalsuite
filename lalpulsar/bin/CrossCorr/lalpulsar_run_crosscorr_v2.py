# script to call lalpulsar_crosscorr_v2 (for use with e.g. condor)

# Copyright (C) 2014 John Whelan

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with with program; see the file COPYING. If not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301  USA

from __future__ import division
from argparse import ArgumentParser
from subprocess import check_call
from configparser import SafeConfigParser
from time import sleep

ap = ArgumentParser()
ap.add_argument(
    "--configFile",
    action="store",
    required=True,
    help="Configuration file containing arguments",
)
ap.add_argument(
    "--numJobs",
    action="store",
    type=int,
    required=True,
    help="Number of jobs into which run is split",
)
ap.add_argument(
    "--jobNum", action="store", type=int, required=True, help="Number of this job"
)

args = ap.parse_args()

cp = SafeConfigParser()
cp.optionxform = str
cp.read(args.configFile)

fMin = cp.getfloat("param-space", "f_min")
fMax = cp.getfloat("param-space", "f_max")
fFullBand = fMax - fMin
fBand = fFullBand / args.numJobs
fStart = fMin + args.jobNum * fBand

toplistPattern = cp.get("filename-patterns", "toplist_name")
toplistName = toplistPattern % (args.jobNum, args.numJobs)

logfilePattern = cp.get("filename-patterns", "logfile_name")
logfileName = logfilePattern % (args.jobNum, args.numJobs)


# Pass along the arguments from the ini file
program_args = ["--%s=%s" % a for a in cp.items("raw-program-arguments")]

# Add calculated frequency band
program_args += ["--fStart=%.11f" % fStart]
program_args += ["--fBand=%.11f" % fBand]
program_args += ["--toplistFilename=%s" % toplistName]
program_args += ["--logFilename=%s" % logfileName]

# check if latticeFilename is in the ini file
if cp.has_section("program") and cp.has_option("filename-patterns", "latticefile_name"):
    #    print("has latticefile")
    latticefilePattern = cp.get("filename-patterns", "latticefile_name")
    latticefileName = latticefilePattern % (args.jobNum, args.numJobs)
    program_args += ["--LatticeOutputFilename=%s" % latticefileName]

if cp.has_section("raw-program-arguments") and cp.has_option(
    "raw-program-arguments", "latticeType"
):
    if (
        cp.get("raw-program-arguments", "latticeType") == "byHand"
        or cp.get("raw-program-arguments", "latticeType") == "byhand"
        or cp.get("raw-program-arguments", "latticeType") == "byhand"
    ):
        print("no lattice")
    else:
        print("has lattice--uselattice")
        program_args += ["--useLattice"]
        #    latticeType = cp.get('raw-program-arguments','latticeType')
        #    program_args += ['--latticeType=%s' % latticeType]


if cp.has_section("raw-program-arguments") and cp.has_option(
    "raw-program-arguments", "useShearedPeriod"
):
    print("has --useShearedPeriod")
    program_args += ["--useShearedPeriod"]

# Variable delay to stagger start times of lalpulsar_crosscorr_v2 code
if cp.has_section("program") and cp.has_option("program", "delay_secs"):
    sleep(args.jobNum * cp.getfloat("program", "delay_secs"))

# Check if program was specified
if cp.has_section("program") and cp.has_option("program", "executable"):
    program = cp.get("program", "executable")
else:
    program = "lalpulsar_crosscorr_v2"

print(program, program_args)
check_call(([program] + program_args))
