#
#  @file 
#
#  @brief The makefile for building tangxun software.
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

TOPDIR := $(shell /bin/pwd)

export TOPDIR

SUBDIRS = jiutai parsedata rule trade model trade_persistency backtesting \
    bgad transd cli misc

include $(TOPDIR)/mak/lnxsubdirs.mak

#---------------------------------------------------------------------------------------------------


