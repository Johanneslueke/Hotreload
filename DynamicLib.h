//
// Created by johannes on 10/12/16.
//

#ifndef HOTRELOAD_DYNAMICLIB_H
#define HOTRELOAD_DYNAMICLIB_H


#include <string>
#include "hotreload.h"

class DynamicLib{
public:

    DynamicLib(const std::string& Libname);
    virtual ~DynamicLib();

    void unload();
    void reload();

    void* returnFunction(const std::string&);

    void* operator[](const std::string&);

    std::string& libName();
    timespec& LastWrite();

protected:

    void copyTo(const std::string&);



private:

    ReloadableCode m_Code;

    timespec m_last_modifcation;
    std::string m_Libname;
    void* m_handle;

};

#endif //HOTRELOAD_DYNAMICLIB_H
