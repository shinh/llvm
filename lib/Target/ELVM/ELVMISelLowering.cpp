//===-- ELVMISelLowering.cpp - ELVM DAG Lowering Implementation  ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that ELVM uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "ELVMISelLowering.h"
#include "ELVM.h"
#include "ELVMSubtarget.h"
#include "ELVMTargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "elvm-lower"

static void fail(const SDLoc &DL, SelectionDAG &DAG, const char *Msg) {
  MachineFunction &MF = DAG.getMachineFunction();
  DAG.getContext()->diagnose(
      DiagnosticInfoUnsupported(*MF.getFunction(), Msg, DL.getDebugLoc()));
}

static void fail(const SDLoc &DL, SelectionDAG &DAG, const char *Msg,
                 SDValue Val) {
  MachineFunction &MF = DAG.getMachineFunction();
  std::string Str;
  raw_string_ostream OS(Str);
  OS << Msg;
  Val->print(OS);
  OS.flush();
  DAG.getContext()->diagnose(
      DiagnosticInfoUnsupported(*MF.getFunction(), Str, DL.getDebugLoc()));
}

ELVMTargetLowering::ELVMTargetLowering(const TargetMachine &TM,
                                     const ELVMSubtarget &STI)
    : TargetLowering(TM) {

  // Set up the register classes.
  addRegisterClass(MVT::i32, &ELVM::GPRRegClass);

#if 0
  setTruncStoreAction(MVT::i16, MVT::i8, Legal);
  setTruncStoreAction(MVT::i32, MVT::i16, Legal);
  setTruncStoreAction(MVT::i32, MVT::i8, Legal);
#endif

  // Compute derived properties from the register classes
  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(ELVM::SP);

  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BRIND, MVT::Other, Expand);
  setOperationAction(ISD::BRCOND, MVT::Other, Expand);
  setOperationAction(ISD::SETCC, MVT::i32, Expand);
  setOperationAction(ISD::SELECT, MVT::i32, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);

  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);

  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32, Custom);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);

#if 0
  setOperationAction(ISD::SDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::SREM, MVT::i32, Expand);
  setOperationAction(ISD::UREM, MVT::i32, Expand);
#endif
  setOperationAction(ISD::SDIV, MVT::i32, LibCall);
  setOperationAction(ISD::UDIV, MVT::i32, LibCall);
  setLibcallName(RTLIB::SDIV_I32, "__elvm_builtin_div");
  setLibcallName(RTLIB::UDIV_I32, "__elvm_builtin_div");
  setOperationAction(ISD::SREM, MVT::i32, LibCall);
  setOperationAction(ISD::UREM, MVT::i32, LibCall);
  setLibcallName(RTLIB::SREM_I32, "__elvm_builtin_mod");
  setLibcallName(RTLIB::UREM_I32, "__elvm_builtin_mod");
  // TODO: Check its ABI and implement it.
  //setOperationAction(ISD::SDIVREM, MVT::i32, LibCall);
  //setOperationAction(ISD::UDIVREM, MVT::i32, LibCall);

  setOperationAction(ISD::MUL, MVT::i32, LibCall);
  setOperationAction(ISD::MULHU, MVT::i32, LibCall);
  setOperationAction(ISD::MULHS, MVT::i32, LibCall);
  setOperationAction(ISD::UMUL_LOHI, MVT::i32, LibCall);
  setOperationAction(ISD::SMUL_LOHI, MVT::i32, LibCall);
  setLibcallName(RTLIB::MUL_I32, "__elvm_builtin_mul");

  setOperationAction(ISD::ADDC, MVT::i32, Expand);
  setOperationAction(ISD::ADDE, MVT::i32, Expand);
  setOperationAction(ISD::SUBC, MVT::i32, Expand);
  setOperationAction(ISD::SUBE, MVT::i32, Expand);

  setOperationAction(ISD::ROTR, MVT::i32, Expand);
  setOperationAction(ISD::ROTL, MVT::i32, Expand);
  setOperationAction(ISD::SHL_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i32, Expand);

  setOperationAction(ISD::CTTZ, MVT::i32, Custom);
  setOperationAction(ISD::CTLZ, MVT::i32, Custom);
  setOperationAction(ISD::CTTZ_ZERO_UNDEF, MVT::i32, Custom);
  setOperationAction(ISD::CTLZ_ZERO_UNDEF, MVT::i32, Custom);
  setOperationAction(ISD::CTPOP, MVT::i32, Expand);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32, Expand);

#if 1
  // Extended load operations for i1 types must be promoted
  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1, Promote);

    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i8, Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i16, Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i32, Expand);
  }
