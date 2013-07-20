PROJECT = quasicrystal
SOURCES = quasicrystal.cc window.cc
OBJDIR = obj

LIBS = -lm -lgflags -lGL -lGLU -lX11

LD = g++
CXX = g++

CXX_FLAGS = -std=c++0x -Wall -Wextra -O3 -ftree-vectorize -g -fopenmp
LDFLAGS = $(LIBS) -fopenmp

OBJFILES = $(patsubst %.cc, $(OBJDIR)/%.o, $(SOURCES))

$(PROJECT): $(OBJFILES)
	@echo +ld $(@)
	$(LD) $(OBJFILES) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: %.cc
	@echo +cc $<
	@mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR); rm -f $(PROJECT)