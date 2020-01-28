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
SOURCES = common.c fromstock.c fromday.c backtesting.c

# Jiutai source files
JIUTAI_SRCS =

# For code complile
EXTRA_INC_DIR += -I. -I../jtk/inc
EXTRA_CFLAGS =

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L../jtk/lib
EXTRA_LIBS = -ljf_logger -ljf_files -ltx_model -ltx_persistency -ltx_trade

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


