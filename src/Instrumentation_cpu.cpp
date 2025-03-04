
#include "Instrumentation_cpu.h"
#include "Utility.h"
#include "CodeMatching.h"
#include "Logging.h"

#include <llvm/IR/Type.h>
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Constants.h"

#include <list>
#include <string>

using namespace CPUAnalysis;
//using namespace CUDAAnalysis;
using namespace llvm;

/* This function configures the function found (e.g., calling conventions) and
saves pointer if needed. We also do logging. */
void confFunction(Function *found, Function **saveHere,
    GlobalValue::LinkageTypes linkage, const char *name)
{
#ifdef FPC_DEBUG
	std::string out = std::string("Found ") + std::string(name);
	CUDAAnalysis::Logging::info(out.c_str());
#endif

  if (saveHere != nullptr) // if we want to save the function pointer
  	*saveHere = found;
  if (found->getLinkage() != linkage)
  	found->setLinkage(linkage);
}

/** Set linkage as ODR **/
//void setODRLikage(Function *f, const char *name) {
//  if (f->getName().str().find(name) != std::string::npos) {
//    f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
//  }
//}
#define SET_ODR_LIKAGE(name) \
    if (f->getName().str().find(name) != std::string::npos) { \
      f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage); \
    }

CPUFPInstrumentation::CPUFPInstrumentation(Module *M) :
		mod(M),
		fp32_check_function(nullptr),
		fp64_check_function(nullptr),
		//fpc_init_htable(nullptr),
		fpc_init(nullptr),
		fpc_init_args(nullptr),
		fpc_print_locations(nullptr) {

#ifdef FPC_DEBUG
  CUDAAnalysis::Logging::info("Initializing instrumentation");
#endif

  // Find and configure instrumentation functions
  for(auto F = M->begin(), e = M->end(); F!=e; ++F)
  {
    Function *f = &(*F);
    if (f->getName().str().find("_FPC_FP32_CHECK_") != std::string::npos)
    {
      confFunction(f, &fp32_check_function,
      GlobalValue::LinkageTypes::LinkOnceODRLinkage, "_FPC_FP32_CHECK_");
    }
    if (f->getName().str().find("_FPC_FP64_CHECK_") != std::string::npos)
    {
      confFunction(f, &fp64_check_function,
      GlobalValue::LinkageTypes::LinkOnceODRLinkage, "_FPC_FP64_CHECK_");
    }
    if (f->getName().str().find("_FPC_INIT_FPCHECKER") != std::string::npos)
    {
      confFunction(f, &fpc_init,
      GlobalValue::LinkageTypes::LinkOnceODRLinkage, "_FPC_INIT_FPCHECKER");
    }
    if (f->getName().str().find("_FPC_INIT_ARGS_FPCHECKER") != std::string::npos)
    {
      confFunction(f, &fpc_init_args,
      GlobalValue::LinkageTypes::LinkOnceODRLinkage, "_FPC_INIT_ARGS_FPCHECKER");
    }
    if (f->getName().str().find("_FPC_PRINT_LOCATIONS_") != std::string::npos)
    {
      confFunction(f, &fpc_print_locations,
      GlobalValue::LinkageTypes::LinkOnceODRLinkage, "_FPC_PRINT_LOCATIONS_");
    }

    SET_ODR_LIKAGE("_FPC_FP32_IS_INF")
    SET_ODR_LIKAGE("_FPC_FP32_GET_MANTISSA")
    SET_ODR_LIKAGE("_FPC_FP32_GET_EXPONENT")
    SET_ODR_LIKAGE("_FPC_FP32_IS_INFINITY_POS")
    SET_ODR_LIKAGE("_FPC_FP32_IS_INFINITY_NEG")
    SET_ODR_LIKAGE("_FPC_FP32_IS_NAN")
    SET_ODR_LIKAGE("_FPC_FP32_IS_DIVISON_ZERO")
    SET_ODR_LIKAGE("_FPC_FP32_IS_CANCELLATION")
    SET_ODR_LIKAGE("_FPC_FP32_IS_COMPARISON")
    SET_ODR_LIKAGE("_FPC_FP32_IS_SUBNORMAL")
    SET_ODR_LIKAGE("_FPC_FP32_IS_LATENT_INFINITY")
    SET_ODR_LIKAGE("_FPC_FP32_IS_LATENT_INFINITY_POS")
    SET_ODR_LIKAGE("_FPC_FP32_IS_LATENT_INFINITY_NEG")
    SET_ODR_LIKAGE("_FPC_FP32_IS_LATENT_SUBNORMAL")
    SET_ODR_LIKAGE("_FPC_FP64_IS_INF")
    SET_ODR_LIKAGE("_FPC_FP64_GET_MANTISSA")
    SET_ODR_LIKAGE("_FPC_FP64_GET_EXPONENT")
    SET_ODR_LIKAGE("_FPC_FP64_IS_INFINITY_POS")
    SET_ODR_LIKAGE("_FPC_FP64_IS_INFINITY_NEG")
    SET_ODR_LIKAGE("_FPC_FP64_IS_NAN")
    SET_ODR_LIKAGE("_FPC_FP64_IS_DIVISON_ZERO")
    SET_ODR_LIKAGE("_FPC_FP64_IS_CANCELLATION")
    SET_ODR_LIKAGE("_FPC_FP64_IS_COMPARISON")
    SET_ODR_LIKAGE("_FPC_FP64_IS_SUBNORMAL")
    SET_ODR_LIKAGE("_FPC_FP64_IS_LATENT_INFINITY")
    SET_ODR_LIKAGE("_FPC_FP64_IS_LATENT_INFINITY_POS")
    SET_ODR_LIKAGE("_FPC_FP64_IS_LATENT_INFINITY_NEG")
    SET_ODR_LIKAGE("_FPC_FP64_IS_LATENT_SUBNORMAL")
    SET_ODR_LIKAGE("_FPC_EVENT_OCURRED")
    SET_ODR_LIKAGE("_FPC_FP32_CHECK_")
    SET_ODR_LIKAGE("_FPC_FP64_CHECK_")
    SET_ODR_LIKAGE("_FPC_TRAP_HERE")
    SET_ODR_LIKAGE("_FPC_STRING_ENDS_WITH")
    SET_ODR_LIKAGE("_FPC_CHECK_AND_TRAP")
    // Hash table
    SET_ODR_LIKAGE("_FPC_HT_CREATE_")
    SET_ODR_LIKAGE("_FPC_HT_HASH_")
    SET_ODR_LIKAGE("_FPC_HT_NEWPAIR_")
    SET_ODR_LIKAGE("_FPC_ITEMS_EQUAL_")
    SET_ODR_LIKAGE("_FPC_HT_SET_")
    SET_ODR_LIKAGE("_FPC_PRINT_HASH_TABLE_")
    SET_ODR_LIKAGE("_FPC_INIT_HASH_TABLE_")
  }

  // Globals initialization
  GlobalVariable *table = nullptr;
  table = mod->getGlobalVariable ("_FPC_HTABLE_", true);
  assert(table && "Invalid table!");
  table->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
  //table->setLinkage(GlobalValue::LinkageTypes::LinkOnceAnyLinkage);
  
  GlobalVariable *prog_inputs = nullptr;
  prog_inputs = mod->getGlobalVariable ("_FPC_PROG_INPUTS", true);
  assert(prog_inputs && "Invalid table!");
  prog_inputs->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);

  GlobalVariable *prog_args = nullptr;
  prog_args = mod->getGlobalVariable ("_FPC_PROG_ARGS", true);
  assert(prog_args && "Invalid table!");
  prog_args->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);

  GlobalVariable *fpc_lock = nullptr;
  fpc_lock = mod->getGlobalVariable ("fpc_lock", true);
  if (fpc_lock) {
    assert(fpc_lock && "Invalid lock!");
    fpc_lock->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
  }
 }

