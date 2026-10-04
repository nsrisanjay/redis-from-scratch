#pragma once

#include "resp_types.hpp"
#include<queue>

class RespProtocol
{
private:
    char* buffer;
    int capacity;
    int bytesInBuffer;
    int startIndex;
    bool protocolError = false;

    std::queue<respValueStruct> completedValues;
public:
    RespProtocol(int capacity);
    ~RespProtocol();
    bool hasError() const;
    void feed(const char* bytes, int length);
    bool hasValue();
    respValueStruct getValue();
};