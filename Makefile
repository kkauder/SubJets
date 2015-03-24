os = $(shell uname -s)

#INCFLAGS      = -I$(ROOTSYS)/include -I$(FASTJETDIR)/include -I$(PYTHIA8DIR)/include -I$(STARPICOPATH)
INCFLAGS      = -I$(AJDIR)/src -I$(ROOTSYS)/include -I$(FASTJETDIR)/include -I$(PYTHIA8DIR)/include -I$(PYTHIA8DIR)/include/Pythia8/ -I$(STARPICOPATH)

ifeq ($(os),Linux)
CXXFLAGS      = 
else
CXXFLAGS      = -O -fPIC -pipe -Wall -Wno-deprecated-writable-strings -Wno-unused-variable -Wno-unused-private-field -Wno-gnu-static-float-init
## for debugging:
# CXXFLAGS      = -g -O0 -fPIC -pipe -Wall -Wno-deprecated-writable-strings -Wno-unused-variable -Wno-unused-private-field -Wno-gnu-static-float-init
endif

ifeq ($(os),Linux)
LDFLAGS       = -g
LDFLAGSS      = -g --shared 
else
LDFLAGS       = -O -Xlinker -bind_at_load -flat_namespace
LDFLAGSS      = -flat_namespace -undefined suppress
LDFLAGSSS     = -bundle
endif

ifeq ($(os),Linux)
CXX          = g++ 
else
CXX          = clang
endif


ROOTLIBS      = $(shell root-config --libs)

LIBPATH       = $(ROOTLIBS) -L$(FASTJETDIR)/lib -L$(PYTHIA8DIR)/lib -L$(STARPICOPATH)
LIBS          = -lfastjet -lfastjettools -lpythia8  -llhapdfdummy -lTStarJetPico

LIBPATH       += -L$(AJDIR)/lib
LIBS	      += -lMyJetlib


# for cleanup
SDIR          = src
ODIR          = src/obj
BDIR          = bin


###############################################################################
################### Remake when these headers are touched #####################
###############################################################################
#INCS = $(AJDIR)/JetAnalyzer.hh
INCS = 

###############################################################################
# standard rules
$(ODIR)/%.o : $(SDIR)/%.cxx $(INCS)
	@echo 
	@echo COMPILING
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@

$(BDIR)/%  : $(ODIR)/%.o 
	@echo 
	@echo LINKING
	$(CXX) $(LDFLAGS) $(LIBPATH) $(LIBS) $^ -o $@
###############################################################################

###############################################################################
############################# Main Targets ####################################
###############################################################################
all    : $(BDIR)/SubjetWrapper \
	 $(BDIR)/06-area

#$(BDIR)/PythiaInAuAuSubjetWrapper

$(ODIR)/06-area.o			: $(SDIR)/06-area.cxx $(INCS)
$(BDIR)/06-area				: $(ODIR)/06-area.o

$(ODIR)/SubjetAnalysis.o 		: $(SDIR)/SubjetAnalysis.cxx $(INCS) $(SDIR)/SubjetParameters.hh 
# Force recompile if parameters change
$(ODIR)/SubjetWrapper.o		 	: $(SDIR)/SubjetWrapper.cxx 		$(INCS) $(SDIR)/SubjetParameters.hh
$(ODIR)/PythiaInAuAuSubjetWrapper.o 	: $(SDIR)/PythiaInAuAuSubjetWrapper.cxx $(INCS) $(SDIR)/SubjetParameters.hh


#SubJets
$(BDIR)/PythiaInAuAuSubjetWrapper 	: $(ODIR)/PythiaInAuAuSubjetWrapper.o 	$(ODIR)/SubjetAnalysis.o
$(BDIR)/SubjetWrapper			: $(ODIR)/SubjetWrapper.o 		$(ODIR)/SubjetAnalysis.o
$(BDIR)/MySubjets			: $(ODIR)/MySubjets.o


###############################################################################
##################################### MISC ####################################
###############################################################################


# doxy: html/index.html

# html/index.html : $(INCS) src/* Doxyfile
# #	doxygen
# 	@echo 
# 	@echo Updating documentation
# 	( cat Doxyfile ; echo "QUIET=YES" ) | doxygen -

clean :
	@echo 
	@echo CLEANING
	rm -vf $(ODIR)/*.o
	rm -vf $(BDIR)/*

.PHONY : clean doxy
