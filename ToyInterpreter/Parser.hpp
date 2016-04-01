//
//  Parser.hpp
//  ToyInterpreter
//
//  Created by admin on 4/1/16.
//  Copyright © 2016 admin. All rights reserved.
//

#ifndef Parser_hpp
#define Parser_hpp

#include <stdio.h>
#include <map>
#include "AST.hpp"
#include "Tokens.hpp"
#include <iostream>

class Parser{
public:
    static int CurTok;
    static std::map<char, int> BinopPrecedence;
public:
    static std::map<char, int> initMap();
    static int getNextToken();
    static int GetTokPrecedence();
    static ExprAST *Error(const char *str);
    static PrototypeAST *ErrorP(const char *str);
    static ExprAST *ParseExpresion();
    static ExprAST *ParseIdentifierExpr();
    static ExprAST *ParseNumExpr();
    static ExprAST *ParseParenExpr();
    static ExprAST *ParsePrimary();
    static ExprAST *ParseBinopRHS(int ExprPrec, ExprAST *LHS);
    static PrototypeAST *ParsePrototype();
    static FunctionAST *ParseDefinition();
    static FunctionAST *ParseTopLevelExpr();
    static PrototypeAST *ParseExtern();
    static void HandleDefinition();
    static void HandleExtern();
    static void HandleTopLevelExpression();
    static void MainLoop();
    
};
#endif /* Parser_hpp */
