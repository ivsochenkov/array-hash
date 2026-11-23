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
#ifndef TSL_ARRAY_HASH_GLAZE_H
#define TSL_ARRAY_HASH_GLAZE_H

// Glaze is an optional C++23 dependency. Keep it out of the core headers so
// consumers that do not use Glaze retain the library's C++11 requirements.
#include <glaze/beve.hpp>
#include <glaze/bson.hpp>
#include <glaze/cbor.hpp>
#include <glaze/json.hpp>
#include <glaze/jsonb.hpp>
#include <glaze/msgpack.hpp>
#include <glaze/toml.hpp>
#include <glaze/yaml.hpp>

// EETF is an optional Glaze feature and is available only when Glaze itself
// has been built with its Erlang support enabled.
#if defined(GLZ_ENABLE_EETF)
#include <glaze/eetf.hpp>
#endif

#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "array_map.h"
#include "array_set.h"

namespace glz {

// array_hash's map/set classes deliberately do not expose the iterator layout
// expected by Glaze's standard-container concepts. Mark them as custom types so
// the format-specific from/to specializations below own their wire layout.
template <class CharT, class MappedT, class Hash, class KeyEqual,
          bool StoreNullTerminator, class KeySizeT, class IndexSizeT,
          class GrowthPolicy, class Allocator>
struct meta<tsl::array_map<CharT, MappedT, Hash, KeyEqual,
                          StoreNullTerminator, KeySizeT, IndexSizeT,
                          GrowthPolicy, Allocator>> {
  static constexpr bool custom_read = true;
  static constexpr bool custom_write = true;
};

template <class CharT, class Hash, class KeyEqual, bool StoreNullTerminator,
          class KeySizeT, class IndexSizeT, class GrowthPolicy,
          class Allocator>
struct meta<tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator,
                          KeySizeT, IndexSizeT, GrowthPolicy, Allocator>> {
  static constexpr bool custom_read = true;
  static constexpr bool custom_write = true;
};

}  // namespace glz

// The adapter intentionally uses Glaze's native object/array wire shapes:
//   * array_map -> object with string keys and mapped values
//   * array_set -> array of strings
//
// For maps, shared_ptr is used only as a non-owning serialization view. It lets
// Glaze serialize the mapped object without copying it, including move-only
// mapped types. On input Glaze allocates each mapped value through shared_ptr;
// the adapter then moves it into the destination array_map.
#define TSL_ARRAY_HASH_GLAZE_DEFINE_MAP_ADAPTER(Format)                    \
  template <class CharT, class MappedT, class Hash, class KeyEqual,        \
            bool StoreNullTerminator, class KeySizeT, class IndexSizeT,    \
            class GrowthPolicy, class Allocator>                           \
  struct from<                                                             \
      Format,                                                              \
      tsl::array_map<CharT, MappedT, Hash, KeyEqual, StoreNullTerminator,   \
                     KeySizeT, IndexSizeT, GrowthPolicy, Allocator>> {     \
    using map_type =                                                       \
        tsl::array_map<CharT, MappedT, Hash, KeyEqual,                     \
                       StoreNullTerminator, KeySizeT, IndexSizeT,          \
                       GrowthPolicy, Allocator>;                           \
    template <auto Opts>                                                   \
    static void op(map_type& value, is_context auto&& ctx, auto&& it,      \
                   auto&& end) {                                           \
      static_assert(std::is_same<CharT, char>::value,                      \
                    "Glaze object keys require array_hash CharT = char"); \
      std::map<std::string, std::shared_ptr<MappedT>> entries;             \
      parse<Format>::template op<Opts>(entries, ctx, it, end);              \
      if (bool(ctx.error)) {                                               \
        return;                                                            \
      }                                                                     \
      for (const auto& entry : entries) {                                  \
        if (!entry.second) {                                               \
          ctx.error = error_code::syntax_error;                            \
          return;                                                          \
        }                                                                   \
      }                                                                     \
      value.clear();                                                       \
      for (auto& entry : entries) {                                        \
        value.insert(entry.first, std::move(*entry.second));               \
      }                                                                     \
    }                                                                       \
  };                                                                        \
  template <class CharT, class MappedT, class Hash, class KeyEqual,        \
            bool StoreNullTerminator, class KeySizeT, class IndexSizeT,    \
            class GrowthPolicy, class Allocator>                           \
  struct to<                                                               \
      Format,                                                              \
      tsl::array_map<CharT, MappedT, Hash, KeyEqual, StoreNullTerminator,   \
                     KeySizeT, IndexSizeT, GrowthPolicy, Allocator>> {     \
    using map_type =                                                       \
        tsl::array_map<CharT, MappedT, Hash, KeyEqual,                     \
                       StoreNullTerminator, KeySizeT, IndexSizeT,          \
                       GrowthPolicy, Allocator>;                           \
    template <auto Opts>                                                   \
    static void op(const map_type& value, is_context auto&& ctx, auto&& b, \
                   auto&& ix) {                                            \
      static_assert(std::is_same<CharT, char>::value,                      \
                    "Glaze object keys require array_hash CharT = char"); \
      std::map<std::string, std::shared_ptr<const MappedT>> entries;       \
      for (auto it = value.cbegin(); it != value.cend(); ++it) {           \
        std::shared_ptr<const MappedT> mapped(                             \
            &it.value(), [](const MappedT*) {});                           \
        entries.emplace(std::string(it.key(), it.key_size()),              \
                        std::move(mapped));                                \
      }                                                                     \
      serialize<Format>::template op<Opts>(entries, ctx, b, ix);            \
    }                                                                       \
  };

