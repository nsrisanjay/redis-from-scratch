#pragma once

#include "resp_types.hpp"

class RespProtocol
{
private:
    char* buffer;
    int capacity;
    int bytesInBuffer;
    int startIndex;

public:
    RespProtocol(int capacity);
    ~RespProtocol();

    void feed(const char* bytes, int length);
};