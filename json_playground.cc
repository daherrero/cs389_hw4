#include <iostream>
#include <string>
#include "json.hpp"
#include <curl/curl.h>
#include "cache.hh"
#include <sstream>

std::string SERVER_NAME = "0.0.0.0";
std::string PORT = "17017";


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main(void)
{
    Cache::key_type key = "tnto";

    CURL *curl;
    CURLcode res;

    std::stringstream url;
    url << SERVER_NAME << ":" << PORT << "/"<< "key/" << key;
    auto url_str = url.str();

    curl = curl_easy_init();
    if(curl) {
        std::string readBuffer;
        long response_code;
        curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        std::cout << response_code << std::endl;

        if (response_code == 400) {
            std::cout <<"FOLLOWED" << std::endl;
            return NULL;
        }
        auto readBuffer_json = nlohmann::json::parse(readBuffer);
        auto key_name = readBuffer_json.at("key");
        auto pointer_to_val = readBuffer_json.at("value");

        std::stringstream ss;
        ss << pointer_to_val;
        std::string point_string = ss.str();
        unsigned long ul;
        const char* cstr = point_string.c_str();
        sscanf(cstr,"%lx",&ul);
        void * ptv = (void *)(uintptr_t) ul;
    }
    return NULL;

}

//Cache::index_type Cache::space_used() const {

//}


