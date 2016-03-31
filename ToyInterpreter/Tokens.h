//
//  Tokens.h
//  ToyInterpreter
//
//  Created by admin on 3/31/16.
//  Copyright © 2016 admin. All rights reserved.
//

#ifndef Tokens_h
#define Tokens_h

enum Token{
    tokEof = -1,
    tokDef = -2,
    tokExtern = -3,
    tokIdentifier = -4,
    tokNum = -5
};
void tests();
static int getTok();

#endif /* Tokens_h */
