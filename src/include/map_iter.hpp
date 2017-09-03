
#include <iterator>
#include <functional>
#include <utility>

#ifndef POC_MAP_ITER_H
#define POC_MAP_ITER_H

namespace map_iter {

template<class Key, class T>
struct base_iter_funcs {
  std::function<void*(const void*)> copy;
  std::function<void(void*)> del;
  std::function<bool(const void*, const void*)> eq;
  std::function<void(void*)> inc;
  std::function<void(void*)> dec;
  /*
   * Dereference is return by value, in order to hide differences
   * between pair<const T,T> used by map and pair<T,T> used by
   * vector (since vector elements are not allowed to contain const
   * members). Compiler RVO will optimize away unecessary copies when
   * appropriate.
   */
  std::function<std::pair<const Key, T>(void*)> deref;
  std::function<T(const void*)> get_value;
  std::function<Key(const void*)> get_key;
};

template<class Key, class T, class Iter>
base_iter_funcs<Key,T> default_base_iter_funcs() {
  base_iter_funcs<Key,T> funcs{
    [](const void* iter) -> void* {
        return reinterpret_cast<void*>(new Iter(
            *reinterpret_cast<const Iter*>(iter)));
    },

    [](void* iter) {
      delete reinterpret_cast<Iter*>(iter);
    },

    [](const void* iter, const void* rhs) -> bool {
      return *reinterpret_cast<const Iter*>(iter) ==
          *reinterpret_cast<const Iter*>(rhs);
    },

    [](void* iter) {
      ++(*reinterpret_cast<Iter*>(iter));
    },

    [](void* iter) {
      --(*reinterpret_cast<Iter*>(iter));
    },

    [](void* iter) -> std::pair<const Key, T> {
      return *(*reinterpret_cast<Iter*>(iter));
    },

    [](const void* iter) -> T {
      return (*(*reinterpret_cast<const Iter*>(iter))).second;
    },

    [](const void* iter) -> Key {
      return (*(*reinterpret_cast<const Iter*>(iter))).first;
    }
  };
  return funcs;
}


template<class Key, class T>
struct mut_iter_funcs {
  base_iter_funcs<Key,T> base;
  std::function<void(void*, const T&)> set_value;
};

template<class Key, class T, class Iter>
mut_iter_funcs<Key,T> default_mut_iter_funcs() {
  mut_iter_funcs<Key,T> funcs{
    default_base_iter_funcs<Key,T,Iter>(),

    [](void* iter, const T& value) {
      (*reinterpret_cast<Iter*>(iter))->second = value;
    }
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
    template<class Iter>
    explicit iterator(const Iter& impl):
        _iter(reinterpret_cast<void*>(new Iter(impl))) {

      static mut_iter_funcs<Key,T> def_funcs =
          default_mut_iter_funcs<Key,T,Iter>();

      _funcs = &def_funcs;
    }

    iterator(const iterator& iter):
        _iter(iter._funcs->base.copy(iter._iter)) {
      _funcs = iter._funcs;
    }

    iterator &operator=(const iterator &iter) {
      _funcs->base.del(_iter);
      _funcs = iter._funcs;
      _iter = _funcs->base.copy(iter._iter);
      return *this;
    }

    ~iterator() {
      _funcs->base.del(_iter);
    }

    bool operator==(const iterator<Key,T>& rhs) const {
      return _funcs->base.eq(_iter, rhs._iter);
    }

    bool operator!=(const iterator<Key,T>& rhs) const {
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

    iterator<Key,T> &operator++() {
      _funcs->base.inc(_iter);
      return *this;
    }

    iterator<Key,T> operator++(int) {
      iterator<Key,T> prev(*this);
      _funcs->base.inc(_iter);
      return prev;
    }

    iterator<Key,T> &operator--() {
      _funcs->base.dec(_iter);
      return *this;
    }

    iterator<Key,T> operator--(int) {
      iterator<Key,T> prev(*this);
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

    const_iterator(const const_iterator& iter) :
        _iter(iter._funcs->base.copy(iter._iter)) {
      _funcs = iter._funcs;
    }

    const_iterator &operator=(const const_iterator &iter) {
      _funcs->base.del(_iter);
      _funcs = iter._funcs;
      _iter = _funcs->base.copy(iter._iter);
      return *this;
    }

    ~const_iterator() {
      _funcs->base.del(_iter);
    }

    bool operator==(const const_iterator<Key,T>& rhs) const {
      return _funcs->base.eq(_iter, rhs._iter);
    }

    bool operator!=(const const_iterator<Key,T>& rhs) const {
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

    const_iterator<Key,T> &operator++() {
      _funcs->base.inc(_iter);
      return *this;
    }

    const_iterator<Key,T> operator++(int) {
      const_iterator<Key,T> prev(*this);
      _funcs->base.inc(_iter);
      return prev;
    }

    const_iterator<Key,T> &operator--() {
      _funcs->base.dec(_iter);
      return *this;
    }

    const_iterator<Key,T> operator--(int) {
      const_iterator<Key,T> prev(*this);
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

    bool operator==(const map_iter::abs_map_container<T>& rhs) const {
      if (size() != rhs.size())
        return false;
      for (auto i = begin(), rhsi = rhs.begin(); i != end(); ++i)
        if (*i != *rhsi)
          return false;
      return true;
    }

    bool operator!=(const map_iter::abs_map_container<T>& rhs) const {
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

};

}

#endif