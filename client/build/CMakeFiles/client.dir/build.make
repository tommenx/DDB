# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/tommenx/share/client

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tommenx/share/client/build

# Include any dependencies generated for this target.
include CMakeFiles/client.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/client.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/client.dir/flags.make

CMakeFiles/client.dir/rpc_sql.cpp.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/rpc_sql.cpp.o: ../rpc_sql.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tommenx/share/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/client.dir/rpc_sql.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/client.dir/rpc_sql.cpp.o -c /home/tommenx/share/client/rpc_sql.cpp

CMakeFiles/client.dir/rpc_sql.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/client.dir/rpc_sql.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tommenx/share/client/rpc_sql.cpp > CMakeFiles/client.dir/rpc_sql.cpp.i

CMakeFiles/client.dir/rpc_sql.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/client.dir/rpc_sql.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tommenx/share/client/rpc_sql.cpp -o CMakeFiles/client.dir/rpc_sql.cpp.s

CMakeFiles/client.dir/rpc_sql.cpp.o.requires:

.PHONY : CMakeFiles/client.dir/rpc_sql.cpp.o.requires

CMakeFiles/client.dir/rpc_sql.cpp.o.provides: CMakeFiles/client.dir/rpc_sql.cpp.o.requires
	$(MAKE) -f CMakeFiles/client.dir/build.make CMakeFiles/client.dir/rpc_sql.cpp.o.provides.build
.PHONY : CMakeFiles/client.dir/rpc_sql.cpp.o.provides

CMakeFiles/client.dir/rpc_sql.cpp.o.provides.build: CMakeFiles/client.dir/rpc_sql.cpp.o


CMakeFiles/client.dir/local_sql.cpp.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/local_sql.cpp.o: ../local_sql.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tommenx/share/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/client.dir/local_sql.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/client.dir/local_sql.cpp.o -c /home/tommenx/share/client/local_sql.cpp

CMakeFiles/client.dir/local_sql.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/client.dir/local_sql.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tommenx/share/client/local_sql.cpp > CMakeFiles/client.dir/local_sql.cpp.i

CMakeFiles/client.dir/local_sql.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/client.dir/local_sql.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tommenx/share/client/local_sql.cpp -o CMakeFiles/client.dir/local_sql.cpp.s

CMakeFiles/client.dir/local_sql.cpp.o.requires:

.PHONY : CMakeFiles/client.dir/local_sql.cpp.o.requires

CMakeFiles/client.dir/local_sql.cpp.o.provides: CMakeFiles/client.dir/local_sql.cpp.o.requires
	$(MAKE) -f CMakeFiles/client.dir/build.make CMakeFiles/client.dir/local_sql.cpp.o.provides.build
.PHONY : CMakeFiles/client.dir/local_sql.cpp.o.provides

CMakeFiles/client.dir/local_sql.cpp.o.provides.build: CMakeFiles/client.dir/local_sql.cpp.o


CMakeFiles/client.dir/parser.cpp.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/parser.cpp.o: ../parser.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tommenx/share/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/client.dir/parser.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/client.dir/parser.cpp.o -c /home/tommenx/share/client/parser.cpp

CMakeFiles/client.dir/parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/client.dir/parser.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tommenx/share/client/parser.cpp > CMakeFiles/client.dir/parser.cpp.i

CMakeFiles/client.dir/parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/client.dir/parser.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tommenx/share/client/parser.cpp -o CMakeFiles/client.dir/parser.cpp.s

CMakeFiles/client.dir/parser.cpp.o.requires:

.PHONY : CMakeFiles/client.dir/parser.cpp.o.requires

CMakeFiles/client.dir/parser.cpp.o.provides: CMakeFiles/client.dir/parser.cpp.o.requires
	$(MAKE) -f CMakeFiles/client.dir/build.make CMakeFiles/client.dir/parser.cpp.o.provides.build
.PHONY : CMakeFiles/client.dir/parser.cpp.o.provides

CMakeFiles/client.dir/parser.cpp.o.provides.build: CMakeFiles/client.dir/parser.cpp.o


CMakeFiles/client.dir/sql_exec.cpp.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/sql_exec.cpp.o: ../sql_exec.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tommenx/share/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/client.dir/sql_exec.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/client.dir/sql_exec.cpp.o -c /home/tommenx/share/client/sql_exec.cpp

CMakeFiles/client.dir/sql_exec.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/client.dir/sql_exec.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tommenx/share/client/sql_exec.cpp > CMakeFiles/client.dir/sql_exec.cpp.i

CMakeFiles/client.dir/sql_exec.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/client.dir/sql_exec.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tommenx/share/client/sql_exec.cpp -o CMakeFiles/client.dir/sql_exec.cpp.s

CMakeFiles/client.dir/sql_exec.cpp.o.requires:

.PHONY : CMakeFiles/client.dir/sql_exec.cpp.o.requires