#else
  for (MVT VT : MVT::integer_valuetypes()) {
    for (auto N : {ISD::EXTLOAD, ISD::SEXTLOAD, ISD::ZEXTLOAD}) {
      setLoadExtAction(N, VT, MVT::i1, Legal);
      setLoadExtAction(N, VT, MVT::i8, Legal);
      setLoadExtAction(N, VT, MVT::i16, Legal);
    }
  }
#endif

  setBooleanContents(ZeroOrOneBooleanContent);

  // Function alignments (log2)
  //setMinFunctionAlignment(1);
  //setPrefFunctionAlignment(1);

  // inline memcpy() for kernel to see explicit copy
  MaxStoresPerMemset = MaxStoresPerMemsetOptSize = 128;
  MaxStoresPerMemcpy = MaxStoresPerMemcpyOptSize = 128;
  MaxStoresPerMemmove = MaxStoresPerMemmoveOptSize = 128;
}

SDValue ELVMTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::BR_CC:
    return LowerBR_CC(Op, DAG);
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::SELECT_CC:
    return LowerSELECT_CC(Op, DAG);
  default:
    llvm_unreachable("unimplemented operand");
  }
}

// Calling Convention Implementation
#include "ELVMGenCallingConv.inc"

SDValue ELVMTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  switch (CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    break;
  }

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_ELVM);

  for (auto &VA : ArgLocs) {
    // Sanity check.
    assert(VA.isMemLoc());

    EVT LocVT = VA.getLocVT();

    // One for return address, the other for frame address.
    int32_t Offset = VA.getLocMemOffset() + 2;
    // Create the frame index object for this incoming parameter.
    int FI = MFI.CreateFixedObject(LocVT.getSizeInBits() / 32, Offset, true);
    fprintf(stderr, "size=%u fi=%d\n", LocVT.getSizeInBits(), FI);

    // Create the SelectionDAG nodes corresponding to a load
    // from this parameter.
    SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
    InVals.push_back(DAG.getLoad(LocVT, DL, Chain, FIN,
                                 MachinePointerInfo::getFixedStack(MF, FI)));
  }

  if (IsVarArg || MF.getFunction()->hasStructRetAttr()) {
    fail(DL, DAG, "functions with VarArgs or StructRet are not supported");
  }

  return Chain;
}

SDValue ELVMTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                      SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  auto &Outs = CLI.Outs;
  auto &OutVals = CLI.OutVals;
  auto &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &IsTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;
  MachineFunction &MF = DAG.getMachineFunction();

  // ELVM target does not support tail call optimization.
  IsTailCall = false;

  switch (CallConv) {
  default:
    report_fatal_error("Unsupported calling convention");
  case CallingConv::Fast:
  case CallingConv::C:
    break;
  }

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_ELVM);

  unsigned NumBytes = CCInfo.getNextStackOffset();

  for (auto &Arg : Outs) {
    ISD::ArgFlagsTy Flags = Arg.Flags;
    if (!Flags.isByVal())
      continue;

    fail(CLI.DL, DAG, "pass by value not supported ", Callee);
  }

  auto PtrVT = getPointerTy(MF.getDataLayout());
  Chain = DAG.getCALLSEQ_START(
      Chain, DAG.getConstant(NumBytes, CLI.DL, PtrVT, true), CLI.DL);

#if 0
  // Walk arg assignments
  for (unsigned i = 0; i != ArgLocs.size(); ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
    default:
      llvm_unreachable("Unknown loc info");
    case CCValAssign::Full:
      break;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, CLI.DL, VA.getLocVT(), Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, CLI.DL, VA.getLocVT(), Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, CLI.DL, VA.getLocVT(), Arg);
      break;
    }

    // Push arguments into RegsToPass vector
    if (VA.isRegLoc())
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    else
      llvm_unreachable("call arg pass bug");
  }
#endif

#if 0
  fprintf(stderr, "hey\n");
  const DataLayout &TD = DAG.getDataLayout();
  for (unsigned i = 0; i != ArgLocs.size(); i++) {
    fprintf(stderr, "hey %u/%zu\n", i, ArgLocs.size());
    MVT LocVT = ArgLocs[i].getLocVT();
    unsigned Offset = CCInfo.AllocateStack(
        TD.getTypeAllocSize(EVT(LocVT).getTypeForEVT(CCInfo.getContext())),
        TD.getABITypeAlignment(
            EVT(LocVT).getTypeForEVT(CCInfo.getContext())));
    CCInfo.addLoc(CCValAssign::getMem(i, LocVT, Offset, LocVT,
                                      CCValAssign::Full));
  }
