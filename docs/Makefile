# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++17 -I $(VCPKG_ROOT)/installed/x64-windows/include -I.. -MMD -MP

# Output directory for object files
OBJDIR := obj

# Define source files common to all examples
COMMON_SRCS := ../ConfigurationSpace.cpp ../Lattice.cpp ../ModuleManager.cpp ../MoveManager.cpp ../MetaModule.cpp ../LatticeSetup.cpp ../Scenario.cpp
COMMON_OBJS := $(COMMON_SRCS:../%.cpp=$(OBJDIR)/%.o)
COMMON_DEPS := $(COMMON_OBJS:.o=.d)

# Use wildcard to find all .cpp files in docs/examples
EXAMPLE_SRCS := $(wildcard examples/*.cpp)
EXAMPLE_OBJS := $(EXAMPLE_SRCS:examples/%.cpp=$(OBJDIR)/%.o)
EXAMPLE_EXES := $(EXAMPLE_SRCS:examples/%.cpp=%)
EXAMPLE_DEPS := $(EXAMPLE_OBJS:.o=.d)

# Default target
all: $(EXAMPLE_EXES)

# Rule to create the object directory
$(OBJDIR):
	mkdir -p $@

# Rule to compile common source object files
$(OBJDIR)/%.o: ../%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile example source object files
$(OBJDIR)/%.o: examples/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Pattern rule for building examples
$(EXAMPLE_EXES): % : $(OBJDIR)/%.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Include dependency files
-include $(COMMON_DEPS) $(EXAMPLE_DEPS)

# Phony targets
.PHONY: all clean

# Clean target
clean:
	rm -rf $(OBJDIR) $(EXAMPLE_EXES)