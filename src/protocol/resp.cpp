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

parseResult errorParser(const char *bytes,int length){
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
    respValObject.dataType = simpleString;
    parseResult res;
    if (length == 0)
    {
        res.parserRes = INCOMPLETE;
        res.bytesConsumed = 0;
        return res;
    }
    if(length == 1)
    {
        res.bytesConsumed=1;
        res.parserRes = INCOMPLETE;
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

// int main()
// {
//     char buffer[] = "+934\r\n";
//     // we get this from the tcp server, for now using the arbitrary value;
//     int bufferSize = sizeof(buffer) - 1;
//     // simple string parser
//     int totalBytesConsumed = 0;
//     while(totalBytesConsumed < bufferSize)
//     {
//         // get the first result
//         parseResult res = integerParser(&buffer[totalBytesConsumed],bufferSize-totalBytesConsumed);
//         totalBytesConsumed += res.bytesConsumed;
//     }
//     // testMultipleSimpleStrings();
// }

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





