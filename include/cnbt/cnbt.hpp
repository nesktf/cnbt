#ifndef CNBT_HPP
#define CNBT_HPP

#include <cnbt/cnbt.h>

#ifndef CNBT_ASSERT
#include <cassert>
#define CNBT_ASSERT assert
#endif

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#ifndef CNBT_NO_FSTREAM
#include <fstream>
#include <ios>
#endif

namespace cnbt {

using TagType = cnbt_Type;
using EndianMode = cnbt_EndianMode;

template<typename T>
class ArrayView {
public:
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using iterator = T*;
  using const_iterator = const T*;

  constexpr ArrayView() noexcept : _data(nullptr), _size(0) {}

  constexpr ArrayView(T* data, size_t size) noexcept :
      _data(data && size ? data : nullptr), _size(data && size ? size : 0) {}

public:
  constexpr bool empty() const noexcept { return _size == 0; }

  constexpr T* data() const noexcept { return _data; }

  constexpr size_t size() const noexcept { return _size; }

  constexpr T& operator[](size_t index) const {
    CNBT_ASSERT(index < _size && "Index out of range in ArrayView");
    return _data[index];
  }

  constexpr T& at(size_t index) const {
#ifdef __cpp_exceptions
    if (index >= _size) {
      throw std::out_of_range("ArrayView::at index out of range");
    }
#else
    CNBT_ASSERT(index < _size && "Index out of range in ArrayView");
#endif
    return _data[index];
  }

  constexpr T& front() const {
    CNBT_ASSERT(_size > 0 && "ArrayView::front called on empty view");
    return _data[0];
  }

  constexpr T& back() const {
    CNBT_ASSERT(_size > 0 && "ArrayView::back called on empty view");
    return _data[_size - 1];
  }

  constexpr T* begin() const noexcept { return _data; }

  constexpr T* end() const noexcept { return _data + _size; }

  constexpr const T* cbegin() const noexcept { return _data; }

  constexpr const T* cend() const noexcept { return _data + _size; }

private:
  T* _data;
  size_t _size;
};

using std::in_place_t;
constexpr inline in_place_t in_place;

struct unexpect_t {};

constexpr inline unexpect_t unexpect;

class Error final : public std::exception {
public:
  Error() noexcept : _status(CNBT_OK) {}

  explicit Error(cnbt_Status status) noexcept : _status(status) {}

public:
  const char* what() const noexcept override {
    switch (_status) {
      case CNBT_OK:
        return "OK";
      case CNBT_INVALID_DATA:
        return "Invalid data";
      case CNBT_ALLOC_FAILED:
        return "Out of memory";
      case CNBT_EOF:
        return "End of file";
      default:
        return "Unknown error";
    }
  }

  cnbt_Status code() const noexcept { return _status; }

private:
  cnbt_Status _status;
};

template<typename T>
class [[nodiscard]] Expected final {
private:
  static_assert(!std::is_reference_v<T> && !std::is_void_v<T>, "T must be an object type");
  static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "T must not be CV qualified");

public:
  template<typename = std::enable_if_t<std::is_default_constructible_v<T>>>
  Expected() noexcept(std::is_nothrow_default_constructible_v<T>) : _obj(), _status(CNBT_OK) {}

  template<typename... Args>
  Expected(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) :
      _obj(std::forward<Args>(args)...), _status(CNBT_OK) {}

  Expected(const T& obj) noexcept(std::is_nothrow_copy_constructible_v<T>) :
      _obj(obj), _status(CNBT_OK) {}

  Expected(T&& obj) noexcept(std::is_nothrow_move_constructible_v<T>) :
      _obj(std::move(obj)), _status(CNBT_OK) {}

  Expected(unexpect_t, cnbt_Status status) : _dummy(), _status(status) {
    CNBT_ASSERT(_status != CNBT_OK);
  }

  Expected(const Error& err) : _dummy(), _status(err.code()) { CNBT_ASSERT(_status != CNBT_OK); }

  Expected(Expected&& other) noexcept(std::is_nothrow_move_constructible_v<T>) :
      _status(other.code()) {
    if (other.has_value()) {
      new (&_obj) T(std::move(*other));
    }
  }

  Expected(const Expected& other) noexcept(std::is_nothrow_copy_constructible_v<T>) :
      _status(other.code()) {
    if (other.has_value()) {
      new (&_obj) T(*other);
    }
  }

  ~Expected() noexcept {
    if (has_value()) {
      _obj.~T();
    }
  }

  Expected& operator=(const Expected& other) noexcept(std::is_nothrow_copy_constructible_v<T> &&
                                                      std::is_nothrow_copy_assignable_v<T>) {
    if (this == &other) {
      return *this;
    }
    if (has_value() && other.has_value()) {
      _obj = other._obj;
    } else if (has_value() && !other.has_value()) {
      _obj.~T();
      _status = other._status;
    } else if (!has_value() && other.has_value()) {
      new (&_obj) T(other._obj);
      _status = CNBT_OK;
    } else {
      _status = other._status;
    }
    return *this;
  }

  Expected& operator=(Expected&& other) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                 std::is_nothrow_move_assignable_v<T>) {
    if (this == &other) {
      return *this;
    }
    if (has_value() && other.has_value()) {
      _obj = std::move(other._obj);
    } else if (has_value() && !other.has_value()) {
      _obj.~T();
      _status = other._status;
    } else if (!has_value() && other.has_value()) {
      new (&_obj) T(std::move(other._obj));
      _status = CNBT_OK;
    } else {
      _status = other._status;
    }
    return *this;
  }

  Expected& operator=(const T& obj) noexcept(std::is_nothrow_copy_constructible_v<T> &&
                                             std::is_nothrow_copy_assignable_v<T>) {
    if (has_value()) {
      _obj = obj;
    } else {
      new (&_obj) T(obj);
      _status = CNBT_OK;
    }
    return *this;
  }

  Expected& operator=(T&& obj) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                        std::is_nothrow_move_assignable_v<T>) {
    if (has_value()) {
      _obj = std::move(obj);
    } else {
      new (&_obj) T(std::move(obj));
      _status = CNBT_OK;
    }
    return *this;
  }

  Expected& operator=(const Error& err) noexcept {
    if (has_value()) {
      _obj.~T();
    }
    _status = err.code();
    CNBT_ASSERT(_status != CNBT_OK);
    return *this;
  }

public:
  cnbt_Status code() const noexcept { return _status; }

  bool has_value() const noexcept { return _status == CNBT_OK; }

  bool has_error() const noexcept { return !has_value(); }

  explicit operator bool() const noexcept { return has_value(); }

  Error error() const noexcept { return Error(_status); }

public:
  T& operator*() & {
    CNBT_ASSERT(has_value() && "Valueless Expected");
    return _obj;
  }

  const T& operator*() const& {
    CNBT_ASSERT(has_value() && "Valueless Expected");
    return _obj;
  }

  T&& operator*() && {
    CNBT_ASSERT(has_value() && "Valueless Expected");
    return std::move(_obj);
  }

  const T&& operator*() const&& {
    CNBT_ASSERT(has_value() && "Valueless Expected");
    return std::move(_obj);
  }

  T* operator->() {
    CNBT_ASSERT(has_value() && "Valueless Expected");
    return &_obj;
  }

  const T* operator->() const {
    CNBT_ASSERT(has_value() && "Valueless Expected");
    return &_obj;
  }

  T& value() & {
#ifdef __cpp_exceptions
    if (!has_value()) {
      throw Error(_status);
    }
#else
    CNBT_ASSERT(has_value() && "Valueless Expected");
#endif
    return _obj;
  }

  const T& value() const& {
#ifdef __cpp_exceptions
    if (!has_value()) {
      throw Error(_status);
    }
#else
    CNBT_ASSERT(has_value() && "Valueless Expected");
#endif
    return _obj;
  }

  T&& value() && {
#ifdef __cpp_exceptions
    if (!has_value()) {
      throw Error(_status);
    }
#else
    CNBT_ASSERT(has_value() && "Valueless Expected");
#endif
    return std::move(_obj);
  }

  const T&& value() const&& {
#ifdef __cpp_exceptions
    if (!has_value()) {
      throw Error(_status);
    }
#else
    CNBT_ASSERT(has_value() && "Valueless Expected");
#endif
    return std::move(_obj);
  }

  template<typename U>
  T value_or(U&& def) const& {
    return has_value() ? **this : static_cast<T>(std::forward<U>(def));
  }

  template<typename U>
  T value_or(U&& def) && {
    return has_value() ? std::move(**this) : static_cast<T>(std::forward<U>(def));
  }

