//
//  Globals.hpp
//  ToyInterpreter
//
//  Created by admin on 4/2/16.
//  Copyright © 2016 admin. All rights reserved.
//

#ifndef Globals_hpp
#define Globals_hpp

#include <stdio.h>
#include <map>
#include "LLVMHead.hpp"
#include "llvm/IR/LLVMContext.h"

using namespace llvm;
class Globals{
public:
    static std::unique_ptr<Module> TheModule;
    static IRBuilder<> Builder;
    static std::map<std::string, Value *> NamedValues;
public:
    static Value *LogErrorV(const char *str);
};
#endif /* Globals_hpp */
