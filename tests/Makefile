CXX = g++

CXXFLAGS = -w -std=c++17 -I $(VCPKG_ROOT)/installed/x64-windows/include

LDFLAGS = -L $(VCPKG_ROOT)/installed/x64-windows/lib -lgmock -lgtest -lbenchmark -lpthread -fopenmp

SOURCES = unit-tests/colors/colorTest.cpp $(wildcard ../pathfinder/*/*.cpp)

SOURCES2 = $(wildcard */*.cpp)

OBJECTS = $(SOURCES:.cpp=.o)

EXECUTABLE = main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(BOOST_LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)