#
#  @file linux.mak
#
#  @brief The makefile for model framework library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library
SONAME = tx_model

# Source files
SOURCES = model_common.c model_lib.c model_xml.c model_manager.c damodel.c

# Jiutai source files
JIUTAI_SRCS = stocktrade.c

# For code complile
EXTRA_INC_DIR += -I. -I$(TOPDIR)/jtk/inc
EXTRA_OBJECTS = $(TOPDIR)/jtk/inc/jf_dynlib.o
EXTRA_CFLAGS =

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L$(TOPDIR)/jtk/lib
EXTRA_LIBS = -ljf_logger -ljf_files -ltx_rule

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


