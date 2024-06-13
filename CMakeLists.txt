cmake_minimum_required(VERSION 3.21)

# Configure cross-compilation for Windows using Clang
if(NOT WIN32)
  set(CMAKE_SYSTEM_NAME Windows)
  set(CMAKE_SYSTEM_VERSION 10)
  set(CMAKE_C_COMPILER clang)
  set(CMAKE_CXX_COMPILER clang++)
  set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

  # Specify the target triple for Clang
  set(CMAKE_C_COMPILER_TARGET x86_64-w64-mingw32)
  set(CMAKE_CXX_COMPILER_TARGET x86_64-w64-mingw32)
  set(CMAKE_ASM_COMPILER_TARGET x86_64-w64-mingw32)

  # Add additional flags for Clang
  set(CMAKE_C_FLAGS "--target=x86_64-w64-mingw32")
  set(CMAKE_CXX_FLAGS "--target=x86_64-w64-mingw32")

  set(IMPORTEDLIB)
endif()

project(
  ReShadeEffectToggler
  VERSION 3.0.1
  LANGUAGES CXX)

# Set your project version and specify C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_TESTS OFF)
# Hide useless warning...
add_definitions(-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)

# Include SKSEPlugin.cmake from the same directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(SKSEPlugin)

# Configure MSVC-specific settings for C++23
if(MSVC_VERSION GREATER_EQUAL 1936 AND MSVC_IDE) # 17.6+
  file(
    CONFIGURE
    OUTPUT
    "${CMAKE_BINARY_DIR}/Directory.Build.props"
    CONTENT
    [==[
    <Project>
      <ItemDefinitionGroup>
        <ClCompile>
          <BuildStlModules>false</BuildStlModules>
        </ClCompile>
      </ItemDefinitionGroup>
    </Project>
    ]==]
    @ONLY)
endif()
