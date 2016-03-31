//
//  Tokens.cpp
//  ToyInterpreter
//
//  Created by admin on 3/31/16.
//  Copyright © 2016 admin. All rights reserved.
//

#include "Tokens.hpp"
#include <string>
#include <stdlib.h>

using namespace std;

string Lexer::IdentifierStr = "";
double Lexer::NumVal = 0;

int Lexer::getTok(){
    static int lastChar = ' ';
    //ignore white spaces
    while(isspace(lastChar)){
        lastChar = getchar();
    }
    //identifiers
    if(isalpha(lastChar)){
        IdentifierStr = lastChar;
        while(isalnum(lastChar = getchar())){
            IdentifierStr += lastChar;
        }
        
        if(IdentifierStr == "def"){
            return tokDef;
        }
        if(IdentifierStr == "extern"){
            return tokExtern;
        }
        return tokIdentifier;
    }
    //numbers
    if(isdigit(lastChar) || lastChar == '.'){
        string NumStr;
        //todo bug what about 1.1.1.1
        do{
            NumStr += lastChar;
            lastChar = getchar();
        }while(isdigit(lastChar) || lastChar == '.');
        NumVal = strtod(NumStr.c_str(), 0);
        return tokNum;
    }
    //comments
    if(lastChar == '#'){
        do{
            lastChar = getchar();
        }while(lastChar != EOF && lastChar != '\n' && lastChar != '\r');
        
        if(lastChar != EOF){
            return getTok();
        }
    }
    
    //end of file
    if(lastChar == EOF){
        return tokEof;
    }
    
    int unIdentified = lastChar;
    lastChar = getchar();
    return unIdentified;
}

