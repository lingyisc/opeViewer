cmake_minimum_required(VERSION 3.21)
project(opeViewer)

set(CMAKE_CXX_STANDARD 17)

add_compile_options($<$<BOOL:${MSVC}>:/utf-8>)

if(NOT ${CMAKE_PROJECT_NAME} STREQUAL opeViewer)
  add_subdirectory(src)
  add_subdirectory(examples)
  return()
endif()

# OpenSceneGraph
find_package(OpenSceneGraph REQUIRED osgDB osgGA osgUtil)
# osg alias target
add_library(osg::osg IMPORTED INTERFACE)
target_include_directories(osg::osg INTERFACE ${OPENSCENEGRAPH_INCLUDE_DIRS})
target_link_libraries(osg::osg INTERFACE ${OPENSCENEGRAPH_LIBRARIES})

# Qt5
find_package(Qt5 5.12 COMPONENTS Widgets)

# OpenGL
find_package(OpenGL REQUIRED)

add_subdirectory(src)
add_subdirectory(examples)
