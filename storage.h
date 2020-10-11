#pragma once

#include <type_traits>
#include <cstddef>
#include "type_descriptor.h"

template<typename R, typename... Args>
struct storage {
  storage() noexcept
      : buf(),
        desc(empty_type_descriptor<R, Args...>()) {}

  template<typename T>
  storage(T val)
      :  buf(),
         desc(function_traits<T>::template get_type_descriptor<R, Args...>()) {
    function_traits<T>::initialize_storage(*this, std::move(val));
  }

  storage(storage const& other) {
    other.desc->copy(&other, this);
  }

  storage(storage&& other) noexcept {
    other.desc->move(&other, this);
  }

  storage& operator=(storage const& other) {
    if (this != &other) {
      storage(other).swap(*this);
    }
    return *this;
  }

  storage& operator=(storage&& other) noexcept {
    if (this != &other) {
      storage(std::move(other)).swap(*this);
    }
    return *this;
  }

  void swap(storage& other) noexcept {
    using std::swap;
    swap(buf, other.buf);
    swap(desc, other.desc);
  }

  ~storage() {
    desc->destroy(this);
  }

  template<typename T>
  T const* get_static() const noexcept {
    return reinterpret_cast<T const*>(&buf);
  }

  template<typename T>
  T* get_static() noexcept {
    return reinterpret_cast<T*>(&buf);
  }

  template<typename T>
  T* get_dynamic() const noexcept {
    return *reinterpret_cast<T* const*>(&buf);
    // return *static_cast<T* const*>(reinterpret_cast<void const*>(&buf)); // is it better?
  }

  template<typename T>
  T* get_dynamic() noexcept {
    return *reinterpret_cast<T**>(&buf);
    // return *static_cast<T**>(reinterpret_cast<void*>(&buf)); // is it better?
  }

  void set_dynamic(void* value) noexcept {
    reinterpret_cast<void*&>(buf) = value;
  }

  explicit operator bool() const noexcept {
    return desc != empty_type_descriptor<R, Args...>();
  }

  R invoke(Args... args) const {
    return (*desc->invoke)(this, std::forward<Args>(args)...);
  }

  template<typename T>
  T* target() noexcept {
    return const_cast<T*>(static_cast<storage const*>(this)->target<T>());
  }

  template<typename T>
  T const* target() const noexcept {
    if (function_traits<T>::template get_type_descriptor<R, Args...>() != desc) {
      return nullptr;
    }
    if constexpr (fits_small_storage<T>) {
      return get_static<T>();
    } else {
      return get_dynamic<T>();
    }
  }

  template<typename R_, typename... Args_>
  friend type_descriptor<R_, Args_...> const* empty_type_descriptor();

  template<typename U, typename E> friend
  struct function_traits;

 private:
  inplace_buffer buf;
  type_descriptor<R, Args...> const* desc;
};