public:
  template<typename F>
  auto and_then(F&& f) & -> decltype(std::forward<F>(f)(std::declval<T&>())) {
    using Ret = decltype(std::forward<F>(f)(std::declval<T&>()));
    if (has_value()) {
      return std::forward<F>(f)(**this);
    } else {
      return Ret(unexpect, _status);
    }
  }

  template<typename F>
  auto and_then(F&& f) && -> decltype(std::forward<F>(f)(std::declval<T&&>())) {
    using Ret = decltype(std::forward<F>(f)(std::declval<T&&>()));
    if (has_value()) {
      return std::forward<F>(f)(std::move(**this));
    } else {
      return Ret(unexpect, _status);
    }
  }

  template<typename F>
  auto and_then(F&& f) const& -> decltype(std::forward<F>(f)(std::declval<const T&>())) {
    using Ret = decltype(std::forward<F>(f)(std::declval<const T&>()));
    if (has_value()) {
      return std::forward<F>(f)(**this);
    } else {
      return Ret(unexpect, _status);
    }
  }

  template<typename F>
  auto and_then(F&& f) const&& -> decltype(std::forward<F>(f)(std::declval<const T&&>())) {
    using Ret = decltype(std::forward<F>(f)(std::declval<const T&&>()));
    if (has_value()) {
      return std::forward<F>(f)(std::move(**this));
    } else {
      return Ret(unexpect, _status);
    }
  }

  template<typename F>
  auto transform(
    F&& f) & -> Expected<std::remove_cv_t<decltype(std::forward<F>(f)(std::declval<T&>()))>> {
    using RetVal = std::remove_cv_t<decltype(std::forward<F>(f)(std::declval<T&>()))>;
    using Ret = Expected<RetVal>;
    if (has_value()) {
      return Ret(in_place, std::forward<F>(f)(**this));
    } else {
      return Ret(unexpect, _status);
    }
  }

  template<typename F>
  auto transform(
    F&& f) && -> Expected<std::remove_cv_t<decltype(std::forward<F>(f)(std::declval<T&&>()))>> {
    using RetVal = std::remove_cv_t<decltype(std::forward<F>(f)(std::declval<T&&>()))>;
    using Ret = Expected<RetVal>;
    if (has_value()) {
      return Ret(in_place, std::forward<F>(f)(std::move(**this)));
    } else {
      return Ret(unexpect, _status);
    }
  }

  template<typename F>
  auto transform(F&& f)
    const& -> Expected<std::remove_cv_t<decltype(std::forward<F>(f)(std::declval<const T&>()))>> {
    using RetVal = std::remove_cv_t<decltype(std::forward<F>(f)(std::declval<const T&>()))>;
    using Ret = Expected<RetVal>;
    if (has_value()) {
      return Ret(in_place, std::forward<F>(f)(**this));
    } else {
      return Ret(unexpect, _status);
    }
  }

  template<typename F>
  auto transform(F&& f) const&& -> Expected<
    std::remove_cv_t<decltype(std::forward<F>(f)(std::declval<const T&&>()))>> {
    using RetVal = std::remove_cv_t<decltype(std::forward<F>(f)(std::declval<const T&&>()))>;
    using Ret = Expected<RetVal>;
    if (has_value()) {
      return Ret(in_place, std::forward<F>(f)(std::move(**this)));
    } else {
      return Ret(unexpect, _status);
    }
  }

  template<typename F>
  auto or_else(F&& f) & {
    using Ret = decltype(_invoke_error_fn(std::forward<F>(f), _status));
    if constexpr (std::is_void_v<Ret>) {
      if (!has_value()) {
        _invoke_error_fn(std::forward<F>(f), _status);
      }
      return *this;
    } else if constexpr (std::is_same_v<std::decay_t<Ret>, Expected<T>>) {
      if (has_value()) {
        return *this;
      } else {
        return _invoke_error_fn(std::forward<F>(f), _status);
      }
    } else {
      if (has_value()) {
        return Expected<T>(in_place, **this);
      } else {
        return Expected<T>(in_place, _invoke_error_fn(std::forward<F>(f), _status));
      }
    }
  }

  template<typename F>
  auto or_else(F&& f) && {
    using Ret = decltype(_invoke_error_fn(std::forward<F>(f), _status));
    if constexpr (std::is_void_v<Ret>) {
      if (!has_value()) {
        _invoke_error_fn(std::forward<F>(f), _status);
      }
      return std::move(*this);
    } else if constexpr (std::is_same_v<std::decay_t<Ret>, Expected<T>>) {
      if (has_value()) {
        return std::move(*this);
      } else {
        return _invoke_error_fn(std::forward<F>(f), _status);
      }
    } else {
      if (has_value()) {
        return Expected<T>(in_place, std::move(**this));
      } else {
        return Expected<T>(in_place, _invoke_error_fn(std::forward<F>(f), _status));
      }
    }
  }

  template<typename F>
  auto or_else(F&& f) const& {
    using Ret = decltype(_invoke_error_fn(std::forward<F>(f), _status));
    if constexpr (std::is_void_v<Ret>) {
      if (!has_value()) {
        _invoke_error_fn(std::forward<F>(f), _status);
      }
      return *this;
    } else if constexpr (std::is_same_v<std::decay_t<Ret>, Expected<T>>) {
      if (has_value()) {
        return *this;
      } else {
        return _invoke_error_fn(std::forward<F>(f), _status);
      }
    } else {
      if (has_value()) {
        return Expected<T>(in_place, **this);
      } else {
        return Expected<T>(in_place, _invoke_error_fn(std::forward<F>(f), _status));
      }
    }
  }

  template<typename F>
  auto or_else(F&& f) const&& {
    using Ret = decltype(_invoke_error_fn(std::forward<F>(f), _status));
    if constexpr (std::is_void_v<Ret>) {
      if (!has_value()) {
        _invoke_error_fn(std::forward<F>(f), _status);
      }
      return std::move(*this);
    } else if constexpr (std::is_same_v<std::decay_t<Ret>, Expected<T>>) {
      if (has_value()) {
        return std::move(*this);
      } else {
        return _invoke_error_fn(std::forward<F>(f), _status);
      }
    } else {
      if (has_value()) {
        return Expected<T>(in_place, std::move(**this));
      } else {
        return Expected<T>(in_place, _invoke_error_fn(std::forward<F>(f), _status));
      }
    }
  }

private:
  template<typename F>
  static decltype(auto) _invoke_error_fn(F&& f, cnbt_Status status) {
    if constexpr (std::is_invocable_v<F, cnbt_Status>) {
      return std::forward<F>(f)(status);
    } else if constexpr (std::is_invocable_v<F, Error>) {
      return std::forward<F>(f)(Error(status));
    } else if constexpr (std::is_invocable_v<F, const Error&>) {
      const Error err(status);
      return std::forward<F>(f)(err);
    } else {
      return std::forward<F>(f)();
    }
  }

private:
  union {
    char _dummy;
    T _obj;
  };

  cnbt_Status _status;
};

template<>
class [[nodiscard]] Expected<void> final {
public:
  constexpr Expected() noexcept : _status(CNBT_OK) {}

  constexpr Expected(in_place_t) noexcept : _status(CNBT_OK) {}

  constexpr Expected(unexpect_t, cnbt_Status status) noexcept : _status(status) {
    CNBT_ASSERT(_status != CNBT_OK);
  }

  Expected(const Error& err) noexcept : _status(err.code()) { CNBT_ASSERT(_status != CNBT_OK); }

  constexpr cnbt_Status code() const noexcept { return _status; }

  constexpr bool has_value() const noexcept { return _status == CNBT_OK; }

  constexpr bool has_error() const noexcept { return !has_value(); }

  constexpr explicit operator bool() const noexcept { return has_value(); }

  Error error() const noexcept { return Error(_status); }

