#pragma once
#include "protocol/resp_types.hpp"
#include "storage/keyspace.hpp"

class commandDispatcher{
    public:
    respValueStruct execute(const respValueStruct &request);
    commandDispatcher(Keyspace &store);
    private:
    Keyspace *keyspace;
    
};
