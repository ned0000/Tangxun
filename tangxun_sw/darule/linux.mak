#
#  @file linux.mak
#
#  @brief The Makefile for darule library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library
SONAME = darule

# Source files
SOURCES = rule_vol.c rule_vol.c rule_indi_macd.c rule_st.c rule_rectangle.c rule_limit.c \
    rule_price.c rule_bottom.c rule_misc.c darule.c

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


