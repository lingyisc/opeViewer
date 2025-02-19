
add_subdirectory(opeViewer)

if(Qt5_FOUND OR Qt6_FOUND)
  add_subdirectory(opeViewerQt)
endif()
