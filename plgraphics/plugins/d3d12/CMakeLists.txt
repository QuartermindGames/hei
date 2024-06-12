add_library(pl_d3d12_driver SHARED
        d3d12_driver.cpp
        plugin.c
        plugin.h
        )

target_link_libraries(pl_d3d12_driver D3d12 DXGI d3dcompiler)

target_include_directories(pl_d3d12_driver PRIVATE
        ../../plcore/include/
        ../../plgraphics/include/)
