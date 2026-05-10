#pragma once
#include <cstddef>
#include <limits>
#include <new>
#include <openssl/crypto.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>
template <typename T>

struct SecureAllocator {

  using value_type = T;

  SecureAllocator() noexcept {}

  SecureAllocator(const SecureAllocator &) noexcept {}
  template <typename U>

  SecureAllocator(const SecureAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
      throw std::bad_alloc();
    }

    size_t size = n * sizeof(T);
    void *p = ::operator new(size);

    if (!p)
      throw std::bad_alloc();

    mlock(p, size);

    return static_cast<T *>(p);
  }

  void deallocate(T *p, std::size_t n) {
    if (p) {
      OPENSSL_cleanse(p, n * sizeof(T));

      munlock(p, n);

      ::operator delete(p);
    }
  }

  template <typename U>

  bool operator==(const SecureAllocator<U> &) const noexcept {
    return true;
  }

  template <typename U>

  bool operator!=(const SecureAllocator<U> &) const noexcept {
    return false;
  }
};

using bytes_data = std::vector<uint8_t, SecureAllocator<uint8_t>>;
using secure_string =
    std::basic_string<char, std::char_traits<char>, SecureAllocator<char>>;
