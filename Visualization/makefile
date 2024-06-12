CFLAGS=-I$(IDIR) 

ifeq ($(OS), Windows_NT)
	SHELL=powershell
	DIRSLASH = \\
	DELCMD = Remove-Item
	LIBSUBFOLDER=win64
	CC=x86_64-w64-mingw32-g++
	OSCFLAGS+= -lgdi32
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
_OBJ = main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

# $(info LIBS is $(LIBS))
# $(info DEPS is $(DEPS))
# $(info OBj is $(OBJ))

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(OSCFLAGS)

clean:
	$(DELCMD) *.exe *.out $(ODIR)$(DIRSLASH)*.o
