#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>

inline ssize_t read_buffer(const char* path, char* buffer, size_t buffer_size) {
   int fd = open(path, O_RDONLY);
   if (fd < 0) {
      perror("open failed");
      return -1;
   }

   ssize_t total_read = 0;
   while (total_read < static_cast<ssize_t>(buffer_size)) {
      ssize_t bytes_read = read(fd, buffer + total_read, buffer_size - total_read);
      if (bytes_read < 0) {
         perror("read failed");
         close(fd);
         return -1;
      }
      if (bytes_read == 0)
         break; // EOF
      total_read += bytes_read;
   }

   close(fd);
   return total_read;
}

inline ssize_t write_buffer(const char* path, const char* buffer, size_t buffer_size) {
   int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644); // 0644 = rw-r--r--
   if (fd < 0) {
      perror("open failed");
      return -1;
   }

   ssize_t total_written = 0;
   while (total_written < static_cast<ssize_t>(buffer_size)) {
      ssize_t bytes_written = write(fd, buffer + total_written, buffer_size - total_written);
      if (bytes_written < 0) {
         perror("write failed");
         close(fd);
         return -1;
      }
      total_written += bytes_written;
   }

   close(fd);
   return total_written;
}