#define TSL_ARRAY_HASH_GLAZE_DEFINE_SET_ADAPTER(Format)                    \
  template <class CharT, class Hash, class KeyEqual,                       \
            bool StoreNullTerminator, class KeySizeT, class IndexSizeT,    \
            class GrowthPolicy, class Allocator>                           \
  struct from<                                                             \
      Format,                                                              \
      tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator, KeySizeT, \
                     IndexSizeT, GrowthPolicy, Allocator>> {              \
    using set_type =                                                       \
        tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator,         \
                       KeySizeT, IndexSizeT, GrowthPolicy, Allocator>;     \
    template <auto Opts>                                                   \
    static void op(set_type& value, is_context auto&& ctx, auto&& it,      \
                   auto&& end) {                                           \
      static_assert(std::is_same<CharT, char>::value,                      \
                    "Glaze string arrays require array_hash CharT = char");\
      std::vector<std::string> entries;                                    \
      parse<Format>::template op<Opts>(entries, ctx, it, end);              \
      if (bool(ctx.error)) {                                               \
        return;                                                            \
      }                                                                     \
      value.clear();                                                       \
      for (const auto& entry : entries) {                                  \
        value.insert(entry);                                               \
      }                                                                     \
    }                                                                       \
  };                                                                        \
  template <class CharT, class Hash, class KeyEqual,                       \
            bool StoreNullTerminator, class KeySizeT, class IndexSizeT,    \
            class GrowthPolicy, class Allocator>                           \
  struct to<                                                               \
      Format,                                                              \
      tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator, KeySizeT, \
                     IndexSizeT, GrowthPolicy, Allocator>> {              \
    using set_type =                                                       \
        tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator,         \
                       KeySizeT, IndexSizeT, GrowthPolicy, Allocator>;     \
    template <auto Opts>                                                   \
    static void op(const set_type& value, is_context auto&& ctx,           \
                   auto&& b, auto&& ix) {                                  \
      static_assert(std::is_same<CharT, char>::value,                      \
                    "Glaze string arrays require array_hash CharT = char");\
      std::vector<std::string> entries;                                    \
      entries.reserve(value.size());                                       \
      for (auto it = value.cbegin(); it != value.cend(); ++it) {           \
        entries.emplace_back(it.key(), it.key_size());                    \
      }                                                                     \
      serialize<Format>::template op<Opts>(entries, ctx, b, ix);            \
    }                                                                       \
  };

namespace tsl {
namespace glaze_detail {

// BSON requires a document at the root. Keep the set payload as an array in a
// single, named field so the logical set representation remains an array.
struct bson_set_wire {
  std::vector<std::string> values;
};

}  // namespace glaze_detail
}  // namespace tsl

