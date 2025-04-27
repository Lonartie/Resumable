# Resumable
Resumable is a framework based on coroutines and memory monitoring which
allows you to write resumable programs in C++. It captures the heap state
of the program and since functions are coroutines they get captured too.
This allows you to write checkpoints in your code. When the program terminates
you can restore the programs state and continue execution from the last
checkpoint.

## Disclaimer
This is a work in progress and is not yet complete. This project is also not
designed to be used in production code. It is a research project and should
not be used in production code. It is not optimized for performance and does
not handle all edge cases.

## Example
Just include the `Main.h` header and write your `start` function. The 
rest is handled by the framework.

```c++
#include "Main.h"

task start(int, char**) {
   int pauses = 0;
   int n;
   char c;

   // ask for counting limit
   std::cout << "count up to: ";
   std::cin >> n;

   for (int i = 0; i < n; i++) {
      // print number
      std::cout << "counting " << i+1 << " pause? (y/n):";

      // pausing execution
      if ((std::cin >> c, c) == 'y') {
         std::cout << "pausing\n";

         co_yield exec_pause(); // <-- stops execution and stores the state
         // or use co_yield checkpoint(); to store the state and continue

         // resuming execution
         pauses++;
         std::cout << "resuming counting to " << n << " after " << pauses << " pauses\n";
      }
   }
}
```