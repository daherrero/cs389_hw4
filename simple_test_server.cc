//
// Created by David Herrero Quevedo on 11/13/18.
//
#include "testing_client.cc"


int main(){
    Cache new_cache(10000);
    Cache::index_type sized1;
    Cache::index_type sized2;

    sized1 = new_cache.space_used();

    std::cout << sized1 << std::endl;

    const Cache::key_type key1 = "one";
    const Cache::key_type key2 = "two";

    const int val = 1;
    const int val2 = 2;

    const Cache::val_type point1 = &val;
    const Cache::val_type point2 = &val2;

    const Cache::index_type size1 = sizeof(val);
    const Cache::index_type size2 = sizeof(val2);

    new_cache.set(key1, point1, size1);
    new_cache.set(key2, point2, size2);

    sized2 = new_cache.space_used();
    std::cout << sized2 << std::endl;

    new_cache.del(key1);
    sized2 = new_cache.space_used();
    std::cout << sized2 << std::endl;


    new_cache.~Cache();

    return 0;
}