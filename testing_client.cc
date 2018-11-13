#include <iostream>
#include <string>
#include "json.hpp"
#include <curl/curl.h>
#include "cache.hh"


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main(void)
{
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "0.0.0.0:17017/memsize");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        auto readBuffer_json = nlohmann::json::parse(readBuffer);
        Cache::index_type memused = readBuffer_json.at("memused");
        std::cout << memused << std::endl;

        return memused;
    }
    return NULL;
}

//Cache::index_type Cache::space_used() const {

//}


