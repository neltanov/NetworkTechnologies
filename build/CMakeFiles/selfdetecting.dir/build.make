# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/nelta/net_tech/selfdetecting

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/nelta/net_tech/build

# Include any dependencies generated for this target.
include CMakeFiles/selfdetecting.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/selfdetecting.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/selfdetecting.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/selfdetecting.dir/flags.make

CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.o: CMakeFiles/selfdetecting.dir/flags.make
CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.o: /home/nelta/net_tech/selfdetecting/src/backend/selfdetector.cpp
CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.o: CMakeFiles/selfdetecting.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nelta/net_tech/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.o -MF CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.o.d -o CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.o -c /home/nelta/net_tech/selfdetecting/src/backend/selfdetector.cpp

CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/nelta/net_tech/selfdetecting/src/backend/selfdetector.cpp > CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.i

CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/nelta/net_tech/selfdetecting/src/backend/selfdetector.cpp -o CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.s

# Object files for target selfdetecting
selfdetecting_OBJECTS = \
"CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.o"

# External object files for target selfdetecting
selfdetecting_EXTERNAL_OBJECTS = \
"/home/nelta/net_tech/build/CMakeFiles/corelib.dir/src/backend/multicast_sender.cpp.o" \
"/home/nelta/net_tech/build/CMakeFiles/corelib.dir/src/backend/multicast_receiver.cpp.o"

selfdetecting: CMakeFiles/selfdetecting.dir/src/backend/selfdetector.cpp.o
selfdetecting: CMakeFiles/corelib.dir/src/backend/multicast_sender.cpp.o
selfdetecting: CMakeFiles/corelib.dir/src/backend/multicast_receiver.cpp.o
selfdetecting: CMakeFiles/selfdetecting.dir/build.make
selfdetecting: CMakeFiles/selfdetecting.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/nelta/net_tech/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable selfdetecting"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/selfdetecting.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/selfdetecting.dir/build: selfdetecting
.PHONY : CMakeFiles/selfdetecting.dir/build

CMakeFiles/selfdetecting.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/selfdetecting.dir/cmake_clean.cmake
.PHONY : CMakeFiles/selfdetecting.dir/clean

CMakeFiles/selfdetecting.dir/depend:
	cd /home/nelta/net_tech/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nelta/net_tech/selfdetecting /home/nelta/net_tech/selfdetecting /home/nelta/net_tech/build /home/nelta/net_tech/build /home/nelta/net_tech/build/CMakeFiles/selfdetecting.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/selfdetecting.dir/depend

