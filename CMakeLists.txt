cmake_minimum_required(VERSION 3.25)
project(breakpoint C CXX)

find_program(CLANG_C_COMPILER NAMES clang)
find_program(CLANG_CXX_COMPILER NAMES clang++)

if(CLANG_C_COMPILER AND CLANG_CXX_COMPILER)
    set(CMAKE_C_COMPILER "${CLANG_C_COMPILER}")
    set(CMAKE_CXX_COMPILER "${CLANG_CXX_COMPILER}")
    message(STATUS "Using Clang compilers.")
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 3 REQUIRED)
find_package(fmt CONFIG REQUIRED)

string(TOUPPER "${CMAKE_BUILD_TYPE}" CONFIG_TYPE)

if (CONFIG_TYPE STREQUAL "DEBUG")
    add_definitions(-DDEBUG_MODE)
endif()

# ---------------------------------------------------------------------------
# 1 External Library: linenoise
# ---------------------------------------------------------------------------
# We define linenoise as a static library.
add_library(linenoise STATIC lib/linenoise/linenoise.c)

# Tell users of linenoise where to find its header
target_include_directories(linenoise PUBLIC lib/linenoise)

# ---------------------------------------------------------------------------
# 2. Main Library: breakpoint
# ---------------------------------------------------------------------------
# Collect all source files from src

add_library(breakpoint STATIC
    src/process.cpp
    src/pipe.cpp
    src/registers.cpp
    src/breakpoint_site.cpp
)

# Include directories:
# - inc: Public headers
# - src: Internal headers
target_include_directories(breakpoint PUBLIC inc)
target_include_directories(breakpoint PRIVATE src)
target_compile_options(breakpoint PRIVATE -Wall -Wextra -Wpedantic)

# ---------------------------------------------------------------------------
# 3. Final Executable: bkpt
# ---------------------------------------------------------------------------

add_executable(bkpt
    app/commands.cpp
    app/main.cpp
)

target_include_directories(bkpt PRIVATE app)

# The app needs the public headers and the engine library
target_link_libraries(bkpt PRIVATE linenoise breakpoint fmt::fmt)
target_compile_options(bkpt PRIVATE -Wall -Wextra -Wpedantic)

# ---------------------------------------------------------------------------
# 4. Testing Library: breakpoint
# ---------------------------------------------------------------------------
enable_testing()

add_executable(two_seconds test/guinea/two_seconds.c)
add_executable(outta_here  test/guinea/outta_here.c)
add_executable(asm_magic   test/guinea/asm_magic.c)
add_executable(hello       test/guinea/hello.c)

target_compile_options(two_seconds PRIVATE -g -O0)
target_compile_options(outta_here  PRIVATE -g -O0)
target_compile_options(asm_magic   PRIVATE -g -O0)
target_compile_options(hello       PRIVATE -g -O0)

add_executable(test_launch test/test_launch.cpp)
target_include_directories(test_launch PRIVATE inc test)
target_link_libraries(test_launch PRIVATE breakpoint Catch2::Catch2WithMain)

add_executable(test_attach test/test_attach.cpp)
target_include_directories(test_attach PRIVATE inc test)
target_link_libraries(test_attach PRIVATE breakpoint Catch2::Catch2WithMain)

add_executable(test_commands test/test_commands.cpp app/commands.cpp)
target_include_directories(test_commands PRIVATE app test)
target_link_libraries(test_commands PRIVATE Catch2::Catch2WithMain)

add_executable(test_register test/test_register.cpp)
target_include_directories(test_register PRIVATE inc test)
target_link_libraries(test_register PRIVATE breakpoint Catch2::Catch2WithMain)

add_executable(test_breakpoint test/test_breakpoint.cpp)
target_include_directories(test_breakpoint PRIVATE inc test)
target_link_libraries(test_breakpoint PRIVATE breakpoint Catch2::Catch2WithMain)

add_test(NAME TestGuinea1  COMMAND two_seconds)
add_test(NAME TestGuinea2  COMMAND outta_here)

add_test(NAME TestLaunch   COMMAND test_launch)
add_test(NAME TestAttach   COMMAND test_attach)
add_test(NAME TestCommands COMMAND test_commands)
add_test(NAME TestRegister COMMAND test_register)
add_test(NAME TestBreakpoint COMMAND test_breakpoint)