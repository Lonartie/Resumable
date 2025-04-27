#pragma once
#include <coroutine>
#include <exception>
#include "../Memory/Memory.h"

struct exec_pause {
   bool await_ready() const noexcept {
      return false;
   }

   void await_suspend(std::coroutine_handle<>) noexcept {
      Memory::instance().write("heap.bin");
      std::exit(0);
   }

   void await_resume() noexcept {
      // No-op
   }
};

struct checkpoint {
   bool await_ready() const noexcept {
      return false;
   }

   void await_suspend(std::coroutine_handle<>) noexcept {
      Memory::instance().write("heap.bin");
   }

   void await_resume() noexcept {
      // No-op
   }
};

/// @brief C++20 coroutine promise type for void return types
struct promise {
   using handle_type = std::coroutine_handle<promise>;

   promise() {
   }

   ~promise() {
   }

   void unhandled_exception() {
      std::terminate();
   }

   void return_void() {
   }

   auto get_return_object() {
      return handle_type::from_promise(*this);
   }

   auto initial_suspend() {
      return std::suspend_always{};
   }

   auto final_suspend() noexcept {
      return std::suspend_always{};
   }

   void yield_value() {
      // No-op for void return type
   }

   std::suspend_always yield_value(std::suspend_always) {
      return {};
   }

   exec_pause yield_value(exec_pause v) {
      return v;
   }

   checkpoint yield_value(checkpoint v) {
      return v;
   }
};

/// @brief C++20 coroutine task for void return types
struct task {
   using promise_type = promise;
   using handle_type = std::coroutine_handle<promise_type>;
   task() : handle(nullptr) {
   }

   task(handle_type h) : handle(h) {
   }

   task(void* address) : handle(handle_type::from_address(address)) {
   }

   ~task() {
      if (handle) {
         handle.destroy();
      }
   }

   task(const task&) = delete;
   task& operator=(const task&) = delete;

   task(task&& other) noexcept : handle(other.handle) {
      other.handle = nullptr;
   }

   task& operator=(task&& other) noexcept {
      if (this != &other) {
         if (handle) {
            handle.destroy();
         }
         handle = other.handle;
         other.handle = nullptr;
      }
      return *this;
   }

   void* address() {
      return handle.address();
   }

   void resume() {
      if (handle) {
         handle.resume();
      }
   }

   bool done() const {
      return handle.done();
   }

   handle_type handle;
};
