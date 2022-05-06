#include "mwl-compile.hh"

namespace mwl {

mwl::Value *
Environment::generate_code_for() {
  return llvm::ConstantFP::get(TheContext, APFloat(Val));
}

}  // namespace mwl

