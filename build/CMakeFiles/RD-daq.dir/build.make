# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/cmake/3.26.3/bin/cmake

# The command to remove a file.
RM = /opt/cmake/3.26.3/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/msgc/RDsoft/source

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/msgc/RDsoft/build

# Include any dependencies generated for this target.
include CMakeFiles/RD-daq.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/RD-daq.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/RD-daq.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/RD-daq.dir/flags.make

CMakeFiles/RD-daq.dir/RD-daq.cpp.o: CMakeFiles/RD-daq.dir/flags.make
CMakeFiles/RD-daq.dir/RD-daq.cpp.o: /home/msgc/RDsoft/source/RD-daq.cpp
CMakeFiles/RD-daq.dir/RD-daq.cpp.o: CMakeFiles/RD-daq.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/msgc/RDsoft/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/RD-daq.dir/RD-daq.cpp.o"
	/opt/gcc/9.4.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/RD-daq.dir/RD-daq.cpp.o -MF CMakeFiles/RD-daq.dir/RD-daq.cpp.o.d -o CMakeFiles/RD-daq.dir/RD-daq.cpp.o -c /home/msgc/RDsoft/source/RD-daq.cpp

CMakeFiles/RD-daq.dir/RD-daq.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RD-daq.dir/RD-daq.cpp.i"
	/opt/gcc/9.4.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/msgc/RDsoft/source/RD-daq.cpp > CMakeFiles/RD-daq.dir/RD-daq.cpp.i

CMakeFiles/RD-daq.dir/RD-daq.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RD-daq.dir/RD-daq.cpp.s"
	/opt/gcc/9.4.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/msgc/RDsoft/source/RD-daq.cpp -o CMakeFiles/RD-daq.dir/RD-daq.cpp.s

# Object files for target RD-daq
RD__daq_OBJECTS = \
"CMakeFiles/RD-daq.dir/RD-daq.cpp.o"

# External object files for target RD-daq
RD__daq_EXTERNAL_OBJECTS =

RD-daq: CMakeFiles/RD-daq.dir/RD-daq.cpp.o
RD-daq: CMakeFiles/RD-daq.dir/build.make
RD-daq: CMakeFiles/RD-daq.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/msgc/RDsoft/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable RD-daq"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/RD-daq.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/RD-daq.dir/build: RD-daq
.PHONY : CMakeFiles/RD-daq.dir/build

CMakeFiles/RD-daq.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/RD-daq.dir/cmake_clean.cmake
.PHONY : CMakeFiles/RD-daq.dir/clean

CMakeFiles/RD-daq.dir/depend:
	cd /home/msgc/RDsoft/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/msgc/RDsoft/source /home/msgc/RDsoft/source /home/msgc/RDsoft/build /home/msgc/RDsoft/build /home/msgc/RDsoft/build/CMakeFiles/RD-daq.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/RD-daq.dir/depend

