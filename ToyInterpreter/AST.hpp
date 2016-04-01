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

//expression node
class ExprAST{
public:
    virtual ~ExprAST(){}
};

class NumExprAST : public ExprAST{
    double Val;
public:
    NumExprAST(double val) : Val(val){}
};

class VaribleExprAST : public ExprAST{
    std::string Name;
public:
    VaribleExprAST(const std::string &name) : Name(name){}
};

class BinaryExprAST : public ExprAST{
    ExprAST * LHS, * RHS;
    char Op;
public:
    BinaryExprAST(char op, ExprAST * lhs, ExprAST * rhs){}
};

class CallExprAST : public ExprAST{
    std::string Callee;
    std::vector<ExprAST*> Args;
public:
    CallExprAST(const std::string &callee, std::vector<ExprAST*> &args) : Callee(callee), Args(args){}
};

class PrototypeAST{
    std::string Name;
    std::vector<std::string> Args;
public:
    PrototypeAST(const std::string &name, const std::vector<std::string> &args): Name(name), Args(args){}
};

class FunctionAST{
    PrototypeAST *Proto;
    ExprAST *Body;
public:
    FunctionAST(PrototypeAST *proto, ExprAST *body) : Proto(proto), Body(body){}
};
#endif /* AST_hpp */
