

#ifndef POC_ARRAY_CONTAINER_H
#define POC_ARRAY_CONTAINER_H

#include <algorithm>
#include <map>
#include <vector>

#include "include/map_iter.hpp"

template<class T>
class array_container : public map_iter::abs_map_container<T>
{
  public:
    typedef std::vector<std::pair<T,T>> map_t;
    typedef typename std::map<T,T>::value_type value_type;
    typedef map_iter::iterator<T,T> iterator;
    typedef map_iter::const_iterator<T,T> const_iterator;
    using map_iter::abs_map_container<T>::operator=;

    array_container(): m(){
    }

    array_container(const typename map_iter::abs_map_container<T>& rhs):
        m() {
      *this = rhs;
    }

    std::size_t size() const {
      return m.size();
    }

    void clear() {
      return m.clear();
    }

    bool operator==(const array_container& rhs) const {
      return m == rhs.m;
    }

    void swap(map_iter::abs_map_container<T>& other) {
      array_container temp;
      swap(temp);
      *this = other;
      other = temp;
    }

    void swap(array_container& other) {
      m.swap(other.m);
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
          *reinterpret_cast<const typename decltype(m)::iterator*>(
          mi.get_impl()), m.end(), conv_value);
      return typename array_container::iterator(
          m.emplace(p, std::move(conv_value)));
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

#endif