void CPUFPInstrumentation::instrumentFunction(Function *f, long int *c)
{
	if (CUDAAnalysis::CodeMatching::isUnwantedFunction(f))
		return;

  assert((fp32_check_function!=nullptr) && "Function not initialized!");
  assert((fp64_check_function!=nullptr) && "Function not initialized!");

#ifdef FPC_DEBUG
  CUDAAnalysis::Logging::info("Entering main loop in instrumentFunction");
#endif

	long int instrumentedOps = 0;
	for (auto bb=f->begin(), end=f->end(); bb != end; ++bb) {
		for (auto i=bb->begin(), bend=bb->end(); i != bend; ++i) {
			Instruction *inst = &(*i);

			if (isFPOperation(inst) && 
        (isSingleFPOperation(inst) || isDoubleFPOperation(inst))) {
				DebugLoc loc = inst->getDebugLoc();

				// Create builder to add stuff after the instruction
			  BasicBlock::iterator nextInst(inst);
        nextInst++;
			  IRBuilder<> builder( &(*nextInst) );

			  // Push parameters
			  std::vector<Value *> args;
			  if (!isCmpEqual(inst)) {
			    args.push_back(inst);
			  } else {
          if (isSingleFPOperation(inst))
			      args.push_back(ConstantFP::get(builder.getFloatTy(), 0.0));
          else
			      args.push_back(ConstantFP::get(builder.getDoubleTy(), 0.0));
			  }
				args.push_back(inst->getOperand(0));
				args.push_back(inst->getOperand(1));

				// Push location parameter (line number)
				int lineNumber = CUDAAnalysis::getLineOfCode(inst);
        // Discard if line number is invalid (no debug info for inst)
        if (lineNumber == -1)
          continue;
				ConstantInt* locId = ConstantInt::get(mod->getContext(),
				    APInt(32, lineNumber, true));
				args.push_back(locId);

				// Push file name
				// Get global fileName pointer
        GlobalVariable *fName = nullptr;
        fName = mod->getGlobalVariable("_ZL15_FPC_FILE_NAME_", true); // C++ binding
        if (fName==nullptr)
          fName = mod->getGlobalVariable("_FPC_FILE_NAME_", true); // try C binding
        assert((fName!=nullptr) && "Global filename var not found");
        auto loadInst = builder.CreateAlignedLoad(fName, MaybeAlign(), "my");

        std::string fileName = CUDAAnalysis::getFileNameFromInstruction(inst);
        Constant *c = builder.CreateGlobalStringPtr(fileName);
        fName->setInitializer(NULL);
        fName->setInitializer(c);
        args.push_back(loadInst);

        // Push operation type
        int operationType = 0;
        if      (inst->getOpcode() == Instruction::FAdd) operationType=0;
        else if (inst->getOpcode() == Instruction::FSub) operationType=1;
        else if (inst->getOpcode() == Instruction::FMul) operationType=2;
        else if (inst->getOpcode() == Instruction::FDiv) operationType=3;
        else if (isCmpEqual(inst))                       operationType=4;
        else if (inst->getOpcode() == Instruction::FRem) operationType=5;
        else operationType=-1;
        assert(operationType >=0 && "Unknown operation");

        ConstantInt* opType = ConstantInt::get(mod->getContext(),
            APInt(32, operationType, true));
        args.push_back(opType);

        // Check if instruction is selected based on a condition
        Instruction *select_inst = nullptr;
        Value *condition = nullptr; // condition value
        Value *cond_instr = nullptr; // condition instruction
        int inverse; // inverse the semantics of the condition?
        if (selectedBasedOnCondition(inst, f, &select_inst, &condition, &inverse)) {
          // Set insertion point after the select instruction
          assert(select_inst && "Invalid select instruction");
          BasicBlock::iterator nextOne(select_inst);
          nextOne++;
          builder.SetInsertPoint(&(*nextOne));
          // Inverse semantics of condition if needed
          if (inverse) {
            // Add XOR to negate the condition
            auto neg_inst = builder.CreateXor(condition, 1, "my");
            cond_instr = builder.CreateZExt(neg_inst, builder.getInt32Ty(), "my");
            assert(cond_instr && "Invalid extension instruction");
            args.push_back(cond_instr);
          } else {
            // Add extension of condition (from i1 to i32 integer)
            cond_instr = builder.CreateZExt(condition, builder.getInt32Ty(), "my");
            assert(cond_instr && "Invalid extension instruction");
            args.push_back(cond_instr);
          }

          // Add extension of condition (from i1 to i32 integer)
          /*cond_instr = builder.CreateZExt(condition, builder.getInt32Ty(), "my");
          assert(cond_instr && "Invalid extension instruction");
          args.push_back(cond_instr);
          // Inverse semantics of condition if needed
          if (inverse) {
            // Add XOR to negate the condition
            //auto neg_inst = builder.CreateXor(condition, 1, "my");
            //args.push_back(neg_inst);
            ConstantInt* inv = ConstantInt::get(mod->getContext(),
              APInt(32, 1, true));
            args.push_back(inv);
           } else {
            ConstantInt* inv = ConstantInt::get(mod->getContext(),
              APInt(32, 0, true));
            args.push_back(inv);
          }*/
        } else {
          ConstantInt* cond = ConstantInt::get(mod->getContext(),
            APInt(32, 1, true));
          args.push_back(cond);
          //ConstantInt* inv = ConstantInt::get(mod->getContext(),
          //  APInt(32, 0, true));
          //args.push_back(inv);
        }
				
        ArrayRef<Value *> args_ref(args);

				CallInst *callInst = nullptr;
        if (isSingleFPOperation(inst)) {
          callInst = builder.CreateCall(fp32_check_function, args_ref);
        } else if (isDoubleFPOperation(inst)) {
          callInst = builder.CreateCall(fp64_check_function, args_ref);
        }
        instrumentedOps++;
      
				assert(callInst && "Invalid call instruction!");
        setFakeDebugLocation(inst, callInst, f);
			}
		}
	}

#ifdef FPC_DEBUG
	std::stringstream out;
	out << "Instrumented operations: " << instrumentedOps;
	CUDAAnalysis::Logging::info(out.str().c_str());
	CUDAAnalysis::Logging::info("Leaving main loop in instrumentFunction");
#endif
  *c = instrumentedOps;
}


