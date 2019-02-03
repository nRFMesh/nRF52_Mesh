#ifndef _STR_MAP_HPP_
#define _STR_MAP_HPP_

#include <map>
#include <string>

//typedef std::map<std::string,std::string> strmap_t;

class strmap_c
{
    public:
        strmap_c();
        strmap_c(const char*msg,uint8_t size);
    public:
        bool hasgot(std::string key);
        //std::string& operator [] (std::string key);
    public:
        //strmap_t dict;
};


#endif /*_STR_MAP_HPP_*/
