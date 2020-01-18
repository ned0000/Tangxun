#
#  @file linux.mak
#
#  @brief The makefile for Tangxun background activity daemon.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the executable file
EXE = tx_bgad

# Source files
SOURCES = dabgad.c main.c

# Jiutai source files
JIUTAI_SRCS = 

# For code complile
EXTRA_INC_DIR = -I../jtk/inc
EXTRA_CFLAGS =

# For executable file build 
EXTRA_LDFLAGS = 
EXTRA_OBJECTS = ../jtk/inc/jf_process.o
EXTRA_LIB_DIR = -L../jtk/lib 
EXTRA_LIBS = -ljf_logger -ljf_files -ljf_ifmgmt -ljf_network -ljf_string -lpthread -ljf_jiukun

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------