// We check if the instruction inst is used only by a select instruction.
// If that is the case, we set the condition value and select instruction.
// Logic: the function returns true if:
//    - The inst is used only by one select instruction
//    - The inst is not used by any other instruction 
bool CPUFPInstrumentation::selectedBasedOnCondition(Instruction *inst, 
      Function *f, Instruction **select_inst, Value **condition,
      int *inverse) {
  bool ret = false;
  int numSelect = 0;
  int others = 0;
  for (User *U : inst->users()) {
    if (Instruction *use_inst = dyn_cast<Instruction>(U)) {
      if (SelectInst *s_inst = dyn_cast<SelectInst>(use_inst)) {
        numSelect++;
        *select_inst = s_inst;
        *condition = s_inst->getOperand(0);
        if ( dyn_cast<Instruction>(s_inst->getOperand(1)) == inst) *inverse = 0;
        else *inverse = 1;
      } else {
        others++;
      }
    }
  }
  
  if (numSelect==1 && others==0)
    ret = true;

  return ret;
}

bool CPUFPInstrumentation::isCmpEqual(const Instruction *inst) {
  if (inst->getOpcode() == Instruction::FCmp) {
    if (const CmpInst *cmpInst = dyn_cast<CmpInst>(inst)) {
      if (cmpInst->getPredicate() == llvm::CmpInst::Predicate::FCMP_OEQ ||
          cmpInst->getPredicate() == llvm::CmpInst::Predicate::FCMP_UEQ)
          return true;
    }
  }
  return false;
}

