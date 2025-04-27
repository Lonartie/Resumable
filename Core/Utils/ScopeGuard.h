#pragma once
#include <stdexcept>
#include <utility>

template <typename F>
struct ScopeGuard {
   explicit ScopeGuard(F&& f): m_f(std::forward<F>(f)) {}
   ~ScopeGuard() {
      m_f();
   }
   ScopeGuard(const ScopeGuard&) = delete;
   ScopeGuard& operator=(const ScopeGuard&) = delete;
   ScopeGuard(ScopeGuard&&) = default;
   ScopeGuard& operator=(ScopeGuard&&) = default;
private:
   F m_f;
};

template <typename Tag, typename F1, typename F2>
auto if_recursion_else(F1&& f1, F2&& f2) -> decltype(auto) {
   thread_local bool guard = false;
   if (guard) {
      return f1();
   }
   guard = true;
   auto guard1 = ScopeGuard([&] { guard = false; });
   return f1();
}