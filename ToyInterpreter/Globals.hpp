//
//  Globals.hpp
//  ToyInterpreter
//
//  Created by admin on 4/2/16.
//  Copyright © 2016 admin. All rights reserved.
//

#ifndef Globals_hpp
#define Globals_hpp

#include <iostream>
#include <map>
#include "LLVMHead.hpp"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include "KaleidoscopeJIT.hpp"
#include "AST.hpp"

using namespace llvm;
using namespace llvm::orc;
class Globals{
public:
    static std::unique_ptr<Module> TheModule;
    static std::unique_ptr<legacy::FunctionPassManager> TheFPM;
    static IRBuilder<> Builder;
    static std::map<std::string, Value *> NamedValues;
    static std::unique_ptr<KaleidoscopeJIT> TheJIT;
    static std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;
public:
    static Value *LogErrorV(const char *str);
    static void InitializeModuleAndPassManger(void);
    static Function *getFunction(std::string name);
};
#endif /* Globals_hpp */
