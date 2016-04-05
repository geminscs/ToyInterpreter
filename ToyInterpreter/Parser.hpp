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
    static std::unique_ptr<ExprAST> LogError(const char *str);
    static std::unique_ptr<PrototypeAST> LogErrorP(const char *str);
    static std::unique_ptr<ExprAST> ParseExpresion();
    static std::unique_ptr<ExprAST> ParseIdentifierExpr();
    static std::unique_ptr<ExprAST> ParseNumExpr();
    static std::unique_ptr<ExprAST> ParseParenExpr();
    static std::unique_ptr<ExprAST> ParsePrimary();
    static std::unique_ptr<ExprAST> ParseBinopRHS(int ExprPrec, std::unique_ptr<ExprAST>LHS);
    static std::unique_ptr<PrototypeAST> ParsePrototype();
    static std::unique_ptr<FunctionAST> ParseDefinition();
    static std::unique_ptr<FunctionAST> ParseTopLevelExpr();
    static std::unique_ptr<PrototypeAST> ParseExtern();
    static std::unique_ptr<ExprAST> ParseIfExpr();
    static std::unique_ptr<ExprAST> ParseForExpr();
    static void HandleDefinition();
    static void HandleExtern();
    static void HandleTopLevelExpression();
    static void MainLoop();
    
};
#endif /* Parser_hpp */
