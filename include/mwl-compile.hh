#pragma once

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

namespace mwl {

class Environment {
private:
    llvm::LLVMContext _context;
    llvm::IRBuilder<> _builder;
    std::unique_ptr<Module> _module;
    std::map<std::string, Value *> _namedValues;
public:
    Environment() : _builder(_context) {}

    //static Value *LogErrorV(const char *Str) { LogError(Str); return nullptr; }
    
    Value * generate_code_for( const mwl_ConstVal * );
};

}  // namespace mwl

