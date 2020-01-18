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

SOURCES = tx_fixdata.c tx_datastat.c tx_regression.c tx_env.c

EXTRA_INC_DIR += -I../jtk/inc

include $(TOPDIR)/mak/lnxobj.mak

#---------------------------------------------------------------------------------------------------


