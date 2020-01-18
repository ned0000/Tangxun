#
#  @file linux.mak
#
#  @brief The Makefile for model roi
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library, MUST be start with "damodel_"
SONAME = tx_model_roi

# Source files
SOURCES = roi.c

# Jiutai source files
JIUTAI_SRCS = stocktrade.c parsedata.c

# For code complile
EXTRA_INC_DIR += -I. -I$(TOPDIR)/jtk/inc
EXTRA_CFLAGS =
EXTRA_OBJECTS =

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L$(TOPDIR)/jtk/lib
EXTRA_LIBS = -ljf_logger -ltx_rule

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


