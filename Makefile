CXX       := g++
TARGET    := main

SRCDIR    := src
BUILDDIR  := obj
TARGETDIR := bin

SRCEXT    := cpp
DEPEXT    := d
OBJEXT    := o

CXXFLAGS  := -std=c++17 -Wno-format-security

# If client mac
# LDFLAGS   := -L/opt/homebrew/lib -lfftw3 -lm
# INC       := -Isrc -Iinclude -I/opt/homebrew/include/

LDFLAGS     := -pthread `pkg-config gtkmm-3.0 --libs` `pkg-config opencv4 --cflags` `pkg-config opencv4 --libs`
INC         := -pthread -Iinclude -Iinclude/modules `pkg-config gtkmm-3.0 --cflags` `pkg-config opencv4 --cflags` 
sources     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
objects     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(subst $(SRCEXT),$(OBJEXT),$(sources)))
dependencies := $(subst .$(OBJEXT),.$(DEPEXT),$(objects))

all: directories $(TARGETDIR)/$(TARGET) plot

install-mac:
	brew install gtkmm3

install-ubuntu:
	sudo apt install gtkmm3

remake: cleaner all
run:
	-mkdir outputs
	#open ./$(TARGETDIR)/$(TARGET) 
	./$(TARGETDIR)/plotter & ./$(TARGETDIR)/$(TARGET) 

directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

clean:
	@$(RM) -rf $(TARGETDIR)
	@$(RM) -rf $(BUILDDIR)	

cleaner: clean
	@$(RM) -rf $(TARGETDIR)
	
-include $(dependencies)

$(TARGETDIR)/$(TARGET): $(objects)
	$(CXX) -o $(TARGETDIR)/$(TARGET) $^ $(LDFLAGS)

# TODO: なおす
plot:
	$(CXX) $(CXXFLAGS) plotter/plotter.cpp -o $(TARGETDIR)/plotter -Iinclude `pkg-config opencv4 --cflags` `pkg-config opencv4 --libs` -pthread

$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c -o $@ $<
	@$(CXX) $(CXXFLAGS) $(INC) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

.PHONY: all remake clean cleanerCXX  = g++