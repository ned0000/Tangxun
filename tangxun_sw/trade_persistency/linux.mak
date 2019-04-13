#
#  @file Makefile
#
#  @brief The Makefile for trade persistency library
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the library
SONAME = trade_persistency

# Source files
SOURCES = sqlitepersistency.c trade_persistency.c 

# Jiutai source files
JIUTAI_SRCS = 

# For code complile
EXTRA_INC_DIR += -I../jtk/inc
EXTRA_CFLAGS = 

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L../jtk/lib 
EXTRA_LIBS = -ljf_logger
EXTRA_OBJECTS = ../jtk/inc/jf_sqlite.o

include $(TOPDIR)/mak/lnxlib.mak

#---------------------------------------------------------------------------------------------------


