//
//  AST.hpp
//  ToyInterpreter
//
//  Created by admin on 4/1/16.
//  Copyright © 2016 admin. All rights reserved.
//

#ifndef AST_hpp
#define AST_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "LLVMHead.hpp"
#include "Globals.hpp"

using namespace llvm;
//expression node
class ExprAST{
public:
    virtual ~ExprAST(){}
    virtual Value *codegen() = 0;
};

class NumExprAST : public ExprAST{
    double Val;
public:
    NumExprAST(double val) : Val(val){}
    virtual Value *codegen() override;
};

class VaribleExprAST : public ExprAST{
    std::string Name;
public:
    VaribleExprAST(const std::string &name) : Name(name){}
    virtual Value *codegen() override;
};

class BinaryExprAST : public ExprAST{
    std::unique_ptr<ExprAST> LHS, RHS;
    char Op;
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs): Op(op), LHS(std::move(lhs)), RHS(std::move(rhs)){}
    
    virtual Value *codegen() override;
};

class CallExprAST : public ExprAST{
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    CallExprAST(const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args) : Callee(callee), Args(std::move(args)){}
    virtual Value *codegen() override;
};

class PrototypeAST{
    std::string Name;
    std::vector<std::string> Args;
public:
    PrototypeAST(const std::string &name, const std::vector<std::string> args): Name(name), Args(std::move(args)){}
    std::string getName();
    Function *codegen();
};

class FunctionAST{
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;
public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body):Proto(std::move(proto)), Body(std::move(body)){}
    Function *codegen();
};
#endif /* AST_hpp */
