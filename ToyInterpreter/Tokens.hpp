//
//  Tokens.hpp
//  ToyInterpreter
//
//  Created by admin on 3/31/16.
//  Copyright © 2016 admin. All rights reserved.
//

#ifndef Tokens_hpp
#define Tokens_hpp
#include <string>
enum Token{
    tokEof = -1,
    tokDef = -2,
    tokExtern = -3,
    tokIdentifier = -4,
    tokNum = -5,
    tokIf = -6,
    tokThen = -7,
    tokElse = -8,
    tokFor = -9,
    tokIn = -10,
    tokVar = -11
};

class Lexer{
public:
    static std::string IdentifierStr;
    static double NumVal;
public:
    static int getTok();
    
};


#endif /* Tokens_hpp */
