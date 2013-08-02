PROJECT = quasicrystal
SOURCES = shader_util.cc window.cc quasicrystal.cc
OBJDIR = obj

LIBS = -lgflags -lGL -lX11 -lGLEW

LD = g++
CXX = g++

CXX_FLAGS = -std=c++0x -Wall -Wextra -O2 -g
LDFLAGS = $(LIBS)

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