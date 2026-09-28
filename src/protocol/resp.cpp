#include<iostream>
#include<variant>
#include<string>
#include<vector>
#include <cassert>
#include<string.h>

using namespace std;

enum datatTypes {
    simpleString,
    bulkString,
    integers,
    array,
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

parseResult integerParser(const char* bytes, int length);
parseResult simpleStringParser(const char* bytes, int length);
parseResult bulkStringParser(const char* bytes, int length);
parseResult errorParser(const char* bytes, int length);
parseResult arrayParser(const char* bytes, int length);


// helper functions
int dynamicPointerMover(parseResult parserResult,vector<respValueStruct> &vectorStore,char **dynamicPosition)
{
    if(parserResult.parserRes == INCOMPLETE)
        return 0;
    else if(parserResult.parserRes == MALFORMED)
        return 1;
    else{
        // move the dynamicPosition pointer to the next element starting position.
        *dynamicPosition += parserResult.bytesConsumed;
        // extract the number stored in respValue and add it to the vector
        vectorStore.push_back(parserResult.respValue);
    }
    return 2;
}

bool nullArrayChecker(const char *bytes, int length)
{
    if(length < 5)
        return false;
    if(bytes[0] == '*' && bytes[1] == '-' &&
       bytes[2] == '1' && bytes[3] == '\r' && bytes[4] == '\n')
        return true;
    return false;
}

parseResult arrayParser(const char* bytes,int length)
{
    parseResult res;
    respValueStruct respValObject;

    // store vector elements here in this vector
    vector<respValueStruct> vectorStore;

    respValObject.dataType = datatTypes::array;
    
    if (length == 0)
    {
        res.parserRes = INCOMPLETE;
        res.bytesConsumed = 0;
        return res;
    }
    if(bytes[0] != '*')
    {
        res.bytesConsumed = 1;
        res.parserRes = MALFORMED;
        return res;
    }
    if(nullArrayChecker(bytes,length))
    {
        res.bytesConsumed = 5;
        res.parserRes = COMPLETED;
        respValObject.respValue = monostate{};
        res.respValue = respValObject;
        return res;
    }
    else{
        //if bytes[1] is not a number telling us how ,many elements in the array
        int64_t numberOfElements = 0;
        char* pos = (char*)memchr(bytes+1,static_cast<int>('\r'),static_cast<size_t>(length-1));
        if(pos == nullptr)
        {
            // means that the the \r is not found
            res.bytesConsumed = length; // all bytes consumed
            res.parserRes = INCOMPLETE;
            return res;
        }
        else if(pos-bytes == length-1)
        {
            // this means that the the character is found at the end of the string
            // res.parserRes would be incomeplete
            res.bytesConsumed = pos-bytes+1;
            res.parserRes = INCOMPLETE;
            return res;
        }
        else if(*(pos+1) != '\n')
        {
            // request is malfoprmed something after the \r
            res.parserRes = MALFORMED;
            res.bytesConsumed = -1*(bytes-pos)+2;
            return res;
        }
        else{
            // means \r\n both are pressent and the request is complete
            // compute the number of elements in the array
            if(pos == bytes+1)
            {
                res.parserRes = MALFORMED;
                res.bytesConsumed = 1;
                return res;
            }
            for(const char*ch = bytes+1;ch<pos;ch++)
            {
                if(isdigit(*ch))
                {
                    numberOfElements = numberOfElements*10 + (*ch - '0');
                    continue;
                }
                // malformed request
                res.parserRes = MALFORMED;
                return res;
            }
            // now parse the incoming numberOfelements Lines
            char *initPos = pos+2;
            char *dynamicPosition = initPos;
            while(numberOfElements--)
            {
                char ch = *dynamicPosition;
                switch(ch){
                    // if element is integer
                    case ':':
                    {
                        int newLength = length - (dynamicPosition - bytes);
                        parseResult parserResult = integerParser(dynamicPosition, newLength);
                        int status = dynamicPointerMover(parserResult, vectorStore, &dynamicPosition);
                        if (status == 0)
                            return parserResult;
                        else if (status == 1)
                            return parserResult;
                        break;
                    }
                    // if element is simple string
                    case '+':
                    {
                        int newLength = length - (dynamicPosition - bytes);
                        parseResult parserResult = simpleStringParser(dynamicPosition, newLength);
                        int status = dynamicPointerMover(parserResult, vectorStore, &dynamicPosition);
                        if (status == 0)
                            return parserResult;
                        else if (status == 1)
                            return parserResult;
                        break;
                    }
                    // if element is a bulk string
                    case '$':
                    {
                            int newLength = length - (dynamicPosition - bytes);
                            parseResult parserResult = bulkStringParser(dynamicPosition, newLength);
                            int status = dynamicPointerMover(parserResult, vectorStore, &dynamicPosition);
                            if (status == 0)
                                return parserResult;
                            else if (status == 1)
                                return parserResult;
                            break;
                    }
                    // if element is another array (nested arrays)
                    case '*':
                    {
                        int newLength = length - (dynamicPosition - bytes);
                        parseResult parserResult = arrayParser(dynamicPosition, newLength);
                        int status = dynamicPointerMover(parserResult, vectorStore, &dynamicPosition);
                        if (status == 0)
                            return parserResult;
                        else if (status == 1)
                            return parserResult;
                        break;
                    }
                    // if lement is of type error
                    case '-':
                    {
                        int newLength = length - (dynamicPosition - bytes);
                        parseResult parserResult = errorParser(dynamicPosition, newLength);
                        int status = dynamicPointerMover(parserResult, vectorStore, &dynamicPosition);
                        if (status == 0)
                            return parserResult;
                        else if (status == 1)
                            return parserResult;
                        break;
                    }
                    // if no matching elemetn tyoe return a malformed reques
                    default:
                    {
                        res.parserRes = MALFORMED;
                        res.bytesConsumed = 1;
                        return res;
                    }
                }
            }
            res.bytesConsumed = dynamicPosition - bytes;;
            res.parserRes = COMPLETED;
            respValObject.respValue = vectorStore;
            res.respValue = respValObject;
            return res;
            
        }
    }
}

parseResult errorParser(const char *bytes,int length)
{
    parseResult res;
    respValueStruct respValObject;
    respValObject.dataType = error;
    // assume error value as string
    respValObject.respValue = "";
    if(length == 0)
    {
        res.parserRes = INCOMPLETE;
        res.bytesConsumed = 0;
        return res;
    }
    if(bytes[0] != '-')
    {
        // then malfoemed command
        res.bytesConsumed = 1;
        res.parserRes = MALFORMED;
        return res;
    }
    else{
        // first find pos of the clrf characters.
        // char CRLF[] = "\r\n";
        // find the pos of the crlf \r
        const char* pos = (const char *)memchr(bytes+1,(int)'\r',(size_t)length-1);
        if(pos == nullptr)
        {
            // \r not found so incomplete
            res.parserRes = INCOMPLETE;
            res.bytesConsumed = length;
            return res;
        }else{
            // found the \r char at position now check if the next char is \n or is the \r is the last byte
            if(pos-bytes == length-1)
            {
                // then it means last charcter is the \r which means incomplete request
                res.parserRes = INCOMPLETE;
                res.bytesConsumed = length;
                return res;
            }else if(bytes[pos-bytes+1] != '\n')
            {
                // then some other chracter after the \r then request is malformed
                res.parserRes = MALFORMED;
                return res;
            }
            else{
                // \r\n bothe are present in the curent request itself
                // extract the error message
                string errorMessage = "";
                for(const char *ch= bytes + 1;ch<pos;ch++)
                {
                    errorMessage += *ch;
                }
                respValObject.respValue = errorMessage;
                res.respValue = respValObject;
                res.parserRes = COMPLETED;
                res.bytesConsumed = pos-bytes+2;
                return res;
            }
        }
    }
}

parseResult bulkStringParser(const char *bytes,int length)
{
    parseResult res;
    respValueStruct respValObject;
    respValObject.dataType = bulkString;
    respValObject.respValue = "";
    // char CRLF[] = "\r\n";
    res.respValue = respValObject;
    if(length == 0)
    {
        res.parserRes = INCOMPLETE;
        res.bytesConsumed = 0;
        // res.parserRes = result;
        return res;
    }
    if(bytes[0] != '$')
    {
        res.parserRes = MALFORMED;
        cout<<"malformed"<<endl;
        return res;
    }
    else{

        int64_t sizeOfStringInBytes = 0;
        const char* pos = (const char *)memchr(bytes,(int)'\r',(size_t)length);

        if(pos != nullptr && (pos-bytes) < length-1){
            if(bytes[pos-bytes+1] != '\n')
            {
                res.parserRes = MALFORMED;
                return res;
            }else{
                if(pos == bytes+1)
                {
                    res.parserRes = MALFORMED;
                    return res;
                }
                // FIX: Handle RESP null bulk string: $-1\r\n
                if(bytes[1] == '-')
                {
                    // $-1 is the only valid negative bulk-string length.
                    if(pos - bytes != 3 || bytes[2] != '1')
                    {
                        res.parserRes = MALFORMED;
                        return res;
                    }
                    // FIX: Represent RESP null using monostate.
                    res.respValue.respValue = monostate{};
                    res.parserRes = COMPLETED;
                    // "$-1\r\n" = 5 bytes
                    res.bytesConsumed = pos - bytes + 2;
                    return res;
                }
                for(const char *ch=bytes+1;ch<pos;ch++)
                {
                    if(isdigit(*ch))
                        sizeOfStringInBytes  = sizeOfStringInBytes*10 + (*ch-'0');
                    else{
                        // not a numeric character, break saying that its malformed
                        res.parserRes = MALFORMED;
                        res.bytesConsumed = ch-bytes+1;
                        return res;
                    }
                }
            }
        }else{
            res.parserRes = INCOMPLETE;
            res.bytesConsumed = length;
            return res;
        }
        // now extract the sizeOfStringInBytes from the pos+2
        string contentOfBulkString = "";
        for(const char* ch = pos+2;ch<pos+2+sizeOfStringInBytes;ch++)
        {
            if(ch-bytes < length)
                contentOfBulkString += *ch;
            else{
                res.parserRes = INCOMPLETE;
                res.bytesConsumed = length;
                return res;
            }
        }
        if(pos + 2 - bytes + sizeOfStringInBytes + 2 > length)
        {
            res.parserRes = INCOMPLETE;
            res.bytesConsumed = length;
            return res;
        }
        if(bytes[pos+2-bytes+sizeOfStringInBytes] == '\r' &&
            bytes[pos+2-bytes+sizeOfStringInBytes+1] == '\n')
        {
            res.parserRes = COMPLETED;
            res.respValue.respValue = contentOfBulkString;
            res.bytesConsumed = pos+2-bytes+sizeOfStringInBytes+2;
            return res;
        }else{
            res.parserRes = MALFORMED;
            return res;
        }
    }
    return res;
}

parseResult simpleStringParser(const char *bytes,int length)
{
    parserResult result;
    respValueStruct respValObject;
    respValObject.respValue = "";
    respValObject.dataType = simpleString;
    parseResult res;
    if (length == 0)
    {
        res.parserRes = INCOMPLETE;
        res.bytesConsumed = 0;
        return res;
    }
    if(bytes[0] != '+')
    {
        result = MALFORMED;
        cout<<"malformed";
        res.parserRes = result;
        return res;
    }
    else{
        int index = 1;
        int bytesReadByTCPServer = length;//abitrary value
        while(index < bytesReadByTCPServer){
            if(bytes[index] == '\r')
            {
                if(index+1<bytesReadByTCPServer && bytes[index+1] == '\n')
                {
                    result = COMPLETED;
                    res.parserRes = result;
                    // '+' + string length + crlf
                    res.bytesConsumed = 1+get<string>(respValObject.respValue).size()+2;
                    res.respValue = respValObject;
                    break;
                }else if(index+1 >= bytesReadByTCPServer)
                {
                    result = INCOMPLETE;
                    res.bytesConsumed = index+1;
                    res.parserRes=result;
                    res.respValue = respValObject;
                    break;
                }else{
                    result = MALFORMED;
                    res.bytesConsumed = index+1;
                    res.parserRes = result;
                    res.respValue = respValObject;
                    break;
                }
            }
            get<string>(respValObject.respValue) += bytes[index++];
        }
        if(index == bytesReadByTCPServer)
        {
            res.parserRes = INCOMPLETE;
            res.respValue = respValObject;
            res.bytesConsumed = index;
        }
    }
    return res;
}

parseResult integerParser(const char *bytes,int length){
    parserResult result;
    respValueStruct respValObject;
    respValObject.respValue = "";
    respValObject.dataType = integers;
    parseResult res;
    if (length == 0)
    {
        res.parserRes = INCOMPLETE;
        res.bytesConsumed = 0;
        return res;
    }
    if(bytes[0] != ':')
    {
        result = MALFORMED;
        cout<<"malformed";
        res.parserRes = result;
        return res;
    }
    else{
        int index = 2;
        int bytesReadByTCPServer = length;//abitrary value
        if (bytes[1] == '-')
        {
            respValObject.respValue = "-";
        }else if(isdigit(bytes[1]) == true){
            get<string>(respValObject.respValue) += bytes[1];
        }
        else
        {
            result = MALFORMED;
            res.bytesConsumed = index;
            res.parserRes = result;
            res.respValue = respValObject;
            return res;
        }
        while(index < bytesReadByTCPServer){
            if(bytes[index] == '\r')
            {
                if(index+1<bytesReadByTCPServer && bytes[index+1] == '\n')
                {
                    result = COMPLETED;
                    res.parserRes = result;
                    // '+' + string length + crlf
                    res.bytesConsumed = 1+get<string>(respValObject.respValue).size()+2;
                    // convert to integer after complete value is extracted.
                    respValObject.respValue = stoll(get<string>(respValObject.respValue));
                    respValObject.dataType = integers;
                    res.respValue = respValObject;
                    break;
                }else if(index+1 >= bytesReadByTCPServer)
                {
                    result = INCOMPLETE;
                    res.bytesConsumed = index+1;
                    res.parserRes=result;
                    res.respValue = respValObject;
                    break;
                }else{
                    result = MALFORMED;
                    res.bytesConsumed = index+1;
                    res.parserRes = result;
                    res.respValue = respValObject;
                    break;
                }
            }
            if(isdigit(bytes[index]) && index>1)
                get<string>(respValObject.respValue) += bytes[index++];
            else{
                result = MALFORMED;
                res.bytesConsumed = index+1;
                res.parserRes = result;
                res.respValue = respValObject;
                break;
            }
        }
        if(index == bytesReadByTCPServer)
        {
            res.parserRes = INCOMPLETE;
            res.respValue = respValObject;
            res.bytesConsumed = index;
        }
    }
    return res;
}

int main()
{
    char buffer[] = ":934\r\n:123456789\r\n:-42\r\n";

    int bufferSize = sizeof(buffer) - 1;
    int totalBytesConsumed = 0;

    while(totalBytesConsumed < bufferSize)
    {
        parseResult res = integerParser(
            &buffer[totalBytesConsumed],
            bufferSize - totalBytesConsumed
        );

        assert(res.parserRes == COMPLETED);
        cout << get<int64_t>(res.respValue.respValue) << '\n';
        totalBytesConsumed += res.bytesConsumed;
    }
}

