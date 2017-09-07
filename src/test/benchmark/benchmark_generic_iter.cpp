#include <string>

#include "include/map_iter.hpp"
#include "include/map_container.hpp"

int main(int argc, char *argv[])
{
  const int size = std::stoi(argv[1]);
  map_container<int> m;

  // populate m by iterator
  decltype(m)::iterator mi = m.begin();
  for (int i = 0; i < size; ++i) {
    const decltype(m)::value_type value{i, 1};
    m.insert(mi, value);
  }

  // copy m by iterator
  map_container<int> m2;
  mi = m2.begin();
  const decltype(m)::const_iterator m_end = m.end();
  for (decltype(m)::const_iterator i = m.begin(); i != m_end; ++i)
    m2.insert(mi, *i);
}
