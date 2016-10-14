//
// Created by johannes on 10/12/16.
//

#include "../DynamicLib.h"
#include <fstream>
#include <iostream>
#include <exception>

#include <dlfcn.h>
#include <stdexcept>

DynamicLib::DynamicLib(const std::string &Libname) : m_handle(nullptr),m_Libname(Libname) {

    reload();

}

DynamicLib::~DynamicLib() {

    unload();

}

void DynamicLib::reload() {

    this->m_last_modifcation = GetLastWriteTime(this->m_Libname);

    this->copyTo("Tempory.so");

    this->m_handle = dlopen("Tempory.so",RTLD_LAZY);

    if(m_handle == nullptr){
        std::cerr<<"Could not load Shared Object because: "<<dlerror()<<"\n";
    }


}

void DynamicLib::unload() {

    if( this->m_handle ){
        if(dlclose(this->m_handle)){
            std::cerr<<"Could not unload lib\n";
        };
        this->m_handle = nullptr;
    }

}

void DynamicLib::copyTo(const std::string & Copyname) {

    std::ifstream source(this->libName(),std::ios::binary);
    std::ofstream dest(Copyname,std::ios::binary);

    dest << source.rdbuf();


    if(dest.good())
        std::cerr<<"Success\n";
    else
        std::cerr<<"meh Error. Could not successfull copy file.\n";

    source.close();
    dest.close();
}

void* DynamicLib::returnFunction(const std::string & func) {
    return dlsym(this->m_handle,func.c_str());
}

void* DynamicLib::operator[](const std::string & function) {
    return returnFunction(function);
}

timespec& DynamicLib::LastWrite() { return m_last_modifcation;}

std::string& DynamicLib::libName() {
    return m_Libname;
}