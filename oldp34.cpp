/* Name Surname */

// STL
#include <iostream>
#include <map>
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

    std::map<Value *, bool> initVals;

    // Basic block
    for (auto &BB : F) {
      // Instruction
      for (auto &I : BB) {
        // Get the operands of the instruction
        SmallVector<Value *, 4> Operands;
        for (User::op_iterator OI = I.op_begin(), OE = I.op_end(); OI != OE;
             ++OI) {
          Value *Op = *OI;
          if (Op->getType()->isPointerTy()) {
            Operands.push_back(Op);
          }
        }

        // Check if the operands are variables
        for (Value *Op : Operands) {
          // If the variable has not been seen before, add it to the map with a
          // value of false
          if (initVals.count(Op) == 0) {
            initVals[Op] = false;
          }

          // Mark the variable as initialized
          initVals[Op] = true;
        }
      }
    }

    // Iterate over all entries in the map
    for (auto &var : initVals) {
      // If the value is false, print the name of the variable as a warning
      if (!var.second) {
        errs() << "WARNING: Variable '" << *var.first << "' is uninitialized\n";
      }
    }

    errs() << "def-pass\n";
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
