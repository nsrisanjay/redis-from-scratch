#include "protocol/resp_protocol.hpp"
#include "protocol/resp_parser.hpp"

#include <iostream>
#include <cstring>

using namespace std;

RespProtocol::RespProtocol(int capacity)
{
    this->capacity = capacity;
    this->buffer = new char[capacity];
    this->bytesInBuffer = 0;
    this->startIndex = 0;
}

RespProtocol::~RespProtocol()
{
    delete[] buffer;
}

void RespProtocol::feed(const char* bytes, int length)
{
    // Incoming data itself is larger than the entire buffer.
    if(length > capacity)
    {
        cout<<"Input too large for buffer"<<endl;
        return;
    }

    // Check whether the new data fits in the remaining buffer space.
    if(bytesInBuffer + length > capacity)
    {
        // Number of unprocessed bytes currently in the buffer.
        int remainingBytes = bytesInBuffer - startIndex;
        // Move unprocessed bytes to the beginning of the buffer.(compaction)
        // returns void *
        // memmove(destination pointer,source pointer,no.of bytes to move from src pointer to dest pointer)
        memmove(buffer,buffer + startIndex,remainingBytes);
        bytesInBuffer = remainingBytes;
        startIndex = 0;

        // Even after compaction, the new data must fit.
        if(bytesInBuffer + length > capacity)
        {
            cout<<"Buffer is full"<<endl;
            return;
        }
    }

    // Append newly received TCP bytes.
    memcpy(buffer + bytesInBuffer,bytes,length);
    bytesInBuffer += length;

    // Process every complete RESP value currently in the buffer.
    while(startIndex < bytesInBuffer)
    {
        const char* current = buffer + startIndex;
        int remainingBytes = bytesInBuffer - startIndex;
        parseResult res;

        switch(current[0])
        {
            case '+':
                res = simpleStringParser(current, remainingBytes);
                break;
            case '-':
                res = errorParser(current, remainingBytes);
                break;
            case ':':
                res = integerParser(current, remainingBytes);
                break;
            case '$':
                res = bulkStringParser(current, remainingBytes);
                break;
            case '*':
                res = arrayParser(current, remainingBytes);
                break;
            default:
                cout << "Protocol error" << endl;
                return;
        }

        if(res.parserRes == COMPLETED)
        {
            // Mark these bytes as processed.
            startIndex += res.bytesConsumed;

            // TODO:
            // Do something with res.respValue.
            // cout<<"completed parsing the message"<<endl;
        }
        else if(res.parserRes == INCOMPLETE)
        {
            // We don't have enough bytes yet.
            // Keep everything from startIndex onward.
            // cout<<"waiting for more data"<<endl;
            break;
        }
        else if(res.parserRes == MALFORMED)
        {
            // cout << "Protocol error" << endl;
            return;
        }
    }
}