#
#  @file 
#
#  @brief The makefile for building Tangxun software
#
#  @author Min Zhang
#
#  @note
#  

#---------------------------------------------------------------------------------------------------

TOPDIR := $(shell /bin/pwd)

export TOPDIR

SUBDIRS = jiutai trade_persistency darule damodel tradehelper backtesting \
    dabgad datransd cli misc

include $(TOPDIR)/mak/lnxsubdirs.mak

#---------------------------------------------------------------------------------------------------


