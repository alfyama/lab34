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
    std::set<Value *> definedVariables;
    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *LI = dyn_cast<LoadInst>(&I)) {
          Value *ptrOperand = LI->getPointerOperand();
          if (definedVariables.count(ptrOperand) == 0) {
            llvm::StoreInst *SI;

            auto *type = LI->getType();
            if (type->isIntegerTy()) {
              Value *init_int = ConstantInt::get(type, 10, true);
              SI = new StoreInst(init_int, ptrOperand, LI);
            } else if (type->isFloatTy()) {
              Value *init_float = ConstantFP::get(type, 20.0);
              SI = new StoreInst(init_float, ptrOperand, LI);
            } else if (type->isDoubleTy()) {
              Value *init_double = ConstantFP::get(type, 30.0);
              SI = new StoreInst(init_double, ptrOperand, LI);
            }

            // NOTE: Can there be a problem with inserting an instruction while
            // looping over them?
            SI->insertBefore(LI);
            definedVariables.insert(ptrOperand);
          }
        } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
          // If the instruction is a store instruction, add the stored variable
          // to the set of defined variables
          definedVariables.insert(SI->getPointerOperand());
        }
      }
    }

    // FIXME: should only return true if the IR was modified, i.e. there were
    // uses of uninitialized variables
    return true;
  }
};
} // namespace

char DefinitionPass::ID = 0;
char FixingPass::ID = 1;

// Pass registrations
static RegisterPass<DefinitionPass> X("def-pass", "Reaching definitions pass");
static RegisterPass<FixingPass> Y("fix-pass", "Fixing initialization pass");
