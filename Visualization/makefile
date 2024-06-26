CFLAGS=-I$(IDIR) 
_COMMA:=,

ODIR=obj
LDIR=lib
IDIR=include
SDIR=src

_DEPS = glfw.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
_OBJ = main.opp Shader.opp glad.o stb_image.opp Cube.opp ObjectCollection.opp
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

ifeq ($(OS), Windows_NT)
	SHELL=powershell
	DIRSLASH = \\
	DELCMD = Remove-Item
	CC=x86_64-w64-mingw32-g++

	LIBSUBFOLDER=win64
    _LIBS=libglfw3.a OpenGL32.Lib
    LIBS = $(patsubst %,$(LDIR)/$(LIBSUBFOLDER)/%,$(_LIBS))

	OSCFLAGS += -lgdi32 -static
    OSCFLAGS := $(LIBS) $(OSCFLAGS)
else
	DIRSLASH=/
	DELCMD=rm -f
	CC=clang++

	LIBSUBFOLDER=arm64
    _LIBS=libglfw3_arm64.a
    LIBS = $(patsubst %,$(LDIR)/$(LIBSUBFOLDER)/%,$(_LIBS))

    OSCFLAGS += -framework Cocoa -framework OpenGL -framework IOKit
    OSCFLAGS := $(LIBS) $(OSCFLAGS)
endif

# $(info OS is $(OS))
# $(info SHELL is $(SHELL))
# $(info DIRSLASH is $(DIRSLASH))
# $(info DELCMD is $(DELCMD))
# $(info LIBSUBFOLDER is $(LIBSUBFOLDER))

# $(info LIBS is $(LIBS))
# $(info DEPS is $(DEPS))
# $(info OBJ is $(OBJ))

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/%.opp: $(SDIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(OSCFLAGS)

JUNKFILES = *.exe *.out $(ODIR)$(DIRSLASH)*.o $(ODIR)$(DIRSLASH)*.opp
ifeq ($(OS), Windows_NT)
	JUNKFILES := $(subst $() $(),$(_COMMA),$(JUNKFILES))
endif
# $(info JUNKFILES is $(JUNKFILES))
clean:
	$(DELCMD) $(JUNKFILES)
