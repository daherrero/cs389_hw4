#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "testing_client.cc"
// g++ -std=c++17 -o server server.cpp cache.cpp -l boost_system -pthread
TEST_CASE("Set/Get","[single-file]")
{
    SECTION("A value in the cache can be retrieved.")
    {
        // Initialize cache and key/value variables/pointers
        const Cache::key_type key1 = "one";
        const int val = 1;
        const Cache::val_type point1 = &val;
        const Cache::index_type size = sizeof(val);
        Cache new_cache(1000);
        Cache::index_type sized;

        // Set the value
        new_cache.set(key1, point1, size);

        // Get the value
        Cache::val_type the_point = new_cache.get(key1, sized);
        const uint32_t &theval = *(static_cast<const u_int32_t *>(the_point));

        // Require the value returned is the initial value
        REQUIRE(theval == val);
    }
    SECTION("The size of the value returned is correct.")
    {
        // Initialize cache and key/value variables/pointers
        const Cache::key_type key1 = "one";
        const int val = 1;
        const Cache::val_type point1 = &val;
        const Cache::index_type size = sizeof(val);
        Cache new_cache(1000);
        Cache::index_type sized;

        // Set the value
        new_cache.set(key1, point1, size);

        // Get the value
        new_cache.get(key1, sized);

        // Require the size of the value returned is the size of the inital value
        REQUIRE(sized == size);
    }
    SECTION("Getting a value not in the cache returns null.")
    {
        // Initialize cache and key/value variables/pointers
        const Cache::key_type key1 = "one";
        Cache new_cache(1000);
        Cache::index_type sized;

        // Get a pointer to the "value"
        Cache::val_type the_point = new_cache.get(key1, sized);

        // Require the pointer is NULL
        REQUIRE(!the_point);
    }
    SECTION("Setting a value with size > the capacity of the cache fails.")
    {
        const Cache::key_type key1 = "one";
        const Cache::key_type key2 = "two";
        const int val = 1;
        const Cache::val_type point1 = &val;
        const Cache::index_type size1 = sizeof(val);
        const Cache::index_type size2 = 10000000;
        Cache new_cache(100);

        // Set the value
        new_cache.set(key1, point1, size1);

        // Get the memory
        Cache::index_type size_before = new_cache.space_used();

        // Try and insert a value with size > capacity
        new_cache.set(key2, point1, size2);

        Cache::index_type size_after = new_cache.space_used();

        // Require the size of the value returned is the size of the inital value
        REQUIRE(size_before == size_after);
    }
    SECTION("Setting a value of a key already in the cache updates space_used and value correctly.")
    {
        // Initialize cache and key/value variables/pointers
        const Cache::key_type key1 = "one";
        const int val = 1;
        const bool val2 = 0;
        const Cache::val_type point1 = &val;
        const Cache::val_type point2 = &val2;
        const Cache::index_type size1 = sizeof(val);
        const Cache::index_type size2 = sizeof(val2);
        Cache new_cache(1000);
        Cache::index_type sized;

        // Set the value
        new_cache.set(key1, point1, size1);

        // Reset it with value of new type
        new_cache.set(key1,point2,size2);

        // Get the new value
        Cache::val_type the_point = new_cache.get(key1, sized);
        const bool &theval = *(static_cast<const bool *>(the_point));

        // Get the memory used
        Cache::index_type memory = new_cache.space_used();

        // Require the value returned is the initial value
        REQUIRE(memory == size2);
        REQUIRE(theval == val2);
    }
}

TEST_CASE("Space_used")
{
    SECTION("Ensure space_used is 0 when no values are inserted.")
    {
        // Initialize cache and key/value variables/pointers
        Cache new_cache(1000);
        Cache::index_type sized;

        // Get space_used
        sized = new_cache.space_used();

        // Require space_used by empty cache is 0
        REQUIRE(sized == 0);
    }
    SECTION("Ensure space_used is correct when cache has k/v's in it.")
    {
        // Initialize cache and key/value variables/pointers
        const Cache::key_type key1 = "one";
        const int val = 1;
        const Cache::val_type point1 = &val;
        const Cache::index_type size = sizeof(val);
        Cache new_cache(1000);
        Cache::index_type sized;

        // Set the value
        new_cache.set(key1, point1, size);

        // Get space_used
        sized = new_cache.space_used();

        // Require space_used is the size of the value we've inserted
        REQUIRE(sized == size);
    }
}

TEST_CASE("Del")
{
    SECTION("Delete removes the key/value from the cache.")
    {
        // Initialize cache and key/value variables/pointers
        const Cache::key_type key1 = "one";
        const int val = 1;
        const Cache::val_type point1 = &val;
        const Cache::index_type size = sizeof(val);
        Cache new_cache(1000);
        Cache::index_type sized = 0;

        // Set the value
        new_cache.set(key1, point1, size);

        // Delete it
        new_cache.del(key1);

        // Get a pointer to the "value"
        Cache::val_type the_point = new_cache.get(key1, sized);

        // Require space used is 0 and pointer is NULL
        REQUIRE(sized == 0);
        REQUIRE(!the_point);
    }
    SECTION("Delete on k/v not in cache has no effect.")
    {
        // Initialize cache and key/value variables/pointers
        const Cache::key_type key1 = "one";
        const Cache::key_type key2 = "two";
        const int val = 1;
        const Cache::val_type point1 = &val;
        const Cache::index_type size = sizeof(val);
        Cache new_cache(1000);
        Cache::index_type sized = size;

        // Set the value
        new_cache.set(key1, point1, size);

        // Delete nonexistent key/value
        new_cache.del(key2);

        // Get pointer to value in cache
        Cache::val_type the_point = new_cache.get(key1, sized);
        const uint32_t &theval = *(static_cast<const u_int32_t *>(the_point));

        // Require the value is still there and the memory used is correct
        REQUIRE(size == new_cache.space_used());
        REQUIRE(theval == val);
    }
}
