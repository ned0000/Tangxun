#
#  @file linux.mak
#
#  @brief The makefile for indicator library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library
SONAME = tx_indi

# Source files
SOURCES = common.c indicator_dmi.c indicator_macd.c indicator_mtm.c indicator_kdj.c \
    indicator_rsi.c indicator_asi.c indicator_atr.c indicator_obv.c indicator.c

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


