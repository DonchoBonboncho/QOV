# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.27

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
CMAKE_SOURCE_DIR = /home/don40/QOV_GitHub/encode

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/don40/QOV_GitHub/encode

# Include any dependencies generated for this target.
include CMakeFiles/encode.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/encode.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/encode.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/encode.dir/flags.make

CMakeFiles/encode.dir/encode.cpp.o: CMakeFiles/encode.dir/flags.make
CMakeFiles/encode.dir/encode.cpp.o: encode.cpp
CMakeFiles/encode.dir/encode.cpp.o: CMakeFiles/encode.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/don40/QOV_GitHub/encode/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/encode.dir/encode.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/encode.dir/encode.cpp.o -MF CMakeFiles/encode.dir/encode.cpp.o.d -o CMakeFiles/encode.dir/encode.cpp.o -c /home/don40/QOV_GitHub/encode/encode.cpp

CMakeFiles/encode.dir/encode.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/encode.dir/encode.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/don40/QOV_GitHub/encode/encode.cpp > CMakeFiles/encode.dir/encode.cpp.i

CMakeFiles/encode.dir/encode.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/encode.dir/encode.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/don40/QOV_GitHub/encode/encode.cpp -o CMakeFiles/encode.dir/encode.cpp.s

# Object files for target encode
encode_OBJECTS = \
"CMakeFiles/encode.dir/encode.cpp.o"

# External object files for target encode
encode_EXTERNAL_OBJECTS =

encode: CMakeFiles/encode.dir/encode.cpp.o
encode: CMakeFiles/encode.dir/build.make
encode: /usr/local/lib/libopencv_gapi.so.4.8.0
encode: /usr/local/lib/libopencv_highgui.so.4.8.0
encode: /usr/local/lib/libopencv_ml.so.4.8.0
encode: /usr/local/lib/libopencv_objdetect.so.4.8.0
encode: /usr/local/lib/libopencv_photo.so.4.8.0
encode: /usr/local/lib/libopencv_stitching.so.4.8.0
encode: /usr/local/lib/libopencv_video.so.4.8.0
encode: /usr/local/lib/libopencv_videoio.so.4.8.0
encode: /usr/local/lib/libopencv_imgcodecs.so.4.8.0
encode: /usr/local/lib/libopencv_dnn.so.4.8.0
encode: /usr/local/lib/libopencv_calib3d.so.4.8.0
encode: /usr/local/lib/libopencv_features2d.so.4.8.0
encode: /usr/local/lib/libopencv_flann.so.4.8.0
encode: /usr/local/lib/libopencv_imgproc.so.4.8.0
encode: /usr/local/lib/libopencv_core.so.4.8.0
encode: CMakeFiles/encode.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/don40/QOV_GitHub/encode/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable encode"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/encode.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/encode.dir/build: encode
.PHONY : CMakeFiles/encode.dir/build

CMakeFiles/encode.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/encode.dir/cmake_clean.cmake
.PHONY : CMakeFiles/encode.dir/clean

CMakeFiles/encode.dir/depend:
	cd /home/don40/QOV_GitHub/encode && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/don40/QOV_GitHub/encode /home/don40/QOV_GitHub/encode /home/don40/QOV_GitHub/encode /home/don40/QOV_GitHub/encode /home/don40/QOV_GitHub/encode/CMakeFiles/encode.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/encode.dir/depend

