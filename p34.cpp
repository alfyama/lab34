/* Name Surname */

// STL
#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <utility>
#include <vector>

// LLVM
#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>

// For older versions of llvm you may have to include instead:
// #include "llvm/Support/CFG.h"
// #include <llvm/Support/InstIterator.h>

using namespace llvm;

namespace {

struct DefinitionPass : public FunctionPass {
public:
  static char ID;
  DefinitionPass() : FunctionPass(ID) {}

  virtual void getAnalysisUsage(AnalysisUsage &au) const override {
    au.setPreservesAll();
  }

  virtual bool runOnFunction(Function &F) override {
    // errs() << "Function " << F.getName() << "\n";
    std::set<Value *> definedVariables;
    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *LI = dyn_cast<LoadInst>(&I)) {
          // If the instruction is a load instruction
          Value *ptrOperand = LI->getPointerOperand();
          // Check if the loaded variable has been defined yet
          if (definedVariables.count(ptrOperand) == 0) {
            // If not, print a message indicating a possible
            // use-before-initialization
            errs() << ptrOperand->getName() << "\n";
          }
        } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
          // If the instruction is a store instruction, add the stored variable
          // to the set of defined variables
          definedVariables.insert(SI->getPointerOperand());
        }
      }
    }
    return false;
  }
};

class FixingPass : public FunctionPass {
public:
  static char ID;
  FixingPass() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F) {
    // TODO
    errs() << "fix-pass\n";
    return true;
  }
};
} // namespace

char DefinitionPass::ID = 0;
char FixingPass::ID = 1;

// Pass registrations
static RegisterPass<DefinitionPass> X("def-pass", "Reaching definitions pass");
static RegisterPass<FixingPass> Y("fix-pass", "Fixing initialization pass");
