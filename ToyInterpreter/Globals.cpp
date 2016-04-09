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
std::map<std::string, AllocaInst *> Globals::NamedValues;
std::unique_ptr<KaleidoscopeJIT> Globals::TheJIT;
std::map<std::string, std::unique_ptr<PrototypeAST>> Globals::FunctionProtos;

std::map<char, int> Globals::BinopPrecedence = initMap();


std::map<char, int> Globals::initMap(){
    std::map<char, int> temp;
    temp['='] = 2;
    temp['<'] = 10;
    temp['+'] = 20;
    temp['-'] = 20;
    temp['*'] = 40;
    temp['/'] = 40;
    return temp;
}

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
    
    //std::unique_ptr<ExecutionEngine> TheExecutionEngine = new ExecutionEngine();
    //TheFPM->add(new DataLayout(*TheExecutionEngine->getDataLayout));
    //TheFPM->add(createPromoteMemoryToRegisterPass());
    //TheFPM->add(createInstructionCombiningPass());
    //TheFPM->add(createReassociatePass());
    
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

AllocaInst *Globals::CreateEntryBlockAlloc(llvm::Function *TheFunction, const std::string &VarName){
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type::getDoubleTy(getGlobalContext()), 0, VarName.c_str());
}