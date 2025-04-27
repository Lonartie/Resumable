#include <iostream>
#include "Main.h"

task start(int, char**) {
   size_t sizeMB = 0;
   std::cout << "Enter size in MB: ";
   std::cin >> sizeMB;
   // convert sizeMB to count of size_t
   size_t size = (sizeMB * 1024 * 1024) / sizeof(size_t);

   size_t* ptr = new size_t[size];
   for (size_t i = 0; i < size; ++i) {
      ptr[i] = i;
   }
   std::cout << "Allocated " << size * sizeof(size_t) << " bytes at " << ptr << "\n";

   co_yield exec_pause();

   std::cout << "Verifying " << size * sizeof(size_t) << " bytes at " << ptr << "\n";
   for (size_t i = 0; i < size; ++i) {
      if (ptr[i] != i) {
         std::cout << "Error at index " << i << ": expected " << i << ", got " << ptr[i] << "\n";
         delete[] ptr;
         co_return;
      }
   }
   delete[] ptr;
   std::cout << "Verification successful\n";
}

// task start(int argc, char** argv) {
//    int pauses = 0;
//    int n;
//    char c;
//
//    // ask for counting limit
//    std::cout << "count up to: ";
//    std::cin >> n;
//
//    for (int i = 0; i < n; i++) {
//       // print number
//       std::cout << "counting " << i+1 << " pause? (y/n):";
//
//       // pausing execution
//       if ((std::cin >> c, c) == 'y') {
//          std::cout << "pausing\n";
//
//          co_yield exec_pause();
//
//          // resuming execution
//          pauses++;
//          std::cout << "resuming counting to " << n << " after " << pauses << " pauses\n";
//       }
//    }
// }