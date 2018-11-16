/*
Jillian James and Marika Swanberg
This is the implementation file for cache.hh called cache.cc

Run with the command:
g++ -Wall -Wextra -Werror -pedantic cache.cc test.cc -o test

On a machine using:
OS: Ubuntu 17.10
Memory: 3.9 GiB
Processor: Intel Core i5-7267U CPU @3.10GHz
OS Type: 64-bit
Disk 66.3 GB
*/


#include "cache.hh"
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <tuple>

using evictor_type = std::function<uint32_t(void)>;

struct Cache::Impl {
	index_type maxmem_;
	evictor_type evictor_;
	hash_func hasher_;
	index_type memused_;
	index_type newest_;
	std::unordered_map<std::string, std::tuple<val_type, index_type, index_type>> unorderedmap_;
public:
	Impl(index_type maxmem, hash_func hasher) 
	: maxmem_(maxmem), evictor_(), hasher_(hasher), memused_(0), newest_(0), unorderedmap_(){
	}
	~Impl(){
		auto it = unorderedmap_.begin();
		while (it != unorderedmap_.end()) {
			delete[] static_cast<const char*>(std::get<0>(it->second));
			it = unorderedmap_.erase(it);
		}
	}

	int set(key_type key, val_type val, index_type size){
		// First check if key already has a value in the cache. 
		// Delete old tuple if necessary
		if (size > maxmem_ || size < 0){
		return -1;
		}
		index_type tmp = 1;
		if (get(key, tmp) != nullptr || tmp != 0) {
			del(key);
		}
		// Set the new value
		val_type copy = new char[size];
		std::memcpy((void*)copy, val, size);
		std::tuple<val_type, index_type, index_type> entry = std::make_tuple((void*)copy, size, newest_);
		while ((memused_ + size) > maxmem_) {
			evictor();
		}
		this->unorderedmap_[key] = entry;
		memused_+= size;
		newest_ += 1;
		return 0;
	}

	val_type get(key_type key, index_type& val_size) const {
		std::tuple<val_type, index_type, index_type> entry;
		try{
			entry = unorderedmap_.at(key);
		} catch(const std::out_of_range& oor) {
			val_size = 0; 	// set val_size to 0 if we don't find the value
			return NULL;
		}
		
		val_size = std::get<1>(entry);
		return std::get<0>(entry);
	}
	
	// This deletes a (key, tuple) entry from the map
	int del(key_type key){
		index_type val_size = 0;
		auto val = static_cast<const char*>(get(key, val_size));
		if (val == NULL) {
			return -1;
		}
		unorderedmap_.erase(key);
		delete[] val;
		memused_ -= val_size;
		return 0;
	}

	index_type space_used() const{
		return memused_;
	}


	void evictor() {
		std::string oldest_key;
		index_type oldest_age = newest_;
		for( auto& pair: unorderedmap_){
			std::tuple<val_type, index_type, index_type> entry = unorderedmap_.at(pair.first); // entry = (value, age) tuple
			index_type age =  std::get<2>(entry); // get the age
			if (age <= oldest_age){
				oldest_age = age;
				oldest_key = pair.first;
			}
		}
		del(oldest_key);
	
	}

};

Cache::Cache(index_type maxmem,
        hash_func hasher) 
	: pImpl_(new Impl(maxmem,hasher))
 {

}

Cache::~Cache(){
	// we don't need anything here because pimpl is a unique_ptr
}

// Add a <key, value> pair to the cache.
// If key already exists, it will overwrite the old value.
// Both the key and the value are to be deep-copied (not just pointer copied).
// If maxmem capacity is exceeded, sufficient values will be removed
// from the cache to accomodate the new value.
int Cache::set(key_type key, val_type val, index_type size){
	return pImpl_->set(key,val,size);
}

// Retrieve a pointer to the value associated with key in the cache,
// or NULL if not found.
// Sets the actual size of the returned value (in bytes) in val_size
Cache::val_type Cache::get(key_type key, index_type& val_size) const {
	return pImpl_->get(key,val_size);
}

// Delete an object from the cache, if it's still there
int Cache::del(key_type key){
	return pImpl_->del(key);
}

// Compute the total amount of memory used up by all cache values (not keys)
Cache::index_type Cache::space_used() const {
	return pImpl_->space_used();
}
