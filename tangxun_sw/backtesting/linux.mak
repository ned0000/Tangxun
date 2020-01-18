#
#  @file linux.mak
#
#  @brief The makefile for backtesting library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------
# Name of the library
SONAME = tx_backtesting

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
EXTRA_LIBS = -ljf_logger -ljf_files -ltx_model -ltx_trade_persistency -ltx_tradehelper

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


