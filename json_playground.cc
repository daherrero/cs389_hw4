#include <iostream>
#include <string>
#include "json.hpp"

int main(){
    nlohmann::json j = "{\"memsize\":4}"_json;
    std::cout << j.at("memsize");
    return 0;
}