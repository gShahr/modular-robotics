# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++17 -I D:/vcpkg/installed/x64-windows/include

# Linker flags
LDFLAGS = -L D:/vcpkg/installed/x64-windows/lib -lgmock -lgtest -lbenchmark -lpthread 

# Source files
SOURCES = main.cpp ConfigurationSpace.cpp Lattice.cpp ModuleManager.cpp MoveManager.cpp Metamodule.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable name
EXECUTABLE = main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(BOOST_LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
