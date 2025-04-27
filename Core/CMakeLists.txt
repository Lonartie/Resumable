project(Core)

add_library(Core STATIC
        Memory/Memory.cpp
        Memory/Memory.h
        Memory/Allocator.h
        Memory/Mapping.h
        Memory/IO.h
        Utils/ScopeGuard.h
        Tasks/Task.h
        Memory/Allocator.cpp
        Memory/Mapping.cpp
        Memory/ControlHeader.h
        Memory/ControlHeader.cpp
        Main.h)