CMakeFiles/client.dir/sql_exec.cpp.o.provides: CMakeFiles/client.dir/sql_exec.cpp.o.requires
	$(MAKE) -f CMakeFiles/client.dir/build.make CMakeFiles/client.dir/sql_exec.cpp.o.provides.build
.PHONY : CMakeFiles/client.dir/sql_exec.cpp.o.provides

CMakeFiles/client.dir/sql_exec.cpp.o.provides.build: CMakeFiles/client.dir/sql_exec.cpp.o


CMakeFiles/client.dir/metadata.cpp.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/metadata.cpp.o: ../metadata.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tommenx/share/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/client.dir/metadata.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/client.dir/metadata.cpp.o -c /home/tommenx/share/client/metadata.cpp

CMakeFiles/client.dir/metadata.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/client.dir/metadata.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tommenx/share/client/metadata.cpp > CMakeFiles/client.dir/metadata.cpp.i

CMakeFiles/client.dir/metadata.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/client.dir/metadata.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tommenx/share/client/metadata.cpp -o CMakeFiles/client.dir/metadata.cpp.s

CMakeFiles/client.dir/metadata.cpp.o.requires:

.PHONY : CMakeFiles/client.dir/metadata.cpp.o.requires

CMakeFiles/client.dir/metadata.cpp.o.provides: CMakeFiles/client.dir/metadata.cpp.o.requires
	$(MAKE) -f CMakeFiles/client.dir/build.make CMakeFiles/client.dir/metadata.cpp.o.provides.build
.PHONY : CMakeFiles/client.dir/metadata.cpp.o.provides

CMakeFiles/client.dir/metadata.cpp.o.provides.build: CMakeFiles/client.dir/metadata.cpp.o


CMakeFiles/client.dir/main.cpp.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tommenx/share/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/client.dir/main.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/client.dir/main.cpp.o -c /home/tommenx/share/client/main.cpp

CMakeFiles/client.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/client.dir/main.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tommenx/share/client/main.cpp > CMakeFiles/client.dir/main.cpp.i

CMakeFiles/client.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/client.dir/main.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tommenx/share/client/main.cpp -o CMakeFiles/client.dir/main.cpp.s

CMakeFiles/client.dir/main.cpp.o.requires:

.PHONY : CMakeFiles/client.dir/main.cpp.o.requires

CMakeFiles/client.dir/main.cpp.o.provides: CMakeFiles/client.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/client.dir/build.make CMakeFiles/client.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/client.dir/main.cpp.o.provides

CMakeFiles/client.dir/main.cpp.o.provides.build: CMakeFiles/client.dir/main.cpp.o


# Object files for target client
client_OBJECTS = \
"CMakeFiles/client.dir/rpc_sql.cpp.o" \
"CMakeFiles/client.dir/local_sql.cpp.o" \
"CMakeFiles/client.dir/parser.cpp.o" \
"CMakeFiles/client.dir/sql_exec.cpp.o" \
"CMakeFiles/client.dir/metadata.cpp.o" \
"CMakeFiles/client.dir/main.cpp.o"

# External object files for target client
client_EXTERNAL_OBJECTS =

client: CMakeFiles/client.dir/rpc_sql.cpp.o
client: CMakeFiles/client.dir/local_sql.cpp.o
client: CMakeFiles/client.dir/parser.cpp.o
client: CMakeFiles/client.dir/sql_exec.cpp.o
client: CMakeFiles/client.dir/metadata.cpp.o
client: CMakeFiles/client.dir/main.cpp.o
client: CMakeFiles/client.dir/build.make
client: /home/tommenx/share/rpclib/build/librpc.a
client: /home/tommenx/download/jsoncpp-src-0.5.0/libs/linux-gcc-5.4.0/libjson.a
client: /home/tommenx/share/rpclib/build/libcurl.a
client: CMakeFiles/client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/tommenx/share/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX executable client"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/client.dir/build: client

.PHONY : CMakeFiles/client.dir/build

CMakeFiles/client.dir/requires: CMakeFiles/client.dir/rpc_sql.cpp.o.requires
CMakeFiles/client.dir/requires: CMakeFiles/client.dir/local_sql.cpp.o.requires
CMakeFiles/client.dir/requires: CMakeFiles/client.dir/parser.cpp.o.requires
CMakeFiles/client.dir/requires: CMakeFiles/client.dir/sql_exec.cpp.o.requires
CMakeFiles/client.dir/requires: CMakeFiles/client.dir/metadata.cpp.o.requires
CMakeFiles/client.dir/requires: CMakeFiles/client.dir/main.cpp.o.requires

.PHONY : CMakeFiles/client.dir/requires

CMakeFiles/client.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/client.dir/cmake_clean.cmake
.PHONY : CMakeFiles/client.dir/clean

CMakeFiles/client.dir/depend:
	cd /home/tommenx/share/client/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tommenx/share/client /home/tommenx/share/client /home/tommenx/share/client/build /home/tommenx/share/client/build /home/tommenx/share/client/build/CMakeFiles/client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/client.dir/depend
