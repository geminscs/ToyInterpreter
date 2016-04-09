//
//  AST.cpp
//  ToyInterpreter
//
//  Created by admin on 4/1/16.
//  Copyright © 2016 admin. All rights reserved.
//

#include "AST.hpp"
#include "Globals.hpp"

Value *NumExprAST::codegen(){
    return ConstantFP::get(getGlobalContext(), APFloat(Val));
    
}

Value *VaribleExprAST::codegen(){
    Value *v = Globals::NamedValues[Name];
    if(!v){
        Globals::LogErrorV("unknown variable name");
    }
    return Globals::Builder.CreateLoad(v, Name.c_str());
}

std::string VaribleExprAST::getName(){
    return Name;
}

Value *BinaryExprAST::codegen(){
    if (Op == '=') {
        VaribleExprAST *LHSE = (VaribleExprAST *)(LHS.get());
        if (!LHSE) {
            return Globals::LogErrorV("destination of '=' must be a variable");
        }
        Value *Val = RHS->codegen();
        if (!Val) {
            return nullptr;
        }
        
        Value *Var = Globals::NamedValues[LHSE->getName()];
        if (!Var) {
            return Globals::LogErrorV("unknown variable name");
        }
        
        Globals::Builder.CreateStore(Val, Var);
        return Val;
    }
    Value *L = LHS->codegen();
    Value *R = RHS->codegen();
    if (!L || !R) {
        return nullptr;
    }
    
    switch (Op) {
        case '+':
            return Globals::Builder.CreateFAdd(L, R, "addtmp");
        case '-':
            return Globals::Builder.CreateFSub(L, R, "subtmp");
        case '*':
            return Globals::Builder.CreateFMul(L, R, "multmp");
        case '/':
            return Globals::Builder.CreateFDiv(L, R, "divtmp");
        case '<':
            L = Globals::Builder.CreateFCmpULT(L, R, "cmptmp");
            return Globals::Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()),"booltmp");
        default:
            //return Globals::LogErrorV("invalid binary operator");
            break;
    }
    
    Function *F = Globals::getFunction(std::string("binary") + Op);
    assert(F && "binary operator not found!");
    Value *Ops[2] = {L, R};
    return Globals::Builder.CreateCall(F, Ops, "binop");
}

Value *CallExprAST::codegen(){
    //Function *CalleeF = Globals::TheModule->getFunction(Callee);
    Function *CalleeF = Globals::getFunction(Callee);
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

void PrototypeAST::CreateArgumentAllocas(llvm::Function *F){
    Function::arg_iterator AI = F->arg_begin();
    for (unsigned long Idx = 0, e = Args.size(); Idx != e; Idx ++, AI ++) {
        AllocaInst *Alloca = Globals::CreateEntryBlockAlloc(F, Args[Idx]);
        //Globals::Builder.CreateStore(AI, Alloca);
        Globals::NamedValues[Args[Idx]] = Alloca;
    }
}

bool PrototypeAST::isUnaryOp(){
    return IsOperator && Args.size() == 1;
}

bool PrototypeAST::isBinaryOp(){
    return IsOperator && Args.size() == 2;
}

char PrototypeAST::getOperatorName(){
    assert(isUnaryOp() || isBinaryOp());
    return Name[Name.size() - 1];
}

unsigned PrototypeAST::getBinaryPrecedence(){
    return Precedence;
}

Function *FunctionAST::codegen(){
    /*Function *TheFunction = Globals::TheModule->getFunction(Proto->getName());
    if (!TheFunction) {
        TheFunction = Proto->codegen();
    }
    
    if (!TheFunction) {
        return nullptr;
    }
    
    if (!TheFunction->empty()) {
        return (Function *)Globals::LogErrorV("function cannot be redefined");
    }*/

    auto &P = *Proto;
    Globals::FunctionProtos[Proto->getName()] =std::move(Proto);
    Function *TheFunction = Globals::getFunction(P.getName());
    if (!TheFunction) {
        return nullptr;
    }
    
    if (P.isBinaryOp()) {
        Globals::BinopPrecedence[P.getOperatorName()] = P.getBinaryPrecedence();
    }
    
    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", TheFunction);
    Globals::Builder.SetInsertPoint(BB);
    
    Globals::NamedValues.clear();
    for(auto &Arg : TheFunction->args()){
        //Globals::NamedValues[Arg.getName()] = &Arg;
        AllocaInst *Alloca = Globals::CreateEntryBlockAlloc(TheFunction, Arg.getName());
        Globals::Builder.CreateStore(&Arg, Alloca);
        Globals::NamedValues[Arg.getName()] = Alloca;
    }
    
    if (Value *RetVal = Body->codegen()) {
        Globals::Builder.CreateRet(RetVal);
        verifyFunction(*TheFunction);
        Globals::TheFPM->run(*TheFunction);
        return TheFunction;
    }
    
    TheFunction->eraseFromParent();
    return nullptr;
}

Value *IfExprAST::codegen(){
    Value *CondV = Cond->codegen();
    if (!CondV) {
        return nullptr;
    }
    
    CondV = Globals::Builder.CreateFCmpONE(CondV, ConstantFP::get(getGlobalContext(), APFloat(0.0)), "ifcond");
    
    Function *TheFunction = Globals::Builder.GetInsertBlock()->getParent();
    
    BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "then", TheFunction);
    BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "else");
    BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");
    
    Globals::Builder.CreateCondBr(CondV, ThenBB, ElseBB);
    
    Globals::Builder.SetInsertPoint(ThenBB);
    Value *ThenV = Then->codegen();
    if (!ThenV) {
        return nullptr;
    }
    
    Globals::Builder.CreateBr(MergeBB);
    ThenBB = Globals::Builder.GetInsertBlock();
    
    TheFunction->getBasicBlockList().push_back(ElseBB);
    Globals::Builder.SetInsertPoint(ElseBB);
    
    Value *ElseV = Else->codegen();
    if (!ElseV) {
        return nullptr;
    }
    
    Globals::Builder.CreateBr(MergeBB);
    ElseBB = Globals::Builder.GetInsertBlock();
    
    TheFunction->getBasicBlockList().push_back(MergeBB);
    Globals::Builder.SetInsertPoint(MergeBB);
    PHINode *PN = Globals::Builder.CreatePHI(Type::getDoubleTy(getGlobalContext()), 2, "iftmp");
    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return PN;
}


