#pragma once

#include <memory>

template <class T> class Ref {
protected:
  std::shared_ptr<T *> m_ref = std::make_shared<T *>(nullptr);

  Ref() = delete;

public:
  explicit Ref(T *ref) : m_ref(std::make_shared<T *>(ref)) {}
  static Ref create(T &ref) { return Ref(&ref); }
  static Ref create(T *ref) { return Ref(ref); }

  bool is(T *ref) const { return *m_ref == ref; }
  void set(T *ref) { *this->m_ref = ref; }
  void reset() { *this->m_ref = nullptr; }

  bool has_value() const { return *m_ref && **m_ref; }

  T &operator*() const { return **m_ref; }
  T &value() const { return **m_ref; }
  operator T &() const { return **m_ref; }
};

template <class T, typename Raw> class RawRef : public Ref<T> {
public:
  explicit RawRef(T *ref) : Ref<T>(ref) {}
  Raw raw() const { return ***this->m_ref; }
  operator Raw() const { return ***this->m_ref; }
};
