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
# target_compile_options(breakpoint PRIVATE -Wall -Wextra -Wpedantic)

# ---------------------------------------------------------------------------
# 3. Final Executable: bkpt
# ---------------------------------------------------------------------------
file(GLOB_RECURSE APP_SOURCES "app/*.c")

add_executable(bkpt ${APP_SOURCES})

# The app needs the public headers and the engine library
target_link_libraries(bkpt PRIVATE breakpoint)
target_link_libraries(bkpt PRIVATE linenoise)
# target_compile_options(bkpt PRIVATE -Wall -Wextra -Wpedantic)

# ---------------------------------------------------------------------------
# 4. Testing Library: breakpoint
# ---------------------------------------------------------------------------
enable_testing()

add_executable(two_seconds test/guinea/two_seconds.c)
add_executable(outta_here test/guinea/outta_here.c)
add_executable(asm_magic test/guinea/asm_magic.c)

target_compile_options(two_seconds PRIVATE -g)
target_compile_options(outta_here PRIVATE -g)
target_compile_options(asm_magic PRIVATE -g)

add_executable(test_launch test/test_launch.c)
target_link_libraries(test_launch PRIVATE breakpoint)

add_executable(test_attach test/test_attach.c)
target_link_libraries(test_attach PRIVATE breakpoint)

add_executable(test_register test/test_register.c)
target_link_libraries(test_register PRIVATE breakpoint)

add_test(NAME TestGuinea1  COMMAND two_seconds)
add_test(NAME TestGuinea2  COMMAND outta_here)

add_test(NAME TestLaunch   COMMAND test_launch)
add_test(NAME TestAttach   COMMAND test_attach)
add_test(NAME TestRegister COMMAND test_register)