Value *ForExprAst::codegen(){
    Function *TheFunction = Globals::Builder.GetInsertBlock()->getParent();
    AllocaInst *Alloca = Globals::CreateEntryBlockAlloc(TheFunction, VarName);
    
    Value *StartVal = Start->codegen();
    if (!StartVal) {
        return nullptr;
    }
    Globals::Builder.CreateStore(StartVal, Alloca);
    
    //BasicBlock *PreheaderBB = Globals::Builder.GetInsertBlock();
    BasicBlock *LoopBB = BasicBlock::Create(getGlobalContext(), "loop", TheFunction);
    Globals::Builder.CreateBr(LoopBB);
    
    Globals::Builder.SetInsertPoint(LoopBB);
    //PHINode *Variable = Globals::Builder.CreatePHI(Type::getDoubleTy(getGlobalContext()), 2, VarName.c_str());
    //Variable->addIncoming(StartVal, PreheaderBB);
    
    AllocaInst *OldVal = Globals::NamedValues[VarName];
    Globals::NamedValues[VarName] = Alloca;
    
    if (!Body->codegen()) {
        return nullptr;
    }
    
    Value *StepVal = nullptr;
    if (Step) {
        StepVal = Step->codegen();
        if (!StepVal) {
            return nullptr;
        }
    }
    else{
        StepVal = ConstantFP::get(getGlobalContext(), APFloat(1.0));
    }
    //Value *NextVar = Globals::Builder.CreateFAdd(Variable, StepVal, "nextvar");
    
    Value *EndCond = End->codegen();
    if (!EndCond) {
        return nullptr;
    }
    
    Value *CurVar = Globals::Builder.CreateLoad(Alloca, VarName.c_str());
    Value *NextVar = Globals::Builder.CreateFAdd(CurVar, StepVal, "nextvar");
    //Value *NextVar = Globals::Builder.CreateLoad(CurVar, StepVal, "nextvar");
    Globals::Builder.CreateStore(NextVar, Alloca);
    EndCond = Globals::Builder.CreateFCmpONE(EndCond, ConstantFP::get(getGlobalContext(), APFloat(0.0)), "loopcond");
    
    //BasicBlock *LoopEndBB = Globals::Builder.GetInsertBlock();
    BasicBlock *AfterBB = BasicBlock::Create(getGlobalContext(), "afterloop", TheFunction);
    
    Globals::Builder.CreateCondBr(EndCond, LoopBB, AfterBB);
    Globals::Builder.SetInsertPoint(AfterBB);
    //Variable->addIncoming(NextVar, LoopEndBB);
    
    if (OldVal) {
        Globals::NamedValues[VarName] = OldVal;
    }
    else{
        Globals::NamedValues.erase(VarName);
    }
    
    return ConstantFP::getNullValue(Type::getDoubleTy(getGlobalContext()));
}


Value *VarExprAST::codegen(){
    std::vector<AllocaInst *> OldBindings;
    Function *TheFunction = Globals::Builder.GetInsertBlock()->getParent();
    for (unsigned long i = 0, e = VarNames.size(); i != e; i ++) {
        const std::string &VarName = VarNames[i].first;
        ExprAST *Init = VarNames[i].second.get();
        Value *InitVal;
        if (Init) {
            InitVal = Init->codegen();
            if (!InitVal) {
                return nullptr;
            }
        }
        else{
            InitVal = ConstantFP::get(getGlobalContext(), APFloat(0.0));
        }
        
        AllocaInst *Alloca = Globals::CreateEntryBlockAlloc(TheFunction, VarName);
        Globals::Builder.CreateStore(InitVal, Alloca);
        OldBindings.push_back(Globals::NamedValues[VarName]);
        Globals::NamedValues[VarName] = Alloca;
    }
    
    Value *BodyVal = Body->codegen();
    if (!BodyVal) {
        return nullptr;
    }
    
    for (unsigned long i = 0, e = VarNames.size(); i != e; ++i){
        Globals::NamedValues[VarNames[i].first] = OldBindings[i];
    }
    
    return BodyVal;
}

Value *UnaryExprAST::codegen(){
    Value *OperandV = Operend->codegen();
    if (!Operend) {
        return nullptr;
    }
    
    Function *F = Globals::TheModule->getFunction(std::string("unary") + Opcode);
    if (!F) {
        return Globals::LogErrorV("unknown unary operator");
    }
    
    return Globals::Builder.CreateCall(F, OperandV, "unop");
}











