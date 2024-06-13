/*
Usage: 
opt-10 -load /home/lwfdev/state_instruement/SVInstrument_Pass.so --SV-dump <IR> -o <IR.bc>
*/


#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/BinaryFormat/MachO.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Comdat.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/ScopedPrinter.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Utils/ASanStackFrameLayout.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <climits>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <fstream>
#include <regex>
#include "json.hpp"

using namespace llvm;
using json = nlohmann::json;

static json* SVInfos = nullptr;
static FILE* Instrument_log = nullptr;
std::string SV_output_path;
std::vector<std::string> SVs;
std::vector<uint32_t> SVs_random;

/*

*/
static size_t TypeSizeToSizeIndex(uint32_t TypeSize) {
  if (TypeSize == 1) TypeSize = 8;
  size_t Res = countTrailingZeros(TypeSize / 8);
  return Res;
}

/*********************************************************

***********************************************************/
static void init_SVs(){

    for(auto &js_SVs : (*SVInfos)["StateVarables"]){
        std::string SV_name = js_SVs["name"].get<std::string>();        
        uint32_t SV_rand = random();
        SVs.push_back(SV_name);
        SVs_random.push_back(SV_rand);
    }
}



static int16_t getSVID(llvm::Value *V){
    if(V->hasName()){
        std::string Vname = V->getName().str();
        int16_t cc= 1;
        for(auto it = SVs.begin(), eit = SVs.end();it != eit;it++){
            //TODO
            std::string substr = *it;
            std::string pattern = substr+"\\d*";
            std::regex reg(pattern);
            bool matched = std::regex_match(Vname,reg);
            if(matched){
                return cc;
            }
            cc++;
        }
        return 0;
    }
    else 
        return 0;
}

/*
插桩类
*/
struct SVInstrument{
    Type *VoidTy, *Int8Ty, *Int16Ty, *Int32Ty, *Int64Ty, *FloatTy, *DoubleTy,
    *StructTy, *Int8PTy, *Int16PTy, *Int32PTy, *Int64PTy, *FloatPTy,
    *DoublePTy, *StructPTy, *FuncTy;
    LLVMContext *C;
    int PtrSize;
    Module& M;
    Function &F;
    LoopInfo &LI;

    Type *IntTypeSized[4];
    Function* dbgDeclareFn;
    FunctionCallee SVHashFns[4];
    FunctionCallee SVlockFn;
    FunctionCallee SVunlockFn;
    FunctionCallee SVdumpFn;

    std::string funcname;


    SVInstrument(Module& _M, Function &_F, LoopInfo &_LI) : M(_M), F(_F), LI(_LI){ 
      initialize();
   }

    //黑名单里标记的函数不做处理
    static bool isBlacklisted(const Function *F){
        static const char *Blacklist[] = {
        "asan.", "llvm.", "sancov.", "__ubsan_handle_", "ign.", "__afl_",
        "_fini", "__libc_csu", "__asan",  "__msan", "msan."
        };
        for (auto const &BlacklistFunc : Blacklist){
            if (F->getName().startswith(BlacklistFunc)) return true;
        }
        if (F->getName() == "_start") return true;

        return false;
   }
    void initialize();
    bool instrumentFunction();
};


