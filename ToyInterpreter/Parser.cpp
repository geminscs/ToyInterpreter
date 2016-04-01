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

ExprAST* Parser::Error(const char *str){
    std::cout<<"Error: "<<str<<std::endl;
    return NULL;
}

PrototypeAST* Parser::ErrorP(const char *str){
    Error(str);
    return NULL;
}

ExprAST* Parser::ParseExpresion(){
    ExprAST *LHS = ParsePrimary();
    if (!LHS) {
        return NULL;
    }
    
    return ParseBinopRHS(0, LHS);
}

ExprAST* Parser::ParseIdentifierExpr(){
    std::string IdName = Lexer::IdentifierStr;
    getNextToken();
    if(CurTok != '('){
        return new VaribleExprAST(IdName);
    }
    
    getNextToken();
    std::vector<ExprAST*> Args;
    if(CurTok != ')'){
        while (true) {
            ExprAST *Arg = ParseExpresion();
            if(!Arg){
                return NULL;
            }
            Args.push_back(Arg);
            
            if(CurTok == ')'){
                break;
            }
            
            if(CurTok != ','){
                return Error("Expected ')' or ',' in argument list");
            }
            getNextToken();
        }
    }
    
    getNextToken();
    return new CallExprAST(IdName, Args);
}


ExprAST* Parser::ParseNumExpr(){
    ExprAST *res = new NumExprAST(Lexer::NumVal);
    getNextToken();
    return res;
}


ExprAST* Parser::ParseParenExpr(){
    getNextToken();
    ExprAST *v = ParseExpresion();
    if(!v){
        return NULL;
    }
    
    if(CurTok != ')'){
        return Error("expected ')'");
    }
    getNextToken();
    return v;
}

ExprAST* Parser::ParsePrimary(){
    switch (CurTok) {
        case tokIdentifier:
            return  ParseIdentifierExpr();
        case tokNum:
            return ParseNumExpr();
        case '(':
            return ParseParenExpr();
        default:
            return Error("unknown token when expecting an expression");
    }
}
ExprAST* Parser::ParseBinopRHS(int ExprPrec, ExprAST *LHS){
    while (true) {
        int TokPrec = GetTokPrecedence();
        if(TokPrec < ExprPrec){
            return LHS;
        }
        
        int BinOp = CurTok;
        getNextToken();
        ExprAST *RHS = ParsePrimary();
        if(!RHS){
            return NULL;
        }
        
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinopRHS(TokPrec + 1, RHS);
            if (RHS == NULL) {
                return NULL;
            }
        }
        
        LHS = new BinaryExprAST(BinOp, LHS, RHS);
    }
}

//todo function definition: def foo(x y) ->def foo(x, y) end
PrototypeAST* Parser::ParsePrototype(){
    if (CurTok != tokIdentifier) {
        return ErrorP("expected function name in prototype");
    }
    
    std::string funName = Lexer::IdentifierStr;
    getNextToken();
    
    if (CurTok != '(') {
        return ErrorP("expected '(' in prototype");
    }
    
    std::vector<std::string> Args;
    while (getNextToken() == tokIdentifier) {
        Args.push_back(Lexer::IdentifierStr);
    }
    
    if (CurTok != ')') {
        return ErrorP("expected ')' in prototype");
    }
    
    getNextToken();
    
    return new PrototypeAST(funName, Args);
}

FunctionAST* Parser::ParseDefinition(){
    getNextToken();
    PrototypeAST *Proto = ParsePrototype();
    if (!Proto) {
        return NULL;
    }
    //todo function only contains one expression -> block;
    if (ExprAST *e = ParseExpresion()) {
        return new FunctionAST(Proto, e);
    }
    return NULL;
}

FunctionAST* Parser::ParseTopLevelExpr(){
    if (ExprAST *e = ParseExpresion()) {
        PrototypeAST *Proto = new PrototypeAST("", std::vector<std::string>());
        return new FunctionAST(Proto, e);
    }
    return NULL;
}

PrototypeAST* Parser::ParseExtern(){
    getNextToken();
    return ParsePrototype();
}
void Parser::HandleDefinition(){
    if (ParseDefinition()) {
        std::cout<<"parsed a function definition."<<std::endl;
    }
    else{
        getNextToken();
    }
}
void Parser::HandleExtern(){
    if (ParseExtern()) {
        std::cout<<"parsed an extern."<<std::endl;
    }
    else{
        getNextToken();
    }
}
void Parser::HandleTopLevelExpression(){
    if (ParseTopLevelExpr()) {
        std::cout<<"parsed a top-level expression."<<std::endl;
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




