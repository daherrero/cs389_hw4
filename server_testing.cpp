// server.cc
#include "crow_all.h"
#include "cache.hh"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>

int MAXMEM = 1080;
int PORTNUM = 17017;
std::unique_ptr<Cache> CACHE_PTR;

int main(int argc, char *argv[])
{
    crow::SimpleApp app;

    // Parse command line args
    int c;
    while( ( c = getopt (argc, argv, "m:t:") ) != -1 ) 
    {
        switch(c)
        {
            case 'm':
                if(optarg) MAXMEM = std::atoi(optarg);
                break;
            case 't':
                if(optarg) PORTNUM = std::atoi(optarg) ;
                break;
        }
    }
    
    CACHE_PTR.reset(new Cache(MAXMEM));

    app.route_dynamic("/memsize")
        ([](const crow::request& req) {
            Cache::index_type mem = (*CACHE_PTR).space_used();
            crow::json::wvalue return_json;
            return_json["memused"] = mem;
            return crow::response{return_json};
        });

    app.route_dynamic("/test")
        ([](const crow::request& /*req*/, crow::response& res){
        std::string key = "the";
        Cache::index_type val = 1;
        Cache::index_type val_size = sizeof(val);
        Cache::val_type val_point = &val;
        int set_return = (*CACHE_PTR).set(key,val_point,val_size);
        std::ostringstream os;
        os << set_return;
        res.write(os.str());
        res.end();
    });
    app.route_dynamic("/key/<string>/<int>")
    .methods("POST"_method)
    ([](const crow::request& req,std::string key, int val){
        if (req.method == "POST"_method){
            Cache::val_type val_point = &val;
            uint32_t val_size = sizeof(val);
            int set_return = (*CACHE_PTR).set(key,val_point,val_size);
            Cache::index_type memused = (*CACHE_PTR).space_used();
            crow::json::wvalue return_json;
            return_json["value"] = val;
            return_json["key"] = key;
            return_json["setreturn"] = set_return;
            return_json["memused"] = memused;
            return crow::response{return_json};
            } else {
            return(crow::response(400));
        }
    });
    
    app.route_dynamic("/key/<string>") 
    .methods("DELETE"_method,"GET"_method)
    ([](const crow::request& req,std::string key) {
        if (req.method == "DELETE"_method)
        {
            Cache::index_type delete_return = (*CACHE_PTR).del(key);
            crow::json::wvalue return_json;
            return_json["value"] = delete_return;
            return crow::response{return_json};
            // NEED ERROR CODES HERE
        } else if (req.method == "GET"_method) {
            Cache::index_type sized;
            Cache::val_type the_point = (*CACHE_PTR).get(key, sized);
            const uint32_t &val = *(static_cast<const u_int32_t *>(the_point));
            crow::json::wvalue return_json;
            return_json["value"] = val;
            return_json["key"] = key;
            return crow::response{return_json};
        } else {
            return(crow::response(400));
        }
    });



    app.port(PORTNUM)
        .run();
}