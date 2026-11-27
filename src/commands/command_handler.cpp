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
    returnValue.dataType = bulkString;
    // extract the array of command arguments whic is array od respValueStruct
    vector<respValueStruct> commandArray = getCommandArray(request);
    // command: PING AIKillsUs
    // extract "AIKillsUs" and return it
    if(commandArray.size() > 2)
    {
        returnValue.dataType = error;
        returnValue.respValue = "ERR wrong number of arguments for 'ping' command";
        return returnValue;
    }
    if(commandArray.size()==2)
    {
        string content = get<string>(commandArray[1].respValue);
        returnValue.respValue = content;
        return returnValue;
    }
    returnValue.dataType = simpleString;
    returnValue.respValue = "PONG";
    return returnValue;
}

respValueStruct commandDispatcher :: handleSet(const respValueStruct &request, Keyspace *keyspace){
    respValueStruct returnValue;
    returnValue.dataType = simpleString;

    // extract the array of commands
    vector<respValueStruct> commandArray = getCommandArray(request);
    if(commandArray.size()!=3)
    {
        returnValue.dataType = error;
        returnValue.respValue = "ERR wrong number of arguments for 'set' command";
        return returnValue;
    }
    // command SET situation AIKillsUs
    // extract key and value and store in keyspace
    string key = get<string>(commandArray[1].respValue);
    string value = get<string>(commandArray[2].respValue);

    // set the key and value to the keyspace
    keyspace->set(key,value);
    returnValue.respValue = "OK";
    return returnValue;
}

respValueStruct commandDispatcher :: handleGet(const respValueStruct &request, Keyspace *keyspace){
    respValueStruct returnValue;
    returnValue.dataType = bulkString;
    // extract the array of commands
    vector<respValueStruct> commandArray = getCommandArray(request);
    if(commandArray.size()!=2)
    {
        returnValue.dataType = error;
        returnValue.respValue = "ERR wrong number of arguments for 'get' command";
        return returnValue;
    }
    // command: GET situation
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
    respValueStruct returnValue;
    returnValue.dataType = integers;

    // extract the array of commands
    vector<respValueStruct> commandArray = getCommandArray(request);
    // command: EXISTS situation
    // return 0 if no, else 1
    // if multiple keys are queried in same command, like command: EXISTS situation key2
    // the output must be number of keys that are existing in keyspace.
    if(commandArray.size() < 2)
    {
        returnValue.respValue = "ERR wrong number of arguments for 'exists' command";
        returnValue.dataType = error;
        return returnValue;
    }
    int count = 0;
    for(int i=1;i<commandArray.size();i++)
    {
        string key = get<string>(commandArray[i].respValue);
        if(keyspace->exists(key))
            count++;
    }
   returnValue.respValue = count;
   return returnValue;
}

respValueStruct commandDispatcher :: handleDel(const respValueStruct &request, Keyspace *keyspace){
    respValueStruct returnValue;
    returnValue.dataType = integers;

    // extract the array of commands
    vector<respValueStruct> commandArray = getCommandArray(request);
    // command: EXISTS situation
    // return 0 if no, else 1
    // if multiple keys are queried in same command, like command: EXISTS situation key2
    // the output must be number of keys that are existing in keyspace.
    if(commandArray.size() < 2)
    {
        returnValue.respValue = "ERR wrong number of arguments for 'del' command";
        returnValue.dataType = error;
        return returnValue;
    }
    // int count = 0;
    vector<string> keys;
    for(int i=1;i<commandArray.size();i++)
    {
        keys.push_back(get<string>(commandArray[i].respValue));
    }
    int countOfDeletedKeys = keyspace->del(keys);
    returnValue.respValue = countOfDeletedKeys;
    return returnValue;
}

respValueStruct commandDispatcher :: handleType(const respValueStruct &request, Keyspace *keyspace){
    respValueStruct returnValue;
    returnValue.dataType = simpleString;

    // extract the array of commands
    vector<respValueStruct> commandArray = getCommandArray(request);
    if(commandArray.size()!=2)
    {
        returnValue.dataType = error;
        returnValue.respValue = "ERR wrong number of arguments for 'type' command";
        return returnValue;
    }
    string key = get<string>(commandArray[1].respValue);
    if(keyspace->exists(key))
    {
        returnValue.respValue = "string";
        return returnValue;
    }
    returnValue.respValue = "none";
    return returnValue;
}

respValueStruct commandDispatcher :: handleDbSize(const respValueStruct &request, Keyspace *keyspace){
    respValueStruct returnValue;
    returnValue.dataType = integers;

    // extract the array of commands
    vector<respValueStruct> commandArray = getCommandArray(request);
    if(commandArray.size()!=1)
    {
        returnValue.dataType = error;
        returnValue.respValue = "ERR wrong number of arguments for 'dbsize' command";
        return returnValue;
    }
    int64_t numberOfKeys = keyspace->dbSize();
    returnValue.respValue = numberOfKeys;
    return returnValue;
}
