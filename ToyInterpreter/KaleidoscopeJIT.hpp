//
//  KaleidoscopeJIT.hpp
//  ToyInterpreter
//
//  Created by admin on 4/4/16.
//  Copyright © 2016 admin. All rights reserved.
//

#ifndef KaleidoscopeJIT_hpp
#define KaleidoscopeJIT_hpp
#include <vector>
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/DynamicLibrary.h"

using namespace llvm;
using namespace llvm::orc;

class KaleidoscopeJIT{
public:
    typedef ObjectLinkingLayer<> ObjLayerT;
    typedef IRCompileLayer<ObjLayerT> CompileLayerT;
    typedef CompileLayerT::ModuleSetHandleT ModuleHandleT;
    
    KaleidoscopeJIT();
    TargetMachine &getTargetMachine();
    ModuleHandleT addModule(std::unique_ptr<Module> M);
    void removeModule(ModuleHandleT H);
    JITSymbol findSymbol(const std::string name);
private:
    std::unique_ptr<TargetMachine> TM;
    const DataLayout DL;
    ObjLayerT ObjectLayer;
    CompileLayerT CompileLayer;
    std::vector<ModuleHandleT> ModuleHandles;
    
    std::string mangle(const std::string &name);
    template <typename T> static std::vector<T> singletonSet(T t);
    JITSymbol findMangledSymbol(const std::string &Name);

};

#endif /* KaleidoscopeJIT_hpp */
