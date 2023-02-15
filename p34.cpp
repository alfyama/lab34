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
    // std::set<Value *> definedVariables;
    // for (auto &BB : F) {
    //   for (auto &I : BB) {
    //     if (auto *LI = dyn_cast<LoadInst>(&I)) {
    //       // If the instruction is a load instruction
    //       Value *ptrOperand = LI->getPointerOperand();
    //       // Check if the loaded variable has been defined yet
    //       if (definedVariables.count(ptrOperand) == 0) {
    //         // If not, print a message indicating a possible
    //         // use-before-initialization
    //         errs() << ptrOperand->getName() << "\n";
    //       }
    //     } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
    //       // If the instruction is a store instruction, add the stored
    //       variable
    //       // to the set of defined variables
    //       definedVariables.insert(SI->getPointerOperand());
    //     }
    //   }
    // }

    std::map<Value *, std::set<Instruction *>> defUseMap;
    std::set<Value *> initializedSet;

    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *storeInst = dyn_cast<StoreInst>(&I)) {
          initializedSet.insert(storeInst->getPointerOperand());
          defUseMap[storeInst->getPointerOperand()].insert(storeInst);
        } else if (auto *loadInst = dyn_cast<LoadInst>(&I)) {
          Value *ptrOperand = loadInst->getPointerOperand();
          if (initializedSet.find(ptrOperand) == initializedSet.end()) {
            errs() << ptrOperand->getName() << "\n";
          }
        } else if (auto *brInst = dyn_cast<BranchInst>(&I)) {
          if (brInst->isConditional()) {
            Value *cond = brInst->getCondition();
            if (auto *loadInst = dyn_cast<LoadInst>(cond)) {
              Value *ptrOperand = loadInst->getPointerOperand();
              if (initializedSet.find(ptrOperand) == initializedSet.end()) {
                errs() << ptrOperand->getName() << "\n";
              }
            }

            // FIXME Iterate over instructions within conditional block
          }
        } else if (auto *switchInst = dyn_cast<SwitchInst>(&I)) {
          Value *cond = switchInst->getCondition();
          if (auto *loadInst = dyn_cast<LoadInst>(cond)) {
            Value *ptrOperand = loadInst->getPointerOperand();
            if (initializedSet.find(ptrOperand) == initializedSet.end()) {
              errs() << ptrOperand->getName() << "\n";
            }
          }
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
    // NOTE: we maybe don't even need this because of uses() and users()
    // methods, but for now it is fine.
    std::set<Value *> definedVariables;

    for (auto &BB : F) {
      for (auto &I : BB) {
        if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
          Value *ptrOperand = LI->getPointerOperand();
          if (definedVariables.count(ptrOperand) == 0) {
            StoreInst *SI;

            Instruction *marker = nullptr;
            for (Use &U : ptrOperand->uses()) {
              if (auto AI = dyn_cast<AllocaInst>(U)) {
                // NOTE: The StoreInst constructor forces us to pass an
                // insertBefore parameter. Because of this, we need this kinda
                // awkward logic here.
                marker = AI->getNextNonDebugInstruction();
                break;
              }
            }

            if (marker == nullptr) {
              errs() << "No declaration found for variable "
                     << ptrOperand->getName()
                     << ". Not our problem, crashing ¯\\_(ツ)_/¯\n";
              exit(EXIT_FAILURE);
            }

            Type *type = LI->getType();
            if (type->isIntegerTy()) {
              Value *init_int = ConstantInt::get(type, 10, true);
              SI = new StoreInst(init_int, ptrOperand, marker);
            } else if (type->isFloatTy()) {
              Value *init_float = ConstantFP::get(type, 20.0);
              SI = new StoreInst(init_float, ptrOperand, marker);
            } else if (type->isDoubleTy()) {
              Value *init_double = ConstantFP::get(type, 30.0);
              SI = new StoreInst(init_double, ptrOperand, marker);
            }

            definedVariables.insert(ptrOperand);
          }
        } else if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
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
