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

SOURCES = stocklist.c parsedata.c fixdata.c datastat.c regression.c \
    envvar.c stocktrade.c \
    downloaddata.c indicator.c statarbitrage.c \
    damethod.c 

EXTRA_INC_DIR += -I../jtk/inc

include $(TOPDIR)/mak/lnxobj.mak

#---------------------------------------------------------------------------------------------------


