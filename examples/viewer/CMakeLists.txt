add_executable(plviewer main.c)
target_link_libraries(plviewer
        plcore
        plgraphics
        plmodel
        plwindow)
