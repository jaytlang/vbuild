==========
  vbuild
==========

A sane build system for Vivado's non-project mode.  More documentation is
forthcoming, but for now:

vbuild is a system designed to be extensible, and to remove all the
complexity/added files/random project-mode mess from Vivado's FPGA tooling.
This should enable compilation of various FPGA things on the command line, and
potentially automated simulation and testing down the road. For now, vbuild
supports building projects per a small configuration file, and adding
functionality to clean the working directory / flash the FPGA should be trivial
from here.

