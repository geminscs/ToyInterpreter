//
//  AST.cpp
//  ToyInterpreter
//
//  Created by admin on 4/1/16.
//  Copyright © 2016 admin. All rights reserved.
//

#include "AST.hpp"

Value *NumExprAST::codegen(){
    return ConstantFP::get(getGlobalContext(), APFloat(Val));
    
}

Value *VaribleExprAST::codegen(){
    Value *v = Globals::NamedValues[Name];
    if(!v){
        Globals::LogErrorV("unknown variable name");
    }
    return v;
}

Value *BinaryExprAST::codegen(){
    Value *L = LHS->codegen();
    Value *R = RHS->codegen();
    if (!L || !R) {
        return nullptr;
    }
    
    switch (Op) {
        case '+':
            return Globals::Builder.CreateAdd(L, R, "addtmp");
        case '-':
            return Globals::Builder.CreateSub(L, R, "subtmp");
        case '*':
            return Globals::Builder.CreateMul(L, R, "multmp");
        case '<':
            L = Globals::Builder.CreateFCmpULT(L, R, "cmptmp");
            return Globals::Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()),"booltmp");
        default:
            return Globals::LogErrorV("invalid binary operator");
    }
}

Value *CallExprAST::codegen(){
    Function *CalleeF = Globals::TheModule->getFunction(Callee);
    if (!CalleeF) {
        return Globals::LogErrorV("unknown function referenced");
    }
    
    if (CalleeF->arg_size() != Args.size()) {
        return Globals::LogErrorV("incorrect # argument passed");
    }
    
    std::vector<Value *> ArgsV;
    for (unsigned long i = 0, e = Args.size();i != e ; i ++) {
        ArgsV.push_back(Args[i]->codegen());
        if (!ArgsV.back()) {
            return nullptr;
        }
    }
    
    return Globals::Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

Function *PrototypeAST::codegen(){
    std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(getGlobalContext()));
    FunctionType *FT = FunctionType::get(Type::getDoubleTy(getGlobalContext()), Doubles, false);
    Function *F = Function::Create(FT, Function::ExternalLinkage, Name, Globals::TheModule.get());
    
    unsigned Idx = 0;
    for(auto &Arg : F->args()){
        Arg.setName(Args[Idx ++]);
    }
    return F;
}

std::string PrototypeAST::getName(){
    return Name;
}

Function *FunctionAST::codegen(){
    Function *TheFunction = Globals::TheModule->getFunction(Proto->getName());
    
    if (!TheFunction) {
        TheFunction = Proto->codegen();
    }
    
    if (!TheFunction) {
        return nullptr;
    }
    
    if (!TheFunction->empty()) {
        return (Function *)Globals::LogErrorV("function cannot be redefined");
    }
    
    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", TheFunction);
    Globals::Builder.SetInsertPoint(BB);
    
    Globals::NamedValues.clear();
    for(auto &Arg : TheFunction->args()){
        Globals::NamedValues[Arg.getName()] = &Arg;
    }
    
    if (Value *RetVal = Body->codegen()) {
        Globals::Builder.CreateRet(RetVal);
        verifyFunction(*TheFunction);
        return TheFunction;
    }
    
    TheFunction->eraseFromParent();
    return nullptr;
}




















