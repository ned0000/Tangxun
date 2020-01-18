#
#  @file linux.mak
#
#  @brief The makefile for download library.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library
SONAME = tx_download

# Source files
SOURCES = downloaddata.c 

# Jiutai source files
JIUTAI_SRCS =

# For code complile
EXTRA_INC_DIR += -I. -I../jtk/inc
EXTRA_CFLAGS =

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L../jtk/lib
EXTRA_LIBS = -ljf_logger -ljf_ifmgmt -ljf_network -ljf_httpparser

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