#endif

  fprintf(stderr, "hey\n");
  for (unsigned i = ArgLocs.size(); i != 0;) {
    --i;
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    assert(VA.isMemLoc());

    fprintf(stderr, "arg%u loc=%d\n", i, VA.getLocMemOffset());
    SDValue StackPtr = DAG.getCopyFromReg(Chain, DL, ELVM::SP, getPointerTy(DAG.getDataLayout()));
    // TODO: Check if the comment is true.
    // SP points to one stack slot further so add one to adjust it.
    SDValue PtrOff = DAG.getNode(
        ISD::ADD, DL, getPointerTy(DAG.getDataLayout()),
        //DAG.getRegister(ELVM::SP, getPointerTy(DAG.getDataLayout())),
        StackPtr,
        DAG.getIntPtrConstant(VA.getLocMemOffset(), DL));
    //Chain = DAG.getCopyToReg(Chain, DL, ELVM::SP, PtrOff);
    Chain =
        DAG.getStore(Chain, DL, Arg, PtrOff,
                     MachinePointerInfo::getStack(MF, VA.getLocMemOffset()),
                     0);
  }

  SDValue InFlag;

#if 0
  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, CLI.DL, Reg.first, Reg.second, InFlag);
    InFlag = Chain.getValue(1);
  }
#endif

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), CLI.DL, PtrVT,
                                        G->getOffset(), 0);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), PtrVT, 0);

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

#if 0
  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (auto &Reg : RegsToPass)
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));
#endif

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  Chain = DAG.getNode(ELVMISD::CALL, CLI.DL, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(
      Chain, DAG.getConstant(NumBytes, CLI.DL, PtrVT, true),
      DAG.getConstant(0, CLI.DL, PtrVT, true), InFlag, CLI.DL);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, IsVarArg, Ins, CLI.DL, DAG,
                         InVals);
}

SDValue
ELVMTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               const SDLoc &DL, SelectionDAG &DAG) const {
  unsigned Opc = ELVMISD::RET_FLAG;

  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;
  MachineFunction &MF = DAG.getMachineFunction();

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());

  if (MF.getFunction()->getReturnType()->isAggregateType()) {
    fail(DL, DAG, "only integer returns supported");
    return DAG.getNode(Opc, DL, MVT::Other, Chain);
  }

  // Analize return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_ELVM);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(Opc, DL, MVT::Other, RetOps);
}

SDValue ELVMTargetLowering::LowerCallResult(
    SDValue Chain, SDValue InFlag, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  MachineFunction &MF = DAG.getMachineFunction();
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());

  if (Ins.size() >= 2) {
    fail(DL, DAG, "only small returns supported");
    for (unsigned i = 0, e = Ins.size(); i != e; ++i)
      InVals.push_back(DAG.getConstant(0, DL, Ins[i].VT));
    return DAG.getCopyFromReg(Chain, DL, 1, Ins[0].VT, InFlag).getValue(1);
  }

  CCInfo.AnalyzeCallResult(Ins, RetCC_ELVM);

  // Copy all of the result registers out of their specified physreg.
  for (auto &Val : RVLocs) {
    Chain = DAG.getCopyFromReg(Chain, DL, Val.getLocReg(),
                               Val.getValVT(), InFlag).getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

static void NegateCC(SDValue &LHS, SDValue &RHS, ISD::CondCode &CC) {
  switch (CC) {
  default:
    break;
  case ISD::SETULT:
  case ISD::SETULE:
  case ISD::SETLT:
  case ISD::SETLE:
    CC = ISD::getSetCCSwappedOperands(CC);
    std::swap(LHS, RHS);
    break;
  }
}

SDValue ELVMTargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue Dest = Op.getOperand(4);
  SDLoc DL(Op);

  NegateCC(LHS, RHS, CC);

  return DAG.getNode(ELVMISD::BR_CC, DL, Op.getValueType(), Chain, LHS, RHS,
                     DAG.getConstant(CC, DL, MVT::i32), Dest);
}

SDValue ELVMTargetLowering::LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const {
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueV = Op.getOperand(2);
  SDValue FalseV = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
  SDLoc DL(Op);

  NegateCC(LHS, RHS, CC);

  SDValue TargetCC = DAG.getConstant(CC, DL, MVT::i32);

  SDVTList VTs = DAG.getVTList(Op.getValueType(), MVT::Glue);
  SDValue Ops[] = {LHS, RHS, TargetCC, TrueV, FalseV};

  return DAG.getNode(ELVMISD::SELECT_CC, DL, VTs, Ops);
}