  void value() const {
#ifdef __cpp_exceptions
    if (!has_value()) {
      throw Error(_status);
    }
#else
    CNBT_ASSERT(has_value() && "Valueless Expected");
#endif
  }

  template<typename F>
  auto and_then(F&& f) const {
    using Ret = decltype(std::forward<F>(f)());
    if (has_value()) {
      return std::forward<F>(f)();
    } else {
      return Ret(unexpect, _status);
    }
  }

  template<typename F>
  auto transform(F&& f) const {
    using RetVal = std::remove_cv_t<decltype(std::forward<F>(f)())>;
    if constexpr (std::is_void_v<RetVal>) {
      if (has_value()) {
        std::forward<F>(f)();
        return Expected<void>();
      } else {
        return Expected<void>(unexpect, _status);
      }
    } else {
      if (has_value()) {
        return Expected<RetVal>(in_place, std::forward<F>(f)());
      } else {
        return Expected<RetVal>(unexpect, _status);
      }
    }
  }

  template<typename F>
  auto or_else(F&& f) const {
    using Ret = decltype(_invoke_error_fn(std::forward<F>(f), _status));
    if constexpr (std::is_void_v<Ret>) {
      if (!has_value()) {
        _invoke_error_fn(std::forward<F>(f), _status);
      }
      return *this;
    } else if constexpr (std::is_same_v<std::decay_t<Ret>, Expected<void>>) {
      if (has_value()) {
        return *this;
      } else {
        return _invoke_error_fn(std::forward<F>(f), _status);
      }
    } else {
      if (!has_value()) {
        _invoke_error_fn(std::forward<F>(f), _status);
      }
      return *this;
    }
  }

private:
  template<typename F>
  static decltype(auto) _invoke_error_fn(F&& f, cnbt_Status status) {
    if constexpr (std::is_invocable_v<F, cnbt_Status>) {
      return std::forward<F>(f)(status);
    } else if constexpr (std::is_invocable_v<F, Error>) {
      return std::forward<F>(f)(Error(status));
    } else if constexpr (std::is_invocable_v<F, const Error&>) {
      const Error err(status);
      return std::forward<F>(f)(err);
    } else {
      return std::forward<F>(f)();
    }
  }

private:
  cnbt_Status _status;
};

namespace detail {

struct MemBufferReader {
  const uint8_t* ptr;
  size_t size;
  size_t pos;
};

inline cnbt_IoFunc make_file_cbs() noexcept {
  cnbt_IoFunc cbs{};
  cbs.read = reinterpret_cast<PFN_cnbt_read_func>(fread);
  cbs.write = reinterpret_cast<PFN_cnbt_write_func>(fwrite);
  cbs.seek = reinterpret_cast<PFN_cnbt_seek_func>(fseek);
  cbs.tell = reinterpret_cast<PFN_cnbt_tell_func>(ftell);
  cbs.eof = reinterpret_cast<PFN_cnbt_eof_func>(feof);
  return cbs;
}

inline cnbt_IoFunc make_z_cbs() noexcept {
  cnbt_IoFunc cbs{};
  cbs.read = reinterpret_cast<PFN_cnbt_read_func>(cnbt_zread);
  cbs.write = reinterpret_cast<PFN_cnbt_write_func>(cnbt_zwrite);
  cbs.seek = reinterpret_cast<PFN_cnbt_seek_func>(cnbt_zseek);
  cbs.tell = reinterpret_cast<PFN_cnbt_tell_func>(cnbt_ztell);
  cbs.eof = nullptr;
  return cbs;
}

inline cnbt_IoFunc make_mem_reader_cbs() noexcept {
  cnbt_IoFunc cbs{};
  cbs.read = +[](void* buff, size_t sz, size_t nmemb, void* src) -> size_t {
    auto* mem = static_cast<MemBufferReader*>(src);
    if (!mem || !buff || sz == 0)
      return 0;
    size_t to_read = sz * nmemb;
    size_t avail = (mem->pos < mem->size) ? (mem->size - mem->pos) : 0;
    if (to_read > avail) {
      to_read = avail - (avail % sz);
    }
    std::memcpy(buff, mem->ptr + mem->pos, to_read);
    mem->pos += to_read;
    return to_read / sz;
  };
  cbs.seek = +[](void* src, long offset, int origin) -> int {
    auto* mem = static_cast<MemBufferReader*>(src);
    if (!mem)
      return -1;
    ptrdiff_t new_pos = 0;
    if (origin == CNBT_SEEK_SET)
      new_pos = offset;
    else if (origin == CNBT_SEEK_CUR)
      new_pos = static_cast<ptrdiff_t>(mem->pos) + offset;
    else if (origin == CNBT_SEEK_END)
      new_pos = static_cast<ptrdiff_t>(mem->size) + offset;
    else
      return -1;
    if (new_pos < 0 || static_cast<size_t>(new_pos) > mem->size)
      return -1;
    mem->pos = static_cast<size_t>(new_pos);
    return 0;
  };
  cbs.tell = +[](void* src) -> long {
    auto* mem = static_cast<MemBufferReader*>(src);
    return mem ? static_cast<long>(mem->pos) : -1;
  };
  cbs.eof = +[](void* src) -> int {
    auto* mem = static_cast<MemBufferReader*>(src);
    return (mem && mem->pos >= mem->size) ? 1 : 0;
  };
  return cbs;
}

#ifndef CNBT_NO_FSTREAM
inline cnbt_IoFunc make_istream_cbs() noexcept {
  cnbt_IoFunc cbs{};
  cbs.read = +[](void* buff, size_t sz, size_t nmemb, void* src) -> size_t {
    if (!src || !buff || sz == 0)
      return 0;
    auto& in = *static_cast<std::istream*>(src);
    if (!in.good())
      return 0;
    in.read(reinterpret_cast<char*>(buff), static_cast<std::streamsize>(sz * nmemb));
    return static_cast<size_t>(in.gcount()) / sz;
  };
  cbs.seek = +[](void* src, long offset, int origin) -> int {
    if (!src)
      return -1;
    auto& in = *static_cast<std::istream*>(src);
    std::ios_base::seekdir dir;
    if (origin == CNBT_SEEK_SET)
      dir = std::ios_base::beg;
    else if (origin == CNBT_SEEK_CUR)
      dir = std::ios_base::cur;
    else if (origin == CNBT_SEEK_END)
      dir = std::ios_base::end;
    else
      return -1;
    in.clear();
    in.seekg(offset, dir);
    return in.fail() ? -1 : 0;
  };
  cbs.tell = +[](void* src) -> long {
    if (!src)
      return -1;
    auto& in = *static_cast<std::istream*>(src);
    return static_cast<long>(in.tellg());
  };
  cbs.eof = +[](void* src) -> int {
    if (!src)
      return 1;
    auto& in = *static_cast<std::istream*>(src);
    return in.eof() ? 1 : 0;
  };
  return cbs;
}

inline cnbt_IoFunc make_ostream_cbs() noexcept {
  cnbt_IoFunc cbs{};
  cbs.write = +[](const void* buff, size_t sz, size_t nmemb, void* src) -> size_t {
    if (!src || !buff || sz == 0)
      return 0;
    auto& out = *static_cast<std::ostream*>(src);
    if (!out.good())
      return 0;
    out.write(reinterpret_cast<const char*>(buff), static_cast<std::streamsize>(sz * nmemb));
    return out.good() ? nmemb : 0;
  };
  cbs.seek = +[](void* src, long offset, int origin) -> int {
    if (!src)
      return -1;
    auto& out = *static_cast<std::ostream*>(src);
    std::ios_base::seekdir dir;
    if (origin == CNBT_SEEK_SET)
      dir = std::ios_base::beg;
    else if (origin == CNBT_SEEK_CUR)
      dir = std::ios_base::cur;
    else if (origin == CNBT_SEEK_END)
      dir = std::ios_base::end;
    else
      return -1;
    out.seekp(offset, dir);
    return out.fail() ? -1 : 0;
  };
  cbs.tell = +[](void* src) -> long {
    if (!src)
      return -1;
    auto& out = *static_cast<std::ostream*>(src);
    return static_cast<long>(out.tellp());
  };
  cbs.eof = +[](void*) -> int {
    return 0;
  };
  return cbs;
}
#endif

constexpr cnbt_ZWriteArgs DEFAULT_Z_WRITE_ARGS = {
  CNBT_ZDEFAULT_LEVEL,
  CNBT_ZGZIP_WINDOW_BITS,
  CNBT_ZDEFAULT_MEM_LEVEL,
  CNBT_ZDEFAULT_STRATEGY,
};

} // namespace detail

