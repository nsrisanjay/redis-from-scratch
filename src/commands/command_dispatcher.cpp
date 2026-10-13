#include<string>
#include<cctype>
#include "commands/command_dispatcher.hpp"
#include "protocol/resp_types.hpp"
#include "commands/command_handler.hpp"

using namespace std;

const string INVALID_COMMAND_STRING = "Invalid command";
const string NO_COMMAND = "Null command";

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
        // check if the type of the content is monostate for null array
        if(holds_alternative<monostate>(request.respValue)){
            // if true
            response.dataType = error;
            response.respValue = NO_COMMAND;
            return response;
        }
        else if (holds_alternative<vector<respValueStruct>>(request.respValue))
        { 
            // if respVlaue is vector
            vector<respValueStruct> command = get<vector<respValueStruct>>(request.respValue);
            if (command.size() == 0)
            {
                response.dataType = error;
                response.respValue = NO_COMMAND;
                return response;
            }
            // iterate through all the elements in the vector and confirm if they are bulk strings or not
            for(auto element : command)
            {
                if(element.dataType != bulkString
                 || !holds_alternative<string>(element.respValue))
                {
                    response.dataType = error;
                    response.respValue = INVALID_COMMAND_STRING;
                    return response;
                }
            }
            string opCommand = get<string>(command[0].respValue);
            // convert opCommand into small case
            for (char &c : opCommand)
            {
                c = std::tolower(static_cast<unsigned char>(c));
            }
            if (opCommand == "ping")
                return handlePing(arguments);
            else if (opCommand == "set")
                return handleSet(arguments, *keyspace);
            else if (opCommand == "get")
                return handleGet(arguments, *keyspace);
            else if (opCommand == "del")
                return handleDel(arguments, *keyspace);
            else if (opCommand == "exists")
                return handleExists(arguments, *keyspace);
            else if (opCommand == "type")
                return handleType(arguments, *keyspace);
            else if (opCommand == "dbsize")
                return handleDbsize(arguments, *keyspace);

            return makeUnknownCommandError(opCommand);
        }else{
            // the request's respvalue is not null or not a vector, must be some otther type
            // error invalid command
            response.dataType = error;
            response.respValue = INVALID_COMMAND_STRING;
            return response;
        }
    }
}