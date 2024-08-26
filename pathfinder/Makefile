CXX = g++

CXXFLAGS = -w -std=c++17 -I $(VCPKG_ROOT)/installed/x64-windows/include

LDFLAGS = -L $(VCPKG_ROOT)/installed/x64-windows/lib -lgmock -lgtest -lbenchmark -lpthread -fopenmp

SOURCES = $(wildcard *.cpp) $(wildcard */*.cpp)

SOURCES2 = $(wildcard */*.cpp)

OBJECTS = $(SOURCES:.cpp=.o)

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

astar: tests/proprety-based-checking/astar.cpp $(SOURCES2)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -I. -o astar tests/proprety-based-checking/astar.cpp $(SOURCES2)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)