class TagCompound;
class TagList;

class Tag {
protected:
  enum class InitMode {
    End,
    Compound,
    List,
  };

  explicit Tag(InitMode mode) noexcept {
    if (mode == InitMode::Compound) {
      cnbt_make_compound(&_tag);
    } else if (mode == InitMode::List) {
      cnbt_make_list(&_tag);
    } else {
      cnbt_make_end(&_tag);
    }
  }

public:
  static std::string_view type_name(TagType t) noexcept {
    const char* name = cnbt_tag_name(t);
    return name ? std::string_view(name) : std::string_view();
  }

public:
  Tag() noexcept { cnbt_make_end(&_tag); }

  explicit Tag(cnbt_Tag raw) noexcept : _tag(raw) {}

  explicit Tag(int8_t val) noexcept { cnbt_make_byte(&_tag, val); }

  explicit Tag(int16_t val) noexcept { cnbt_make_short(&_tag, val); }

  explicit Tag(int32_t val) noexcept { cnbt_make_int(&_tag, val); }

  explicit Tag(int64_t val) noexcept { cnbt_make_long(&_tag, val); }

  explicit Tag(float val) noexcept { cnbt_make_float(&_tag, val); }

  explicit Tag(double val) noexcept { cnbt_make_double(&_tag, val); }

  explicit Tag(std::string_view val) {
    auto status = cnbt_make_strn(&_tag, val.data(), val.size());
    if (status != CNBT_OK) {
#ifdef __cpp_exceptions
      throw Error(status);
#else
      CNBT_ASSERT(status == CNBT_OK && "cnbt_make_strn failed in Tag constructor");
#endif
    }
  }

  explicit Tag(ArrayView<const int8_t> val) {
    auto status = cnbt_make_byte_array(&_tag, val.data(), val.size());
    if (status != CNBT_OK) {
#ifdef __cpp_exceptions
      throw Error(status);
#else
      CNBT_ASSERT(status == CNBT_OK && "cnbt_make_byte_array failed in Tag constructor");
#endif
    }
  }

  explicit Tag(ArrayView<const int32_t> val) {
    auto status = cnbt_make_int_array(&_tag, val.data(), val.size());
    if (status != CNBT_OK) {
#ifdef __cpp_exceptions
      throw Error(status);
#else
      CNBT_ASSERT(status == CNBT_OK && "cnbt_make_int_array failed in Tag constructor");
#endif
    }
  }

  explicit Tag(ArrayView<const int64_t> val) {
    auto status = cnbt_make_long_array(&_tag, val.data(), val.size());
    if (status != CNBT_OK) {
#ifdef __cpp_exceptions
      throw Error(status);
#else
      CNBT_ASSERT(status == CNBT_OK && "cnbt_make_long_array failed in Tag constructor");
#endif
    }
  }

  Tag(const Tag& other) {
    auto status = cnbt_clone(&_tag, &other._tag);
    if (status != CNBT_OK) {
      cnbt_make_end(&_tag);
#ifdef __cpp_exceptions
      throw Error(status);
#else
      CNBT_ASSERT(status == CNBT_OK && "cnbt_clone failed in Tag copy");
#endif
    }
  }

  Tag(Tag&& other) noexcept : _tag(other._tag) { cnbt_make_end(&other._tag); }

  ~Tag() noexcept { cnbt_free(&_tag); }

  Tag& operator=(const Tag& other) {
    if (this == &other)
      return *this;
    cnbt_Tag tmp;
    auto status = cnbt_clone(&tmp, &other._tag);
    if (status != CNBT_OK) {
#ifdef __cpp_exceptions
      throw Error(status);
#else
      CNBT_ASSERT(status == CNBT_OK && "cnbt_clone failed in Tag copy assignment");
#endif
    } else {
      cnbt_free(&_tag);
      _tag = tmp;
    }
    return *this;
  }

  Tag& operator=(Tag&& other) noexcept {
    if (this == &other)
      return *this;
    cnbt_free(&_tag);
    _tag = other._tag;
    cnbt_make_end(&other._tag);
    return *this;
  }

public:
  Expected<Tag> clone() const noexcept {
    Tag out;
    auto status = cnbt_clone(&out._tag, &_tag);
    if (status != CNBT_OK) {
      return {unexpect, status};
    }
    return out;
  }

  cnbt_Tag* handle() noexcept { return &_tag; }

  const cnbt_Tag* handle() const noexcept { return &_tag; }

  cnbt_Tag release() noexcept {
    cnbt_Tag out = _tag;
    cnbt_make_end(&_tag);
    return out;
  }

  TagType type() const noexcept { return cnbt_get_type(&_tag); }

  std::string_view type_name() const noexcept { return type_name(type()); }

  bool is_end() const noexcept { return type() == CNBT_TYPE_END; }

  bool is_byte() const noexcept { return type() == CNBT_TYPE_BYTE; }

  bool is_short() const noexcept { return type() == CNBT_TYPE_SHORT; }

  bool is_int() const noexcept { return type() == CNBT_TYPE_INT; }

  bool is_long() const noexcept { return type() == CNBT_TYPE_LONG; }

  bool is_float() const noexcept { return type() == CNBT_TYPE_FLOAT; }

  bool is_double() const noexcept { return type() == CNBT_TYPE_DOUBLE; }

  bool is_string() const noexcept { return type() == CNBT_TYPE_STRING; }

  bool is_byte_array() const noexcept { return type() == CNBT_TYPE_BYTE_ARRAY; }

  bool is_int_array() const noexcept { return type() == CNBT_TYPE_INT_ARRAY; }

  bool is_long_array() const noexcept { return type() == CNBT_TYPE_LONG_ARRAY; }

  bool is_list() const noexcept { return type() == CNBT_TYPE_LIST; }

  bool is_compound() const noexcept { return type() == CNBT_TYPE_COMPOUND; }

public:
  Expected<int8_t> as_byte() const noexcept {
    if (type() != CNBT_TYPE_BYTE)
      return {unexpect, CNBT_INVALID_DATA};
    return cnbt_get_byte(&_tag);
  }

  Expected<int16_t> as_short() const noexcept {
    if (type() != CNBT_TYPE_SHORT)
      return {unexpect, CNBT_INVALID_DATA};
    return cnbt_get_short(&_tag);
  }

  Expected<int32_t> as_int() const noexcept {
    if (type() != CNBT_TYPE_INT)
      return {unexpect, CNBT_INVALID_DATA};
    return cnbt_get_int(&_tag);
  }

  Expected<int64_t> as_long() const noexcept {
    if (type() != CNBT_TYPE_LONG)
      return {unexpect, CNBT_INVALID_DATA};
    return cnbt_get_long(&_tag);
  }

  Expected<float> as_float() const noexcept {
    if (type() != CNBT_TYPE_FLOAT)
      return {unexpect, CNBT_INVALID_DATA};
    return cnbt_get_float(&_tag);
  }

  Expected<double> as_double() const noexcept {
    if (type() != CNBT_TYPE_DOUBLE)
      return {unexpect, CNBT_INVALID_DATA};
    return cnbt_get_double(&_tag);
  }

  Expected<std::string_view> as_string() const noexcept {
    if (type() != CNBT_TYPE_STRING)
      return {unexpect, CNBT_INVALID_DATA};
    auto* data = cnbt_str_data(&_tag);
    auto len = cnbt_str_len(&_tag);
    return data ? std::string_view(data, len) : std::string_view();
  }

  Expected<ArrayView<const int8_t>> as_byte_array() const noexcept {
    if (type() != CNBT_TYPE_BYTE_ARRAY)
      return {unexpect, CNBT_INVALID_DATA};
    auto* data = cnbt_byte_array_data(&_tag);
    auto len = cnbt_byte_array_len(&_tag);
    return ArrayView<const int8_t>(data, len);
  }

