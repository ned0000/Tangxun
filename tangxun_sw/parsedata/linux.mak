#
#  @file linux.mak
#
#  @brief The makefile for parsedata library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library
SONAME = tx_parsedata

# Source files
SOURCES = daysummary.c dayresult.c sector.c quo.c

# Jiutai source files
JIUTAI_SRCS =

# For code complile
EXTRA_INC_DIR += -I. -I../jtk/inc
EXTRA_CFLAGS =

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L../jtk/lib
EXTRA_LIBS = -ljf_logger

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


