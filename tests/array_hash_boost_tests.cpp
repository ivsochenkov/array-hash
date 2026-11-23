/**
 * MIT License
 *
 * Copyright (c) 2017 Thibaut Goetghebuer-Planchon <tessil@gmx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/test/unit_test.hpp>

#include <tsl/array_hash_boost.h>

#include <cstdint>
#include <sstream>

BOOST_AUTO_TEST_SUITE(test_array_hash_boost_serialization)

BOOST_AUTO_TEST_CASE(test_array_map_round_trip) {
  using map_type = tsl::array_pg_map<char, std::int64_t>;

  map_type map;
  map.insert("one", 1);
  map.insert("two", 2);
  map.insert("three", 3);

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  {
    boost::archive::binary_oarchive archive(stream);
    archive << map;
  }

  map_type restored;
  {
    boost::archive::binary_iarchive archive(stream);
    archive >> restored;
  }

  BOOST_CHECK(map == restored);
}

BOOST_AUTO_TEST_CASE(test_array_set_round_trip) {
  using set_type = tsl::array_pg_set<char16_t>;

  set_type set;
  set.insert(u"alpha");
  set.insert(u"beta");
  set.insert(u"gamma");

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  {
    boost::archive::binary_oarchive archive(stream);
    archive << set;
  }

  set_type restored;
  {
    boost::archive::binary_iarchive archive(stream);
    archive >> restored;
  }

  BOOST_CHECK(set == restored);
}

BOOST_AUTO_TEST_SUITE_END()
