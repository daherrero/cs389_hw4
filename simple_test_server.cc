//
// Created by David Herrero Quevedo on 11/13/18.
//
#include "testing_client.cc"


int main(){
    Cache new_cache(10000);
    Cache::index_type sized;
    sized = new_cache.space_used();
    std::cout << sized << std::endl;
    const Cache::key_type key1 = "one";
    const int val = 1;
    const Cache::val_type point1 = &val;
    const Cache::index_type size = sizeof(val);

    new_cache.set(key1, point1, size);
    sized = new_cache.space_used();
    std::cout << sized << std::endl;
    new_cache.del(key1);
    sized = new_cache.space_used();
    std::cout << sized << std::endl;
}