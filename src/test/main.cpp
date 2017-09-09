
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <map>
#include <vector>
#include "include/map_iter.hpp"
#include "include/map_container.hpp"
#include "include/array_container.hpp"


void abs_mut_operation(map_iter::abs_map_container<int> &a,
    const map_iter::abs_map_container<int> &b) {

  auto mi = a.begin();
  for (int i = 0; i < 3; ++i) {
    std::pair<const int, int> value{i, 1};
    mi = a.insert(mi, value);
  }

  for (auto i = a.begin(); i != a.end(); ++i) {
    i.set_value(i.get_value() + 1);
  }

  for (auto i = b.begin(); i != b.end(); ++i) {
    mi = a.insert(mi, *i);
  }
}


void abs_const_operation(const map_iter::abs_map_container<int> &a) {
  int value_sum = 0;
  int key_sum = 0;
  for (auto i = a.begin(); i != a.end(); ++i) {
    key_sum += i.get_key();
    value_sum += i.get_value();
  }
}


TEST_CASE( "all tests", "[all]" ) {
  map_container<int> a, c;
  array_container<int> b;
  typename decltype(a)::value_type foo{5, 3};

  SECTION( "section 1" ) {
    REQUIRE( a == b );
    a.insert(a.begin(), foo);
    REQUIRE( a != b );

    b = a;
    REQUIRE( a == b );

    a.swap(c);
    REQUIRE( c == b );
    a.swap(c);
    REQUIRE( a == b );

    b.clear();
    REQUIRE( a != b );

    a.clear();
    REQUIRE( a == b );
  }

  SECTION( "section 2" ) {
    b.insert(b.begin(), foo);
    abs_mut_operation(a, b);
    abs_const_operation(a);
    abs_const_operation(b);
    REQUIRE( a != b );
    REQUIRE( a.size() == 4 );
    REQUIRE( b.size() == 1 );
    REQUIRE( *(--a.end()) == *(--b.end()) );
    REQUIRE( *(--a.end()) == foo );
    REQUIRE( (--(--a.end())).get_value() == 2 );

    array_container<int> d = a;
    REQUIRE( a == d );
    array_container<int> e = std::move(d);
    REQUIRE( a == e );
    REQUIRE( e == a );
    e.clear();
    REQUIRE( a != e );
    REQUIRE( e != a );

    map_container<int> f = a;
    f.swap(e);
    REQUIRE( e == a );
    REQUIRE( f != a );
    e.swap(f);
    REQUIRE( f == a );
    REQUIRE( e != a );
  }
}