  Expected<ArrayView<const int32_t>> as_int_array() const noexcept {
    if (type() != CNBT_TYPE_INT_ARRAY)
      return {unexpect, CNBT_INVALID_DATA};
    auto* data = cnbt_int_array_data(&_tag);
    auto len = cnbt_int_array_len(&_tag);
    return ArrayView<const int32_t>(data, len);
  }

  Expected<ArrayView<const int64_t>> as_long_array() const noexcept {
    if (type() != CNBT_TYPE_LONG_ARRAY)
      return {unexpect, CNBT_INVALID_DATA};
    auto* data = cnbt_long_array_data(&_tag);
    auto len = cnbt_long_array_len(&_tag);
    return ArrayView<const int64_t>(data, len);
  }

  inline Expected<TagCompound*> as_compound() noexcept;
  inline Expected<const TagCompound*> as_compound() const noexcept;

  inline Expected<TagList*> as_list() noexcept;
  inline Expected<const TagList*> as_list() const noexcept;

  Expected<int8_t> byte() const noexcept { return as_byte(); }

  Expected<int16_t> int16() const noexcept { return as_short(); }

  Expected<int16_t> short_val() const noexcept { return as_short(); }

  Expected<int32_t> integer() const noexcept { return as_int(); }

  Expected<int32_t> int32() const noexcept { return as_int(); }

  Expected<int64_t> int64() const noexcept { return as_long(); }

  Expected<int64_t> long_val() const noexcept { return as_long(); }

  Expected<float> float32() const noexcept { return as_float(); }

  Expected<float> float_val() const noexcept { return as_float(); }

  Expected<double> float64() const noexcept { return as_double(); }

  Expected<double> double_val() const noexcept { return as_double(); }

  Expected<std::string_view> string() const noexcept { return as_string(); }

  Expected<ArrayView<const int8_t>> byte_array() const noexcept { return as_byte_array(); }

  Expected<ArrayView<const int32_t>> int_array() const noexcept { return as_int_array(); }

  Expected<ArrayView<const int64_t>> long_array() const noexcept { return as_long_array(); }

  Expected<TagCompound*> compound() noexcept { return as_compound(); }

  Expected<const TagCompound*> compound() const noexcept { return as_compound(); }

  Expected<TagList*> list() noexcept { return as_list(); }

  Expected<const TagList*> list() const noexcept { return as_list(); }

public:
  static Expected<Tag> read_file(const char* path, EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    FILE* file = fopen(path, "rb");
    if (!file)
      return {unexpect, CNBT_INVALID_DATA};
    auto cbs = detail::make_file_cbs();
    Tag tag;
    auto status = cnbt_read(tag.handle(), mode, file, &cbs);
    fclose(file);
    if (status != CNBT_OK)
      return {unexpect, status};
    return tag;
  }

  static Expected<Tag> read_compressed_file(const char* path,
                                            EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    FILE* file = fopen(path, "rb");
    if (!file)
      return {unexpect, CNBT_INVALID_DATA};
    auto file_cbs = detail::make_file_cbs();
    cnbt_ZStream zstr;
    auto status = cnbt_zopen_read(&zstr, CNBT_ZAUTO_WINDOW_BITS, file, &file_cbs);
    if (status != CNBT_OK) {
      fclose(file);
      return {unexpect, status};
    }
    auto z_cbs = detail::make_z_cbs();
    Tag tag;
    status = cnbt_read(tag.handle(), mode, zstr, &z_cbs);
    cnbt_zclose(zstr);
    fclose(file);
    if (status != CNBT_OK)
      return {unexpect, status};
    return tag;
  }

  static Expected<Tag> read_memory(ArrayView<const uint8_t> buf,
                                   EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    detail::MemBufferReader reader{buf.data(), buf.size(), 0};
    auto cbs = detail::make_mem_reader_cbs();
    Tag tag;
    auto status = cnbt_read(tag.handle(), mode, &reader, &cbs);
    if (status != CNBT_OK)
      return {unexpect, status};
    return tag;
  }

  static Expected<Tag> read_compressed_memory(ArrayView<const uint8_t> buf,
                                              EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    detail::MemBufferReader reader{buf.data(), buf.size(), 0};
    auto mem_cbs = detail::make_mem_reader_cbs();
    cnbt_ZStream zstr;
    auto status = cnbt_zopen_read(&zstr, CNBT_ZAUTO_WINDOW_BITS, &reader, &mem_cbs);
    if (status != CNBT_OK)
      return {unexpect, status};
    auto z_cbs = detail::make_z_cbs();
    Tag tag;
    status = cnbt_read(tag.handle(), mode, zstr, &z_cbs);
    cnbt_zclose(zstr);
    if (status != CNBT_OK)
      return {unexpect, status};
    return tag;
  }

#ifndef CNBT_NO_FSTREAM
  static Expected<Tag> read_stream(std::istream& in, EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    auto cbs = detail::make_istream_cbs();
    Tag tag;
    auto status = cnbt_read(tag.handle(), mode, &in, &cbs);
    if (status != CNBT_OK)
      return {unexpect, status};
    return tag;
  }

  static Expected<Tag> read_compressed_stream(std::istream& in,
                                              EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    auto stream_cbs = detail::make_istream_cbs();
    cnbt_ZStream zstr;
    auto status = cnbt_zopen_read(&zstr, CNBT_ZAUTO_WINDOW_BITS, &in, &stream_cbs);
    if (status != CNBT_OK)
      return {unexpect, status};
    auto z_cbs = detail::make_z_cbs();
    Tag tag;
    status = cnbt_read(tag.handle(), mode, zstr, &z_cbs);
    cnbt_zclose(zstr);
    if (status != CNBT_OK)
      return {unexpect, status};
    return tag;
  }
#endif

  Expected<void> write_file(const char* path, EndianMode mode = CNBT_BIG_ENDIAN) const noexcept {
    FILE* file = fopen(path, "wb");
    if (!file)
      return {unexpect, CNBT_INVALID_DATA};
    auto cbs = detail::make_file_cbs();
    auto status = cnbt_write(&_tag, mode, file, &cbs);
    fclose(file);
    if (status != CNBT_OK)
      return {unexpect, status};
    return {};
  }

  Expected<void> write_compressed_file(const char* path,
                                       EndianMode mode = CNBT_BIG_ENDIAN) const noexcept {
    FILE* file = fopen(path, "wb");
    if (!file)
      return {unexpect, CNBT_INVALID_DATA};
    auto file_cbs = detail::make_file_cbs();
    cnbt_ZStream zstr;
    auto status = cnbt_zopen_write(&zstr, &detail::DEFAULT_Z_WRITE_ARGS, file, &file_cbs);
    if (status != CNBT_OK) {
      fclose(file);
      return {unexpect, status};
    }
    auto z_cbs = detail::make_z_cbs();
    status = cnbt_write(&_tag, mode, zstr, &z_cbs);
    cnbt_zclose(zstr);
    fclose(file);
    if (status != CNBT_OK)
      return {unexpect, status};
    return {};
  }

  Expected<void> write_pretty_file(const char* path) const noexcept {
    FILE* file = fopen(path, "w");
    if (!file)
      return {unexpect, CNBT_INVALID_DATA};
    auto cbs = detail::make_file_cbs();
    auto status = cnbt_write_pretty(&_tag, file, &cbs);
    fclose(file);
    if (status != CNBT_OK)
      return {unexpect, status};
    return {};
  }

#ifndef CNBT_NO_FSTREAM
  Expected<void> write_stream(std::ostream& out,
                              EndianMode mode = CNBT_BIG_ENDIAN) const noexcept {
    auto cbs = detail::make_ostream_cbs();
    auto status = cnbt_write(&_tag, mode, &out, &cbs);
    if (status != CNBT_OK)
      return {unexpect, status};
    return {};
  }

  Expected<void> write_compressed_stream(std::ostream& out,
                                         EndianMode mode = CNBT_BIG_ENDIAN) const noexcept {
    auto stream_cbs = detail::make_ostream_cbs();
    cnbt_ZStream zstr;
    auto status = cnbt_zopen_write(&zstr, &detail::DEFAULT_Z_WRITE_ARGS, &out, &stream_cbs);
    if (status != CNBT_OK)
      return {unexpect, status};
    auto z_cbs = detail::make_z_cbs();
    status = cnbt_write(&_tag, mode, zstr, &z_cbs);
    cnbt_zclose(zstr);
    if (status != CNBT_OK)
      return {unexpect, status};
    return {};
  }

