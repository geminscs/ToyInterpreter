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

#include "llvm/Support/FileSystem.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"


using namespace std;
/// putchard - putchar that takes a double and returns 0.
extern "C" double putchard(double X) {
    fputc((char)X, stderr);
    return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" double printd(double X) {
    fprintf(stderr, "%f\n", X);
    return 0;
}

int main(int argc, const char * argv[]) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    cout<<">>>";
    Parser::getNextToken();
    Globals::TheModule = make_unique<Module>("my cool jit", getGlobalContext());
    Globals::TheJIT = make_unique<KaleidoscopeJIT>();
    
    //auto TargetTriple = sys::getDefaultTargetTriple();
    Globals::InitializeModuleAndPassManger();
    //Globals::TheModule->setTargetTriple(TargetTriple);
    
    Parser::MainLoop();
    Globals::TheModule->dump();
    
    //generate object file
    /*InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();
    
    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
    
    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        errs() << Error;
        return 1;
    }
    
    auto CPU = "generic";
    auto Features = "";
    
    TargetOptions opt;
    auto TargetMachine =
    Target->createTargetMachine(TargetTriple, CPU, Features, opt);
    
    Globals::TheModule->setDataLayout(TargetMachine->createDataLayout());
    
    auto Filename = "output.o";
    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::F_None);
    
    if (EC) {
        errs() << "Could not open file: " << EC.message();
        return 1;
    }
    
    legacy::PassManager pass;
    auto FileType = TargetMachine::CGFT_ObjectFile;
    
    if (TargetMachine->addPassesToEmitFile(pass, dest, FileType)) {
        errs() << "TargetMachine can't emit a file of this type";
        return 1;
    }
    
    pass.run(*Globals::TheModule);
    dest.flush();
    
    outs() << "Wrote " << Filename << "\n";*/
    
    return 0;
}
