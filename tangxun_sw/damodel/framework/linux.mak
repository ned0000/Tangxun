#
#  @file linux.mak
#
#  @brief The Makefile for damodel framework library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library
SONAME = damodel

# Source files
SOURCES = damodel.c model_manager.c

# Jiutai source files
JIUTAI_SRCS = stocktrade.c

# For code complile
EXTRA_INC_DIR += -I. -I$(TOPDIR)/jtk/inc
EXTRA_OBJECTS = $(TOPDIR)/jtk/inc/jf_dynlib.o
EXTRA_CFLAGS =

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L$(TOPDIR)/jtk/lib
EXTRA_LIBS = -ljf_logger -ldarule -ljf_files

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


