
set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE _SRCS *.cpp *.h)

add_library(opeViewerQt STATIC ${_SRCS})
target_link_libraries(opeViewerQt PUBLIC Qt5::Widgets opeViewer)
target_include_directories(opeViewerQt PUBLIC ${PROJECT_SOURCE_DIR}/src)