  Expected<void> write_pretty_stream(std::ostream& out) const noexcept {
    auto cbs = detail::make_ostream_cbs();
    auto status = cnbt_write_pretty(&_tag, &out, &cbs);
    if (status != CNBT_OK)
      return {unexpect, status};
    return {};
  }
#endif

protected:
  cnbt_Tag _tag;
};

struct KeyTagView {
  std::string_view key;
  Tag& tag;
};

struct ConstKeyTagView {
  std::string_view key;
  const Tag& tag;
};

class TagCompound : public Tag {
public:
  class Iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = KeyTagView;
    using difference_type = ptrdiff_t;
    using pointer = void;
    using reference = KeyTagView;

    Iterator() noexcept : _ptr(nullptr) {}

    explicit Iterator(cnbt_KeyTag* ptr) noexcept : _ptr(ptr) {}

    KeyTagView operator*() const noexcept {
      return KeyTagView{_ptr->key ? std::string_view(_ptr->key) : std::string_view{},
                        *reinterpret_cast<Tag*>(&_ptr->value)};
    }

    Iterator& operator++() noexcept {
      ++_ptr;
      return *this;
    }

    Iterator operator++(int) noexcept {
      auto tmp = *this;
      ++_ptr;
      return tmp;
    }

    bool operator==(const Iterator& other) const noexcept { return _ptr == other._ptr; }

    bool operator!=(const Iterator& other) const noexcept { return _ptr != other._ptr; }

  private:
    cnbt_KeyTag* _ptr;
  };

  class ConstIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = ConstKeyTagView;
    using difference_type = ptrdiff_t;
    using pointer = void;
    using reference = ConstKeyTagView;

    ConstIterator() noexcept : _ptr(nullptr) {}

    explicit ConstIterator(const cnbt_KeyTag* ptr) noexcept : _ptr(ptr) {}

    ConstKeyTagView operator*() const noexcept {
      return ConstKeyTagView{_ptr->key ? std::string_view(_ptr->key) : std::string_view{},
                             *reinterpret_cast<const Tag*>(&_ptr->value)};
    }

    ConstIterator& operator++() noexcept {
      ++_ptr;
      return *this;
    }

    ConstIterator operator++(int) noexcept {
      auto tmp = *this;
      ++_ptr;
      return tmp;
    }

    bool operator==(const ConstIterator& other) const noexcept { return _ptr == other._ptr; }

    bool operator!=(const ConstIterator& other) const noexcept { return _ptr != other._ptr; }

  private:
    const cnbt_KeyTag* _ptr;
  };

public:
  TagCompound() noexcept : Tag(InitMode::Compound) {}

  explicit TagCompound(cnbt_Tag raw) noexcept : Tag(raw) {}

  Expected<TagCompound> clone() const noexcept {
    TagCompound out;
    auto status = cnbt_clone(&out._tag, &_tag);
    if (status != CNBT_OK)
      return {unexpect, status};
    return out;
  }

public:
  size_t size() const noexcept { return cnbt_comp_len(&_tag); }

  bool empty() const noexcept { return size() == 0; }

  cnbt_KeyTag* data() noexcept { return cnbt_comp_data(&_tag); }

  const cnbt_KeyTag* data() const noexcept { return cnbt_comp_data(const_cast<cnbt_Tag*>(&_tag)); }

  Iterator begin() noexcept { return Iterator(data()); }

  Iterator end() noexcept { return Iterator(data() ? data() + size() : nullptr); }

  ConstIterator begin() const noexcept { return ConstIterator(data()); }

  ConstIterator end() const noexcept { return ConstIterator(data() ? data() + size() : nullptr); }

  ConstIterator cbegin() const noexcept { return begin(); }

  ConstIterator cend() const noexcept { return end(); }

public:
  bool contains(const char* key) const noexcept {
    return cnbt_comp_get(const_cast<cnbt_Tag*>(&_tag), key) != nullptr;
  }

  bool contains(std::string_view key) const noexcept {
    std::string k(key);
    return contains(k.c_str());
  }

  bool has_key(const char* key) const noexcept { return contains(key); }

  bool has_key(std::string_view key) const noexcept { return contains(key); }

  Expected<TagType> tag_type(const char* key) const noexcept {
    auto* kt = cnbt_comp_get(const_cast<cnbt_Tag*>(&_tag), key);
    if (!kt)
      return {unexpect, CNBT_INVALID_DATA};
    return cnbt_get_type(&kt->value);
  }

  Expected<TagType> tag_type(std::string_view key) const noexcept {
    std::string k(key);
    return tag_type(k.c_str());
  }

public:
  Expected<Tag*> tag(const char* key) noexcept {
    auto* kt = cnbt_comp_get(&_tag, key);
    if (!kt)
      return {unexpect, CNBT_INVALID_DATA};
    return reinterpret_cast<Tag*>(&kt->value);
  }

  Expected<const Tag*> tag(const char* key) const noexcept {
    auto* kt = cnbt_comp_get(const_cast<cnbt_Tag*>(&_tag), key);
    if (!kt)
      return {unexpect, CNBT_INVALID_DATA};
    return reinterpret_cast<const Tag*>(&kt->value);
  }

  Expected<Tag*> tag(std::string_view key) noexcept {
    std::string k(key);
    return tag(k.c_str());
  }

  Expected<const Tag*> tag(std::string_view key) const noexcept {
    std::string k(key);
    return tag(k.c_str());
  }

  Expected<TagCompound*> compound(const char* key) noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_compound();
  }

  Expected<const TagCompound*> compound(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_compound();
  }

  Expected<TagCompound*> compound(std::string_view key) noexcept {
    std::string k(key);
    return compound(k.c_str());
  }

  Expected<const TagCompound*> compound(std::string_view key) const noexcept {
    std::string k(key);
    return compound(k.c_str());
  }

  Expected<TagList*> list(const char* key) noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_list();
  }

  Expected<const TagList*> list(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_list();
  }

  Expected<TagList*> list(std::string_view key) noexcept {
    std::string k(key);
    return list(k.c_str());
  }

  Expected<const TagList*> list(std::string_view key) const noexcept {
    std::string k(key);
    return list(k.c_str());
  }

  Expected<int8_t> byte(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_byte();
  }

  Expected<int8_t> byte(std::string_view key) const noexcept {
    std::string k(key);
    return byte(k.c_str());
  }

  Expected<int16_t> int16(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_short();
  }

  Expected<int16_t> int16(std::string_view key) const noexcept {
    std::string k(key);
    return int16(k.c_str());
  }

  Expected<int16_t> short_val(const char* key) const noexcept { return int16(key); }

  Expected<int16_t> short_val(std::string_view key) const noexcept { return int16(key); }

  Expected<int32_t> integer(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_int();
  }

  Expected<int32_t> integer(std::string_view key) const noexcept {
    std::string k(key);
    return integer(k.c_str());
  }

  Expected<int32_t> int32(const char* key) const noexcept { return integer(key); }

  Expected<int32_t> int32(std::string_view key) const noexcept { return integer(key); }

  Expected<int64_t> int64(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_long();
  }

  Expected<int64_t> int64(std::string_view key) const noexcept {
    std::string k(key);
    return int64(k.c_str());
  }

  Expected<int64_t> long_val(const char* key) const noexcept { return int64(key); }

  Expected<int64_t> long_val(std::string_view key) const noexcept { return int64(key); }

  Expected<float> float32(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_float();
  }

  Expected<float> float32(std::string_view key) const noexcept {
    std::string k(key);
    return float32(k.c_str());
  }

  Expected<float> float_val(const char* key) const noexcept { return float32(key); }

  Expected<float> float_val(std::string_view key) const noexcept { return float32(key); }

  Expected<double> float64(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_double();
  }

  Expected<double> float64(std::string_view key) const noexcept {
    std::string k(key);
    return float64(k.c_str());
  }

  Expected<double> double_val(const char* key) const noexcept { return float64(key); }

  Expected<double> double_val(std::string_view key) const noexcept { return float64(key); }

  Expected<bool> boolean(const char* key) const noexcept {
    auto b = byte(key);
    if (!b)
      return {unexpect, b.code()};
    return *b != 0;
  }

  Expected<bool> boolean(std::string_view key) const noexcept {
    std::string k(key);
    return boolean(k.c_str());
  }

  Expected<std::string_view> string(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_string();
  }

  Expected<std::string_view> string(std::string_view key) const noexcept {
    std::string k(key);
    return string(k.c_str());
  }

  Expected<ArrayView<const int8_t>> byte_array(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_byte_array();
  }

  Expected<ArrayView<const int8_t>> byte_array(std::string_view key) const noexcept {
    std::string k(key);
    return byte_array(k.c_str());
  }

  Expected<ArrayView<const int32_t>> int_array(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_int_array();
  }

  Expected<ArrayView<const int32_t>> int_array(std::string_view key) const noexcept {
    std::string k(key);
    return int_array(k.c_str());
  }

  Expected<ArrayView<const int64_t>> long_array(const char* key) const noexcept {
    auto t = tag(key);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_long_array();
  }

  Expected<ArrayView<const int64_t>> long_array(std::string_view key) const noexcept {
    std::string k(key);
    return long_array(k.c_str());
  }

