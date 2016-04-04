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
std::unique_ptr<legacy::FunctionPassManager> Globals::TheFPM;
std::map<std::string, Value *> Globals::NamedValues;
std::unique_ptr<KaleidoscopeJIT> Globals::TheJIT;
std::map<std::string, std::unique_ptr<PrototypeAST>> Globals::FunctionProtos;

Value *Globals::LogErrorV(const char *str){
    std::cout<<str<<std::endl;
    return nullptr;
}

void Globals::InitializeModuleAndPassManger(){
    TheModule = llvm::make_unique<Module>("my cool jit", getGlobalContext());
    TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());
    
    TheFPM = llvm::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());
    //TheFPM->add(createBasicAliasAnalysisPass());
    TheFPM->add(createInstructionCombiningPass());
    TheFPM->add(createReassociatePass());
    TheFPM->add(createGVNPass());
    TheFPM->add(createCFGSimplificationPass());
    
    TheFPM->doInitialization();
}

Function *Globals::getFunction(std::string name){
    if (auto *F = Globals::TheModule->getFunction(name)) {
        return F;
    }
    
    auto FI = FunctionProtos.find(name);
    if (FI != FunctionProtos.end()) {
        return FI->second->codegen();
    }
    
    return nullptr;
}