bool CPUFPInstrumentation::isFPOperation(const Instruction *inst)
{
	return (
			(inst->getOpcode() == Instruction::FMul) ||
			(inst->getOpcode() == Instruction::FDiv) ||
			(inst->getOpcode() == Instruction::FAdd) ||
			(inst->getOpcode() == Instruction::FSub) ||
			isCmpEqual(inst)                         ||
			(inst->getOpcode() == Instruction::FRem)
				 );
}

bool CPUFPInstrumentation::isDoubleFPOperation(const Instruction *inst)
{
	if (!isFPOperation(inst))
		return false;
	//return inst->getType()->isDoubleTy();
  return inst->getOperand(0)->getType()->isDoubleTy();
}

bool CPUFPInstrumentation::isSingleFPOperation(const Instruction *inst)
{
	if (!isFPOperation(inst))
		return false;
	//return inst->getType()->isFloatTy();
	return inst->getOperand(0)->getType()->isFloatTy();
}

void CPUFPInstrumentation::setFakeDebugLocation(Instruction *old_inst, Instruction *new_inst, Function *f) {
  auto di = old_inst->getDebugLoc();
  if (!di) { // couldn't find debug info
    for (auto bb=f->begin(), end=f->end(); bb != end; ++bb) {
		  for (auto i=bb->begin(), bend=bb->end(); i != bend; ++i) {
			  Instruction *inst = &(*i);
        auto tmp_di = inst->getDebugLoc();
        if (tmp_di) {
          new_inst->setDebugLoc(inst->getDebugLoc());
          return;
        }
      }
    } 
  } else {
    new_inst->setDebugLoc(di);
    return;
  }
  // IF we reach it, it means we couldn't find debug information
  //new_inst->eraseFromParent();
  assert(new_inst->getDebugLoc() && "Invalid debug loc! Please use -g");
}

