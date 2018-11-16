## CS389 HW4: Lets network.

### David Herrero-Quevedo & Michael Kalange

We used modified versions of Marika and Jill's cache that just add error codes and change
the return types for set and del to int.

#### Setup
Dependencies: libcurl\
To install:
```
sudo apt-get install libcurl4-openssl-dev
sudo apt-get update
```


#### Part 1
Compile server using:\
`g++ -std=c++17 -o server server.cc cache.cc -l boost_system -pthread`\
Run server:\
`./server`\
Compile tests:\
`g++ -std=c++11 -o tests tests_server.cc -lcurl`\
Run tests:\
`./tests`

We used the Crow RESTful-API to establish the server side. It was a relatively straightforward process,although the cmake file included in the repo is garbage, hence the need for the linking above. Once we got past that though, it was just a matter of writing up the methods and ensuring they worked for command line curl requests. Because of the way the server takes arguments, our cache only accepts values of type int or string; they're passed as parameters to lambdas and it's impossible to guess what types are going to be passed by the user. A value of type different than string or int will return a 404.

The new functionality in the header (returning ints) allows us to send specific error codes when a request fails in some way. If the cache insertion fails the method returns an error saying so. If deletion fails because the request isnt in the cache, it returns an error saying so.

#### Part 2

We used libcurl and nlohmann's json library to parse strings returned from the server.
It passes all of our tests.
