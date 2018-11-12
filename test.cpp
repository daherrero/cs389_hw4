 #include "cache.cpp"
 #include <iostream>
 int main(){
     const Cache::key_type key1 = "one";
        const Cache::key_type key2 = "two";
        const int val = 1;
        const Cache::val_type point1 = &val;
        const Cache::index_type size = sizeof(val);
        Cache new_cache(1000);
        Cache::index_type sized = size;
            
        // Set the value
        int return_val = new_cache.set(key1, point1, size);
        Cache::val_type the_point = new_cache.get(key1, sized);
        std::cout << the_point << "  " << point1 << "  " << return_val;

 }
