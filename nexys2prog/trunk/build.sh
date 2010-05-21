#!/bin/bash

# Minimal Xilinx build environment to show off the Nexys 2 programming
# script.

PROJ=led

# Choose the proper FPGA part ID.  Digilent ships two versions.
#PART=xc3s500e-fg320-5
PART=xc3s1200e-fg320-5

# You may need to comment in the following if the Xilinx variables are not
# yet set in your environment
#source /opt/Xilinx/11.1/settings32.sh

echo run -ifn $PROJ.v -ifmt verilog -ofn $PROJ.ngc -p $PART -top $PROJ | xst
ngdbuild -i -p $PART $PROJ.ngc
map -p $PART $PROJ.ngd
par $PROJ.ncd ${PROJ}_par.ncd $PROJ.pcf
bitgen ${PROJ}_par.ncd $PROJ.bit $PROJ.pcf

# You may remove the -v (--verbose) switch for less verbose output.
./nexys2prog -v $PROJ.bit

