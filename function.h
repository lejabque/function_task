#pragma once
#include "storage.h"

template<typename F>
struct function;

template<typename R, typename... Args>
struct function<R(Args...)> {
  function() noexcept = default;

  function(function const& other) = default;
  function(function&& other) noexcept = default;

  function& operator=(function const& rhs) = default;
  function& operator=(function&& rhs) noexcept = default;

  template<typename T>
  function(T val)
      : func_storage(std::move(val)) {}

  ~function() = default;

  explicit operator bool() const noexcept {
    return static_cast<bool>(func_storage);
  }

  R operator()(Args... args) const {
    return func_storage.invoke(std::forward<Args>(args)...);
  }

  template<typename T>
  T* target() noexcept {
    return func_storage.template target<T>();
  }

  template<typename T>
  T const* target() const noexcept {
    return func_storage.template target<T>();
  }

  void swap(function& other) noexcept {
    using std::swap;
    swap(func_storage, other.func_storage);
  }

  friend void swap(function& src, function& other) noexcept {
    src.swap(other);
  }

 private:
  storage<R, Args...> func_storage;
};
