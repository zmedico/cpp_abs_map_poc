#include <map>
#include <string>

int main(int argc, char *argv[])
{
  const int size = std::stoi(argv[1]);

  // populate m by iterator
  std::map<int,int> m;
  decltype(m)::iterator mi = m.begin();
  for (int i = 0; i < size; ++i) {
    const decltype(m)::value_type value{i, 1};
    mi = m.insert(mi, value);
  }

  // copy m by iterator
  std::map<int,int> m2;
  mi = m2.begin();
  for (decltype(m)::const_iterator i = m.begin(); i != m.end(); ++i)
    mi = m2.insert(mi, *i);
}
