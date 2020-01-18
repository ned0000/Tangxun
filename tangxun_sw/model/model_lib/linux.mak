#
#  @file linux.mak
#
#  @brief The makefile for model module.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

ALL_FILES = $(wildcard *) 
OUT_FILE = $(wildcard *.make.out)
TEMP_FILE = $(wildcard *~)
MAKEFILE = $(wildcard *.mak)

SUBDIRS = $(filter-out $(OUT_FILE) $(TEMP_FILE) $(MAKEFILE),$(ALL_FILES))

include $(TOPDIR)/mak/lnxsubdirs.mak

#---------------------------------------------------------------------------------------------------


