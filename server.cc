// server.cc
// Mike Kalange and David Herrero 
#include "crow_all.h"
#include "cache.hh"
#include "json.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <time.h>

// GLOBALS
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
            // Memory arg
            case 'm':
                if(optarg) MAXMEM = std::atoi(optarg);
                break;
            // Port number arg
            case 't':
                if(optarg) PORTNUM = std::atoi(optarg) ;
                break;
        }
    }
    
    //Initialize the cache pointer with a new cache of size MAXMEM
    CACHE_PTR.reset(new Cache(MAXMEM));

    // memsize GET
    // No errors other than 404 which get handled by the Crow API
    app.route_dynamic("/memsize")
    .methods("GET"_method)
    ([](const crow::request& req) {
        if (req.method == "GET"_method){
            // Call the cache and get its space_used
            Cache::index_type mem = (*CACHE_PTR).space_used();
            
            crow::json::wvalue return_json;
            return_json["memused"] = mem;

            return crow::response{return_json};
        
        } else {
            return(crow::response(400));
        }
    });
   
    // key PUT
    app.route_dynamic("/key/<string>/<string>")
    .methods("PUT"_method)
    ([](const crow::request& req,std::string key, std::string val){
        if (req.method == "PUT"_method){
            std::string s = val;
            int n = s.length();  
      
            // declaring character array 
            char char_array[n+1];  
      
            // copying the contents of the  
            // string to char array 
            strcpy(char_array, s.c_str());  

            Cache::val_type val_point = &char_array;

            Cache::index_type val_size = sizeof(val);



            int set_return = (*CACHE_PTR).set(key,val_point,val_size);
            
            // returns error if insertion fails
            if (set_return != 0) {
                return(crow::response(400));
            }
            
            crow::json::wvalue return_json;
            return_json["value"] = val;
            return_json["key"] = key;
            
            return crow::response{return_json};
            
            } else {
            return(crow::response(400));
        }
    });
    
    // key DELETE, GET, HEAD
    app.route_dynamic("/key/<string>") 
    .methods("DELETE"_method,"GET"_method,"HEAD"_method)
    ([](const crow::request& req,std::string key) {
        // Delete method removes key/val if in cache
        if (req.method == "DELETE"_method)
        {
            Cache::index_type delete_return = (*CACHE_PTR).del(key);

            if (delete_return != 0)
            {
                return(crow::response(400));
            }
            return crow::response(200);
        
        // Get method returns string of pointer to value
        } else if (req.method == "GET"_method) {
            Cache::index_type sized;
            auto the_point = (*CACHE_PTR).get(key, sized);
            
            if (the_point == NULL)
            {
                return(crow::response(400));
            }

            char *cptr = (char *)(the_point);
	        std::string out_string(cptr);

            crow::json::wvalue return_json;
            std::cout << out_string << "OUT_STRING" << std::endl;
            return_json["value"] = out_string;
            return_json["key"] = key;
            
            return crow::response{return_json};
        // Head method returns json
        } else if (req.method == "HEAD"_method) {
            // crows default header has most of the relevant parameters, we just needed to add these ones
            crow::response resp;
            Cache::index_type space = (*CACHE_PTR).space_used();
            
            resp.add_header("Accept","key:string, value:string");
            resp.add_header("Accept-Charset","utf-8");
            resp.add_header("Content-Type","text/plain");
            resp.add_header("Memory Used",std::to_string(space));
            return resp;

        } else {
            return(crow::response(400));
        }
    });

    app.route_dynamic("/shutdown")
    .methods("POST"_method)
    ([&app](const crow::request& req) {
        if (req.method == "POST"_method) {
            // this cleans up the server and exits, returning a message to the caller
            app.stop();
            return(crow::response(200));
        }
    });

    // opens the port specified in arguments or default if unspecified
    app.port(PORTNUM)
        .run();
}