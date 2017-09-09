
#include <iterator>
#include <ostream>
#include <utility>

#ifndef POC_MAP_ITER_H
#define POC_MAP_ITER_H

namespace map_iter {

namespace wrappers {

  template<class Iter>
  inline void* copy(const void* iter) {
    return reinterpret_cast<void*>(new Iter(
        *reinterpret_cast<const Iter*>(iter)));
  }

  template<class Iter>
  inline void del (void* iter) {
    delete reinterpret_cast<Iter*>(iter);
  }

  template<class Iter>
  inline bool eq(const void* iter, const void* rhs) {
    return *reinterpret_cast<const Iter*>(iter) ==
        *reinterpret_cast<const Iter*>(rhs);
  }

  template<class Iter>
  inline void inc(void* iter) {
    ++(*reinterpret_cast<Iter*>(iter));
  }

  template<class Iter>
  inline void dec(void* iter) {
    --(*reinterpret_cast<Iter*>(iter));
  }

  template<class Key, class T, class Iter>
  inline std::pair<const Key, T> deref(const void* iter) {
    return *(*reinterpret_cast<const Iter*>(iter));
  }

  template<class T, class Iter>
  inline T get_value(const void* iter) {
    return (*(*reinterpret_cast<const Iter*>(iter))).second;
  }

  template<class Key, class Iter>
  inline Key get_key(const void* iter) {
    return (*(*reinterpret_cast<const Iter*>(iter))).first;
  }

  template<class T, class Iter>
  inline void set_value(void* iter, const T& value) {
    (*reinterpret_cast<Iter*>(iter))->second = value;
  }
}

template<class Key, class T>
struct base_iter_funcs {
  void* (*copy)(const void*);
  void (*del)(void*);
  bool (*eq)(const void*, const void*);
  void (*inc)(void*);
  void (*dec)(void*);
  /*
   * Dereference is return by value, in order to hide differences
   * between pair<const T,T> used by map and pair<T,T> used by
   * vector (since vector elements are not allowed to contain const
   * members). Compiler RVO will optimize away unecessary copies when
   * appropriate.
   */
  std::pair<const Key, T>(*deref)(const void*);
  T(*get_value)(const void*);
  T(*get_key)(const void*);
};

template<class Key, class T, class Iter>
base_iter_funcs<Key,T> default_base_iter_funcs() {
  base_iter_funcs<Key,T> funcs{
    wrappers::copy<Iter>,
    wrappers::del<Iter>,
    wrappers::eq<Iter>,
    wrappers::inc<Iter>,
    wrappers::dec<Iter>,
    wrappers::deref<Key,T,Iter>,
    wrappers::get_value<T,Iter>,
    wrappers::get_key<Key,Iter>
  };
  return funcs;
}


template<class Key, class T>
struct mut_iter_funcs {
  base_iter_funcs<Key,T> base;
  void (*set_value)(void*, const T&);
};

template<class Key, class T, class Iter>
mut_iter_funcs<Key,T> default_mut_iter_funcs() {
  mut_iter_funcs<Key,T> funcs{
    default_base_iter_funcs<Key,T,Iter>(),
    wrappers::set_value<T,Iter>
  };
  return funcs;
}


template<class Key, class T>
struct const_iter_funcs {
  base_iter_funcs<Key,T> base;
};

template<class Key, class T, class Iter>
const_iter_funcs<Key,T> default_const_iter_funcs() {
  const_iter_funcs<Key,T> funcs{
    default_base_iter_funcs<Key,T,Iter>(),
  };
  return funcs;
}

template<class Key, class T>
class iterator: public std::iterator <
    std::forward_iterator_tag, std::pair<const Key, T>> {

  public:
    iterator(): _iter(NULL), _funcs(NULL) {}

    template<class Iter>
    explicit iterator(const Iter& impl):
        _iter(reinterpret_cast<void*>(new Iter(impl))) {

      static mut_iter_funcs<Key,T> def_funcs =
          default_mut_iter_funcs<Key,T,Iter>();

      _funcs = &def_funcs;
    }

    iterator(const iterator& iter) {
      if (iter._funcs != NULL)
        _iter = iter._funcs->base.copy(iter._iter);
      else
        _iter = NULL;
      _funcs = iter._funcs;
    }

    iterator &operator=(const iterator &iter) {
      if (_funcs != NULL)
        _funcs->base.del(_iter);
      _funcs = iter._funcs;
      if (_funcs != NULL)
        _iter = _funcs->base.copy(iter._iter);
      else
        _iter = NULL;
      return *this;
    }

    ~iterator() {
      if (_funcs != NULL)
        _funcs->base.del(_iter);
    }

    bool operator==(const iterator& rhs) const {
      return _funcs->base.eq(_iter, rhs._iter);
    }

    bool operator!=(const iterator& rhs) const {
      return !_funcs->base.eq(_iter, rhs._iter);
    }

    std::pair<const Key, T> operator*() const {
      return _funcs->base.deref(_iter);
    }

   /*
    * -> is not supported because deref is
    * return by value
    */
   /*
    std::pair<const Key, T> *operator->() {
      return &(_funcs->base.deref(_iter));
    }
    */

    Key get_key() const {
      return _funcs->base.get_key(_iter);
    }

    T get_value() const {
      return _funcs->base.get_value(_iter);
    }

    void set_value(T value) {
      _funcs->set_value(_iter, value);
    }

    iterator &operator++() {
      _funcs->base.inc(_iter);
      return *this;
    }

    iterator operator++(int) {
      iterator prev(*this);
      _funcs->base.inc(_iter);
      return prev;
    }

    iterator &operator--() {
      _funcs->base.dec(_iter);
      return *this;
    }

    iterator operator--(int) {
      iterator prev(*this);
      _funcs->base.dec(_iter);
      return prev;
    }

    const void* get_impl() const {
      return _iter;
    }

  private:
		void *_iter;
    mut_iter_funcs<Key,T>* _funcs;
};


template<class Key, class T>
class const_iterator:
		public std::iterator <
        std::forward_iterator_tag, std::pair<const Key, T>> {

  public:
    const_iterator(): _iter(NULL), _funcs(NULL) {}

    /*
     * Not explicit, allowing a mutable impl to be wrapped, so that
     * const_iterator is interoperable begin/end in addition to
     * cbegin/cend.
     */
    template<class Iter>
    const_iterator(const Iter& impl):
        _iter(reinterpret_cast<void*>(new Iter(impl))) {

      static const_iter_funcs<Key,T> def_funcs =
          default_const_iter_funcs<Key,T,Iter>();

      _funcs = &def_funcs;
    }

    const_iterator(const const_iterator& iter) {
      if (iter._funcs != NULL)
        _iter = iter._funcs->base.copy(iter._iter);
      else
        _iter = NULL;
      _funcs = iter._funcs;
    }

    const_iterator &operator=(const const_iterator &iter) {
      if (_funcs != NULL)
        _funcs->base.del(_iter);
      _funcs = iter._funcs;
      if (_funcs != NULL)
        _iter = _funcs->base.copy(iter._iter);
      else
        _iter = NULL;
      return *this;
    }

    ~const_iterator() {
      if (_funcs != NULL)
        _funcs->base.del(_iter);
    }

    bool operator==(const const_iterator& rhs) const {
      return _funcs->base.eq(_iter, rhs._iter);
    }

    bool operator!=(const const_iterator& rhs) const {
      return !_funcs->base.eq(_iter, rhs._iter);
    }

    const std::pair<const Key, T> operator*() const {
      return _funcs->base.deref(_iter);
    }

   /*
    * -> is not supported because deref is
    * return by value
    */
   /*
    std::pair<const Key, T> *operator->() {
      return &(_funcs->base.deref(_iter));
    }
    */

    Key get_key() const {
      return _funcs->base.get_key(_iter);
    }

    T get_value() const {
      return _funcs->base.get_value(_iter);
    }

    const_iterator &operator++() {
      _funcs->base.inc(_iter);
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator prev(*this);
      _funcs->base.inc(_iter);
      return prev;
    }

    const_iterator &operator--() {
      _funcs->base.dec(_iter);
      return *this;
    }

    const_iterator operator--(int) {
      const_iterator prev(*this);
      _funcs->base.dec(_iter);
      return prev;
    }

    const void* get_impl() const {
      return _iter;
    }

  private:
		void *_iter;
    const_iter_funcs<Key,T>* _funcs;
};

template<class T>
class abs_map_container {