const char *ELVMTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((ELVMISD::NodeType)Opcode) {
  case ELVMISD::FIRST_NUMBER:
    break;
  case ELVMISD::RET_FLAG:
    return "ELVMISD::RET_FLAG";
  case ELVMISD::CALL:
    return "ELVMISD::CALL";
  case ELVMISD::SELECT_CC:
    return "ELVMISD::SELECT_CC";
  case ELVMISD::BR_CC:
    return "ELVMISD::BR_CC";
  case ELVMISD::Wrapper:
    return "ELVMISD::Wrapper";
  }
  return nullptr;
}

SDValue ELVMTargetLowering::LowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  SDLoc DL(Op);
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  SDValue GA = DAG.getTargetGlobalAddress(GV, DL, MVT::i32);

  return DAG.getNode(ELVMISD::Wrapper, DL, MVT::i32, GA);
}

MachineBasicBlock *
ELVMTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                               MachineBasicBlock *BB) const {
  const TargetInstrInfo &TII = *BB->getParent()->getSubtarget().getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  assert(MI.getOpcode() == ELVM::Select && "Unexpected instr type to insert");

  // To "insert" a SELECT instruction, we actually have to insert the diamond
  // control-flow pattern.  The incoming instruction knows the destination vreg
  // to set, the condition code register to branch on, the true/false values to
  // select between, and a branch opcode to use.
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator I = ++BB->getIterator();

  // ThisMBB:
  // ...
  //  TrueVal = ...
  //  jmp_XX r1, r2 goto Copy1MBB
  //  fallthrough --> Copy0MBB
  MachineBasicBlock *ThisMBB = BB;
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *Copy0MBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *Copy1MBB = F->CreateMachineBasicBlock(LLVM_BB);

  F->insert(I, Copy0MBB);
  F->insert(I, Copy1MBB);
  // Update machine-CFG edges by transferring all successors of the current
  // block to the new block which will contain the Phi node for the select.
  Copy1MBB->splice(Copy1MBB->begin(), BB,
                   std::next(MachineBasicBlock::iterator(MI)), BB->end());
  Copy1MBB->transferSuccessorsAndUpdatePHIs(BB);
  // Next, add the true and fallthrough blocks as its successors.
  BB->addSuccessor(Copy0MBB);
  BB->addSuccessor(Copy1MBB);

  // Insert Branch if Flag
  unsigned LHS = MI.getOperand(1).getReg();
  unsigned RHS = MI.getOperand(2).getReg();
  int CC = MI.getOperand(3).getImm();
  switch (CC) {
  case ISD::SETGT:
    BuildMI(BB, DL, TII.get(ELVM::JSGT_rr))
        .addReg(LHS)
        .addReg(RHS)
        .addMBB(Copy1MBB);
    break;
  case ISD::SETUGT:
    BuildMI(BB, DL, TII.get(ELVM::JUGT_rr))
        .addReg(LHS)
        .addReg(RHS)
        .addMBB(Copy1MBB);
    break;
  case ISD::SETGE:
    BuildMI(BB, DL, TII.get(ELVM::JSGE_rr))
        .addReg(LHS)
        .addReg(RHS)
        .addMBB(Copy1MBB);
    break;
  case ISD::SETUGE:
    BuildMI(BB, DL, TII.get(ELVM::JUGE_rr))
        .addReg(LHS)
        .addReg(RHS)
        .addMBB(Copy1MBB);
    break;
  case ISD::SETEQ:
    BuildMI(BB, DL, TII.get(ELVM::JEQ_rr))
        .addReg(LHS)
        .addReg(RHS)
        .addMBB(Copy1MBB);
    break;
  case ISD::SETNE:
    BuildMI(BB, DL, TII.get(ELVM::JNE_rr))
        .addReg(LHS)
        .addReg(RHS)
        .addMBB(Copy1MBB);
    break;
  default:
    report_fatal_error("unimplemented select CondCode " + Twine(CC));
  }

  // Copy0MBB:
  //  %FalseValue = ...
  //  # fallthrough to Copy1MBB
  BB = Copy0MBB;

  // Update machine-CFG edges
  BB->addSuccessor(Copy1MBB);

  // Copy1MBB:
  //  %Result = phi [ %FalseValue, Copy0MBB ], [ %TrueValue, ThisMBB ]
  // ...
  BB = Copy1MBB;
  BuildMI(*BB, BB->begin(), DL, TII.get(ELVM::PHI), MI.getOperand(0).getReg())
      .addReg(MI.getOperand(5).getReg())
      .addMBB(Copy0MBB)
      .addReg(MI.getOperand(4).getReg())
      .addMBB(ThisMBB);

  MI.eraseFromParent(); // The pseudo instruction is gone now.
  return BB;
}
