
#include "Instrumentation.h"
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

using namespace CUDAAnalysis;
using namespace llvm;

FPInstrumentation::FPInstrumentation(Module *M) :
		mod(M),
		fp32_check_add_function(nullptr),
		fp32_check_sub_function(nullptr),
		fp32_check_mul_function(nullptr),
		fp32_check_div_function(nullptr),
		fp64_check_add_function(nullptr),
		fp64_check_sub_function(nullptr),
		fp64_check_mul_function(nullptr),
		fp64_check_div_function(nullptr)
{

#ifdef FPC_DEBUG
	Logging::info("Initializing instrumentation");
	std::stringstream out;
	out << "Pointer value (fp32_check_add_function): " << fp32_check_add_function;
	Logging::info(out.str().c_str());
#endif

  // Find instrumentation function
  for(auto F = M->begin(), e = M->end(); F!=e; ++F)
  {
    Function *f = &(*F);
    if (f->getName().str().find("_FPC_FP32_CHECK_ADD_") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP32_CHECK_ADD_");
#endif

    	fp32_check_add_function = f;
    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    	fp32_check_add_function->setCallingConv(CallingConv::PTX_Device);
    	f->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
    else if (f->getName().str().find("_FPC_FP32_CHECK_SUB_") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP32_CHECK_SUB_");
#endif

    	fp32_check_sub_function = f;
    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    	fp32_check_sub_function->setCallingConv(CallingConv::PTX_Device);
    	f->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
    else if (f->getName().str().find("_FPC_FP32_CHECK_MUL_") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP32_CHECK_MUL_");
#endif

    	fp32_check_mul_function = f;
    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    	fp32_check_mul_function->setCallingConv(CallingConv::PTX_Device);
	f->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
    else if (f->getName().str().find("_FPC_FP32_CHECK_DIV_") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP32_CHECK_DIV_");
#endif

    	fp32_check_div_function = f;
    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    	fp32_check_div_function->setCallingConv(CallingConv::PTX_Device);
	f->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
    else if (f->getName().str().find("_FPC_FP64_CHECK_ADD_") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP64_CHECK_ADD_");
#endif

    	fp64_check_add_function = f;
    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    	fp64_check_add_function->setCallingConv(CallingConv::PTX_Device);
	f->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
    else if (f->getName().str().find("_FPC_FP64_CHECK_SUB_") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP64_CHECK_SUB_");
#endif

    	fp64_check_sub_function = f;
    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    	fp64_check_sub_function->setCallingConv(CallingConv::PTX_Device);
	f->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
    else if (f->getName().str().find("_FPC_FP64_CHECK_MUL_") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP64_CHECK_MUL_");
#endif

    	fp64_check_mul_function = f;
    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    	fp64_check_mul_function->setCallingConv(CallingConv::PTX_Device);
	f->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
    else if (f->getName().str().find("_FPC_FP64_CHECK_DIV_") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP64_CHECK_DIV_");
#endif

    	fp64_check_div_function = f;
    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    	fp64_check_div_function->setCallingConv(CallingConv::PTX_Device);
	f->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
    else if (f->getName().str().find("_FPC_INTERRUPT_") != std::string::npos)
    {
    	_fpc_interrupt_ = f;
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_INTERRUPT_");
#endif

    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    }
    else if (f->getName().str().find("_FPC_FP32_IS_SUBNORMAL") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP32_IS_SUBNORMAL");
#endif

    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    }
    else if (f->getName().str().find("_FPC_FP64_IS_SUBNORMAL") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP64_IS_SUBNORMAL");
#endif

    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    }
    else if (f->getName().str().find("_FPC_DEVICE_CODE_FUNC_") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_DEVICE_CODE_FUNC_");
#endif

    	if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    		f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    }
    else if (f->getName().str().find("_FPC_WARNING_") != std::string::npos)
    {
    	_fpc_warning_ = f;
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_WARNING_");
#endif
    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    }
    else if (f->getName().str().find("_FPC_FP32_IS_ALMOST_OVERFLOW") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP32_IS_ALMOST_OVERFLOW");
#endif

    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    }
    else if (f->getName().str().find("_FPC_FP32_IS_ALMOST_SUBNORMAL") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP32_IS_ALMOST_SUBNORMAL");
#endif

    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    }
    else if (f->getName().str().find("_FPC_FP64_IS_ALMOST_OVERFLOW") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP64_IS_ALMOST_OVERFLOW");
#endif

    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    }
    else if (f->getName().str().find("_FPC_FP64_IS_ALMOST_SUBNORMAL") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_FP64_IS_ALMOST_SUBNORMAL");
#endif

    	//if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    	//	f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    }
    else if (f->getName().str().find("_FPC_PRINT_ERRORS_") != std::string::npos)
    {
    	_print_errors_ = f;
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_PRINT_ERRORS_");
#endif
    }
  }

  //printf("Value:  %p\n", fp32_check_add_function);

  // ---- Find instrumentation function in HOST code ----
  for(auto F = M->begin(), e = M->end(); F!=e; ++F)
  {
    Function *f = &(*F);
    if (f->getName().str().find("_FPC_PRINT_AT_MAIN_") != std::string::npos)
    {
#ifdef FPC_DEBUG
    	Logging::info("Found _FPC_PRINT_AT_MAIN_");
#endif
    	print_at_main = f;
    	if (f->getLinkage() != GlobalValue::LinkageTypes::LinkOnceODRLinkage)
    		f->setLinkage(GlobalValue::LinkageTypes::LinkOnceODRLinkage);
    }
  }
}

void FPInstrumentation::instrumentFunction(Function *f)
{
	if (CodeMatching::isUnwantedFunction(f))
		return;

  assert((fp32_check_add_function!=NULL) && "Function not initialized!");
  assert((fp64_check_add_function!=nullptr) && "Function not initialized!");

#ifdef FPC_DEBUG
	Logging::info("Entering main loop in instrumentFunction");
#endif

	int instrumentedOps = 0;
	for (auto bb=f->begin(), end=f->end(); bb != end; ++bb)
	{
		for (auto i=bb->begin(), bend=bb->end(); i != bend; ++i)
		{
			Instruction *inst = &(*i);

			if (isFPOperation(inst))
			{
				DebugLoc loc = inst->getDebugLoc();
				IRBuilder<> builder = createBuilderAfter(inst);

				// Push parameters
				std::vector<Value *> args;
				args.push_back(inst);
				args.push_back(inst->getOperand(0));
				args.push_back(inst->getOperand(1));
				//args.push_back(ConstantFP::get(builder.getDoubleTy(), 999.0));

				// Push location parameter (line number)
				int lineNumber = getLineOfCode(inst);
				ConstantInt* locId = ConstantInt::get(mod->getContext(), APInt(32, lineNumber, true));
				args.push_back(locId);

				// Update max location number
				if (lineNumber > maxNumLocations)
					maxNumLocations = lineNumber;

				ArrayRef<Value *> args_ref(args);

				CallInst *callInst = nullptr;

				if (inst->getOpcode() == Instruction::FAdd)
				{
					if (isSingleFPOperation(inst))
					{
						callInst = builder.CreateCall(fp32_check_add_function, args_ref);
						instrumentedOps++;
					}
					else if (isDoubleFPOperation(inst))
					{
						callInst = builder.CreateCall(fp64_check_add_function, args_ref);
						instrumentedOps++;
					}
				}
				else if (inst->getOpcode() == Instruction::FSub)
				{
					if (isSingleFPOperation(inst))
					{
						callInst = builder.CreateCall(fp32_check_sub_function, args_ref);
						instrumentedOps++;
					}
					else if (isDoubleFPOperation(inst))
					{
						callInst = builder.CreateCall(fp64_check_sub_function, args_ref);
						instrumentedOps++;
					}
				}
				else if (inst->getOpcode() == Instruction::FMul)
				{
					if (isSingleFPOperation(inst))
					{
						callInst = builder.CreateCall(fp32_check_mul_function, args_ref);
						instrumentedOps++;
					}
					else if (isDoubleFPOperation(inst))
					{
						callInst = builder.CreateCall(fp64_check_mul_function, args_ref);
						instrumentedOps++;
					}
				}
				else if (inst->getOpcode() == Instruction::FDiv)
				{
					if (isSingleFPOperation(inst))
					{
						callInst = builder.CreateCall(fp32_check_div_function, args_ref);
						instrumentedOps++;
					}
					else if (isDoubleFPOperation(inst))
					{
						callInst = builder.CreateCall(fp64_check_div_function, args_ref);
						instrumentedOps++;
					}
				}

				assert(callInst && "Invalid call instruction!");
				setFakeDebugLocation(f, callInst);
			}
		}
	}

#ifdef FPC_DEBUG
	std::stringstream out;
	out << "Instrumented operations: " << instrumentedOps;
	Logging::info(out.str().c_str());
	Logging::info("Leaving main loop in instrumentFunction");
#endif
}

bool FPInstrumentation::isFPOperation(const Instruction *inst)
{
	return (
			(inst->getOpcode() == Instruction::FMul) ||
			(inst->getOpcode() == Instruction::FDiv) ||
			(inst->getOpcode() == Instruction::FAdd) ||
			(inst->getOpcode() == Instruction::FSub)
				 );
}

bool FPInstrumentation::isDoubleFPOperation(const Instruction *inst)
{
	if (!isFPOperation(inst))
		return false;
	return inst->getType()->isDoubleTy();
}

bool FPInstrumentation::isSingleFPOperation(const Instruction *inst)
{
	if (!isFPOperation(inst))
		return false;
	return inst->getType()->isFloatTy();
}

IRBuilder<> FPInstrumentation::createBuilderAfter(Instruction *inst)
{
	// Get next instruction
  BasicBlock::iterator tmpIt(inst);
  tmpIt++;
  Instruction *nextInst = &(*(tmpIt));
  assert(nextInst && "Invalid instruction!");

	IRBuilder<> builder(nextInst);

	return builder;
}

IRBuilder<> FPInstrumentation::createBuilderBefore(Instruction *inst)
{
	IRBuilder<> builder(inst);

	return builder;
}

void FPInstrumentation::setFakeDebugLocation(Function *f, Instruction *inst)
{
	MDNode *node = f->getMetadata(0);
	assert(node && "Invalid node!");
	DebugLoc newLoc = DebugLoc::get(1, 1, node);
	inst->setDebugLoc(newLoc);
}

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
Instruction* FPInstrumentation::firstInstrution()
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

void FPInstrumentation::generateCodeForInterruption()
{

#ifdef FPC_DEBUG
	std::string out = "Generating code for interruption...";
	Logging::info(out.c_str());
#endif

	//for (auto i = mod->global_begin(), end = mod->global_end(); i != end; ++i)
	//{
	//	outs() << "Global: " << *i << "\n";
	//}

	//errs() << "in generteCodeForInterruption\n";
	GlobalVariable *gArray = nullptr;
	gArray = mod->getGlobalVariable ("_ZL15_FPC_FILE_NAME_", true);
	assert((gArray!=nullptr) && "Global array not found");
	//errs() << "setting linkage\n";
	gArray->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
	//errs() << "got array ptr\n";

	std::vector<Constant*> values;
	Instruction *i = firstInstrution();
	//printf("Value:  i: %p\n", i);
	IRBuilder<> builder(i);

	//errs() << "created builder\n";
	std::string fileName = getFileNameFromModule(mod);
	Constant *c = builder.CreateGlobalStringPtr(fileName);
	values.push_back(c);
	//errs() << "pushed value\n";

	ArrayType *t = ArrayType::get(Type::getInt8PtrTy(mod->getContext()), 1);
	//errs() << "got array type\n";
	Constant* init = ConstantArray::get(t, values);

	gArray->setInitializer(NULL);
	//errs() << "Removed existing initializer\n";
	gArray->setInitializer(init);
	//errs() << "initialized\n";
}

void FPInstrumentation::instrumentMainFunction(Function *f)
{
	BasicBlock *bb = &(*(f->begin()));
	Instruction *inst = bb->getFirstNonPHIOrDbg();
	IRBuilder<> builder = createBuilderBefore(inst);
	std::vector<Value *> args;

	CallInst *callInst = nullptr;
	callInst = builder.CreateCall(print_at_main, args);
	assert(callInst && "Invalid call instruction!");
	setFakeDebugLocation(f, callInst);
}


// Generates a global array on n integers, initialized to zero
GlobalVariable* FPInstrumentation::generateIntArrayGlobalVariable(ArrayType *arrType)
{
	GlobalVariable *newGv = nullptr;
	newGv = new GlobalVariable(*mod, arrType, false,
				GlobalValue::LinkageTypes::InternalLinkage, 0,"myVar",
				nullptr, GlobalValue::ThreadLocalMode::NotThreadLocal, 1, true);

	ConstantAggregateZero* const_array = ConstantAggregateZero::get(arrType);
	newGv->setInitializer(const_array);

	return newGv;
}

void FPInstrumentation::createReadFunctionForGlobalArray(GlobalVariable *arr, ArrayType *arrType, std::string funcName)
{
	for (auto f = mod->begin(), fend = mod->end(); f != fend; ++f)
	{
		if (f->getName().str().find(funcName) != std::string::npos)
		{
#ifdef FPC_DEBUG
			std::string out = "found function: " + funcName;
			Logging::info(out.c_str());
#endif

			// Find return instruction (last instruction)
			// We only have a single basic block
			Instruction *retInst = &(f->begin()->back());
			assert(isa<ReturnInst>(retInst) && "Not a return instruction");

			// Get instruction before the return
			BasicBlock::iterator tmpIt(retInst);
			tmpIt--;
			Instruction *prevInst = &(*(tmpIt));
			assert(prevInst && "Invalid instruction!");

			IRBuilder<> builder = createBuilderBefore(retInst);

			// Create signed extension of parameter
			auto arg = f->arg_begin();
			//auto sext = builder.CreateSExt(arg, Type::getInt64Ty(mod->getContext()), "my");

			// Create GEP inst and addr-space cast inst
			std::vector<Value *> args;
			args.push_back(ConstantInt::get(Type::getInt64Ty(mod->getContext()), 0));
			args.push_back(arg);
			ArrayRef<Value *> indexList(args);
			auto gep = builder.CreateInBoundsGEP(arrType, arr, indexList, "my");
			auto addCast = new AddrSpaceCastInst(gep, Type::getInt64PtrTy(mod->getContext(), 0), "my", retInst);
			auto loadInst = builder.CreateAlignedLoad (addCast, 4, "my");
			retInst->setOperand(0, loadInst);

			// Now we remove old (unused) instructions
			auto iter = f->begin()->begin();
			Instruction *old = &(*iter);
			std::list<Instruction *> iList;
			while (old != prevInst)
			{
				iList.push_back(old);
				iter++;
				old = &(*iter);
			}
			iList.push_back(prevInst);

			for (std::list<Instruction *>::reverse_iterator rit=iList.rbegin(); rit!=iList.rend(); ++rit)
			{
				//outs() << "removing: " << inst2str(*rit) << "\n";
			  (*rit)->eraseFromParent();
			}

			break;
		}
	}
}

void FPInstrumentation::createWriteFunctionForGlobalArray(GlobalVariable *arr, ArrayType *arrType, std::string funcName)
{
	for (auto f = mod->begin(), fend = mod->end(); f != fend; ++f)
	{
		if (f->getName().str().find(funcName) != std::string::npos)
		{
#ifdef FPC_DEBUG
			std::string out = "found function: " + funcName;
			Logging::info(out.c_str());
#endif

			// Find return instruction (last instruction)
			// We only have a single basic block
			Instruction *retInst = &(f->begin()->back());
			assert(isa<ReturnInst>(retInst) && "Not a return instruction");

			// Get instruction before the return
			BasicBlock::iterator tmpIt(retInst);
			tmpIt--;
			Instruction *prevInst = &(*(tmpIt));
			assert(prevInst && "Invalid instruction!");

			IRBuilder<> builder = createBuilderBefore(retInst);

			// Create signed extension of parameter
			auto arg = f->arg_begin();
			//auto sext = builder.CreateSExt(arg, Type::getInt64Ty(mod->getContext()), "my");

			// Create GEP inst and addr-space cast inst
			std::vector<Value *> args;
			args.push_back(ConstantInt::get(Type::getInt64Ty(mod->getContext()), 0));
			args.push_back(arg);
			ArrayRef<Value *> indexList(args);
			auto gep = builder.CreateInBoundsGEP(arrType, arr, indexList, "my");
			auto addCast = new AddrSpaceCastInst(gep, Type::getInt64PtrTy(mod->getContext(), 0), "my", retInst);
			arg++;
			Value *val = &(*arg);
			auto storeInst = builder.CreateAlignedStore(val, addCast, 8, false);
			//auto loadInst = builder.CreateAlignedLoad (addCast, 4, "my");

			// Now we remove old (unused) instructions
			auto iter = f->begin()->begin();
			Instruction *old = &(*iter);
			std::list<Instruction *> iList;
			while (old != prevInst)
			{
				iList.push_back(old);
				iter++;
				old = &(*iter);
			}
			iList.push_back(prevInst);

			for (std::list<Instruction *>::reverse_iterator rit=iList.rbegin(); rit!=iList.rend(); ++rit)
			{
				//outs() << "removing: " << inst2str(*rit) << "\n";
			  (*rit)->eraseFromParent();
			}

			break;
		}
	}

}

void FPInstrumentation::instrumentErrorArray()
{
	// Set size of the global array
	int extra = 10;
	int elems = maxNumLocations + extra;

	// Modify initializer of array size for global error array
	GlobalVariable *arrSize = nullptr;
	arrSize = mod->getGlobalVariable ("_ZL17errors_array_size", true);
	assert((arrSize!=nullptr) && "Global array not found");
	auto constSize = ConstantInt::get (Type::getInt64Ty(mod->getContext()), (uint64_t)elems, true);
	arrSize->setInitializer(constSize);


	/* --- Modify begin of INTERRUPT runtime function ------------------------- */
	ArrayType *arrType = ArrayType::get(Type::getInt64Ty(mod->getContext()), elems);
	GlobalVariable *newGv = generateIntArrayGlobalVariable(arrType);
#ifdef FPC_DEBUG
        Logging::info("Global errors array created");
#endif

	auto bb = _fpc_interrupt_->begin();
	Instruction *inst = &(*(bb->getFirstNonPHIOrDbg()));
	IRBuilder<> builder = createBuilderBefore(inst);

	auto arg = _fpc_interrupt_->arg_begin();
	arg++; arg++; // get third arg
	auto sext = builder.CreateSExt(arg, Type::getInt64Ty(mod->getContext()), "my");

	std::vector<Value *> args;
	args.push_back(ConstantInt::get(Type::getInt64Ty(mod->getContext()), 0));
	args.push_back(sext);
	ArrayRef<Value *> indexList(args);

	auto gep = builder.CreateInBoundsGEP(arrType, newGv, indexList, "my");
	auto addCast = new AddrSpaceCastInst(gep, Type::getInt64PtrTy(mod->getContext(), 0), "my", inst);
	/*AtomicRMWInst *atomic = builder.CreateAtomicRMW(
				AtomicRMWInst::Add,
				addCast,
				ConstantInt::get(Type::getInt64Ty(mod->getContext()), 1),
				AtomicOrdering::SequentiallyConsistent, SyncScope::System);
#ifdef FPC_DEBUG
	std::string out = "atomic " + inst2str(atomic) + " created";
	Logging::info(out.c_str());
#endif
	 */

	auto errType = _fpc_interrupt_->arg_begin();
	// subtract 1 from the error type
	auto ext = builder.CreateSExt(errType, Type::getInt64Ty(mod->getContext()), "my");
	Value *subInst = builder.CreateSub (ext, ConstantInt::get(Type::getInt64Ty(mod->getContext()), 3), "my", false, false);

	AtomicCmpXchgInst *cmpXchg = builder.CreateAtomicCmpXchg(
			addCast,
			ConstantInt::get(Type::getInt64Ty(mod->getContext()), 0),
			subInst,
			AtomicOrdering::SequentiallyConsistent,
			AtomicOrdering::SequentiallyConsistent,
			SyncScope::System);
#ifdef FPC_DEBUG
	std::string out = "cmpxchg " + inst2str(cmpXchg) + " created";
	Logging::info(out.c_str());
#endif

	/* ----------- Instrument _FPC_READ_GLOBAL_ERRORS_ARRAY -------------*/
	createReadFunctionForGlobalArray(newGv, arrType, "_FPC_READ_GLOBAL_ERRORS_ARRAY_");
	/* ------------------------------------------------------------------------ */
	/* ----------- Instrument _FPC_WRITE_GLOBAL_ERRORS_ARRAY -------------*/
	createWriteFunctionForGlobalArray(newGv, arrType, "_FPC_WRITE_GLOBAL_ERRORS_ARRAY_");
	/* ------------------------------------------------------------------------ */

	/* ============= Instrumentation for Warning function ======================*/
	GlobalVariable *newWarningsGv = generateIntArrayGlobalVariable(arrType);

	auto bbTmp = _fpc_warning_->begin();
	Instruction *firstInst = &(*(bbTmp->getFirstNonPHIOrDbg()));
	IRBuilder<> builderTmp = createBuilderBefore(firstInst);

	auto argTmp = _fpc_warning_->arg_begin();
	argTmp++; argTmp++; argTmp++; argTmp++; // get 5th arg
	auto bitcastTmp = builderTmp.CreateBitCast (argTmp, Type::getInt64Ty(mod->getContext()), "my");

	auto argTmp2 = _fpc_warning_->arg_begin();
	argTmp2++; argTmp2++; // get third arg
	auto sextTmp = builderTmp.CreateSExt(argTmp2, Type::getInt64Ty(mod->getContext()), "my");

	std::vector<Value *> argsTmp;
	argsTmp.push_back(ConstantInt::get(Type::getInt64Ty(mod->getContext()), 0));
	argsTmp.push_back(sextTmp);
	ArrayRef<Value *> indexTmp(argsTmp);

	auto gepTmp = builderTmp.CreateInBoundsGEP(arrType, newWarningsGv, indexTmp, "my");
	//setFakeDebugLocation(_fpc_interrupt_, gep);

	auto addCastTmp = new AddrSpaceCastInst(gepTmp, Type::getInt64PtrTy(mod->getContext(), 0), "my", firstInst);
	//setFakeDebugLocation(_fpc_interrupt_, addCast);

	AtomicCmpXchgInst *cmpX = builderTmp.CreateAtomicCmpXchg(
			addCastTmp,
			ConstantInt::get(Type::getInt64Ty(mod->getContext()), 0),
			bitcastTmp,
			AtomicOrdering::SequentiallyConsistent,
			AtomicOrdering::SequentiallyConsistent,
			SyncScope::System);
#ifdef FPC_DEBUG
	std::string out2 = "cmpxchg " + inst2str(cmpX) + " created";
	Logging::info(out2.c_str());
#endif

	/* ----------- Instrument _Z28_FPC_READ_FP64_GLOBAL_ARRAY_Pyi -------------*/
	createReadFunctionForGlobalArray(newWarningsGv, arrType, "_FPC_READ_FP64_GLOBAL_ARRAY_");

	/*for (auto f = mod->begin(), fend = mod->end(); f != fend; ++f)
	{
		if (f->getName().str().find("_Z28_FPC_READ_FP64_GLOBAL_ARRAY_i") != std::string::npos)
		{
#ifdef FPC_DEBUG
			Logging::info("found function: _Z28_FPC_READ_FP64_GLOBAL_ARRAY_i");
#endif

			// Find return instruction (last instruction)
			// We only have a single basic block
			Instruction *retInst = &(f->begin()->back());
			assert(isa<ReturnInst>(retInst) && "Not a return instruction");

			// Get instruction before the return
			BasicBlock::iterator tmpIt(retInst);
			tmpIt--;
			Instruction *prevInst = &(*(tmpIt));
			assert(prevInst && "Invalid instruction!");

			IRBuilder<> builder = createBuilderBefore(retInst);

			// Create signed extension of parameter
			auto arg = f->arg_begin();
			//arg++; // get 2nd arg
			auto sext = builder.CreateSExt(arg, Type::getInt64Ty(mod->getContext()), "my");

			// Create GEP inst and addr-space cast inst
			std::vector<Value *> args;
			args.push_back(ConstantInt::get(Type::getInt64Ty(mod->getContext()), 0));
			args.push_back(sext);
			ArrayRef<Value *> indexList(args);
			auto gep = builder.CreateInBoundsGEP(warArrayType, newWarningsGv, indexList, "my");
			auto addCast = new AddrSpaceCastInst(gep, Type::getInt64PtrTy(mod->getContext(), 0), "my", retInst);
			auto loadInst = builder.CreateAlignedLoad (addCast, 8, "my");
			retInst->setOperand(0, loadInst);

			// Now we remove old (unused) instructions
			auto iter = f->begin()->begin();
			Instruction *old = &(*iter);
			std::list<Instruction *> iList;
			while (old != prevInst)
			{
				iList.push_back(old);
				iter++;
				old = &(*iter);
			}
			iList.push_back(prevInst);

			for (std::list<Instruction *>::reverse_iterator rit=iList.rbegin(); rit!=iList.rend(); ++rit)
			{
				//outs() << "removing: " << inst2str(*rit) << "\n";
			  (*rit)->eraseFromParent();
			}

			break;
		}
	}*/
	/* ------------------------------------------------------------------------ */

	/* ----------- Instrument _Z31_FPC_WRITE_GLOBAL_ERRORS_ARRAY_ii -------------*/
	createWriteFunctionForGlobalArray(newWarningsGv, arrType, "_FPC_WRITE_FP64_GLOBAL_ARRAY_");
	/*
	for (auto f = mod->begin(), fend = mod->end(); f != fend; ++f)
	{
		if (f->getName().str().find("_Z29_FPC_WRITE_FP64_GLOBAL_ARRAY_iy") != std::string::npos)
		{
#ifdef FPC_DEBUG
      Logging::info("found function: _Z29_FPC_WRITE_FP64_GLOBAL_ARRAY_iy");
#endif

			// Find return instruction (last instruction)
			// We only have a single basic block
			Instruction *retInst = &(f->begin()->back());
			assert(isa<ReturnInst>(retInst) && "Not a return instruction");

			// Get instruction before the return
			BasicBlock::iterator tmpIt(retInst);
			tmpIt--;
			Instruction *prevInst = &(*(tmpIt));
			assert(prevInst && "Invalid instruction!");

			IRBuilder<> builder = createBuilderBefore(retInst);

			// Create signed extension of parameter
			auto arg = f->arg_begin();
			//arg++; // 2nd arg
			auto sext = builder.CreateSExt(arg, Type::getInt64Ty(mod->getContext()), "my");

			// Create GEP inst and addr-space cast inst
			std::vector<Value *> args;
			args.push_back(ConstantInt::get(Type::getInt64Ty(mod->getContext()), 0));
			args.push_back(sext);
			ArrayRef<Value *> indexList(args);
			arg = f->arg_begin();
			auto gep = builder.CreateInBoundsGEP(warArrayType, newWarningsGv, indexList, "my");
			auto addCast = new AddrSpaceCastInst(gep, Type::getInt64PtrTy(mod->getContext(), 0), "my", retInst);
			arg++; // 2rd arg
			Value *val = &(*arg);
			auto storeInst = builder.CreateAlignedStore(val, addCast, 8, false);
			//auto loadInst = builder.CreateAlignedLoad(addCast, 8, "my");

			// Now we remove old (unused) instructions
			auto iter = f->begin()->begin();
			Instruction *old = &(*iter);
			std::list<Instruction *> iList;
			while (old != prevInst)
			{
				iList.push_back(old);
				iter++;
				old = &(*iter);
			}
			iList.push_back(prevInst);

			for (std::list<Instruction *>::reverse_iterator rit=iList.rbegin(); rit!=iList.rend(); ++rit)
			{
				//outs() << "removing: " << inst2str(*rit) << "\n";
			  	(*rit)->eraseFromParent();
			}

			break;
		}
	}*/
	/* ------------------------------------------------------------------------ */
}

void FPInstrumentation::instrumentEndOfKernel(Function *f)
{
	// Find the return instructions
	for (auto bb=f->begin(), end=f->end(); bb != end; ++bb)
	{
		for (auto i=bb->begin(), iend=bb->end(); i != iend; ++i)
		{
			Instruction *inst = &(*i);
			if (isa<ReturnInst>(inst))
			{
				IRBuilder<> builder = createBuilderBefore(inst);
				std::vector<Value *> args;
				CallInst *callInst = builder.CreateCall(_print_errors_, args);
				assert(callInst && "Invalid call instruction!");
				setFakeDebugLocation(f, callInst);
			}
		}
	}
}
