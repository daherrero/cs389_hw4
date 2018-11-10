#include "cache.hh"
#include <iostream>
#include <functional>
#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <list>

#pragma


using evictor_type = std::function<uint32_t(void)>;
using key_equality = std::function<bool(Cache::key_type, Cache::key_type)>;

// This is a universal hash function adapted from:
// https://en.wikipedia.org/wiki/Universal_hashing and
// djb2 from http://www.cse.yorku.ca/~oz/hash.html
struct DefaultHash {
    u_int32_t operator()(const Cache::key_type &key) const {
        // Initialize p to be large prime number
        u_int32_t p = 5381;
        // Loop over each character of key
        for (auto letter : key) {
            // Bitshift the prime number p, add it to that result
            // add the ASCII value of the letter to that
            p = ((p << 5) + p) + letter;
        }
        return p;
    }
};


struct Cache::Impl {


    index_type maxmem_;
    hash_func hasher_;
    index_type memused_;
    index_type items_in_;
    float max_load_;
    std::unordered_map <key_type, val_type, hash_func> my_cache_;
    std::unordered_map <key_type,index_type, hash_func> my_size_cache_;
    std::list <std::string> key_list_;
    std::string evictor_type_;

    Impl(index_type maxmem,
         hash_func hasher = DefaultHash()) : maxmem_(maxmem), hasher_(hasher), memused_(0),
                                             items_in_(0), max_load_(.5), my_cache_(0, hasher), my_size_cache_(0,hasher),
                                             key_list_(maxmem), evictor_type_("fifo") {
        // Set max load factor for my_cache_ to .5
        my_cache_.max_load_factor(max_load_);
        my_size_cache_.max_load_factor(max_load_);
    }

    ~Impl() = default;


    // Add a <key, value> pair to the cache.
    // If key already exists, it will overwrite the old value.
    // Both the key and the value are to be deep-copied (not just pointer copied).
    // If maxmem capacity is exceeded, sufficient values will be removed
    // from the cache to accomodate the new value.
    int set(key_type key, val_type val, index_type size) {
        if (size > maxmem_) 
        {
            return -1;
        } 
        if (size < 0)
        {
            return -1;
        }
        index_type get_size = 0;
        get(key,get_size);
        if (get_size != 0)
        {
            memused_ -= get_size;
        }
        if ((memused_ + size) <= maxmem_) {
            my_cache_.insert_or_assign(key, val);
            my_size_cache_.insert_or_assign(key,size);
            memused_ += size;
            evictor(evictor_type_, "include", key);
            assert(my_cache_.load_factor() <= my_cache_.max_load_factor());
            return 0;
        } else {
            while ((memused_ + size) > maxmem_)
                evictor(evictor_type_, "evict", key);
        }
        my_cache_.insert_or_assign(key, val);
        my_size_cache_.insert_or_assign(key,size);
        memused_ += size;
        evictor(evictor_type_, "include", key);
        assert(my_cache_.load_factor() <= my_cache_.max_load_factor());
        return 0;
    }

    // Retrieve a pointer to the value associated with key in the cache,
    // or NULL if not found.
    // Sets the actual size of the returned value (in bytes) in val_size.
    val_type get(key_type key, index_type &val_size) {
        val_type point_to_val;

        auto search_KV = (my_cache_).find(key);
        if (search_KV != (my_cache_).end()) 
        {
            // Update value stored in val_size if its in the cache.
            val_size = (uint32_t) sizeof(search_KV->second);

            // Update key list for evictor
            evictor(evictor_type_, "update", key);

            // Set pointer to the value.
            point_to_val = search_KV->second;
        } 

        auto search_size = (my_size_cache_).find(key);
        if (search_size != (my_size_cache_).end())
        {
            val_size = search_size->second;
            return point_to_val;
        } else {
            return NULL;
        }
    }

    // Delete an object from the cache, if it's still there
    int del(key_type key) {
        index_type val_size = 0;
        val_type is_in = get(key, val_size);

        if (is_in == NULL) {
            return -1;
        }
        // Reduce the size of memused_appropriately 
        if (val_size != 0) {
            memused_ -= val_size;
        }
        my_cache_.erase(key);
        my_size_cache_.erase(key);
        evictor(evictor_type_, "erase", key);
    }

    // Returns the amount of memory used by all cache values (not keys).
    index_type space_used() const {
        return memused_;
    }

    void evictor(std::string evictor_type = NULL, std::string action = NULL, key_type key = NULL) {
        if (evictor_type == "fifo") {
            fifo_evictor(action, key);
        } else if (evictor_type == "lru") {
            lru_evictor(action, key);
        } else {
            auto first = my_cache_.begin();
            del(first->first);
        }
    }

    void fifo_evictor(std::string action, key_type key) {
        if (action == "include") {
            if ((std::find(key_list_.begin(), key_list_.end(), key) != key_list_.end())) {
                key_list_.remove(key);
                key_list_.push_front(key);
            } else {
                key_list_.push_front(key);
            }
        } else if (action == "delete") {
            key_list_.remove(key);
        } else if (action == "evict") {
            std::string fifo_key = key_list_.back().c_str();
            key_list_.pop_back();
            del(fifo_key);
        } else {
            return;
        }
    }

    void lru_evictor(std::string action, key_type key) {
        if (action == "include") {
            if ((std::find(key_list_.begin(), key_list_.end(), key) != key_list_.end())) {
                key_list_.remove(key);
                key_list_.push_front(key);
            } else {
                key_list_.push_front(key);
            }
        } else if (action == "delete") {
            key_list_.remove(key);
        } else if (action == "update") {
            key_list_.remove(key);
            key_list_.push_front(key);
        } else if (action == "evict") {
            std::string lru_key = key_list_.back();
            key_list_.pop_back();
            del(lru_key);
        } else {
            return;
        }
    }
};


// Create a new cache object with a given maximum memory capacity.
Cache::Cache(index_type maxmem,
             hash_func hasher) : pImpl_(new Impl(maxmem, hasher)) {
}

Cache::~Cache() {
}


// Add a <key, value> pair to the cache.
// If key already exists, it will overwrite the old value.
// Both the key and the value are to be deep-copied (not just pointer copied).
// If maxmem capacity is exceeded, sufficient values will be removed
// from the cache to accomodate the new value.
int Cache::set(key_type key, val_type val, index_type size) {
    pImpl_->set(key, val, size);
}

// Retrieve a pointer to the value associated with key in the cache,
// or NULL if not found.
// Sets the actual size of the returned value (in bytes) in val_size.
Cache::val_type Cache::get(key_type key, index_type &val_size) const {
    return pImpl_->get(key, val_size);
}

// Delete an object from the cache, if it's still there
int Cache::del(key_type key) {
    pImpl_->del(key);
}

// Compute the total amount of memory used up by all cache values (not keys)
Cache::index_type Cache::space_used() const {
    return pImpl_->space_used();
}