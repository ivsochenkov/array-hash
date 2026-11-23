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
#ifndef TSL_ARRAY_HASH_CEREAL_H
#define TSL_ARRAY_HASH_CEREAL_H

#include <cereal/cereal.hpp>

#include <tsl/array_map.h>
#include <tsl/array_set.h>

#include <cstddef>

namespace cereal {
namespace detail_array_hash {

template <class Archive>
class serializer {
 public:
  explicit serializer(Archive& archive) : m_archive(archive) {}

  template <class T>
  void operator()(const T& value) {
    m_archive(value);
  }

  template <class CharT>
  void operator()(const CharT* value, std::size_t value_size) {
    m_archive.saveBinary(value, value_size * sizeof(CharT));
  }

 private:
  Archive& m_archive;
};

template <class Archive>
class deserializer {
 public:
  explicit deserializer(Archive& archive) : m_archive(archive) {}

  template <class T>
  T operator()() {
    T value;
    m_archive(value);
    return value;
  }

  template <class CharT>
  void operator()(CharT* value_out, std::size_t value_size) {
    m_archive.loadBinary(value_out, value_size * sizeof(CharT));
  }

 private:
  Archive& m_archive;
};

}  // namespace detail_array_hash

// array_map

template <class Archive, class CharT, class T, class Hash, class KeyEqual,
          bool StoreNullTerminator, class KeySizeT, class IndexSizeT,
          class GrowthPolicy, class Allocator>
void save(Archive& archive,
          const tsl::array_map<CharT, T, Hash, KeyEqual, StoreNullTerminator,
                               KeySizeT, IndexSizeT, GrowthPolicy, Allocator>&
              map) {
  detail_array_hash::serializer<Archive> serial(archive);
  map.serialize(serial);
}

template <class Archive, class CharT, class T, class Hash, class KeyEqual,
          bool StoreNullTerminator, class KeySizeT, class IndexSizeT,
          class GrowthPolicy, class Allocator>
void load(Archive& archive,
          tsl::array_map<CharT, T, Hash, KeyEqual, StoreNullTerminator,
                         KeySizeT, IndexSizeT, GrowthPolicy, Allocator>& map) {
  detail_array_hash::deserializer<Archive> deserial(archive);
  map = tsl::array_map<CharT, T, Hash, KeyEqual, StoreNullTerminator, KeySizeT,
                       IndexSizeT, GrowthPolicy, Allocator>::deserialize(
      deserial, false, map.get_allocator());
}

// array_set

template <class Archive, class CharT, class Hash, class KeyEqual,
          bool StoreNullTerminator, class KeySizeT, class IndexSizeT,
          class GrowthPolicy, class Allocator>
void save(Archive& archive,
          const tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator,
                               KeySizeT, IndexSizeT, GrowthPolicy, Allocator>&
              set) {
  detail_array_hash::serializer<Archive> serial(archive);
  set.serialize(serial);
}

template <class Archive, class CharT, class Hash, class KeyEqual,
          bool StoreNullTerminator, class KeySizeT, class IndexSizeT,
          class GrowthPolicy, class Allocator>
void load(Archive& archive,
          tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator, KeySizeT,
                         IndexSizeT, GrowthPolicy, Allocator>& set) {
  detail_array_hash::deserializer<Archive> deserial(archive);
  set = tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator, KeySizeT,
                       IndexSizeT, GrowthPolicy, Allocator>::deserialize(
      deserial, false, set.get_allocator());
}

}  // namespace cereal

#endif  // TSL_ARRAY_HASH_CEREAL_H
