#
#  @file linux.mak
#
#  @brief The Makefile for damodel library
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

# Name of the library
SONAME = damodel

# Source files
SOURCES = damodel.c roi.c

# Jiutai source files
JIUTAI_SRCS = stocktrade.c

# For code complile
EXTRA_INC_DIR += -I. -I../jtk/inc
EXTRA_CFLAGS =

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L../jtk/lib
EXTRA_LIBS = -lollogger -ldarule

include $(TOPDIR)/mak/lnxlib.mak

#-----------------------------------------------------------------------------


