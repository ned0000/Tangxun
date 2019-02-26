#
#  @file linux.mak
#
#  @brief The Makefile for Tangxun transaction daemon
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

# Name of the executable file
EXE = datransd

# Source files
SOURCES = datransd.c main.c

# Jiutai source files
JIUTAI_SRCS = stocklist.c envvar.c parsedata.c datastat.c stocktrade.c

# For code complile
EXTRA_INC_DIR = -I../jtk/inc
EXTRA_CFLAGS = -DENT

# For executable file build 
EXTRA_LDFLAGS = 
EXTRA_OBJECTS = ../jtk/inc/syncmutex.o ../jtk/inc/syncsem.o
EXTRA_LIB_DIR = -L../jtk/lib 
EXTRA_LIBS = -lollogger -lolfiles -lolstringparse -lolifmgmt -lolnetwork \
    -loljiukun -lolhttpparser -lolpersistency -lolclieng -lolwebclient \
    -lsqlite3 -lm

include $(TOPDIR)/mak/lnxexe.mak

#-----------------------------------------------------------------------------