public:
  Expected<void> set_tag(const char* key, Tag tag) noexcept {
    cnbt_Tag raw = tag.release();
    auto* res = cnbt_comp_insert(&_tag, key, raw);
    if (!res) {
      cnbt_free(&raw);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> set_tag(std::string_view key, Tag tag) noexcept {
    std::string k(key);
    return set_tag(k.c_str(), std::move(tag));
  }

  Expected<void> set_compound(const char* key, TagCompound comp) noexcept {
    return set_tag(key, std::move(comp));
  }

  Expected<void> set_compound(std::string_view key, TagCompound comp) noexcept {
    return set_tag(key, std::move(comp));
  }

  Expected<void> set_list(const char* key, TagList list) noexcept;
  Expected<void> set_list(std::string_view key, TagList list) noexcept;

  Expected<void> set_byte(const char* key, int8_t val) noexcept {
    cnbt_Byte b;
    cnbt_make_byte(&b, val);
    auto* res = cnbt_comp_insert(&_tag, key, b);
    if (!res) {
      cnbt_free(&b);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> set_byte(std::string_view key, int8_t val) noexcept {
    std::string k(key);
    return set_byte(k.c_str(), val);
  }

  Expected<void> set_short(const char* key, int16_t val) noexcept {
    cnbt_Short s;
    cnbt_make_short(&s, val);
    auto* res = cnbt_comp_insert(&_tag, key, s);
    if (!res) {
      cnbt_free(&s);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> set_short(std::string_view key, int16_t val) noexcept {
    std::string k(key);
    return set_short(k.c_str(), val);
  }

  Expected<void> set_int(const char* key, int32_t val) noexcept {
    cnbt_Int i;
    cnbt_make_int(&i, val);
    auto* res = cnbt_comp_insert(&_tag, key, i);
    if (!res) {
      cnbt_free(&i);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> set_int(std::string_view key, int32_t val) noexcept {
    std::string k(key);
    return set_int(k.c_str(), val);
  }

  Expected<void> set_integer(const char* key, int32_t val) noexcept { return set_int(key, val); }

  Expected<void> set_integer(std::string_view key, int32_t val) noexcept {
    return set_int(key, val);
  }

  Expected<void> set_long(const char* key, int64_t val) noexcept {
    cnbt_Long l;
    cnbt_make_long(&l, val);
    auto* res = cnbt_comp_insert(&_tag, key, l);
    if (!res) {
      cnbt_free(&l);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> set_long(std::string_view key, int64_t val) noexcept {
    std::string k(key);
    return set_long(k.c_str(), val);
  }

  Expected<void> set_float(const char* key, float val) noexcept {
    cnbt_Float f;
    cnbt_make_float(&f, val);
    auto* res = cnbt_comp_insert(&_tag, key, f);
    if (!res) {
      cnbt_free(&f);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> set_float(std::string_view key, float val) noexcept {
    std::string k(key);
    return set_float(k.c_str(), val);
  }

  Expected<void> set_double(const char* key, double val) noexcept {
    cnbt_Double d;
    cnbt_make_double(&d, val);
    auto* res = cnbt_comp_insert(&_tag, key, d);
    if (!res) {
      cnbt_free(&d);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> set_double(std::string_view key, double val) noexcept {
    std::string k(key);
    return set_double(k.c_str(), val);
  }

  Expected<void> set_boolean(const char* key, bool val) noexcept {
    return set_byte(key, val ? 1 : 0);
  }

  Expected<void> set_boolean(std::string_view key, bool val) noexcept {
    return set_byte(key, val ? 1 : 0);
  }

  Expected<void> set_string(const char* key, std::string_view val) noexcept {
    cnbt_String s;
    auto status = cnbt_make_strn(&s, val.data(), val.size());
    if (status != CNBT_OK)
      return {unexpect, status};
    auto* res = cnbt_comp_insert(&_tag, key, s);
    if (!res) {
      cnbt_free(&s);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> set_string(std::string_view key, std::string_view val) noexcept {
    std::string k(key);
    return set_string(k.c_str(), val);
  }

  Expected<void> set_byte_array(const char* key, ArrayView<const int8_t> val) noexcept {
    cnbt_ByteArray arr;
    auto status = cnbt_make_byte_array(&arr, val.data(), val.size());
    if (status != CNBT_OK)
      return {unexpect, status};
    auto* res = cnbt_comp_insert(&_tag, key, arr);
    if (!res) {
      cnbt_free(&arr);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> set_byte_array(std::string_view key, ArrayView<const int8_t> val) noexcept {
    std::string k(key);
    return set_byte_array(k.c_str(), val);
  }

  Expected<void> set_int_array(const char* key, ArrayView<const int32_t> val) noexcept {
    cnbt_IntArray arr;
    auto status = cnbt_make_int_array(&arr, val.data(), val.size());
    if (status != CNBT_OK)
      return {unexpect, status};
    auto* res = cnbt_comp_insert(&_tag, key, arr);
    if (!res) {
      cnbt_free(&arr);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> set_int_array(std::string_view key, ArrayView<const int32_t> val) noexcept {
    std::string k(key);
    return set_int_array(k.c_str(), val);
  }

  Expected<void> set_long_array(const char* key, ArrayView<const int64_t> val) noexcept {
    cnbt_LongArray arr;
    auto status = cnbt_make_long_array(&arr, val.data(), val.size());
    if (status != CNBT_OK)
      return {unexpect, status};
    auto* res = cnbt_comp_insert(&_tag, key, arr);
    if (!res) {
      cnbt_free(&arr);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> set_long_array(std::string_view key, ArrayView<const int64_t> val) noexcept {
    std::string k(key);
    return set_long_array(k.c_str(), val);
  }

public:
  static Expected<TagCompound> read_file(const char* path,
                                         EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    auto res = Tag::read_file(path, mode);
    if (!res)
      return {unexpect, res.code()};
    if (!res->is_compound())
      return {unexpect, CNBT_INVALID_DATA};
    return TagCompound(res->release());
  }

  static Expected<TagCompound> read_compressed_file(const char* path,
                                                    EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    auto res = Tag::read_compressed_file(path, mode);
    if (!res)
      return {unexpect, res.code()};
    if (!res->is_compound())
      return {unexpect, CNBT_INVALID_DATA};
    return TagCompound(res->release());
  }

  static Expected<TagCompound> read_memory(ArrayView<const uint8_t> buf,
                                           EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    auto res = Tag::read_memory(buf, mode);
    if (!res)
      return {unexpect, res.code()};
    if (!res->is_compound())
      return {unexpect, CNBT_INVALID_DATA};
    return TagCompound(res->release());
  }

  static Expected<TagCompound> read_compressed_memory(ArrayView<const uint8_t> buf,
                                                      EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    auto res = Tag::read_compressed_memory(buf, mode);
    if (!res)
      return {unexpect, res.code()};
    if (!res->is_compound())
      return {unexpect, CNBT_INVALID_DATA};
    return TagCompound(res->release());
  }

#ifndef CNBT_NO_FSTREAM
  static Expected<TagCompound> read_stream(std::istream& in,
                                           EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    auto res = Tag::read_stream(in, mode);
    if (!res)
      return {unexpect, res.code()};
    if (!res->is_compound())
      return {unexpect, CNBT_INVALID_DATA};
    return TagCompound(res->release());
  }

  static Expected<TagCompound> read_compressed_stream(std::istream& in,
                                                      EndianMode mode = CNBT_BIG_ENDIAN) noexcept {
    auto res = Tag::read_compressed_stream(in, mode);
    if (!res)
      return {unexpect, res.code()};
    if (!res->is_compound())
      return {unexpect, CNBT_INVALID_DATA};
    return TagCompound(res->release());
  }
#endif
};

class TagList : public Tag {
public:
  TagList() noexcept : Tag(InitMode::List) {}

  explicit TagList(cnbt_Tag raw) noexcept : Tag(raw) {}

  Expected<TagList> clone() const noexcept {
    TagList out;
    auto status = cnbt_clone(&out._tag, &_tag);
    if (status != CNBT_OK)
      return {unexpect, status};
    return out;
  }

public:
  size_t size() const noexcept { return cnbt_list_len(&_tag); }

  bool empty() const noexcept { return size() == 0; }

  cnbt_Tag* data() noexcept { return cnbt_list_data(&_tag); }

  const cnbt_Tag* data() const noexcept { return cnbt_list_data(const_cast<cnbt_Tag*>(&_tag)); }

  Tag* begin() noexcept { return reinterpret_cast<Tag*>(data()); }

  Tag* end() noexcept { return begin() + size(); }

  const Tag* begin() const noexcept { return reinterpret_cast<const Tag*>(data()); }

  const Tag* end() const noexcept { return begin() + size(); }

  const Tag* cbegin() const noexcept { return begin(); }

  const Tag* cend() const noexcept { return end(); }

  Tag& operator[](size_t idx) noexcept {
    CNBT_ASSERT(idx < size() && "TagList::operator[] index out of range");
    return begin()[idx];
  }

  const Tag& operator[](size_t idx) const noexcept {
    CNBT_ASSERT(idx < size() && "TagList::operator[] index out of range");
    return begin()[idx];
  }

  Expected<Tag*> at(size_t idx) noexcept {
    auto* t = cnbt_list_get(&_tag, idx);
    if (!t)
      return {unexpect, CNBT_INVALID_DATA};
    return reinterpret_cast<Tag*>(t);
  }

  Expected<const Tag*> at(size_t idx) const noexcept {
    auto* t = cnbt_list_get(const_cast<cnbt_Tag*>(&_tag), idx);
    if (!t)
      return {unexpect, CNBT_INVALID_DATA};
    return reinterpret_cast<const Tag*>(t);
  }

  Expected<Tag*> tag(size_t idx) noexcept { return at(idx); }

  Expected<const Tag*> tag(size_t idx) const noexcept { return at(idx); }

  Expected<TagCompound*> compound(size_t idx) noexcept {
    auto t = at(idx);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_compound();
  }

  Expected<const TagCompound*> compound(size_t idx) const noexcept {
    auto t = at(idx);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_compound();
  }

  Expected<TagList*> list(size_t idx) noexcept {
    auto t = at(idx);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_list();
  }

  Expected<const TagList*> list(size_t idx) const noexcept {
    auto t = at(idx);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_list();
  }

  Expected<int8_t> byte(size_t idx) const noexcept {
    auto t = at(idx);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_byte();
  }

  Expected<int16_t> int16(size_t idx) const noexcept {
    auto t = at(idx);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_short();
  }

  Expected<int16_t> short_val(size_t idx) const noexcept { return int16(idx); }

  Expected<int32_t> integer(size_t idx) const noexcept {
    auto t = at(idx);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_int();
  }

  Expected<int32_t> int32(size_t idx) const noexcept { return integer(idx); }

  Expected<int64_t> int64(size_t idx) const noexcept {
    auto t = at(idx);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_long();
  }

  Expected<int64_t> long_val(size_t idx) const noexcept { return int64(idx); }

  Expected<float> float32(size_t idx) const noexcept {
    auto t = at(idx);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_float();
  }

  Expected<float> float_val(size_t idx) const noexcept { return float32(idx); }

  Expected<double> float64(size_t idx) const noexcept {
    auto t = at(idx);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_double();
  }

  Expected<double> double_val(size_t idx) const noexcept { return float64(idx); }

  Expected<std::string_view> string(size_t idx) const noexcept {
    auto t = at(idx);
    if (!t)
      return {unexpect, t.code()};
    return (*t)->as_string();
  }

public:
  Expected<void> push_back(Tag tag) noexcept {
    cnbt_Tag raw = tag.release();
    auto* res = cnbt_list_push(&_tag, raw);
    if (!res) {
      cnbt_free(&raw);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> append_byte(int8_t val) noexcept {
    cnbt_Byte b;
    cnbt_make_byte(&b, val);
    auto* res = cnbt_list_push(&_tag, b);
    if (!res) {
      cnbt_free(&b);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> append_short(int16_t val) noexcept {
    cnbt_Short s;
    cnbt_make_short(&s, val);
    auto* res = cnbt_list_push(&_tag, s);
    if (!res) {
      cnbt_free(&s);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> append_int(int32_t val) noexcept {
    cnbt_Int i;
    cnbt_make_int(&i, val);
    auto* res = cnbt_list_push(&_tag, i);
    if (!res) {
      cnbt_free(&i);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> append_long(int64_t val) noexcept {
    cnbt_Long l;
    cnbt_make_long(&l, val);
    auto* res = cnbt_list_push(&_tag, l);
    if (!res) {
      cnbt_free(&l);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> append_float(float val) noexcept {
    cnbt_Float f;
    cnbt_make_float(&f, val);
    auto* res = cnbt_list_push(&_tag, f);
    if (!res) {
      cnbt_free(&f);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> append_double(double val) noexcept {
    cnbt_Double d;
    cnbt_make_double(&d, val);
    auto* res = cnbt_list_push(&_tag, d);
    if (!res) {
      cnbt_free(&d);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }

  Expected<void> append_string(std::string_view val) noexcept {
    cnbt_String s;
    auto status = cnbt_make_strn(&s, val.data(), val.size());
    if (status != CNBT_OK)
      return {unexpect, status};
    auto* res = cnbt_list_push(&_tag, s);
    if (!res) {
      cnbt_free(&s);
      return {unexpect, CNBT_ALLOC_FAILED};
    }
    return {};
  }
};

inline Expected<TagCompound*> Tag::as_compound() noexcept {
  if (type() != CNBT_TYPE_COMPOUND)
    return {unexpect, CNBT_INVALID_DATA};
  return reinterpret_cast<TagCompound*>(this);
}

inline Expected<const TagCompound*> Tag::as_compound() const noexcept {
  if (type() != CNBT_TYPE_COMPOUND)
    return {unexpect, CNBT_INVALID_DATA};
  return reinterpret_cast<const TagCompound*>(this);
}

inline Expected<TagList*> Tag::as_list() noexcept {
  if (type() != CNBT_TYPE_LIST)
    return {unexpect, CNBT_INVALID_DATA};
  return reinterpret_cast<TagList*>(this);
}

inline Expected<const TagList*> Tag::as_list() const noexcept {
  if (type() != CNBT_TYPE_LIST)
    return {unexpect, CNBT_INVALID_DATA};
  return reinterpret_cast<const TagList*>(this);
}

inline Expected<void> TagCompound::set_list(const char* key, TagList list) noexcept {
  return set_tag(key, std::move(list));
}

inline Expected<void> TagCompound::set_list(std::string_view key, TagList list) noexcept {
  return set_tag(key, std::move(list));
}

static_assert(sizeof(Tag) == sizeof(cnbt_Tag), "Tag size must match cnbt_Tag");
static_assert(sizeof(TagCompound) == sizeof(cnbt_Tag), "TagCompound size must match cnbt_Tag");
static_assert(sizeof(TagList) == sizeof(cnbt_Tag), "TagList size must match cnbt_Tag");

} // namespace cnbt

#endif // CNBT_HPP
