#
#  @file linux.mak
#
#  @brief The makefile for cli utility.
#
#  @author Min Zhang
#
#  @note
#
#  

#---------------------------------------------------------------------------------------------------

# Name of the executable file
EXE = tx_cli

# Source files
SOURCES = env.c indi.c model.c stat.c fix.c download.c rule.c find.c parse.c backtest.c \
    analysis.c stock.c misc.c trade.c clicmd.c main.c

# Jiutai source files
JIUTAI_SRCS = tx_fixdata.c tx_datastat.c tx_regression.c tx_env.c tx_err.c

# For code complile
EXTRA_CFLAGS =
EXTRA_INC_DIR = -I$(TOPDIR)/jtk/inc
EXTRA_OBJECTS = $(TOPDIR)/jtk/inc/jf_mem.o $(TOPDIR)/jtk/inc/jf_time.o \
    $(TOPDIR)/jtk/inc/jf_option.o

# For executable file build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L../jtk/lib
EXTRA_LIBS = -ljf_logger -ljf_files -ljf_clieng -ljf_string -ljf_ifmgmt \
    -ljf_network -ljf_httpparser -ljf_jiukun -ljf_matrix -ljf_persistency \
    -ltx_trade -ltx_rule -ltx_model -ltx_persistency -ltx_backtesting -ltx_parsedata \
    -ltx_stock -ltx_download  -ltx_statarbitrage -ltx_indi -lm -lsqlite3

include $(TOPDIR)/mak/lnxexe.mak

#---------------------------------------------------------------------------------------------------

