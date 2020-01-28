#
#  @file linux.mak
#
#  @brief The makefile for stock library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library
SONAME = tx_stock

# Source files
SOURCES = common.c stocklist.c stockindu.c stockindex.c txstock.c

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


