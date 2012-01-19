##
## This is a sample makefile for building Pin tools outside
## of the Pin environment.  This makefile is suitable for
## building with the Pin kit, not a Pin source development tree.
##
## To build the tool, execute the make command:
##
##      make
## or
##      make PIN_HOME=<top-level directory where Pin was installed>
##
## After building your tool, you would invoke Pin like this:
## 
##      $PIN_HOME/pin -t MyPinTool -- /bin/ls
##
##############################################################
#
# User-specific configuration
#
##############################################################

#
# 1. Change PIN_HOME to point to the top-level directory where
#    Pin was installed. This can also be set on the command line,
#    or as an environment variable.
#
PIN_HOME ?= ./..


##############################################################
#
# set up and include *.config files
#
##############################################################

PIN_KIT=$(PIN_HOME)
KIT=1
TESTAPP=$(OBJDIR)cp-pin.exe

TARGET_COMPILER?=gnu
ifdef OS
    ifeq (${OS},Windows_NT)
        TARGET_COMPILER=ms
    endif
endif

ifeq ($(TARGET_COMPILER),gnu)
    include $(PIN_HOME)/source/tools/makefile.gnu.config
    LINKER?=${CXX}
    CXXFLAGS ?= -Wall -Werror -Wno-unknown-pragmas $(DBG) $(OPT) -std=gnu++0x
#    CXXFLAGS ?= -Wall -Werror -Wno-unknown-pragmas $(OPT) -g -std=gnu++0x
    PIN=$(PIN_HOME)/pin
endif

ifeq ($(TARGET_COMPILER),ms)
    include $(PIN_HOME)/source/tools/makefile.ms.config
    DBG?=
    PIN=$(PIN_HOME)/pin.bat
endif


##############################################################
#
# Tools - you may wish to add your tool name to TOOL_ROOTS
#
##############################################################


TOOL_ROOTS = MAIP

TOOLS = $(TOOL_ROOTS:%=$(OBJDIR)%$(PINTOOL_SUFFIX))


##############################################################
#
# build rules
#
##############################################################

all: tools
tools: $(OBJDIR) $(TOOLS) $(OBJDIR)cp-pin.exe
test: $(OBJDIR) $(TOOL_ROOTS:%=%.test)

MAIP.test: $(OBJDIR)cp-pin.exe
      $(MAKE) -k -C MAIP PIN_HOME=$(PIN_HOME)

$(OBJDIR)cp-pin.exe:
	$(CXX) $(PIN_HOME)/source/tools/Tests/cp-pin.cpp $(APP_CXXFLAGS) -o $(OBJDIR)cp-pin.exe

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)%.o : %.cpp
	$(CXX) -c -DTIXML_USE_TICPP $(CXXFLAGS) $(PIN_CXXFLAGS) ${OUTOPT}$@ $<
# 	$(CXX) -c $(CXXFLAGS) $(PIN_CXXFLAGS) ${OUTOPT}$@ $<

$(TOOLS): $(PIN_LIBNAMES) $(OBJDIR)Function.o $(OBJDIR)MemInfo.o $(OBJDIR)ticpp.o $(OBJDIR)tinyxmlparser.o $(OBJDIR)tinystr.o $(OBJDIR)tinyxml.o $(OBJDIR)tinyxmlerror.o $(OBJDIR)xmloutput.o
$(TOOLS): %$(PINTOOL_SUFFIX) : %.o 
	${PIN_LD} $(PIN_LDFLAGS) $(LINK_DEBUG) ${LINK_OUT}$@ $< $(OBJDIR)Function.o $(OBJDIR)MemInfo.o $(OBJDIR)ticpp.o $(OBJDIR)tinyxmlparser.o $(OBJDIR)tinystr.o $(OBJDIR)tinyxml.o $(OBJDIR)tinyxmlerror.o $(OBJDIR)xmloutput.o ${PIN_LPATHS} $(PIN_LIBS) $(DBG)


# $(TOOLS): $(PIN_LIBNAMES) $(OBJDIR)Function.o $(OBJDIR)MemInfo.o
# $(TOOLS): %$(PINTOOL_SUFFIX) : %.o
# 	${LINKER} $(PIN_LDFLAGS) $(LINK_DEBUG) ${LINK_OUT}$@ $< $(OBJDIR)Function.o $(OBJDIR)MemInfo.o ${PIN_LPATHS} $(PIN_LIBS) $(DBG)


## cleaning
clean:
	-rm -rf $(OBJDIR) *.out *.tested *.failed makefile.copy
