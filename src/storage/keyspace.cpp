#include "storage/keyspace.hpp"
#include<optional>
#include<string>
#include<vector>
#include<unordered_map>
#include<cstdint>

using namespace std;

void Keyspace::set(const std::string &key,const std::string &value)
{
    store[key] = value;
    return;
}
optional<std::string> Keyspace::get(const std::string &key)
{
    // if key not present 
    auto it = store.find(key);
    if(it == store.end())
    {
        return std::nullopt;
    }
    return (*it).second;
}

int Keyspace::del(vector<std::string> &keys)
{
    int count = 0;
    for(string key : keys)
    {
        auto it = store.find(key);
        if(it != store.end()) {store.erase(it);count++;}
    }
    return count;
}

bool Keyspace::exists(const std::string &key){
    return store.find(key) != store.end();
}

std::int64_t Keyspace::dbSize()
{
    return (int64_t)store.size();
}