#
#  @file Makefile
#
#  @brief The Makefile for common object file
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

SOURCES = fixdata.c datastat.c regression.c tx_env.c indicator.c \
    statarbitrage.c damethod.c 

EXTRA_INC_DIR += -I../jtk/inc

include $(TOPDIR)/mak/lnxobj.mak

#---------------------------------------------------------------------------------------------------


