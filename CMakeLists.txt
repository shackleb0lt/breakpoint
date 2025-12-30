cmake_minimum_required(VERSION 3.10)
project(breakpoint C)

# Set C standard and ensure symbols are generated for GDB
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# 1.1 External Library: linenoise
# ---------------------------------------------------------------------------
# We define linenoise as a static library.
add_library(linenoise STATIC lib/linenoise/linenoise.c)

# Tell users of linenoise where to find its header
target_include_directories(linenoise PUBLIC lib/linenoise)

# ---------------------------------------------------------------------------
# 2. Main Library: breakpoint
# ---------------------------------------------------------------------------
# Collect all source files from src
file(GLOB LIBRARY_SOURCES "src/*.c")

add_library(breakpoint STATIC ${LIBRARY_SOURCES})

# Include directories:
# - inc: Public headers
# - src: Internal headers
target_include_directories(breakpoint PUBLIC inc)
target_include_directories(breakpoint PRIVATE src)

# ---------------------------------------------------------------------------
# 3. Final Executable: app
# ---------------------------------------------------------------------------
file(GLOB_RECURSE APP_SOURCES "app/*.c")

add_executable(bkpt ${APP_SOURCES})

# The app needs the public headers and the engine library
target_link_libraries(bkpt PRIVATE breakpoint)
# target_link_libraries(bkpt PRIVATE libcli)
target_link_libraries(bkpt PRIVATE linenoise)

# ---------------------------------------------------------------------------
# 4. Testing Target (Commented out)
# ---------------------------------------------------------------------------
# To enable: Create a 'tests' folder, add test files, and uncomment below.
#
# enable_testing()
# file(GLOB_RECURSE TEST_SOURCES "tests/*.c")
# add_executable(run_tests ${TEST_SOURCES})
# target_link_libraries(run_tests PRIVATE breakpoint_lib)
# add_test(NAME AllTests COMMAND run_tests)