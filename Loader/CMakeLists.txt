project(Loader)

add_library(Loader SHARED loader.cpp)


target_include_directories(Loader PUBLIC ../Core)
target_link_libraries(Loader PRIVATE Core)