/**
 * MIT License
 *
 * Copyright (c) 2026 OpenAI
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
#include <tsl/array_hash_glaze.h>

#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

template <std::uint32_t Format>
void test_round_trip_for_format() {
  constexpr glz::opts options{.format = Format};

  using map_type = tsl::array_pg_map<char, std::int32_t>;
  map_type original_map;
  original_map.insert("one", 1);
  original_map.insert("two", 2);
  original_map.insert("three", 3);

  std::string map_buffer;
  const auto map_write_error = glz::write<options>(original_map, map_buffer);
  assert(!map_write_error);

  map_type restored_map;
  const auto map_read_error = glz::read<options>(restored_map, map_buffer);
  assert(!map_read_error);
  assert(restored_map == original_map);

  using set_type = tsl::array_pg_set<char>;
  set_type original_set;
  original_set.insert("alpha");
  original_set.insert("beta");
  original_set.insert("gamma");

  std::string set_buffer;
  const auto set_write_error = glz::write<options>(original_set, set_buffer);
  assert(!set_write_error);

  set_type restored_set;
  const auto set_read_error = glz::read<options>(restored_set, set_buffer);
  assert(!set_read_error);
  assert(restored_set == original_set);
}

void test_json_wire_shapes() {
  using map_type = tsl::array_map<char, std::int32_t>;
  map_type map;
  map.insert("left", 10);
  map.insert("right", 20);

  const auto json_map = glz::write_json(map);
  assert(json_map);

  std::map<std::string, std::int32_t> map_object;
  const auto map_error = glz::read_json(map_object, json_map.value());
  assert(!map_error);
  assert(map_object.size() == 2);
  assert(map_object.at("left") == 10);
  assert(map_object.at("right") == 20);

  using set_type = tsl::array_set<char>;
  set_type set;
  set.insert("red");
  set.insert("blue");

  const auto json_set = glz::write_json(set);
  assert(json_set);

  std::vector<std::string> set_array;
  const auto set_error = glz::read_json(set_array, json_set.value());
  assert(!set_error);
  assert(set_array.size() == 2);
}

int main() {
  test_round_trip_for_format<glz::JSON>();
  test_round_trip_for_format<glz::JSONB>();
  test_round_trip_for_format<glz::BEVE>();
  test_round_trip_for_format<glz::CBOR>();
  test_round_trip_for_format<glz::MSGPACK>();
  test_round_trip_for_format<glz::BSON>();
  test_round_trip_for_format<glz::TOML>();
  test_round_trip_for_format<glz::YAML>();
#if defined(GLZ_ENABLE_EETF)
  test_round_trip_for_format<glz::EETF>();
#endif
  test_json_wire_shapes();
}
