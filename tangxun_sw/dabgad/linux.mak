#
#  @file linux.mak
#
#  @brief The Makefile for Tangxun background activity daemon
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

# Name of the executable file
EXE = dabgad

# Source files
SOURCES = dabgad.c main.c

# Jiutai source files
JIUTAI_SRCS = 

# For code complile
EXTRA_INC_DIR = -I../jtk/inc
EXTRA_CFLAGS =

# For executable file build 
EXTRA_LDFLAGS = 
EXTRA_OBJECTS = ../jtk/inc/process.o
EXTRA_LIB_DIR = -L../jtk/lib 
EXTRA_LIBS = -lollogger -lolfiles -lolifmgmt -lolnetwork -lolstringparse \
    -lolifmgmt -lpthread

include $(TOPDIR)/mak/lnxexe.mak

#-----------------------------------------------------------------------------

