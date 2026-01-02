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
# 3. Final Executable: bkpt
# ---------------------------------------------------------------------------
file(GLOB_RECURSE APP_SOURCES "app/*.c")

add_executable(bkpt ${APP_SOURCES})

# The app needs the public headers and the engine library
target_link_libraries(bkpt PRIVATE breakpoint)
target_link_libraries(bkpt PRIVATE linenoise)

# ---------------------------------------------------------------------------
# 4. Testing Library: breakpoint
# ---------------------------------------------------------------------------
# To enable: Create a 'tests' folder, add test files, and uncomment below.

enable_testing()

add_executable(debug_bin test/debug_bin.c)
target_compile_options(debug_bin PRIVATE -g)

add_executable(test_launch test/test_launch.c)
target_link_libraries(test_launch PRIVATE breakpoint)

add_executable(test_attach test/test_attach.c)
target_link_libraries(test_attach PRIVATE breakpoint)

add_test(NAME TestSimple  COMMAND debug_bin "2")
add_test(NAME TestComplex COMMAND debug_bin "6" "/tmp/iliketomoveit")
add_test(NAME TestLaunch COMMAND test_launch)
add_test(NAME TestAttach COMMAND test_attach)