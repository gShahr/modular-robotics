CFLAGS=-I$(IDIR) 
_COMMA:=,

ifeq ($(OS), Windows_NT)
	SHELL=powershell
	DIRSLASH = \\
	DELCMD = Remove-Item
	LIBSUBFOLDER=win64
	CC=x86_64-w64-mingw32-g++
	OSCFLAGS+= -lgdi32 -static
else
	DIRSLASH=/
	DELCMD=rm -f
	LIBSUBFOLDER=TODO
	CC=g++
endif

# $(info OS is $(OS))
# $(info SHELL is $(SHELL))
# $(info DIRSLASH is $(DIRSLASH))
# $(info DELCMD is $(DELCMD))
# $(info LIBSUBFOLDER is $(LIBSUBFOLDER))

ODIR=obj
LDIR=lib
IDIR=include
SDIR=src

_LIBS=libglfw3.a OpenGL32.Lib
LIBS = $(patsubst %,$(LDIR)/$(LIBSUBFOLDER)/%,$(_LIBS))
_DEPS = glfw.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
_OBJ = main.opp Shader.opp glad.o stb_image.opp
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

# $(info LIBS is $(LIBS))
# $(info DEPS is $(DEPS))
# $(info OBj is $(OBJ))

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)
$(ODIR)/%.opp: $(SDIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(OSCFLAGS)

_JUNKFILES = *.exe *.out $(ODIR)$(DIRSLASH)*.o $(ODIR)$(DIRSLASH)*.opp
ifeq ($(OS), Windows_NT)
	JUNKFILES = $(subst $() $(),$(_COMMA),$(_JUNKFILES))
endif
# $(info JUNKFILES is $(JUNKFILES))
clean:
	$(DELCMD) $(JUNKFILES)
