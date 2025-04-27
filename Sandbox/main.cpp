#include <iostream>
#include "Main.h"

task start(int argc, char** argv) {
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

         co_yield exec_pause();

         // resuming execution
         pauses++;
         std::cout << "resuming counting to " << n << " after " << pauses << " pauses\n";
      }
   }
}