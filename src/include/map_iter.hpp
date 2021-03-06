
#include <iterator>
#include <ostream>
#include <utility>
#include <cassert>

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
  inline void assign(void* lhs, const void* rhs) {
    *reinterpret_cast<Iter*>(lhs) = *reinterpret_cast<const Iter*>(rhs);
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
struct iter_funcs {
  void* (*copy)(const void*);
  void (*assign)(void*, const void*);
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
  void (*set_value)(void*, const T&);
};

template<class Key, class T, class Iter>
iter_funcs<Key,T>* mut_iter_funcs() {
  static iter_funcs<Key,T> funcs{
    wrappers::copy<Iter>,
    wrappers::assign<Iter>,
    wrappers::del<Iter>,
    wrappers::eq<Iter>,
    wrappers::inc<Iter>,
    wrappers::dec<Iter>,
    wrappers::deref<Key,T,Iter>,
    wrappers::get_value<T,Iter>,
    wrappers::get_key<Key,Iter>,
    wrappers::set_value<T,Iter>
  };
  return &funcs;
}

template<class Key, class T, class Iter>
iter_funcs<Key,T>* const_iter_funcs() {
  static iter_funcs<Key,T> funcs{
    wrappers::copy<Iter>,
    wrappers::assign<Iter>,
    wrappers::del<Iter>,
    wrappers::eq<Iter>,
    wrappers::inc<Iter>,
    wrappers::dec<Iter>,
    wrappers::deref<Key,T,Iter>,
    wrappers::get_value<T,Iter>,
    wrappers::get_key<Key,Iter>,
    NULL
  };
  return &funcs;
}

template<class Key, class T>
class iterator: public std::iterator <
    std::forward_iterator_tag, std::pair<const Key, T>> {

  public:
    iterator(): _iter(NULL), _funcs(NULL) {}

    template<class Iter>
    explicit iterator(const Iter& impl):
        _iter(reinterpret_cast<void*>(new Iter(impl))),
        _funcs(mut_iter_funcs<Key,T,Iter>()) {}

    iterator(const iterator& iter) {
      if (iter._funcs != NULL)
        _iter = iter._funcs->copy(iter._iter);
      else
        _iter = NULL;
      _funcs = iter._funcs;
    }

    iterator &operator=(const iterator &iter) {
      if (_funcs != NULL)
        if (iter._funcs == _funcs) {
          _funcs->assign(_iter, iter._iter);
          return *this;
        } else
          _funcs->del(_iter);
      _funcs = iter._funcs;
      if (_funcs != NULL)
        _iter = _funcs->copy(iter._iter);
      else
        _iter = NULL;
      return *this;
    }

    template<class Iter>
    iterator &operator=(const Iter &iter) {
      assert(_funcs != NULL);
      _funcs->assign(_iter, &iter);
      return *this;
    }

    ~iterator() {
      if (_funcs != NULL)
        _funcs->del(_iter);
    }

    bool operator==(const iterator& rhs) const {
      return _funcs->eq(_iter, rhs._iter);
    }

    bool operator!=(const iterator& rhs) const {
      return !_funcs->eq(_iter, rhs._iter);
    }

    std::pair<const Key, T> operator*() const {
      return _funcs->deref(_iter);
    }

   /*
    * -> is not supported because deref is
    * return by value
    */
   /*
    std::pair<const Key, T> *operator->() {
      return &(_funcs->deref(_iter));
    }
    */

    Key get_key() const {
      return _funcs->get_key(_iter);
    }

    T get_value() const {
      return _funcs->get_value(_iter);
    }

    void set_value(T value) {
      _funcs->set_value(_iter, value);
    }

    iterator &operator++() {
      _funcs->inc(_iter);
      return *this;
    }

    iterator operator++(int) {
      iterator prev(*this);
      _funcs->inc(_iter);
      return prev;
    }

    iterator &operator--() {
      _funcs->dec(_iter);
      return *this;
    }

    iterator operator--(int) {
      iterator prev(*this);
      _funcs->dec(_iter);
      return prev;
    }

    const void* get_impl() const {
      return _iter;
    }

  private:
		void *_iter;
    iter_funcs<Key,T>* _funcs;
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
        _iter(reinterpret_cast<void*>(new Iter(impl))),
        _funcs(const_iter_funcs<Key,T,Iter>()) {}

    const_iterator(const const_iterator& iter) {
      if (iter._funcs != NULL)
        _iter = iter._funcs->copy(iter._iter);
      else
        _iter = NULL;
      _funcs = iter._funcs;
    }

    const_iterator &operator=(const const_iterator &iter) {
      if (_funcs != NULL)
        _funcs->del(_iter);
      _funcs = iter._funcs;
      if (_funcs != NULL)
        _iter = _funcs->copy(iter._iter);
      else
        _iter = NULL;
      return *this;
    }

    ~const_iterator() {
      if (_funcs != NULL)
        _funcs->del(_iter);
    }

    bool operator==(const const_iterator& rhs) const {
      return _funcs->eq(_iter, rhs._iter);
    }

    bool operator!=(const const_iterator& rhs) const {
      return !_funcs->eq(_iter, rhs._iter);
    }

    const std::pair<const Key, T> operator*() const {
      return _funcs->deref(_iter);
    }

   /*
    * -> is not supported because deref is
    * return by value
    */
   /*
    std::pair<const Key, T> *operator->() {
      return &(_funcs->deref(_iter));
    }
    */

    Key get_key() const {
      return _funcs->get_key(_iter);
    }

    T get_value() const {
      return _funcs->get_value(_iter);
    }

    const_iterator &operator++() {
      _funcs->inc(_iter);
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator prev(*this);
      _funcs->inc(_iter);
      return prev;
    }

    const_iterator &operator--() {
      _funcs->dec(_iter);
      return *this;
    }

    const_iterator operator--(int) {
      const_iterator prev(*this);
      _funcs->dec(_iter);
      return prev;
    }

    const void* get_impl() const {
      return _iter;
    }

  private:
    void *_iter;
    iter_funcs<Key,T>* _funcs;
};

template<class T>
class abs_map_container {

  public:
    typedef std::pair<const T, T> value_type;
    typedef map_iter::iterator<T,T> iterator;
    typedef map_iter::const_iterator<T,T> const_iterator;

    /*
     * This uses a mutable iterator reference in order to
     * avoid unecessary copying (with heap allocation).
     */
    virtual void insert(
        typename abs_map_container::iterator&,
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

    template <class Container>
    bool operator==(const Container& rhs) const {
      if (size() != rhs.size())
        return false;
      auto rhsi = rhs.get_raw_data().begin();
      auto rhs_end = rhs.get_raw_data().end();
      auto i_end = end();
      for (auto i = begin(); i != i_end; ++i, ++rhsi)
        if (!item_equiv(*i, *rhsi))
          return false;
      return true;
    }

    template <class Container>
    bool operator!=(const Container& rhs) const {
      return !(*this == rhs);
    }

    template <class Container>
    abs_map_container& operator=(const Container& rhs) {
      clear();
      auto rhs_end = rhs.get_raw_data().end();
      iterator i = begin();
      for (auto rhsi = rhs.get_raw_data().begin(); rhsi != rhs_end; ++rhsi)
        insert(i, *rhsi);
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
