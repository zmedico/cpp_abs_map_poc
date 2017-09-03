
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

#include "include/generic_iter.h"

template<class T>
class map_container : public generic_iter::abs_map_container<T>
{
  public:
    typedef std::map<T,T> map_t;
    typedef typename map_t::value_type value_type;
    typedef generic_iter::iterator<value_type> iterator;
    typedef generic_iter::const_iterator<typename map_t::value_type> const_iterator;

    map_container(): m(){
    }

		virtual typename map_container::iterator insert(
				const typename map_container::iterator &mi,
				const typename map_container::value_type &value) {
      return typename map_container::iterator(
          m.insert(*reinterpret_cast<const typename decltype(m)::iterator*>(mi.get_impl()), value));
    }

    typename map_container::iterator begin() {
      return typename map_container::iterator(m.begin());
    }

    typename map_container::iterator end(){
      return typename map_container::iterator(m.end());
    }

    typename map_container::const_iterator begin() const {
      return typename map_container::const_iterator(m.cbegin());
    }

    typename map_container::const_iterator end() const {
      return typename map_container::const_iterator(m.cend());
    }

  private:
    map_t m;
};

template<class T>
class array_container : public generic_iter::abs_map_container<T>
{
  public:
    typedef std::vector<std::pair<T,T>> map_t;
    typedef typename std::map<T,T>::value_type value_type;
    typedef generic_iter::iterator<value_type> iterator;
    typedef generic_iter::const_iterator<value_type> const_iterator;

    array_container(): m(){
    }

		virtual typename array_container::iterator insert(
				const typename array_container::iterator &mi,
				const typename array_container::value_type &value) {
      /*
       * convert from pair<const T,T> because vector cannot
       * store elements containing const members
       */
      std::pair<T,T> conv_value = value;
      typename decltype(m)::iterator p = std::lower_bound(
          *reinterpret_cast<const typename decltype(m)::iterator*>(mi.get_impl()), m.end(), conv_value);
      //std::cout << "insert index: " << p - m.begin() << std::endl << std::flush;
      return typename array_container::iterator(m.emplace(p, std::move(conv_value)));
    }

    typename array_container::iterator begin() {
      return typename array_container::iterator(m.begin());
    }

    typename array_container::iterator end(){
      return typename array_container::iterator(m.end());
    }

    typename array_container::const_iterator begin() const {
      return typename array_container::const_iterator(m.cbegin());
    }

    typename array_container::const_iterator end() const {
      return typename array_container::const_iterator(m.cend());
    }

  private:
    map_t m;
};


void abs_mut_operation(generic_iter::abs_map_container<int> &a, const generic_iter::abs_map_container<int> &b) {
  auto mi = a.begin();
  for (int i = 0; i < 3; ++i) {
    std::pair<const int, int> value{i, 1};
    mi = a.insert(mi, value);
  }
/*
 * Dereference is return-by-value, in order to hide differences
 * between pair<const T,T> used by map and pair<T,T> used by
 * vector (since vector elements are not allowed to contain const
 * members).
 * 
 * Therefore, we'll need an iterator method for (*i).second
 * mutation operations.
 */
/*
  for (auto i = a.begin(); i != a.end(); ++i) {
    // error: using temporary as lvalue [-fpermissive]
    //i->second += 1;
    (*i).second += 1;
  }
*/
  for (auto i = b.begin(); i != b.end(); ++i) {
    mi = a.insert(mi, *i);
  }
}


void abs_const_operation(const generic_iter::abs_map_container<int> &a) {
  for (auto i = a.begin(); i != a.end(); ++i) {
    //std::cout << i->first << " " << i->second << std::endl << std::flush;
    std::cout << (*i).first << " " << (*i).second << std::endl << std::flush;
  }
}


int main(int argc, char *argv[]) {
  map_container<int> a;
  array_container<int> b;
  b.insert(b.begin(), {5, 3});
  abs_mut_operation(a, b);
  abs_const_operation(a);
  std::cout << std::endl << std::flush;
  abs_const_operation(b);
}
