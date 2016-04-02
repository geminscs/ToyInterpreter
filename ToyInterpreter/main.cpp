//
//  main.cpp
//  ToyInterpreter
//
//  Created by admin on 3/31/16.
//  Copyright © 2016 admin. All rights reserved.
//

#include <iostream>
#include "Tokens.hpp"
#include "Parser.hpp"
#include "Globals.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    cout<<">>>";
    Parser::getNextToken();
    Globals::TheModule = make_unique<Module>("my cool jit", getGlobalContext());
    Parser::MainLoop();
    Globals::TheModule->dump();
    return 0;
}
