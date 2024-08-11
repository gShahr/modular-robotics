# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -w -std=c++17 -I $(VCPKG_ROOT)/installed/x64-windows/include

# Linker flags
LDFLAGS = -L $(VCPKG_ROOT)/installed/x64-windows/lib -lgmock -lgtest -lbenchmark -lpthread -fopenmp

# Source files
SOURCES = main.cpp ConfigurationSpace.cpp Lattice.cpp ModuleManager.cpp MoveManager.cpp Metamodule.cpp LatticeSetup.cpp Scenario.cpp Isometry.cpp Colors.cpp

# Source files without main
SOURCES2 = ConfigurationSpace.cpp Lattice.cpp ModuleManager.cpp MoveManager.cpp Metamodule.cpp LatticeSetup.cpp Scenario.cpp Isometry.cpp Colors.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable name
EXECUTABLE = main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(BOOST_LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: tests/metamodule/metamodule_1.cpp $(SOURCES2)
	$(CXX) $(CXXFLAGS) -I. -o test tests/metamodule/metamodule_1.cpp $(SOURCES2)

test2: tests/colorTest.cpp $(SOURCES2)
	$(CXX) $(CXXFLAGS) -I. -o test2 tests/colorTest.cpp $(SOURCES2)

benchmarks: tests/benchmarks/benchmarks.cpp $(SOURCES2)
	$(CXX) $(CXXFLAGS) -I. -o benchmarks tests/benchmarks/benchmarks.cpp $(SOURCES2)

gui: gui/gui.cpp
	$(CXX) $(CXXFLAGS) -I. -o gui gui/gui.cpp

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)