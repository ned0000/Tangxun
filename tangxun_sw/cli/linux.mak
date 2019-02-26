#
#  @file Makefile linux.mak
#
#  @brief The Makefile for cli utility
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

# Name of the executable file
EXE = cli

# Source files
SOURCES = env.c indi.c model.c stat.c fix.c download.c rule.c \
	find.c parse.c backtest.c \
    analysis.c stock.c misc.c trade.c clicmd.c main.c

# Jiutai source files
JIUTAI_SRCS = stocklist.c parsedata.c fixdata.c datastat.c regression.c \
    downloaddata.c indicator.c statarbitrage.c damethod.c envvar.c


# For code complile
EXTRA_INC_DIR = -I../jtk/inc
EXTRA_CFLAGS =

# For executable file build 
EXTRA_LDFLAGS = 
EXTRA_OBJECTS = ../jtk/inc/xmalloc.o ../jtk/inc/xtime.o
EXTRA_LIB_DIR = -L../jtk/lib
EXTRA_LIBS = -lollogger -lolfiles -lolclieng -lolstringparse -lolifmgmt \
    -lolnetwork -lolhttpparser -loljiukun -lolmatrix -lolpersistency \
    -ltradehelper -ldarule -ldamodel -ltrade_persistency -lbacktesting \
    -lm -lsqlite3

include $(TOPDIR)/mak/lnxexe.mak

#-----------------------------------------------------------------------------
