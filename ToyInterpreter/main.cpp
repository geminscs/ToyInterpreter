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

using namespace std;

int main(int argc, const char * argv[]) {
    cout<<">>>";
    Parser::getNextToken();
    Parser::MainLoop();
    return 0;
}
