#
#  @file linux.mak
#
#  @brief The Makefile for tradehelper library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library
SONAME = tradehelper

# Source files
SOURCES = tradehelper.c

# Jiutai source files
JIUTAI_SRCS =

# For code complile
EXTRA_INC_DIR += -I. -I../jtk/inc
EXTRA_CFLAGS =

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L../jtk/lib
EXTRA_LIBS = -ljf_logger -ldamodel -ltrade_persistency

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


