#pragma once

#include "refs/ref.h"

template <class T> class Refable {
protected:
  Ref<T> m_ref = Ref<T>(nullptr);

public:
  Refable() : m_ref(static_cast<T *>(this)) {}

  Ref<T> &ref() { return m_ref; }

  Refable(const Refable &) = delete;
  Refable &operator=(const Refable &) = delete;

  Refable(Refable &&o) noexcept : m_ref(o.m_ref) {
    m_ref.set(static_cast<T *>(this));
  }

  Refable &operator=(Refable &&o) noexcept {
    if (this != &o) {
      m_ref = std::move(o.m_ref);
      o.m_ref.set(this);
    }
    return *this;
  }

  ~Refable() {
    if (m_ref.is(static_cast<T *>(this))) {
      m_ref.reset();
    }
  }
};

template <class T, typename Raw> class RawRefable {
protected:
  RawRef<T, Raw> m_ref = RawRef<T, Raw>(nullptr);

public:
  RawRefable() : m_ref(static_cast<T *>(this)) {}

  RawRef<T, Raw> &ref() { return m_ref; }

  RawRefable(const RawRefable &) = delete;
  RawRefable &operator=(const RawRefable &) = delete;

  RawRefable(RawRefable &&o) noexcept : m_ref(o.m_ref) {
    m_ref.set(static_cast<T *>(this));
  }

  RawRefable &operator=(RawRefable &&o) noexcept {
    if (this != &o) {
      m_ref = std::move(o.m_ref);
      o.m_ref.set(this);
    }
    return *this;
  }

  ~RawRefable() {
    if (m_ref.is(static_cast<T *>(this))) {
      m_ref.reset();
    }
  }
};
