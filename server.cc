// server.cc
// g++ server.cc -std=c++11 -lboost_system -lboost_thread
#include "crow_all.h"
#include "cache.hh"
#include <iostream>
#include <unistd.h>
#include <cstdlib>

Cache server_cache(1080);


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



    CROW_ROUTE(app,"/key/<string>")
        .methods(crow::HTTPMethod::GET, crow::HTTPMethod::HEAD, crow::HTTPMethod::DELETE)
        ([&](const crow::request& req, std::string key)
        {
            if (req.method == crow::HTTPMethod::GET) {
                return crow::response(405);
            }
            else if (req.method == crow::HTTPMethod::HEAD) {
                return crow::response(405);
            }
            else if (req.method == crow::HTTPMethod::DELETE) {
                return crow::response(405);
            }
            else {
                return crow::response(405);
            }
        });



    CROW_ROUTE(app, "/shutdown")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req)
        {
            return crow::response(405);

        });

    app.loglevel(crow::LogLevel::DEBUG);

    app.port(18080)
            .run();

}