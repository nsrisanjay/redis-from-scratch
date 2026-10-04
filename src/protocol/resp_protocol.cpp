#include "protocol/resp_protocol.hpp"
#include "protocol/resp_parser.hpp"

#include <iostream>
#include <cstring>
#include <limits>
#include<queue>


using namespace std;

RespProtocol::RespProtocol(int capacity)
{
    this->capacity = capacity > 0 ? capacity : 1;
    this->buffer = new char[this->capacity];
    this->bytesInBuffer = 0;
    this->startIndex = 0;
}

bool RespProtocol::hasError() const
{
    return protocolError;
}

RespProtocol::~RespProtocol()
{
    delete[] buffer;
}

bool RespProtocol::hasValue()
{
    return !completedValues.empty();
}

respValueStruct RespProtocol::getValue()
{
    respValueStruct value = completedValues.front();
    completedValues.pop();

    return value;
}

void RespProtocol::feed(const char* bytes, int length)
{
    if (protocolError || length < 0)
    {
        protocolError = true;
        return;
    }

    // Number of unprocessed bytes currently in the buffer.
    int remainingBytes = bytesInBuffer - startIndex;
    if (startIndex > 0)
    {
        // Move unprocessed bytes to the beginning of the buffer (compaction).
        // memmove(destination, source, number of bytes to move).
        memmove(buffer, buffer + startIndex, remainingBytes);
        bytesInBuffer = remainingBytes;
        startIndex = 0;
    }

    if (length > numeric_limits<int>::max() - bytesInBuffer)
    {
        protocolError = true;
        return;
    }

    // Even after compaction, make sure the new data fits by growing the buffer.
    int requiredCapacity = bytesInBuffer + length;
    if (requiredCapacity > capacity)
    {
        int newCapacity = capacity;
        while (newCapacity < requiredCapacity)
        {
            if (newCapacity > numeric_limits<int>::max() / 2)
            {
                newCapacity = requiredCapacity;
                break;
            }
            newCapacity *= 2;
        }

        char* newBuffer = new char[newCapacity];
        memcpy(newBuffer, buffer, bytesInBuffer);
        delete[] buffer;
        buffer = newBuffer;
        capacity = newCapacity;
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
                protocolError = true;
                return;
        }

        if(res.parserRes == COMPLETED)
        {
            // pus h the resp value into the queue
            completedValues.push(res.respValue);
            // Mark these bytes as processed.
            startIndex += res.bytesConsumed;

            // TODO:
            // Do something with res.respValue.
            cout<<"completed parsing the message adn returning the resp value : "<<endl;

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
            cout << "Protocol error" << endl;
            protocolError = true;
            return;
        }
    }
}
