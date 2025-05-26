#pragma once

#include <memory>

template <class T> class Reference {
protected:
  std::shared_ptr<T *> m_ref;

  Reference() = delete;

public:
  explicit Reference(T *ref) : m_ref(std::make_shared<T *>(ref)) {}
  static Reference create(T &ref) { return Reference(&ref); }
  static Reference create(T *ref) { return Reference(ref); }

  bool is(const T *const ref) const { return *m_ref == ref; }
  void set(T *ref) { *this->m_ref = ref; }
  void reset() { *this->m_ref = nullptr; }

  bool has_value() const { return *m_ref && **m_ref; }

  T *rawPtr() const { return *m_ref; }

  T &operator*() const { return **m_ref; }
  T &value() const { return **m_ref; }
  operator T &() const { return **m_ref; }
};

template <class T, typename Raw> class RawRef : public Reference<T> {
public:
  explicit RawRef(T *ref) : Reference<T>(ref) {}
  Raw raw() const { return ***this->m_ref; }
  operator Raw() const { return ***this->m_ref; }
};
