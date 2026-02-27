#include"log.h"

#include<string>
#include<iostream>


int main(){

    
    std::string str = "hello world";
    Log* l = Log::Instance();
    
    l->init(1);
    
    LOG_INFO(str.c_str());
    return 0;
}