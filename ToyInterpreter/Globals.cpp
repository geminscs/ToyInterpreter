//
//  Globals.cpp
//  ToyInterpreter
//
//  Created by admin on 4/2/16.
//  Copyright © 2016 admin. All rights reserved.
//

#include "Globals.hpp"

IRBuilder<> Globals::Builder(getGlobalContext());
std::unique_ptr<Module> Globals::TheModule;
std::map<std::string, Value *> Globals::NamedValues;

Value *Globals::LogErrorV(const char *str){
    //LogError(str);
    return nullptr;
}