#include<string>
#include "protocol/resp_serializer.hpp"



std::string serializer(respValueStruct respValueObject)
{
    datatTypes dataType = respValueObject.dataType;
    switch(dataType){
        // simplestring is the datatype
        case 0:{
            string value = get<string>(respValueObject.respValue);
            string serializedString = "+";
            serializedString += value;
            serializedString += "\r\n";
            return serializedString;
        }
        // bulkstring is the datatype
        case 1:{
                if(respValueObject.respValue.index() == 0)
                {
                    // monostate - null bu;k string
                     return "$-1\r\n";
                }
                string value = get<string>(respValueObject.respValue);
                string serializedString = "$";
                serializedString += to_string(value.size());
                serializedString += "\r\n";
                serializedString += value;
                serializedString += "\r\n";
                return serializedString;
           
        }
        // integers is the datatype
        case 2:{
            int64_t value = get<int64_t>(respValueObject.respValue);
            string serializedString = ":";
            serializedString += to_string(value);
            serializedString += "\r\n";
            return serializedString;
        }
        // arrays is the datatype
        case 3:{
            if (respValueObject.respValue.index() == 0)
            {
                // monostate - array is a null array
                return "*-1\r\n";
            }
            string serializedString = "*";
            vector<respValueStruct>v = get<vector<respValueStruct>>(respValueObject.respValue);
            serializedString += to_string(v.size());
            serializedString += "\r\n";
            for(auto element:v)
            {
                serializedString += serializer(element);
            }
            return serializedString; 
        }
        // errors is the datatype
        case 4:{
            string value = get<string>(respValueObject.respValue);
            string serializedString = "-";
            serializedString += value;
            serializedString += "\r\n";
            return serializedString;
        }
        default:{
            return "No serializer found for the dataType....";
        }
    }
}