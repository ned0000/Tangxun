#
#  @file linux.mak
#
#  @brief The Makefile for backtesting library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------
# Name of the library
SONAME = dabacktesting

# Source files
SOURCES = backtesting.c

# Jiutai source files
JIUTAI_SRCS = stocktrade.c

# For code complile
EXTRA_INC_DIR += -I. -I../jtk/inc
EXTRA_CFLAGS =

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L../jtk/lib
EXTRA_LIBS = -ljf_logger -ljf_files -ldamodel -ldatrade_persistency -ldatradehelper

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


