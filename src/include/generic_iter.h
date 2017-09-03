
#include <iterator>
#include <functional>

#ifndef POC_GENERIC_ITER_H
#define POC_GENERIC_ITER_H

namespace generic_iter {

struct base_iter_funcs {
	std::function<void*(const void*)> copy;
	std::function<void(void*)> del;
	std::function<bool(const void*, const void*)> eq;
	std::function<void(void*)> inc;
	std::function<void(void*)> dec;
};

template<class Iter>
base_iter_funcs default_base_iter_funcs() {
	base_iter_funcs funcs{
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
		}
	};
	return funcs;
}


template<class T>
struct mut_iter_funcs {
	base_iter_funcs base;
	/* 
	 * Since vector doesn't allow elements to contain const members
	 * like std::pair<const T,T>, return by value rather than by
	 * reference (compiler RVO should eliminate unecessary copies
	 * when possible).
	 */
	std::function<T(void*)> deref;
};

template<class T, class Iter>
mut_iter_funcs<T> default_mut_iter_funcs() {
	mut_iter_funcs<T> funcs{
		default_base_iter_funcs<Iter>(),

		[](void* iter) -> T {
			return *(*reinterpret_cast<Iter*>(iter));
		}
	};
	return funcs;
}


template<class T>
struct const_iter_funcs {
	base_iter_funcs base;
	std::function<const T(void*)> deref;
};

template<class T, class Iter>
const_iter_funcs<T> default_const_iter_funcs() {
	const_iter_funcs<T> funcs{
		default_base_iter_funcs<Iter>(),

		[](void* iter) -> const T {
			return *(*reinterpret_cast<Iter*>(iter));
		}
	};
	return funcs;
}

template<class T>
class iterator: public std::iterator <std::forward_iterator_tag, T>
{
  public:
    template<class Iter>
    explicit iterator(const Iter& impl, mut_iter_funcs<T>* funcs = NULL):
				_iter(reinterpret_cast<void*>(new Iter(impl))) {

      static mut_iter_funcs<T> def_funcs = default_mut_iter_funcs<T,Iter>();
      if (funcs == NULL)
				funcs = &def_funcs;
      _funcs = funcs;
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

    bool operator==(const iterator<T>& rhs) const {
      return _funcs->base.eq(_iter, rhs._iter);
    }

    bool operator!=(const iterator<T>& rhs) const {
      return !_funcs->base.eq(_iter, rhs._iter);
    }

    T operator*() {
      return _funcs->deref(_iter);
    }
/*
    T *operator->() {
      return &(_funcs->deref(_iter));
    }
*/
    iterator<T> &operator++() {
      _funcs->base.inc(_iter);
      return *this;
    }

    iterator<T> operator++(int) {
      iterator<T> prev(*this);
      _funcs->base.inc(_iter);
      return prev;
    }

    iterator<T> &operator--() {
      _funcs->base.dec(_iter);
      return *this;
    }

    iterator<T> operator--(int) {
      iterator<T> prev(*this);
      _funcs->base.dec(_iter);
      return prev;
    }

    const void* get_impl() const {
      return _iter;
    }

  private:
		void *_iter;
    mut_iter_funcs<T>* _funcs;
};


template<class T>
class const_iterator:
		public std::iterator <std::forward_iterator_tag, T> {
  public:
    // not explicit, allowing it to wrap a mutable impl, so that
    // begin/end can be used instead of cbegin/cend
    template<class Iter>
    const_iterator(const Iter& impl, const_iter_funcs<T>* funcs = NULL):
				_iter(reinterpret_cast<void*>(new Iter(impl))) {

      static const_iter_funcs<T> def_funcs = default_const_iter_funcs<T,Iter>();
      if (funcs == NULL)
				funcs = &def_funcs;
      _funcs = funcs;
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

    bool operator==(const const_iterator<T>& rhs) const {
      return _funcs->base.eq(_iter, rhs._iter);
    }

    bool operator!=(const const_iterator<T>& rhs) const {
      return !_funcs->base.eq(_iter, rhs._iter);
    }

    const T operator*() {
      return _funcs->deref(_iter);
    }
/*
    const T *operator->() {
      return &(_funcs->deref(_iter));
    }
*/
    const_iterator<T> &operator++() {
      _funcs->base.inc(_iter);
      return *this;
    }

    const_iterator<T> operator++(int) {
      const_iterator<T> prev(*this);
      _funcs->base.inc(_iter);
      return prev;
    }

    const_iterator<T> &operator--() {
      _funcs->base.dec(_iter);
      return *this;
    }

    const_iterator<T> operator--(int) {
      const_iterator<T> prev(*this);
      _funcs->base.dec(_iter);
      return prev;
    }

    const void* get_impl() const {
      return _iter;
    }

  private:
		void *_iter;
    const_iter_funcs<T>* _funcs;
};

template<typename T>
class abs_map_container
{
  public:
    typedef std::pair<const T, T> value_type;
    typedef generic_iter::iterator<value_type> iterator;
    typedef generic_iter::const_iterator<value_type> const_iterator;

		virtual typename abs_map_container::iterator insert(
				const typename abs_map_container::iterator&,
				const typename abs_map_container::value_type&) = 0;

    virtual typename abs_map_container::iterator begin() = 0;
    virtual typename abs_map_container::iterator end() = 0;
    virtual typename abs_map_container::const_iterator begin() const = 0;
    virtual typename abs_map_container::const_iterator end() const = 0;
};

}

#endif
