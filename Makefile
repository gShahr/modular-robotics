# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++17 -I $(VCPKG_ROOT)/installed/x64-windows/include

# Linker flags
LDFLAGS = -L $(VCPKG_ROOT)/installed/x64-windows/lib -lgmock -lgtest -lbenchmark -lpthread 

# Source files
SOURCES = main.cpp ConfigurationSpace.cpp Lattice.cpp ModuleManager.cpp MoveManager.cpp MetaModule.cpp LatticeSetup.cpp Scenario.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable name
EXECUTABLE = main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(BOOST_LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: tests/metamodule/test_1.cpp Metamodule.cpp
	$(CXX) $(CXXFLAGS) -I. -o test tests/metamodule/test_1.cpp Metamodule.cpp

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
