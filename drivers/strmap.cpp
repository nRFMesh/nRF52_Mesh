
#include "strmap.hpp"

#include <string.h>

std::string TakeParseTo(std::string &str,char sep)
{
	std::string Parsed;
	size_t first = str.find_first_of(sep);
	if(first != std::string::npos)
	{
		Parsed = str.substr(0 , first);
		str = str.substr(first+1 ,str.length());
	}
	else//not found, then take what's left
	{
		Parsed = str;
		str = "";
	}
	return Parsed;
}

strmap_c::strmap_c(const char*msg,uint8_t size)
{
	int max = 0;
	std::string str(msg,size);
    topic = TakeParseTo(str,'>');
	while((!str.empty()) && (max < 20))
	{
		std::string arg_name = TakeParseTo(str,':');
		dict[arg_name] = TakeParseTo(str,';');
		max++;
	}
}


bool strmap_c::has(std::string key)
{
    return (dict.find(key) != dict.end());
}


std::string& strmap_c::operator [] (std::string key)
{
    return dict[key];
}