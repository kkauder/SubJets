os = $(shell uname -s)

AJDIR     = $(HOME)/PaperAj/BasicAj

INCFLAGS      = -I$(ROOTSYS)/include -I$(FASTJETDIR)/include -I$(PYTHIA8DIR)/include -I$(PYTHIA8DIR)/include/Pythia8/ -I$(PYTHIA8DIR)/include/Pythia8Plugins/ -I$(STARPICOPATH)
INCFLAGS      += -I./src
INCFLAGS      += -I$(AJDIR)/src

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
LIBS          = -lfastjet -lfastjettools -lpythia8 -lTStarJetPico -lRecursiveTools

LIBPATH       += -L$(AJDIR)/lib
LIBS	      += -lMyJetlib

## Unfolding Test
INCFLAGS      += -I/Users/kkauder/software/RooUnfold/src
LIBPATH       += -L/Users/kkauder/software/RooUnfold
LIBS          += -lRooUnfold

## fun with pythia :-/
## make is a horrible horrible tool. Do not touch these lines, any whitespace will make it break
dummy := "$(shell find $(PYTHIA8DIR)/lib/ -name liblhapdfdummy\*)"
ifneq ("",$(dummy))
LIBS         += -llhapdfdummy
endif

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
all    : $(BDIR)/SubjetWrapper $(BDIR)/DevSubjetWrapper $(BDIR)/UnifiedSubjetWrapper \
	 $(BDIR)/UnifiedContribTester \
	 $(BDIR)/Groom \
         $(BDIR)/UnfoldingTest \
	 $(BDIR)/SubjetUnfolding \
	 $(BDIR)/GroomUnfolding \
	 $(BDIR)/DijetGroomUnfolding

#$(BDIR)/PythiaInAuAuSubjetWrapper

$(ODIR)/SubjetAnalysis.o 		: $(SDIR)/SubjetAnalysis.cxx    $(INCS) $(SDIR)/SubjetParameters.hh 
$(ODIR)/DevSubjetAnalysis.o 		: $(SDIR)/DevSubjetAnalysis.cxx $(INCS) $(SDIR)/SubjetParameters.hh 

# Force recompile if parameters change
$(ODIR)/SubjetWrapper.o		 	: $(SDIR)/SubjetWrapper.cxx 		$(INCS) $(SDIR)/SubjetParameters.hh
$(ODIR)/DevSubjetWrapper.o		: $(SDIR)/DevSubjetWrapper.cxx 		$(INCS) $(SDIR)/SubjetParameters.hh
$(ODIR)/DevSubjetWrapper.o		: $(SDIR)/UnifiedSubjetWrapper.cxx	$(INCS) $(SDIR)/SubjetParameters.hh


#SubJets
$(BDIR)/UnifiedSubjetWrapper		: $(ODIR)/UnifiedSubjetWrapper.o 	$(ODIR)/DevSubjetAnalysis.o
$(BDIR)/DevSubjetWrapper		: $(ODIR)/DevSubjetWrapper.o 		$(ODIR)/DevSubjetAnalysis.o
$(BDIR)/SubjetWrapper			: $(ODIR)/SubjetWrapper.o 		$(ODIR)/SubjetAnalysis.o

$(BDIR)/contribtest                     : $(ODIR)/contribtest.o 		$(ODIR)/DevSubjetAnalysis.o
$(BDIR)/UnifiedContribTester		: $(ODIR)/UnifiedContribTester.o 	$(ODIR)/DevSubjetAnalysis.o
$(BDIR)/Groom				: $(ODIR)/Groom.o		 	$(ODIR)/DevSubjetAnalysis.o

## Unfolding
$(BDIR)/GroomUnfolding   		: $(ODIR)/GroomUnfolding.o  		$(ODIR)/dict.o $(ODIR)/ktTrackEff.o
$(BDIR)/DijetGroomUnfolding   		: $(ODIR)/DijetGroomUnfolding.o		$(ODIR)/dict.o $(ODIR)/ktTrackEff.o
$(BDIR)/SubjetUnfolding   		: $(ODIR)/SubjetUnfolding.o  		$(ODIR)/dict.o $(ODIR)/ktTrackEff.o
$(BDIR)/UnfoldingTest   		: $(ODIR)/UnfoldingTest.o  		$(ODIR)/dict.o $(ODIR)/ktTrackEff.o


## for tracking
$(SDIR)/dict.cxx	: $(SDIR)/ktTrackEff.hh
	cd $(SDIR); rootcint -f dict.cxx -c -I. ./ktTrackEff.hh

$(ODIR)/dict.o          : $(SDIR)/dict.cxx
$(ODIR)/ktTrackEff.o    : $(SDIR)/ktTrackEff.cxx $(SDIR)/ktTrackEff.hh




#$(ODIR)/PythiaInAuAuSubjetWrapper.o 	: $(SDIR)/PythiaInAuAuSubjetWrapper.cxx $(INCS) $(SDIR)/SubjetParameters.hh
#$(BDIR)/PythiaInAuAuSubjetWrapper 	: $(ODIR)/PythiaInAuAuSubjetWrapper.o 	$(ODIR)/SubjetAnalysis.o
#$(BDIR)/MySubjets			: $(ODIR)/MySubjets.o


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
