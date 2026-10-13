#pragma once
#include "protocol/resp_types.hpp"
#include "storage/keyspace.hpp"

class commandDispatcher{
    public:
    respValueStruct execute(const respValueStruct &request);
    commandDispatcher(Keyspace &store);
    private:
    Keyspace *keyspace;
    respValueStruct handlePing(const respValueStruct &request, Keyspace *keyspace);
    respValueStruct handleSet(const respValueStruct &request, Keyspace *keyspace);
    respValueStruct handleGet(const respValueStruct &request, Keyspace *keyspace);
    respValueStruct handleDel(const respValueStruct &request, Keyspace *keyspace);
    respValueStruct handleExists(const respValueStruct &request, Keyspace *keyspace);
    respValueStruct handleType(const respValueStruct &request, Keyspace *keyspace);
    respValueStruct handleDbSize(const respValueStruct &request, Keyspace *keyspace);
};
