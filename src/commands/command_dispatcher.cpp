#include "commands/command_dispatcher.hpp"
#include "protocol/resp_types.hpp"

using namespace std;

const string INVALID_COMMAND_STRING = "Invalid command";

commandDispatcher::commandDispatcher(Keyspace &store){
        // address of the keyspace stored inside the member *keyspace
        keyspace = &store;
}

respValueStruct commandDispatcher :: execute(const respValueStruct &request){
    respValueStruct response;
    if(request.dataType != respArray)
    {
        // not an array of bulk strings, ie not valid command
        // return an error response saying that incomplete command
        response.dataType = error;
        response.respValue = INVALID_COMMAND_STRING;
        return response;
    }else{
        // iterate through all the elements in the vector
        vector<respValueStruct> command = get<vector<respValueStruct>>(request.respValue);
        if(command.size() == 0){
            response.dataType = error;
            response.respValue = INVALID_COMMAND_STRING;
            return response;
        }
        for(auto element : command){

        }
    }
}