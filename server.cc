// server.cc
// g++ server.cc -std=c++11 -lboost_system -lboost_thread
#include "crow_all.h"
#include "cache.hh"
#include <iostream>
#include <unistd.h>
#include <cstdlib>


int main(int argc, char *argv[])
{
    crow::SimpleApp app;

    // Initialize default values for maxmem and portnum
    int maxmem = 1080;
    int portnum = 17017;

    // Parse command line args
    int c;
    while( ( c = getopt (argc, argv, "m:t:") ) != -1 )
    {
        switch(c)
        {
            case 'm':
                if(optarg) maxmem = std::atoi(optarg);
                break;
            case 't':
                if(optarg) portnum = std::atoi(optarg) ;
                break;
        }
    }

    Cache server_cache(maxmem);


    CROW_ROUTE(app,"/key/<Cache::key_type>")
        .methods("GET" _method, "HEAD" _method, "DELETE" _method)
    ([server_cache](const crow::request& req, Cache::key_type key)
    {
        if (req.method == crow::HTTPMethod::GET) {

        } else if (req.method == crow::HTTPMethod::HEAD) {

        } else if (req.method == crow::HTTPMethod::DELETE) {
            server_cache.del(key);
        } else {
            return crow::response(405);
        }
    })

    CROW_ROUTE(app, "/key/<Cache::key_type>/<Cache::val_type>")
        .methods("PUT" _method)
    ([server_cache](const crow::request& req, Cache::key_type key, Cache::val_type val)
    {

    })

    CROW_ROUTE(app, "/memsize")
        .methods("GET" _method)
    ([server_cache](const crow::request& req)
    {

    })

    CROW_ROUTE(app, "/shutdown")
        .methods("POST" _method)
    ([server_cache](const crow::request& req)
    {

    })

    app.loglevel(crow::LogLevel::DEBUG);

    app.port(18080)
            .run();

}