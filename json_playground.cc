#include <iostream>
#include <string>
#include "json.hpp"
#include <curl/curl.h>
#include "cache.hh"
#include <sstream>

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

    std::stringstream url;
    url << "0.0.0.0:17017/key/" << key;
    std::cout << url.str();
    auto url_str = url.str();
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        return 0;
    }
    return NULL;

}

//Cache::index_type Cache::space_used() const {

//}


