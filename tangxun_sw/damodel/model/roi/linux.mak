#
#  @file linux.mak
#
#  @brief The Makefile for model roi library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library, MUST be start with "damodel_"
SONAME = damodel_roi

# Source files
SOURCES = roi.c

# Jiutai source files
JIUTAI_SRCS = stocktrade.c

# For code complile
EXTRA_INC_DIR += -I. -I$(TOPDIR)/jtk/inc
EXTRA_CFLAGS =

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L$(TOPDIR)/jtk/lib
EXTRA_LIBS = -ljf_logger -ldarule

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


