//
//  Parser.cpp
//  ToyInterpreter
//
//  Created by admin on 4/1/16.
//  Copyright © 2016 admin. All rights reserved.
//

#include "Parser.hpp"

int Parser::CurTok = 0;
std::map<char, int> Parser::BinopPrecedence = initMap();

std::map<char, int> Parser::initMap(){
    std::map<char, int> temp;
    temp['<'] = 10;
    temp['+'] = 20;
    temp['-'] = 20;
    temp['*'] = 40;
    temp['/'] = 40;
    return temp;
}


int Parser::getNextToken(){
    return CurTok = Lexer::getTok();
}

int Parser::GetTokPrecedence(){
    if(!isascii(CurTok)){
        return -1;
    }
    
    int TokPrec = BinopPrecedence[CurTok];
    if(TokPrec <= 0){
        return -1;
    }
    return TokPrec;
}

std::unique_ptr<ExprAST> Parser::LogError(const char *str){
    std::cout<<"Error: "<<str<<std::endl;
    return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::LogErrorP(const char *str){
    LogError(str);
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::ParseExpresion(){
    auto LHS = ParsePrimary();
    if (!LHS) {
        return nullptr;
    }
    return ParseBinopRHS(0, std::move(LHS));
}

std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr(){
    std::string IdName = Lexer::IdentifierStr;
    getNextToken();
    if(CurTok != '('){
        return make_unique<VaribleExprAST>(IdName);
    }
    
    getNextToken();
    std::vector<std::unique_ptr<ExprAST>> Args;
    if(CurTok != ')'){
        while (true) {
            auto Arg = ParseExpresion();
            if(!Arg){
                return nullptr;
            }
            Args.push_back(std::move(Arg));
            
            if(CurTok == ')'){
                break;
            }
            
            if(CurTok != ','){
                return LogError("Expected ')' or ',' in argument list");
            }
            getNextToken();
        }
    }
    
    getNextToken();
    return make_unique<CallExprAST>(IdName, std::move(Args));
}


std::unique_ptr<ExprAST> Parser::ParseNumExpr(){
    auto res = make_unique<NumExprAST>(Lexer::NumVal);
    getNextToken();
    return std::move(res);
}


std::unique_ptr<ExprAST> Parser::ParseParenExpr(){
    getNextToken();
    auto v = ParseExpresion();
    if(!v){
        return nullptr;
    }
    
    if(CurTok != ')'){
        return LogError("expected ')'");
    }
    getNextToken();
    return v;
}

std::unique_ptr<ExprAST> Parser::ParsePrimary(){
    switch (CurTok) {
        case tokIdentifier:
            return  ParseIdentifierExpr();
        case tokNum:
            return ParseNumExpr();
        case '(':
            return ParseParenExpr();
        default:
            return LogError("unknown token when expecting an expression");
    }
}
std::unique_ptr<ExprAST> Parser::ParseBinopRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS){
    while (true) {
        int TokPrec = GetTokPrecedence();
        if(TokPrec < ExprPrec){
            return LHS;
        }
        
        int BinOp = CurTok;
        getNextToken();
        auto RHS = ParsePrimary();
        if(!RHS){
            return nullptr;
        }
        
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinopRHS(TokPrec + 1, std::move(RHS));
            if (!RHS) {
                return nullptr;
            }
        }
        
        LHS = make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

//todo function definition: def foo(x y) ->def foo(x, y) end
std::unique_ptr<PrototypeAST> Parser::ParsePrototype(){
    if (CurTok != tokIdentifier) {
        return LogErrorP("expected function name in prototype");
    }
    
    std::string funName = Lexer::IdentifierStr;
    getNextToken();
    
    if (CurTok != '(') {
        return LogErrorP("expected '(' in prototype");
    }
    
    std::vector<std::string> Args;
    while (getNextToken() == tokIdentifier) {
        Args.push_back(Lexer::IdentifierStr);
    }
    
    if (CurTok != ')') {
        return LogErrorP("expected ')' in prototype");
    }
    
    getNextToken();
    
    return make_unique<PrototypeAST>(funName, std::move(Args));
}

std::unique_ptr<FunctionAST> Parser::ParseDefinition(){
    getNextToken();
    auto Proto = ParsePrototype();
    if (!Proto) {
        return nullptr;
    }
    //todo function only contains one expression -> block;
    if (auto e = ParseExpresion()) {
        return make_unique<FunctionAST>(std::move(Proto), std::move(e));
    }
    return nullptr;
}

std::unique_ptr<FunctionAST> Parser::ParseTopLevelExpr(){
    if (auto e = ParseExpresion()) {
        auto Proto = make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
        return make_unique<FunctionAST>(std::move(Proto), std::move(e));
    }
    return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::ParseExtern(){
    getNextToken();
    return ParsePrototype();
}
void Parser::HandleDefinition(){
    if (auto FnAST = ParseDefinition()) {
        if (auto *FnIR = FnAST->codegen()) {
            std::cout<<"read function definition:";
            FnIR->dump();
        }
    }
    else{
        getNextToken();
    }
}
void Parser::HandleExtern(){
    if (auto ProtoAST = ParseExpresion()) {
        if (auto *FnIR = ProtoAST->codegen()) {
            std::cout<<"read extern";
            FnIR->dump();
        }
    }
    else{
        getNextToken();
    }
}
void Parser::HandleTopLevelExpression(){
    if (auto FnAST = ParseTopLevelExpr()) {
        if (auto *FnIR = FnAST->codegen()) {
            std::cout<<"read top-level expression";
            FnIR->dump();
        }
    }
    else{
        getNextToken();
    }
}
void Parser::MainLoop(){
    while (true) {
        std::cout<<">>>";
        switch (CurTok) {
            case tokEof:
                return;
            case ';':
                getNextToken();break;
            case tokDef:
                HandleDefinition();break;
            case tokExtern:
                HandleExtern();break;
            default:
                HandleTopLevelExpression();break;
        }
        
    }
}




