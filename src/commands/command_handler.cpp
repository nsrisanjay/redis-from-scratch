#include "commands/command_dispatcher.hpp"
#include "protocol/resp_types.hpp"

// helper functions

vector<respValueStruct> getCommandArray(const respValueStruct &request)
{
    vector<respValueStruct> commandArray = get<vector<respValueStruct>>(request.respValue);
    return commandArray;
}



respValueStruct commandDispatcher :: handlePing(const respValueStruct &request){
    respValueStruct returnValue;
    returnValue.dataType = simpleString;
    // extract the array of command arguments whic is array od respValueStruct
    vector<respValueStruct> commandArray = getCommandArray(request);
    // command: PING AIKillsUs
    // extract "AIKillsUs" and return it
    string content = get<string>(commandArray[1].respValue);
    returnValue.respValue = content;
    return returnValue;
}

respValueStruct commandDispatcher :: handleSet(const respValueStruct &request, Keyspace *keyspace){
    respValueStruct returnValue;
    returnValue.dataType = simpleString;

    // extract the array of commands
    vector<respValueStruct> commandArray = getCommandArray(request);
    // command SET situation AIKillsUs
    // extract key and value and store in keyspace
    string key = get<string>(commandArray[1].respValue);
    string value = get<string>(commandArray[2].respValue);

    // set the key and value to the keyspace
    keyspace->set(key,value);
    returnValue.respValue = "OK!";
    return returnValue;
}

respValueStruct commandDispatcher :: handleGet(const respValueStruct &request, Keyspace *keyspace){
    respValueStruct returnValue;
    returnValue.dataType = bulkString;

    // extract the array of commands
    vector<respValueStruct> commandArray = getCommandArray(request);
    // command GET situation
    // get value if key exists, else return null bulk string
    string key = get<string>(commandArray[1].respValue);
    auto value = keyspace->get(key);
    // if value is returned to be a null from optional<string> return, then monosstate
    // else string value as respvalue
    if(value == nullopt)
    {
        returnValue.respValue = monostate{};
        return returnValue;
    }
    returnValue.respValue = value.value();
    return returnValue;
}

respValueStruct commandDispatcher :: handleExists(const respValueStruct &request, Keyspace *keyspace){
    
}

respValueStruct commandDispatcher :: handleDel(const respValueStruct &request, Keyspace *keyspace){
    
}

respValueStruct commandDispatcher :: handleType(const respValueStruct &request, Keyspace *keyspace){
    
}

respValueStruct commandDispatcher :: handleDbSize(const respValueStruct &request, Keyspace *keyspace){
    
}
