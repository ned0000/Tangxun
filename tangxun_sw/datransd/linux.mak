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

#---------------------------------------------------------------------------------------------------

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
EXTRA_OBJECTS = ../jtk/inc/jf_mutex.o ../jtk/inc/jf_sem.o ../jtk/inc/jf_thread.o
EXTRA_LIB_DIR = -L../jtk/lib 
EXTRA_LIBS = -ljf_logger -ljf_files -ljf_string -ljf_ifmgmt -ljf_network \
    -ljf_jiukun -ljf_httpparser -ljf_persistency -ljf_clieng -ljf_webclient \
    -lsqlite3 -lm

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------

