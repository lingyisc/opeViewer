
file(GLOB_RECURSE _SRCS *.cpp *.h)

add_library(opeViewer STATIC ${_SRCS})
target_link_libraries(opeViewer PUBLIC osg::osg OpenGL::GL)
target_include_directories(opeViewer PUBLIC ${PROJECT_SOURCE_DIR}/src)
