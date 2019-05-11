#pragma once

#include <string.h>
#include <string>

class CFunctions
{
public:
    bool compare(const char* chT1, const char* chT2)
    {
        if(strlen(chT1) != strlen(chT2))
            return false;
        for(unsigned int i=0; i<strlen(chT1); i++)
        {
            if(chT1[i] != chT2[i])
                return false;
        }
        
        return true;
    }

    std::string removeSpace(std::string str)
    {
        for(unsigned int i=0; i<str.length(); i++)
        {
            if(str[i] == ' ')
                str.erase(i, 1);
        }
        return str;
    }

    void ignoreCase(std:: string &str)
    {
        for(unsigned int i=0; i<str.length(); i++)
        {
            int num = static_cast<int>(str[i]); 
            if(str[i] >= 65 && str[i] <= 90)
                str[i] = (char)num + 32;
        }
    }
            
};


