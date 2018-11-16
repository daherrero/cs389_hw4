#include "cache.hh"
#include "json.hpp"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <string>
#include <list>

std::string SERVER_NAME = "0.0.0.0";
std::string PORT = "17017";

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

using evictor_type = std::function<uint32_t(void)>;

struct Cache::Impl {
    index_type maxmem_;
    hash_func hasher_;
    index_type memused_;
    std::list<std::string> key_list;
    std::unordered_map<std::string, std::tuple<val_type, index_type, index_type>> unorderedmap_;
public:
    Impl(index_type maxmem, hash_func hasher)
            : maxmem_(maxmem), hasher_(hasher), memused_(0), unorderedmap_(){
    }
    ~Impl(){
        for (Cache::key_type key : key_list) {
            std::stringstream url;
            url << SERVER_NAME << ":" << PORT << "/" << "key/" << key;
            auto url_str = url.str();

            CURL *curl;
            CURLcode res;
            curl = curl_easy_init();
            if(curl) {
                std::string readBuffer;
                curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
            }
        }
        key_list.clear();
    }

    int set(key_type key, val_type val, index_type size){
        std::stringstream url;
        std::string * str_ptr = (std::string *)val;
        url << SERVER_NAME << ":" << PORT << "/" << "key" << "/" << key << "/" << *str_ptr;
        auto url_str = url.str();

        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        if(curl) {
            key_list.push_back(key);

            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            return 0;
        }
        return -1;
    }

    val_type get(key_type key, index_type& val_size) const {
        std::stringstream url;
        url << SERVER_NAME << ":" << PORT << "/"<< "key/" << key;
        auto url_str = url.str();

        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        if(curl) {
             std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            std::stringstream sss;
            sss << readBuffer;
            std::string code = sss.str().substr(0,3);

            if (code.compare("400") == 0) {
                return NULL;
            }

            auto readBuffer_json = nlohmann::json::parse(readBuffer);
            auto key_name = readBuffer_json.at("key");
            std::string s = readBuffer_json.at("value");
            Cache::val_type val_point = &s;

            val_size = sizeof(s);

            return val_point;
        }
        return NULL;
    }

    // This deletes a (key, tuple) entry from the map
    int del(key_type key){
        std::stringstream url;
        url << SERVER_NAME << ":" << PORT << "/" << "key/" << key;
        auto url_str = url.str();

        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        if(curl) {
            key_list.remove(key);

            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            return 0;
        }
        return -1;
    }

    index_type space_used() const {
        std::stringstream url;
        url << SERVER_NAME << ":" << PORT << "/" << "memsize";
        auto url_str = url.str();

        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        if (curl) {
            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            auto readBuffer_json = nlohmann::json::parse(readBuffer);
            Cache::index_type memused = readBuffer_json.at("memused");

            return memused;
        }
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

