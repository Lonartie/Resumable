#include <iostream>
#include "Memory/Memory.h"

__attribute__((constructor))
inline void init_memory() {
  Memory::instance().init();
}
