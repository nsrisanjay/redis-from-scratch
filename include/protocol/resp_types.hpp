#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

using namespace std;

enum datatTypes {
    simpleString,
    bulkString,
    integers,
    respArray,
    error
};

enum parserResult{
    COMPLETED,
    INCOMPLETE,
    MALFORMED
};

struct respValueStruct{
    datatTypes dataType;
    
    // dynamic allocation of datatype.
    // access value if you know the type via tyep value = get<type>(variantValue)
    // we can get by index as well
    // we can get by get_if which returns the address of the value and we can get by dereferencing it.
    variant<monostate,int64_t,string,vector<respValueStruct>>respValue;    
};
struct parseResult{
    parserResult parserRes;
    respValueStruct respValue;
    int bytesConsumed;
};