/*

*/
void SVInstrument::initialize(){

    init_SVs();

    funcname = M.getModuleIdentifier() + ":" + F.getName().str();

    C = &(M.getContext());    
    PtrSize = M.getDataLayout().getPointerSizeInBits();

    VoidTy = Type::getVoidTy(*C);

    Int8Ty = IntegerType::get(*C, 8);
    Int16Ty = IntegerType::get(*C, 16);
    Int32Ty = IntegerType::get(*C, 32);
    Int64Ty = IntegerType::get(*C, 64);
    FloatTy = Type::getFloatTy(*C);
    DoubleTy = Type::getDoubleTy(*C);
    StructTy = StructType::create(*C);
    Int8PTy  = PointerType::get(Int8Ty, 0);
    Int16PTy = PointerType::get(Int16Ty, 0);
    Int32PTy = PointerType::get(Int32Ty, 0);
    Int64PTy = PointerType::get(Int64Ty, 0);

    FloatPTy = PointerType::get(FloatTy, 0);
    DoublePTy = PointerType::get(DoubleTy, 0);
    StructPTy = PointerType::get(StructTy, 0);
    FuncTy = FunctionType::get(VoidTy, true);

    dbgDeclareFn = M.getFunction("llvm.dbg.declare");

    IntTypeSized[0] = Int8Ty;
    IntTypeSized[1] = Int16Ty;
    IntTypeSized[2] = Int32Ty;
    IntTypeSized[3] = Int64Ty;

    SVHashFns[0] = M.getOrInsertFunction("__SV_hash_u8", VoidTy, Int32Ty, Int8Ty);
    SVHashFns[1] = M.getOrInsertFunction("__SV_hash_u16", VoidTy, Int32Ty, Int16Ty);
    SVHashFns[2] = M.getOrInsertFunction("__SV_hash_u32", VoidTy, Int32Ty, Int32Ty);
    SVHashFns[3] = M.getOrInsertFunction("__SV_hash_u64", VoidTy, Int32Ty, Int64Ty);
    SVlockFn = M.getOrInsertFunction("__SV_lock",VoidTy);
    SVunlockFn = M.getOrInsertFunction("__SV_unlock",VoidTy);    
    SVdumpFn = M.getOrInsertFunction("__state_dump",VoidTy);
}

/*

*/
struct BBInfo {
    bool haveSV = false;
    std::string Name;
    std::vector< StoreInst* > STs;     
    std::vector< int16_t > SVID;       
};




/*

*/
bool SVInstrument::instrumentFunction(){
    bool isFunctionModified = false;
    if (isBlacklisted(&F)) return isFunctionModified;
    std::vector<BasicBlock*> BBs;
    std::map<BasicBlock*, BBInfo> Infos;


    for (auto &BB : F) {        
        std::string BBp;
        raw_string_ostream OS(BBp);
        BB.printAsOperand(OS, false);
        auto BBname = funcname + "#" + OS.str();
        Infos[&BB].Name = BBname;
        for(auto &Inst : BB){

            if (Inst.getMetadata(M.getMDKindID("nosanitize"))) continue;

            if (isa<PHINode>(&Inst)) continue;

            if(auto STORE =dyn_cast<StoreInst>(&Inst)){
                

                std::string temp = STORE->getPointerOperand()->getName().str();
                

                llvm::Value* SPtr = STORE->getPointerOperand();

                int16_t svid = getSVID(SPtr);
                if(svid !=  0){
                    errs() << "\033[32m[*]SVinst: instrument Var found: \""<<temp.c_str()<<"\" STORE in function: "<<F.getName().str()<<"\033[0m\n";
                    fprintf(Instrument_log,"[*]SVinst: instrument Var found: \"%s\",which STORE in function: \"%s\"\n", temp.c_str(), F.getName().str().c_str());
                    Infos[&BB].haveSV = true;
                    Infos[&BB].STs.push_back(STORE);
                    Infos[&BB].SVID.push_back(svid);
                }
            }            
        }
    }


    for (auto &BB : F){
        if(Infos[&BB].haveSV == false) continue;
       
        int iter2 = 0;
        for(auto it = Infos[&BB].STs.begin(), eit = Infos[&BB].STs.end(); it != eit ; it++){
            StoreInst* STORE = *it;
            int16_t SV_ID = Infos[&BB].SVID[iter2];
            iter2 ++;
            int32_t SV_RND = SVs_random[SV_ID-1];
            
            errs() << "[*]SVinst: instrumented variable random:"<<SV_RND<<"\n";
            fprintf(Instrument_log,"[*]SVinst: instrumented variable random:%u\n",SV_RND);
            IRBuilder<> IRB(STORE);

            llvm::Value* SV_Val = STORE->getValueOperand(); 
            Type* SV_val_type = SV_Val->getType();

            CallInst* CI_lock = IRB.CreateCall(SVlockFn);
            CI_lock->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(*C, None));


            switch (SV_val_type->getTypeID())
            {
            case Type::IntegerTyID: {
                TypeSize BitsNum = SV_val_type->getPrimitiveSizeInBits();
                if (BitsNum > 64) break;
                if (BitsNum == 1) 
                    SV_Val = IRB.CreateIntCast(SV_Val, Int8Ty, true);


                Value *SV_ID_VALUE = ConstantInt::get(Type::getInt32Ty(*C), SV_RND);


                size_t SizeIndex = TypeSizeToSizeIndex(BitsNum);
                Value *VALUEINT = IRB.CreateBitCast(SV_Val, IntTypeSized[SizeIndex]);
                
                CallInst* CI_hash = IRB.CreateCall(SVHashFns[SizeIndex], ArrayRef<Value*>{SV_ID_VALUE, VALUEINT});
                CI_hash->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(*C, None));
                
                
                
                isFunctionModified = true;
                break;
            }
            
            
            default:
                break;
            }

            CallInst* CI_dump = IRB.CreateCall(SVdumpFn);
            CI_dump->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(*C, None));
            CallInst* CI_unlock = IRB.CreateCall(SVunlockFn);
            CI_unlock->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(*C, None));
             
        }
    }

    return isFunctionModified;
}


