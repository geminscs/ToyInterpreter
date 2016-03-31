//
//  main.cpp
//  ToyInterpreter
//
//  Created by admin on 3/31/16.
//  Copyright © 2016 admin. All rights reserved.
//

#include <iostream>
#include "Tokens.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    // insert code here...
    while(true){
        std::cout << Lexer::getTok()<<endl;
    }
    return 0;
}
