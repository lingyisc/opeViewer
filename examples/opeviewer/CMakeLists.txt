
add_executable(opeviewer.app opeviewer.cpp)
target_link_libraries(opeviewer.app opeViewerQt)
set_property(TARGET opeviewer.app PROPERTY OUTPUT_NAME opeviewer)
