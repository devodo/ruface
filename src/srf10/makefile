##########################################################
#
# General makefile for building executable programs and
# libraries for Embedded Artists' QuickStart Boards.
# (C) 2001-2005 Embedded Artists AB
#
##########################################################

# Name of target (executable program or library) 
NAME      = srf10

# Link program to RAM or ROM (possible values for LD_RAMROM is RAM or ROM,
# if not specified = ROM)
LD_RAMROM = ROM

# Name if specific CPU used (used by linker scripts to define correct memory map)
# Valid CPUs are: LPC2101, LPC2102, LPC2103, LPC2104, LPC2105, LPC2106
#                 LPC2114, LPC2119
#                 LPC2124, LPC2129
#                 LPC2131, LPC2132, LPC2134, LPC2136, LPC2138
#                 LPC2141, LPC2142, LPC2144, LPC2146, LPC2148
#                 LPC2194
#                 LPC2210, LPC2220, LPC2212, LPC2214,
#                 LPC2290, LPC2292, LPC2294
# If you have a new version not specified above, just select one of the old
# versions with the same memory map.
CPU_VARIANT = LPC2138

# It is possible to override the automatic linker file selection with the variable below.
# No not use this opion unless you have very specific needs.
#LD_SCRIPT      = build_files/myOwnLinkScript_rom.ld
#LD_SCRIPT_PATH = 

# ELF-file contains debug information, or not
# (possible values for DEBUG are 0 or 1)
# Extra debug flags can be specified in DBFLAGS
DEBUG   = 0
#DBFLAGS =

# Optimization setting
# (-Os for small code size, -O2 for speed)
OFLAGS  = -Os

# Extra general flags
# For example, compile for ARM / THUMB interworking (EFLAGS = -mthumb-interwork)
EFLAGS  =

# Program code run in ARM or THUMB mode
# Can be [ARM | THUMB]
CODE    = THUMB

# List C source files here.
CSRCS   = srf10.cpp srf10cntrl.cpp qmanager.cpp cmmdhandler.cpp

# List assembler source files here
ASRCS   = 

# List subdirectories to recursively invoke make in 
SUBDIRS =

# List additional libraries to link with
LIBS    = ../lib/libhelistix.a 
		  
# Add include search path for startup files, and other include directories
INC     = -I . -I./lib  -I./startup 

# Select if an executable program or a library shall be created
LIBRARY_MK  = true

# Output format on hex file (if making a program); can be [srec | ihex]
HEX_FORMAT  = ihex

# Program to download executable program file into microcontroller's FLASH
DOWNLOAD    = lpc21isp

# Configurations for download program
# Which com-pot that is used, which download speed and what crystal frequency on the board.
DL_COMPORT  = /dev/ttyUSB0
DL_BAUDRATE = 115200
DL_CRYSTAL  = 14746

#######################################################################
include ../build_files/general++.mk
#######################################################################
