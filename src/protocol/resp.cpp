#include<iostream>
#include<variant>
#include<string>
#include<vector>
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
    variant<int,string,vector<respValueStruct>>respValue;    
};
struct parseResult{
    parserResult parserRes;
    respValueStruct respValue;
    int bytesConsumed;
};

int main()
{
    // simple string parser
    char bytes[] = "+sanzayyy\r\n";
    // char bytes[] = "+OK\r\n";
    parserResult result;
    respValueStruct respValObject;
    respValObject.respValue = "";
    respValObject.dataType = simpleString;
    parseResult res;
    if(bytes[0] != '+')
    {
        result = MALFORMED;
        cout<<"malformed";
        return 0;
    }
    else{
        int index = 1;
        int bytesReadByTCPServer = 6;//abitrary value
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
        cout<<"parsing done"<<endl;
        cout<<res.bytesConsumed<<endl;
        cout<<res.respValue.dataType<<endl;
        cout<<get<string>(res.respValue.respValue)<<endl;
        cout<<res.parserRes<<endl;
    }
}