//
//  Parser.cpp
//  ToyInterpreter
//
//  Created by admin on 4/1/16.
//  Copyright © 2016 admin. All rights reserved.
//

#include "Parser.hpp"
#include "Globals.hpp"

int Parser::CurTok = 0;
/*std::map<char, int> Parser::BinopPrecedence = initMap();


std::map<char, int> Parser::initMap(){
    std::map<char, int> temp;
    temp['='] = 2;
    temp['<'] = 10;
    temp['+'] = 20;
    temp['-'] = 20;
    temp['*'] = 40;
    temp['/'] = 40;
    return temp;
}*/


int Parser::getNextToken(){
    return CurTok = Lexer::getTok();
}

int Parser::GetTokPrecedence(){
    if(!isascii(CurTok)){
        return -1;
    }
    
    int TokPrec = Globals::BinopPrecedence[CurTok];
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
    auto LHS = ParseUnary();
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
        case tokIf:
            return ParseIfExpr();
        case tokFor:
            return ParseForExpr();
        case tokVar:
            return ParseVarExpr();
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
        auto RHS = ParseUnary();
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
    /*if (CurTok != tokIdentifier) {
        return LogErrorP("expected function name in prototype");
    }
    
    std::string funName = Lexer::IdentifierStr;
    getNextToken();*/
    
    std::string FnName;
    unsigned Kind = 0;
    unsigned BinaryPrec = 30;
    
    switch (CurTok) {
        case tokIdentifier:
            FnName = Lexer::IdentifierStr;
            Kind = 0;
            getNextToken();
            break;
        case tokBinary:
            getNextToken();
            if (!isascii(CurTok)) {
                return LogErrorP("expected binary operator");
            }
            FnName = "binary";
            FnName += (char)CurTok;
            Kind = 2;
            getNextToken();
            if (CurTok == tokNum) {
                if (Lexer::NumVal < 1 || Lexer::NumVal > 100) {
                    return LogErrorP("invalid precedence");
                }
                BinaryPrec = (unsigned)Lexer::NumVal;
                getNextToken();
            }
            break;
        case tokUnary:
            getNextToken();
            if (!isascii(CurTok)) {
                return LogErrorP("expected unary operator");
            }
            FnName = "unary";
            FnName += (char)CurTok;
            Kind = 1;
            getNextToken();
            break;
        default:
            return LogErrorP("expected function name in prototype");
    }
    
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
    if (Kind && Args.size() != Kind) {
        return LogErrorP("invalid number of operands for operator");
    }
    
    return make_unique<PrototypeAST>(FnName, std::move(Args), Kind != 0, BinaryPrec);
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
#ifndef TARGETOBJ
            Globals::TheJIT->addModule(std::move(Globals::TheModule));
            Globals::InitializeModuleAndPassManger();
#endif
        }
    }
    else{
        getNextToken();
    }
}

std::unique_ptr<ExprAST> Parser::ParseIfExpr(){
    getNextToken();
    
    auto Cond = ParseExpresion();
    if (!Cond) {
        return nullptr;
    }
    
    if (CurTok != tokThen) {
        return LogError("expected then");
    }
    getNextToken();
    
    auto Then = ParseExpresion();
    if (!Then) {
        return nullptr;
    }
    
    //todo how about only if no else
    if (CurTok != tokElse) {
        return LogError("expected else");
    }
    getNextToken();
    
    auto Else = ParseExpresion();
    if (!Else) {
        return nullptr;
    }
    
    return llvm::make_unique<IfExprAST>(std::move(Cond), std::move(Then), std::move(Else));
}

std::unique_ptr<ExprAST> Parser::ParseForExpr(){
    getNextToken();
    
    std::string IdName = Lexer::IdentifierStr;
    getNextToken();
    
    if (CurTok != '=') {
        return LogError("expected '=' after for");
    }
    getNextToken();
    
    auto Start = ParseExpresion();
    if (!Start) {
        return nullptr;
    }
    if (CurTok != ',') {
        return LogError("expected ',' after for start value");
    }
    getNextToken();
    
    auto End = ParseExpresion();
    if (!End) {
        return nullptr;
    }
    
    std::unique_ptr<ExprAST> Step;
    if (CurTok == ',') {
        getNextToken();
        Step = ParseExpresion();
        if (!Step) {
            return nullptr;
        }
    }
    if (CurTok != tokIn) {
        return LogError("expected 'in' after for");
    }
    getNextToken();
    
    auto Body = ParseExpresion();
    if (!Body) {
        return nullptr;
    }
    
    return llvm::make_unique<ForExprAst>(IdName, std::move(Start), std::move(End), std::move(Step), std::move(Body));
}

std::unique_ptr<ExprAST> Parser::ParseVarExpr(){
    getNextToken();
    std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;
    
    if (CurTok != tokIdentifier) {
        return LogError("expected identifier after var");
    }
    
    while (true) {
        std::string Name = Lexer::IdentifierStr;
        getNextToken();
        
        std::unique_ptr<ExprAST> Init;
        if (CurTok == '=') {
            getNextToken();
            Init = ParseExpresion();
            if (!Init) {
                return nullptr;
            }
            
        }
        VarNames.push_back(std::make_pair(Name, std::move(Init)));
        
        if (CurTok != ',') {
            break;
        }
        getNextToken();
        if (CurTok != tokIdentifier) {
            return LogError("expected identifier list after var");
        }
    }
    
    if (CurTok != tokIn) {
        return LogError("expected 'in' key after var");
    }
    getNextToken();
    
    auto Body = ParseExpresion();
    if (!Body) {
        return nullptr;
    }
    
    return llvm::make_unique<VarExprAST>(std::move(VarNames), std::move(Body));
}

std::unique_ptr<ExprAST> Parser::ParseUnary(){
    if (!isascii(CurTok) || CurTok == '(' || CurTok == ',') {
        return ParsePrimary();
    }
    
    int Opc = CurTok;
    getNextToken();
    if (auto Operand = ParseUnary()) {
        return llvm::make_unique<UnaryExprAST>(Opc, std::move(Operand));
    }
    return nullptr;
}

void Parser::HandleExtern(){
    if (auto ProtoAST = ParseExtern()) {
        if (auto *FnIR = ProtoAST->codegen()) {
            std::cout<<"read extern";
            FnIR->dump();
            Globals::FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
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
            
            auto H = Globals::TheJIT->addModule(std::move(Globals::TheModule));
            Globals::InitializeModuleAndPassManger();
            
            auto ExprSymbol = Globals::TheJIT->findSymbol("__anon_expr");
            assert(ExprSymbol && "Function not found");
            
            double (*FP)() = (double (*)())(intptr_t)ExprSymbol.getAddress();
            std::cout<<"Evaluated to "<<FP()<<std::endl;
            Globals::TheJIT->removeModule(H);
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




