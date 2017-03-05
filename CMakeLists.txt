cmake_minimum_required(VERSION 3.5.1)
project(crosstext)

include_directories(include)
set(CMAKE_CXX_STANDARD 14)

set(BASE_SOURCES src/CrossText.cpp)
set(FREETYPE_SOURCES src/FreeType.cpp)
set(CT_TEST_SOURCES test/unit/UnitTests.cpp)
set(FT_TEST_SOURCES test/freetype/fttest.cpp)

set(SOURCES
    src/CrossText.cpp
    src/FreeType.cpp
    test/freetype/fttest.cpp)
add_definitions(-DOS_LINUX)
add_executable(fttest ${BASE_SOURCES} ${FREETYPE_SOURCES} ${FT_TEST_SOURCES})
add_executable(cttest ${BASE_SOURCES} ${CT_TEST_SOURCES})

find_package (PNG)
if (PNG_FOUND)
  include_directories(${PNG_INCLUDE_DIRS})
  target_link_libraries (fttest ${PNG_LIBRARIES})
endif (PNG_FOUND)

find_package (Freetype)
if (FREETYPE_FOUND)
  include_directories(${FREETYPE_INCLUDE_DIRS})
  target_link_libraries (fttest ${FREETYPE_LIBRARIES})
endif (FREETYPE_FOUND)

# Enable warnings
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()
