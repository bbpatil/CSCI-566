#
# OMNeT++/OMNEST Makefile for CSCI_566_proj_1
#
# This file was generated with the command:
#  opp_makemake -f --deep -O out -I../inet/src -L../inet/out/$$\(CONFIGNAME\)/src -lINET -KINET_PROJ=../inet
#

# Name of target to be created (-o option)
TARGET = CSCI_566_proj_1$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(QTENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)
#USERIF_LIBS = $(QTENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = -I../inet/src -I. -Iresults

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../inet/out/$(CONFIGNAME)/src  -lINET
LIBS += -Wl,-rpath,`abspath ../inet/out/$(CONFIGNAME)/src`

# Output directory
PROJECT_OUTPUT_DIR = out
PROJECTRELATIVE_PATH =
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = $O/QuestionA1.o $O/QuestionA2.o $O/QuestionB.o $O/QuestionA2_m.o

# Message files
MSGFILES = \
    QuestionA2.msg

# SM files
SMFILES =

# Other makefile variables (-K)
INET_PROJ=../inet

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppmain$D $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(Q)$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders smheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f CSCI_566_proj_1 CSCI_566_proj_1.exe libCSCI_566_proj_1.so libCSCI_566_proj_1.a libCSCI_566_proj_1.dll libCSCI_566_proj_1.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h ./*_sm.cc ./*_sm.h
	$(Q)-rm -f results/*_m.cc results/*_m.h results/*_sm.cc results/*_sm.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES) $(SM_CC_FILES)  ./*.cc results/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/QuestionA1.o: QuestionA1.cc
$O/QuestionA2.o: QuestionA2.cc \
  ./QuestionA2_m.h
$O/QuestionB.o: QuestionB.cc \
  ../inet/src/inet/applications/httptools/common/HttpEventMessages_m.h \
  ../inet/src/inet/applications/httptools/common/HttpMessages_m.h \
  ../inet/src/inet/applications/httptools/common/HttpNodeBase.h \
  ../inet/src/inet/applications/httptools/common/HttpRandom.h \
  ../inet/src/inet/applications/httptools/common/HttpUtils.h \
  ../inet/src/inet/applications/httptools/configurator/HttpController.h \
  ../inet/src/inet/applications/httptools/server/HttpServer.h \
  ../inet/src/inet/applications/httptools/server/HttpServerBase.h \
  ../inet/src/inet/common/Compat.h \
  ../inet/src/inet/common/INETDefs.h \
  ../inet/src/inet/common/INETMath.h \
  ../inet/src/inet/common/InitStages.h \
  ../inet/src/inet/common/ModuleAccess.h \
  ../inet/src/inet/common/NotifierConsts.h \
  ../inet/src/inet/common/lifecycle/ILifecycle.h \
  ../inet/src/inet/common/lifecycle/LifecycleOperation.h \
  ../inet/src/inet/features.h \
  ../inet/src/inet/linklayer/common/MACAddress.h \
  ../inet/src/inet/networklayer/common/InterfaceEntry.h \
  ../inet/src/inet/networklayer/common/InterfaceToken.h \
  ../inet/src/inet/networklayer/common/L3Address.h \
  ../inet/src/inet/networklayer/common/ModuleIdAddress.h \
  ../inet/src/inet/networklayer/common/ModulePathAddress.h \
  ../inet/src/inet/networklayer/contract/IInterfaceTable.h \
  ../inet/src/inet/networklayer/contract/ipv4/IPv4Address.h \
  ../inet/src/inet/networklayer/contract/ipv6/IPv6Address.h \
  ../inet/src/inet/transportlayer/contract/tcp/TCPCommand_m.h \
  ../inet/src/inet/transportlayer/contract/tcp/TCPSocket.h \
  ../inet/src/inet/transportlayer/contract/tcp/TCPSocketMap.h
