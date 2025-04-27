project(Sandbox)

add_executable(Sandbox main.cpp)

target_link_libraries(Sandbox PRIVATE Core Qt6::Core Qt6::Gui Qt6::Widgets)
target_include_directories(Sandbox PUBLIC ../Core)

target_compile_options(Sandbox PRIVATE -fno-pie)
target_link_options(Sandbox PRIVATE -Wl,-no_pie)

add_dependencies(Sandbox Loader)
