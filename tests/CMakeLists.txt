cmake_minimum_required(VERSION 3.10)

add_executable(tsl_array_hash_tests "main.cpp" 
                                    "array_bucket_test.cpp" 
                                    "array_map_tests.cpp" 
                                    "array_set_tests.cpp" 
                                    "policy_tests.cpp")

target_compile_features(tsl_array_hash_tests PRIVATE cxx_std_11)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(tsl_array_hash_tests PRIVATE -Werror -Wall -Wextra -Wold-style-cast -DTSL_DEBUG -UNDEBUG)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(tsl_array_hash_tests PRIVATE /bigobj /WX /W3 /DTSL_DEBUG /UNDEBUG)
endif()

# Boost.Test is used by the base test suite. Boost.Serialization is only
# required when its optional adapter tests are enabled.
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_TEST_COMPONENTS filesystem unit_test_framework)
if(BUILD_BOOST_SERIALIZATION_TESTS)
    list(APPEND BOOST_TEST_COMPONENTS serialization)
endif()
find_package(Boost 1.75 REQUIRED CONFIG COMPONENTS ${BOOST_TEST_COMPONENTS})
target_link_libraries(tsl_array_hash_tests PRIVATE Boost::unit_test_framework)

if(BUILD_CEREAL_TESTS)
    find_package(cereal CONFIG QUIET)

    if(TARGET cereal::cereal)
        set(ARRAY_HASH_CEREAL_TARGET cereal::cereal)
    elseif(TARGET cereal)
        set(ARRAY_HASH_CEREAL_TARGET cereal)
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/cereal/include/cereal/cereal.hpp")
        # The git submodule is header-only; no Cereal build is needed.
        add_library(array_hash_cereal_submodule INTERFACE)
        target_include_directories(array_hash_cereal_submodule SYSTEM INTERFACE
            "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/cereal/include")
        set(ARRAY_HASH_CEREAL_TARGET array_hash_cereal_submodule)
    else()
        message(FATAL_ERROR
            "BUILD_CEREAL_TESTS=ON, but Cereal was not found. Install Cereal or "
            "initialize the third_party/cereal git submodule.")
    endif()

    target_sources(tsl_array_hash_tests PRIVATE "array_hash_cereal_tests.cpp")
    target_link_libraries(tsl_array_hash_tests PRIVATE ${ARRAY_HASH_CEREAL_TARGET})
endif()

if(BUILD_BOOST_SERIALIZATION_TESTS)
    target_sources(tsl_array_hash_tests PRIVATE "array_hash_boost_tests.cpp")
    target_link_libraries(tsl_array_hash_tests PRIVATE Boost::serialization)
endif()

# tsl::array_hash
target_link_libraries(tsl_array_hash_tests PRIVATE tsl::array_hash)  

ADD_TEST(tsl_array_hash_test tsl_array_hash_tests)