namespace glz {
template <>
struct meta<tsl::glaze_detail::bson_set_wire> {
  using T = tsl::glaze_detail::bson_set_wire;
  static constexpr auto value = object("values", &T::values);
};

TSL_ARRAY_HASH_GLAZE_DEFINE_MAP_ADAPTER(JSON)
TSL_ARRAY_HASH_GLAZE_DEFINE_MAP_ADAPTER(JSONB)
TSL_ARRAY_HASH_GLAZE_DEFINE_MAP_ADAPTER(BEVE)
TSL_ARRAY_HASH_GLAZE_DEFINE_MAP_ADAPTER(CBOR)
TSL_ARRAY_HASH_GLAZE_DEFINE_MAP_ADAPTER(MSGPACK)
TSL_ARRAY_HASH_GLAZE_DEFINE_MAP_ADAPTER(BSON)
TSL_ARRAY_HASH_GLAZE_DEFINE_MAP_ADAPTER(TOML)
TSL_ARRAY_HASH_GLAZE_DEFINE_MAP_ADAPTER(YAML)
#if defined(GLZ_ENABLE_EETF)
TSL_ARRAY_HASH_GLAZE_DEFINE_MAP_ADAPTER(EETF)
#endif

TSL_ARRAY_HASH_GLAZE_DEFINE_SET_ADAPTER(JSON)
TSL_ARRAY_HASH_GLAZE_DEFINE_SET_ADAPTER(JSONB)
TSL_ARRAY_HASH_GLAZE_DEFINE_SET_ADAPTER(BEVE)
TSL_ARRAY_HASH_GLAZE_DEFINE_SET_ADAPTER(CBOR)
TSL_ARRAY_HASH_GLAZE_DEFINE_SET_ADAPTER(MSGPACK)
TSL_ARRAY_HASH_GLAZE_DEFINE_SET_ADAPTER(TOML)
TSL_ARRAY_HASH_GLAZE_DEFINE_SET_ADAPTER(YAML)
#if defined(GLZ_ENABLE_EETF)
TSL_ARRAY_HASH_GLAZE_DEFINE_SET_ADAPTER(EETF)
#endif

// A BSON value must be a document, not a bare array. Store the logical set
// array in a single "values" field for BSON only.
template <class CharT, class Hash, class KeyEqual, bool StoreNullTerminator,
          class KeySizeT, class IndexSizeT, class GrowthPolicy,
          class Allocator>
struct from<
    BSON,
    tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator, KeySizeT,
                   IndexSizeT, GrowthPolicy, Allocator>> {
  using set_type =
      tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator, KeySizeT,
                     IndexSizeT, GrowthPolicy, Allocator>;

  template <auto Opts>
  static void op(set_type& value, is_context auto&& ctx, auto&& it,
                 auto&& end) {
    static_assert(std::is_same<CharT, char>::value,
                  "Glaze BSON string arrays require array_hash CharT = char");
    tsl::glaze_detail::bson_set_wire wire;
    parse<BSON>::template op<Opts>(wire, ctx, it, end);
    if (bool(ctx.error)) {
      return;
    }
    value.clear();
    for (const auto& entry : wire.values) {
      value.insert(entry);
    }
  }
};

template <class CharT, class Hash, class KeyEqual, bool StoreNullTerminator,
          class KeySizeT, class IndexSizeT, class GrowthPolicy,
          class Allocator>
struct to<
    BSON,
    tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator, KeySizeT,
                   IndexSizeT, GrowthPolicy, Allocator>> {
  using set_type =
      tsl::array_set<CharT, Hash, KeyEqual, StoreNullTerminator, KeySizeT,
                     IndexSizeT, GrowthPolicy, Allocator>;

  template <auto Opts>
  static void op(const set_type& value, is_context auto&& ctx, auto&& b,
                 auto&& ix) {
    static_assert(std::is_same<CharT, char>::value,
                  "Glaze BSON string arrays require array_hash CharT = char");
    tsl::glaze_detail::bson_set_wire wire;
    wire.values.reserve(value.size());
    for (auto it = value.cbegin(); it != value.cend(); ++it) {
      wire.values.emplace_back(it.key(), it.key_size());
    }
    serialize<BSON>::template op<Opts>(wire, ctx, b, ix);
  }
};
}  // namespace glz

#undef TSL_ARRAY_HASH_GLAZE_DEFINE_MAP_ADAPTER
#undef TSL_ARRAY_HASH_GLAZE_DEFINE_SET_ADAPTER

#endif  // TSL_ARRAY_HASH_GLAZE_H