//void CPUFPInstrumentation::setFakeDebugLocation(Function *f, Instruction *inst)
//void CPUFPInstrumentation::setFakeDebugLocation(Instruction *old_inst, Instruction *new_inst)
//{
	//MDNode *node = f->getMetadata(0);
	//assert(node && "No metadata found - it is possible that debug information is missing (use -g)");
	//DebugLoc newLoc = DebugLoc::get(1, 1, node);
	//DebugLoc newLoc(node);
	//new_inst->setDebugLoc(old_inst->getDebugLoc());
//}

/* Returns the return instructions of a function */
/*InstSet FPInstrumentation::finalInstrutions(Function *f)
{
	InstSet finalInstructions;
	for (auto bb=f->begin(), end=f->end(); bb != end; ++bb)
	{
		for (auto i=bb->begin(), iend=bb->end(); i != iend; ++i)
		{
			Instruction *inst = &(*i);
			if (isa<ReturnInst>(inst))
				finalInstructions.insert(inst);
		}
	}
	return finalInstructions;
}*/

/* Returns the return first (non-phi) instruction of the module */
Instruction* CPUFPInstrumentation::firstInstrution()
{
	Instruction *inst = nullptr;
	for (auto f = mod->begin(), e =mod->end(); f != e; ++f)
	{
		// Discard function declarations
		if (f->isDeclaration())
			continue;

		//Function *F = &(*f);
		BasicBlock *bb = &(f->getEntryBlock());
		inst = bb->getFirstNonPHIOrDbgOrLifetime();
		break;
	}

	assert(inst && "Instruction not valid!");
	return inst;
}


void CPUFPInstrumentation::instrumentMainFunction(Function *f)
{
  /// ----------------- BEGIN --------------------------
  BasicBlock *bb = &(*(f->begin()));
  Instruction *inst = bb->getFirstNonPHIOrDbg();
  IRBuilder<> builder(inst);
  std::vector<Value *> args;

  CallInst *callInst = nullptr;
  // Check if function has argumments or not
  if (f->arg_size() == 2) {
    // Push parameters
    for (auto i = f->arg_begin(); i != f->arg_end(); ++i) {
      Value *v = &(*i);
      args.push_back(v);
    }
    ArrayRef<Value *> args_ref(args);
    callInst = builder.CreateCall(fpc_init_args, args_ref);
  } else {
    //ArrayRef<Value *> args_ref(args);
    callInst = builder.CreateCall(fpc_init, args);
  }
  assert(callInst && "Invalid call instruction!");
 
  // Set debug location 
  for (auto i=bb->begin(), bend=bb->end(); i != bend; ++i) {
    Instruction *inst = &(*i);
    if (inst->getDebugLoc()) {
      callInst->setDebugLoc(inst->getDebugLoc());
      break;
    }
  }
  //callInst->setDebugLoc(inst->getDebugLoc());
  assert(callInst->getDebugLoc() && "Invalid debug loc! Please use -g");


  /// ------------------ END ----------------------------
  /// Print table before end of function
  for (auto bb=f->begin(), end=f->end(); bb != end; ++bb) {
    for (auto i=bb->begin(), iend=bb->end(); i != iend; ++i) {
      Instruction *inst = &(*i);
      if (isa<ReturnInst>(inst) || isa<ResumeInst>(inst)) {
        std::vector<Value *> args;
        ArrayRef<Value *> args_ref(args);
        IRBuilder<> builder(inst);
        auto callInst = builder.CreateCall(fpc_print_locations, args_ref);
        assert(callInst && "Invalid call instruction!");
        callInst->setDebugLoc(inst->getDebugLoc());
        assert(callInst->getDebugLoc() && "Invalid debug loc! Please use -g");
      }
    }
  }
}
