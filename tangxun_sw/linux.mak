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

SUBDIRS = jiutai stock parsedata indi rule trade model persistency statarbitrage \
    backtesting download bgad transd cli misc

include $(TOPDIR)/mak/lnxsubdirs.mak

#---------------------------------------------------------------------------------------------------