/*

*/
class SVInstrumentPass : public FunctionPass{
public:
    static char ID;
    explicit SVInstrumentPass() : FunctionPass(ID){


        std::string SVInfos_path = "SVInfos.json";
        
        if (getenv("SVINFOS_PATH")){
             
            SVInfos_path = std::string(getenv("SVINFOS_PATH")) + std::string("/SVInfos.json");
        }else{
            errs() << "\033[33m[!]SVinst: ENV SVINFOS_PATH not set!\033[0m" << "\n";
            exit(-1);
        }


        if (getenv("SV_OUT_PATH")){
            SV_output_path = std::string(getenv("SV_OUT_PATH")) + std::string("/SVinstrument.log");
        }
        else
        {
            
            SV_output_path = std::string(getenv("SVINFOS_PATH")) + std::string("/SVinstrument.log");
            errs() << "\033[33m[!]Warning: output log will write to SVINFOS_PATH: \033[0m"<<SV_output_path<<"\n";
        }


        SVInfos = new json();
        errs() <<"[*]SVinst: json file for state variable's path is: " <<SVInfos_path << "\n";
        std::ifstream f(SVInfos_path);
        if (!f.good()) {
            errs() << "\033[33m[!]SVinst\033[0m: Failed to open (r) the file: " << SVInfos_path << "'\n";
            exit(-1);
        }
        f >> *SVInfos;
        f.close();
        
        Instrument_log = fopen(SV_output_path.c_str(),"a+");
        if(Instrument_log == nullptr){
            errs() << "\033[33m[!]SVinst\033[0m: Failed to open log: " << Instrument_log << "\n";
            exit(-1);
        }
    }

    ~SVInstrumentPass(){
        if(Instrument_log)fclose(Instrument_log);
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
        AU.setPreservesCFG();
        AU.addRequired<LoopInfoWrapperPass>();
  }

    StringRef getPassName() const override {
        return "State_Variable_Instrument_Pass";
    }

    bool runOnFunction(Function &F) override {
        Module &M = *F.getParent();
        LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
        SVInstrument DI(M, F, LI);
        bool r = DI.instrumentFunction();
        verifyFunction(F);
        return r;
  }    

};

char SVInstrumentPass::ID = 0;

static void registerSVInstPass(const PassManagerBuilder &,
                               legacy::PassManagerBase &PM) {

  PM.add(new SVInstrumentPass());

}

static RegisterStandardPasses RegisterInvsCovPass(
    PassManagerBuilder::EP_OptimizerLast, registerSVInstPass);

static RegisterStandardPasses RegisterInvsCovPass0(
    PassManagerBuilder::EP_EnabledOnOptLevel0, registerSVInstPass);

static RegisterPass<SVInstrumentPass>
    X("SV-dump", "SVInstPass",
      false,
      false
    );

