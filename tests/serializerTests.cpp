#include <iostream>
#include <string>
#include <vector>

#include "protocol/resp_serializer.hpp"

using namespace std;

bool verify(string testName, string expected, string actual)
{
    if(expected == actual)
    {
        cout << "[PASS] " << testName << endl;
        return true;
    }

    cout << "[FAIL] " << testName << endl;
    cout << "Expected: " << expected << endl;
    cout << "Actual:   " << actual << endl;
    cout << "Expected length: " << expected.size() << endl;
    cout << "Actual length:   " << actual.size() << endl;

    return false;
}


bool testSimpleString()
{
    respValueStruct value;

    value.dataType = simpleString;
    value.respValue = string("OK");

    string expected = "+OK\r\n";

    return verify(
        "Simple string",
        expected,
        serializer(value)
    );
}


bool testError()
{
    respValueStruct value;

    value.dataType = error;
    value.respValue = string("ERR unknown command");

    string expected = "-ERR unknown command\r\n";

    return verify(
        "Error",
        expected,
        serializer(value)
    );
}


bool testInteger()
{
    respValueStruct value;

    value.dataType = integers;
    value.respValue = int64_t(12345);

    string expected = ":12345\r\n";

    return verify(
        "Integer",
        expected,
        serializer(value)
    );
}


bool testNegativeInteger()
{
    respValueStruct value;

    value.dataType = integers;
    value.respValue = int64_t(-42);

    string expected = ":-42\r\n";

    return verify(
        "Negative integer",
        expected,
        serializer(value)
    );
}


bool testBulkString()
{
    respValueStruct value;

    value.dataType = bulkString;
    value.respValue = string("hello");

    string expected = "$5\r\nhello\r\n";

    return verify(
        "Bulk string",
        expected,
        serializer(value)
    );
}


bool testEmptyBulkString()
{
    respValueStruct value;

    value.dataType = bulkString;
    value.respValue = string("");

    string expected = "$0\r\n\r\n";

    return verify(
        "Empty bulk string",
        expected,
        serializer(value)
    );
}


bool testNullBulkString()
{
    respValueStruct value;

    value.dataType = bulkString;
    value.respValue = monostate{};

    string expected = "$-1\r\n";

    return verify(
        "Null bulk string",
        expected,
        serializer(value)
    );
}


bool testEmptyArray()
{
    respValueStruct value;

    value.dataType = respArray;
    value.respValue = vector<respValueStruct>{};

    string expected = "*0\r\n";

    return verify(
        "Empty array",
        expected,
        serializer(value)
    );
}


bool testNullArray()
{
    respValueStruct value;

    value.dataType = respArray;
    value.respValue = monostate{};

    string expected = "*-1\r\n";

    return verify(
        "Null array",
        expected,
        serializer(value)
    );
}


bool testArray()
{
    respValueStruct first;
    first.dataType = bulkString;
    first.respValue = string("SET");

    respValueStruct second;
    second.dataType = bulkString;
    second.respValue = string("name");

    respValueStruct third;
    third.dataType = bulkString;
    third.respValue = string("Sanjay");

    respValueStruct array;
    array.dataType = respArray;
    array.respValue = vector<respValueStruct>{
        first,
        second,
        third
    };

    string expected =
        "*3\r\n"
        "$3\r\nSET\r\n"
        "$4\r\nname\r\n"
        "$6\r\nSanjay\r\n";

    return verify(
        "Array",
        expected,
        serializer(array)
    );
}


bool testNestedArray()
{
    respValueStruct first;
    first.dataType = bulkString;
    first.respValue = string("hello");

    respValueStruct second;
    second.dataType = bulkString;
    second.respValue = string("world");

    respValueStruct innerArray;
    innerArray.dataType = respArray;
    innerArray.respValue = vector<respValueStruct>{
        first,
        second
    };

    respValueStruct outerArray;
    outerArray.dataType = respArray;
    outerArray.respValue = vector<respValueStruct>{
        innerArray
    };

    string expected =
        "*1\r\n"
        "*2\r\n"
        "$5\r\nhello\r\n"
        "$5\r\nworld\r\n";

    return verify(
        "Nested array",
        expected,
        serializer(outerArray)
    );
}


int main()
{
    int passed = 0;
    int total = 0;

    total++;
    if(testSimpleString()) passed++;

    total++;
    if(testError()) passed++;

    total++;
    if(testInteger()) passed++;

    total++;
    if(testNegativeInteger()) passed++;

    total++;
    if(testBulkString()) passed++;

    total++;
    if(testEmptyBulkString()) passed++;

    total++;
    if(testNullBulkString()) passed++;

    total++;
    if(testEmptyArray()) passed++;

    total++;
    if(testNullArray()) passed++;

    total++;
    if(testArray()) passed++;

    total++;
    if(testNestedArray()) passed++;

    cout << "\n" << passed << "/" << total << " tests passed." << endl;

    return passed == total ? 0 : 1;
}