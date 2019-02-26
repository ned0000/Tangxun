#
#  @file Makefile
#
#  @brief The Makefile for misc
#
#  @author Min Zhang
#
#  @note
#
#  

#-----------------------------------------------------------------------------

include $(TOPDIR)/mak/lnxcfg.mak

# Program
PROGRAMS = zlib-test

# Source files
SOURCES = zlib-test.c

include $(TOPDIR)/mak/lnxobjdef.mak

FULL_PROGRAMS = $(foreach i, $(PROGRAMS), $(BIN_DIR)/$i)

# For code complile
EXTRA_INC_DIR += -I../jtk/inc
EXTRA_CFLAGS = -D_GNU_SOURCE

# For library build 
EXTRA_LDFLAGS = 
EXTRA_LIB_DIR = -L../jtk/lib

all: $(FULL_PROGRAMS)

$(BIN_DIR)/zlib-test: zlib-test.o ../jtk/inc/xmalloc.o ../jtk/inc/syncmutex.o \
       ../jtk/inc/process.o
	$(CC) $(EXTRA_LDFLAGS) $(EXTRA_LIB_DIR) -L$(LIB_DIR) $^ \
       -o $@ $(SYSLIBS) -lz -lolfiles -lollogger

include $(TOPDIR)/mak/lnxobjbld.mak

clean:
	rm -f $(FULL_PROGRAMS) $(OBJECTS)
