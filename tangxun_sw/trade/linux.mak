#
#  @file linux.mak
#
#  @brief The makefile for trade library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library
SONAME = tx_trade

# Source files
SOURCES = stocktrade.c tradehelper.c

# Jiutai source files
JIUTAI_SRCS =

# For code complile
EXTRA_INC_DIR += -I. -I$(TOPDIR)/jtk/inc
EXTRA_CFLAGS =
EXTRA_OBJECTS = $(TOPDIR)/jtk/inc/jf_date.o

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L$(TOPDIR)/jtk/lib
EXTRA_LIBS = -ljf_logger -ltx_parsedata

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


