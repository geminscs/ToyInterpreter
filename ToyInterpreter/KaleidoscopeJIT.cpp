//
//  KaleidoscopeJIT.cpp
//  ToyInterpreter
//
//  Created by admin on 4/4/16.
//  Copyright © 2016 admin. All rights reserved.
//

#include "KaleidoscopeJIT.hpp"


std::string KaleidoscopeJIT::mangle(const std::string &name){
    std::string MangledName;
    {
        raw_string_ostream MangledNameString(MangledName);
        Mangler::getNameWithPrefix(MangledNameString, name, DL);
    }
    return MangledName;
}

template <typename T> std::vector<T> KaleidoscopeJIT::singletonSet(T t){
    std::vector<T> vec;
    vec.push_back(std::move(t));
    return std::move(vec);
}
JITSymbol KaleidoscopeJIT::findMangledSymbol(const std::string &Name){
    for (auto H : make_range(ModuleHandles.rbegin(), ModuleHandles.rend()))
        if (auto Sym = CompileLayer.findSymbolIn(H, Name, true))
            return Sym;
    
    // If we can't find the symbol in the JIT, try looking in the host process.
    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name))
        return JITSymbol(SymAddr, JITSymbolFlags::Exported);
    
    return nullptr;
}


KaleidoscopeJIT::KaleidoscopeJIT()
: TM(EngineBuilder().selectTarget()), DL(TM->createDataLayout()),
CompileLayer(ObjectLayer, SimpleCompiler(*TM)) {
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

TargetMachine &KaleidoscopeJIT::getTargetMachine(){
    return *TM;
}

KaleidoscopeJIT::ModuleHandleT KaleidoscopeJIT::addModule(std::unique_ptr<Module> M){
    auto Resolver = createLambdaResolver(
                                         [&](const std::string &Name) {
                                             if (auto Sym = findMangledSymbol(Name))
                                                 return RuntimeDyld::SymbolInfo(Sym.getAddress(), Sym.getFlags());
                                             return RuntimeDyld::SymbolInfo(nullptr);
                                         },
                                         [](const std::string &S) { return nullptr; });
    auto H = CompileLayer.addModuleSet(singletonSet(std::move(M)), make_unique<SectionMemoryManager>(),std::move(Resolver));
    ModuleHandles.push_back(H);
    return H;
}
void KaleidoscopeJIT::removeModule(ModuleHandleT H){
    ModuleHandles.erase(std::find(ModuleHandles.begin(), ModuleHandles.end(), H));
    CompileLayer.removeModuleSet(H);
}
JITSymbol KaleidoscopeJIT::findSymbol(const std::string name){
    return findMangledSymbol(mangle(name));
}