  public:
    typedef std::pair<const T, T> value_type;
    typedef map_iter::iterator<T,T> iterator;
    typedef map_iter::const_iterator<T,T> const_iterator;

    virtual typename abs_map_container::iterator insert(
        const typename abs_map_container::iterator&,
        const typename abs_map_container::value_type&) = 0;

    virtual std::size_t size() const = 0;
    virtual void clear() = 0;
    virtual typename abs_map_container::iterator begin() = 0;
    virtual typename abs_map_container::iterator end() = 0;
    virtual typename abs_map_container::const_iterator begin() const = 0;
    virtual typename abs_map_container::const_iterator end() const = 0;

    /*
     * Allow comparison of pair<const T, T> with pair<T, T>, since
     * vector does not allow allow elements with const members.
     */
    template <class ValueTypeA, class ValueTypeB>
    static bool item_equiv(const ValueTypeA& lhs, const ValueTypeB& rhs) {
      return lhs.first == rhs.first && lhs.second == rhs.second;
    }

    bool operator==(const abs_map_container& rhs) const {
      if (size() != rhs.size())
        return false;
      for (auto i = begin(), rhsi = rhs.begin(); i != end(); ++i, ++rhsi)
        if (!item_equiv(*i, *rhsi))
          return false;
      return true;
    }

    bool operator!=(const abs_map_container& rhs) const {
      return !(*this == rhs);
    }

    template <class Container>
    bool operator==(const Container& rhs) const {
      if (size() != rhs.size())
        return false;
      auto rhsi = rhs.begin();
      for (auto i = begin(); i != end(); ++i, ++rhsi)
        if (!item_equiv(*i, *rhsi))
          return false;
      return true;
    }

    template <class Container>
    bool operator!=(const Container& rhs) const {
      return !(*this == rhs);
    }

    abs_map_container& operator=(const abs_map_container& rhs) {
      clear();
      iterator i = begin();
      for (const_iterator rhsi = rhs.begin(); rhsi != rhs.end(); ++rhsi) {
        i = insert(i, *rhsi);
      }
      return *this;
    }

    virtual void swap(abs_map_container& other) = 0;
};

template<class T>
inline std::ostream& operator<<(std::ostream& out, const abs_map_container<T> &s) {
  out << "[";
  const char *prequel = "";
  for (typename abs_map_container<T>::const_iterator i = s.begin();
       i != s.end();
       ++i)
  {
    out << prequel << i.get_key() << "~" << i.get_value();
    prequel = ",";
  }
  out << "]";
  return out;
}

}

#endif
