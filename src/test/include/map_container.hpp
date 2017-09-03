

#ifndef POC_MAP_CONTAINER_H
#define POC_MAP_CONTAINER_H

#include <map>

#include "include/map_iter.hpp"

template<class T>
class map_container : public map_iter::abs_map_container<T>
{
  public:
    typedef std::map<T,T> map_t;
    typedef typename map_t::value_type value_type;
    typedef map_iter::iterator<T,T> iterator;
    typedef map_iter::const_iterator<T,T> const_iterator;
    using map_iter::abs_map_container<T>::operator=;

    map_container(): map_iter::abs_map_container<T>(), m(){
    }

    map_container(const typename map_iter::abs_map_container<T>& rhs):
        m() {
      *this = rhs;
    }

    std::size_t size() const {
      return m.size();
    }

    void clear() {
      return m.clear();
    }

    bool operator==(const map_container& rhs) const {
      m == rhs.m;
    }

    void swap(map_container& other) {
      m.swap(other.m);
    }

    virtual typename map_container::iterator insert(
        const typename map_container::iterator &mi,
        const typename map_container::value_type &value) {
      return typename map_container::iterator(m.insert(
          *reinterpret_cast<const typename decltype(m)::iterator*>(
          mi.get_impl()), value));
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

#endif
