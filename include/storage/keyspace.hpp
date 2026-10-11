#pragma once

#include<unordered_map>
#include<string>
#include<optional>
#include<vector>
#include<cstdint>

class Keyspace{
    private:
        std::unordered_map<std::string,std::string>store;
    public:
        void set(const std::string &key,const std::string &value);
        std::optional<std::string> get(const std::string &key);
        int del(std::vector<std::string> &keys);
        bool exists(const std::string &key);
        std::int64_t dbSize();
};