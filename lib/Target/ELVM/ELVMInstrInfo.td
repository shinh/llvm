//===-- ELVMInstrInfo.td - Target Description for ELVM Target ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the ELVM instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "ELVMInstrFormats.td"

// Instruction Operands and Patterns

// These are target-independent nodes, but have target-specific formats.
def SDT_ELVMCallSeqStart : SDCallSeqStart<[SDTCisVT<0, iPTR>]>;
def SDT_ELVMCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, iPTR>, SDTCisVT<1, iPTR>]>;
def SDT_ELVMCall         : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_ELVMSetFlag      : SDTypeProfile<0, 3, [SDTCisSameAs<0, 1>]>;
def SDT_ELVMSelectCC     : SDTypeProfile<1, 5, [SDTCisSameAs<1, 2>,
                                               SDTCisSameAs<0, 4>,
                                               SDTCisSameAs<4, 5>]>;
def SDT_ELVMBrCC         : SDTypeProfile<0, 4, [SDTCisSameAs<0, 1>,
                                               SDTCisVT<3, OtherVT>]>;
def SDT_ELVMWrapper      : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>,
                                               SDTCisPtrTy<0>]>;

def ELVMcall         : SDNode<"ELVMISD::CALL", SDT_ELVMCall,
                             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                              SDNPVariadic]>;
def ELVMretflag      : SDNode<"ELVMISD::RET_FLAG", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def ELVMcallseq_start: SDNode<"ISD::CALLSEQ_START", SDT_ELVMCallSeqStart,
                             [SDNPHasChain, SDNPOutGlue]>;
def ELVMcallseq_end  : SDNode<"ISD::CALLSEQ_END",   SDT_ELVMCallSeqEnd,
                             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
def ELVMbrcc         : SDNode<"ELVMISD::BR_CC", SDT_ELVMBrCC,
                             [SDNPHasChain, SDNPOutGlue, SDNPInGlue]>;

def ELVMselectcc     : SDNode<"ELVMISD::SELECT_CC", SDT_ELVMSelectCC, [SDNPInGlue]>;
def ELVMWrapper      : SDNode<"ELVMISD::Wrapper", SDT_ELVMWrapper>;

def brtarget : Operand<OtherVT>;
def calltarget : Operand<i32>;

def u32imm   : Operand<i32> {
  let PrintMethod = "printImm32Operand";
}

def i32immSExt32 : PatLeaf<(imm),
                [{return isInt<32>(N->getSExtValue()); }]>;

def addr : ComplexPattern<iPTR, 1, "SelectAddr", [], []>;

// Addressing modes.
def ADDRri : ComplexPattern<i32, 2, "SelectAddr", [], []>;
def FIri : ComplexPattern<i32, 2, "SelectFIAddr", [add, or], []>;

// Address operands
def MEMri : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let EncoderMethod = "getMemoryOpValue";
  let DecoderMethod = "decodeMemoryOpValue";
  let MIOperandInfo = (ops GPR, i16imm);
}

// Conditional code predicates - used for pattern matching for jump instructions
def ELVM_CC_EQ  : PatLeaf<(imm),
                         [{return (N->getZExtValue() == ISD::SETEQ);}]>;
def ELVM_CC_NE  : PatLeaf<(imm),
                         [{return (N->getZExtValue() == ISD::SETNE);}]>;
def ELVM_CC_GE  : PatLeaf<(imm),
                         [{return (N->getZExtValue() == ISD::SETGE);}]>;
def ELVM_CC_GT  : PatLeaf<(imm),
                         [{return (N->getZExtValue() == ISD::SETGT);}]>;
def ELVM_CC_GTU : PatLeaf<(imm),
                         [{return (N->getZExtValue() == ISD::SETUGT);}]>;
def ELVM_CC_GEU : PatLeaf<(imm),
                         [{return (N->getZExtValue() == ISD::SETUGE);}]>;

// jump instructions
class JMP_RR<bits<4> Opc, string OpcodeStr, PatLeaf Cond>
    : InstELVM<(outs), (ins GPR:$dst, GPR:$src, brtarget:$BrDst),
              "j"#OpcodeStr#" $BrDst, $dst, $src",
              [(ELVMbrcc i32:$dst, i32:$src, Cond, bb:$BrDst)]> {
  bits<4> op;
  bits<1> ELVMSrc;
  bits<4> dst;
  bits<4> src;
  bits<16> BrDst;

  let Inst{63-60} = op;
  let Inst{59} = ELVMSrc;
  let Inst{55-52} = src;
  let Inst{51-48} = dst;
  let Inst{47-32} = BrDst;

  let op = Opc;
  let ELVMSrc = 1;
  let ELVMClass = 5; // ELVM_JMP
}

class JMP_RI<bits<4> Opc, string OpcodeStr, PatLeaf Cond>
    : InstELVM<(outs), (ins GPR:$dst, i32imm:$imm, brtarget:$BrDst),
              "j"#OpcodeStr#" $BrDst, $dst, $imm",
              [(ELVMbrcc i32:$dst, i32immSExt32:$imm, Cond, bb:$BrDst)]> {
  bits<4> op;
  bits<1> ELVMSrc;
  bits<4> dst;
  bits<16> BrDst;
  bits<32> imm;

  let Inst{63-60} = op;
  let Inst{59} = ELVMSrc;
  let Inst{51-48} = dst;
  let Inst{47-32} = BrDst;
  let Inst{31-0} = imm;

  let op = Opc;
  let ELVMSrc = 0;
  let ELVMClass = 5; // ELVM_JMP
}

multiclass J<bits<4> Opc, string OpcodeStr, PatLeaf Cond> {
  def _rr : JMP_RR<Opc, OpcodeStr, Cond>;
  def _ri : JMP_RI<Opc, OpcodeStr, Cond>;
}

let isBranch = 1, isTerminator = 1, hasDelaySlot=0 in {
// cmp+goto instructions
defm JEQ  : J<0x1, "eq",  ELVM_CC_EQ>;
defm JUGT : J<0x2, "gt", ELVM_CC_GTU>;
defm JUGE : J<0x3, "ge", ELVM_CC_GEU>;
defm JNE  : J<0x5, "ne",  ELVM_CC_NE>;
defm JSGT : J<0x6, "gt", ELVM_CC_GT>;
defm JSGE : J<0x7, "ge", ELVM_CC_GE>;
}

// ALU instructions
class ALU_RI<bits<4> Opc, string OpcodeStr, SDNode OpNode>
    : InstELVM<(outs GPR:$dst), (ins GPR:$src2, i32imm:$imm),
              OpcodeStr#" $dst, $imm",
              [(set GPR:$dst, (OpNode GPR:$src2, i32immSExt32:$imm))]> {
  bits<4> op;
  bits<1> ELVMSrc;
  bits<4> dst;
  bits<32> imm;

  let Inst{63-60} = op;
  let Inst{59} = ELVMSrc;
  let Inst{51-48} = dst;
  let Inst{31-0} = imm;

  let op = Opc;
  let ELVMSrc = 0;
  let ELVMClass = 7; // ELVM_ALU32
}

class ALU_RR<bits<4> Opc, string OpcodeStr, SDNode OpNode>
    : InstELVM<(outs GPR:$dst), (ins GPR:$src2, GPR:$src),
              OpcodeStr#" $dst, $src",
              [(set GPR:$dst, (OpNode i32:$src2, i32:$src))]> {
  bits<4> op;
  bits<1> ELVMSrc;
  bits<4> dst;
  bits<4> src;

  let Inst{63-60} = op;
  let Inst{59} = ELVMSrc;
  let Inst{55-52} = src;
  let Inst{51-48} = dst;

  let op = Opc;
  let ELVMSrc = 1;
  let ELVMClass = 7; // ELVM_ALU32
}

multiclass ALU<bits<4> Opc, string OpcodeStr, SDNode OpNode> {
  def _rr : ALU_RR<Opc, OpcodeStr, OpNode>;
  def _ri : ALU_RI<Opc, OpcodeStr, OpNode>;
}

let Constraints = "$dst = $src2" in {
let isAsCheapAsAMove = 1 in {
  defm ADD : ALU<0x0, "add", add>;
  defm SUB : ALU<0x1, "sub", sub>;
}
  defm OR  : ALU<0x4, "or", or>;
  defm AND : ALU<0x5, "and", and>;
  defm SLL : ALU<0x6, "sll", shl>;
  defm SRL : ALU<0x7, "srl", srl>;
  defm XOR : ALU<0xa, "xor", xor>;
  defm SRA : ALU<0xc, "srl", sra>;
  //defm MUL : ALU<0x2, "mul", mul>;
  //defm DIV : ALU<0x3, "div", udiv>;
}

class MOV_RR<string OpcodeStr>
    : InstELVM<(outs GPR:$dst), (ins GPR:$src),
              "mov $dst, $src",
              []> {
  bits<4> op;
  bits<1> ELVMSrc;
  bits<4> dst;
  bits<4> src;

  let Inst{63-60} = op;
  let Inst{59} = ELVMSrc;
  let Inst{55-52} = src;
  let Inst{51-48} = dst;

  let op = 0xb;     // ELVM_MOV
  let ELVMSrc = 1;   // ELVM_X
  let ELVMClass = 7; // ELVM_ALU32
}

class MOV_RI<string OpcodeStr>
    : InstELVM<(outs GPR:$dst), (ins i32imm:$imm),
              "mov $dst, $imm",
              [(set GPR:$dst, (i32 i32immSExt32:$imm))]> {
  bits<4> op;
  bits<1> ELVMSrc;
  bits<4> dst;
  bits<32> imm;

  let Inst{63-60} = op;
  let Inst{59} = ELVMSrc;
  let Inst{51-48} = dst;
  let Inst{31-0} = imm;

  let op = 0xb;     // ELVM_MOV
  let ELVMSrc = 0;   // ELVM_K
  let ELVMClass = 7; // ELVM_ALU32
}

class LD_IMM32<bits<4> Pseudo>
    : InstELVM<(outs GPR:$dst), (ins u32imm:$imm),
              "mov $dst, $imm",
              [(set GPR:$dst, (i32 imm:$imm))]> {

  bits<3> mode;
  bits<2> size;
  bits<4> dst;
  bits<32> imm;

  let Inst{63-61} = mode;
  let Inst{60-59} = size;
  let Inst{51-48} = dst;
  let Inst{55-52} = Pseudo;
  let Inst{47-32} = 0;
  let Inst{31-0} = imm{31-0};

  let mode = 0;     // ELVM_IMM
  let size = 3;     // ELVM_DW
  let ELVMClass = 0; // ELVM_LD
}

let isReMaterializable = 1, isAsCheapAsAMove = 1 in {
def LD_imm32 : LD_IMM32<0>;
def MOV_rr : MOV_RR<"=">;
def MOV_ri : MOV_RI<"=">;
}

def FI_ri
    : InstELVM<(outs GPR:$dst), (ins MEMri:$addr),
               "lea\t$dst, $addr",
               [(set i32:$dst, FIri:$addr)]> {
  // This is a tentative instruction, and will be replaced
  // with MOV_rr and ADD_ri in PEI phase
  let Inst{63-61} = 0;
  let Inst{60-59} = 3;
  let Inst{51-48} = 0;
  let Inst{55-52} = 2;
  let Inst{47-32} = 0;
  let Inst{31-0} = 0;
  let ELVMClass = 0;
}

/*
def LD_pseudo
    : InstELVM<(outs GPR:$dst), (ins i32imm:$pseudo, u32imm:$imm),
              "ld_pseudo\t$dst, $pseudo, $imm",
              [(set GPR:$dst, (int_bpf_pseudo imm:$pseudo, imm:$imm))]> {

  bits<3> mode;
  bits<2> size;
  bits<4> dst;
  bits<32> imm;
  bits<4> pseudo;

  let Inst{63-61} = mode;
  let Inst{60-59} = size;
  let Inst{51-48} = dst;
  let Inst{55-52} = pseudo;
  let Inst{47-32} = 0;
  let Inst{31-0} = imm{31-0};

  let mode = 0;     // ELVM_IMM
  let size = 3;     // ELVM_DW
  let ELVMClass = 0; // ELVM_LD
}
*/

// STORE instructions
class STORE<bits<2> SizeOp, list<dag> Pattern>
    : InstELVM<(outs), (ins GPR:$src, GPR:$dst),
               "store $src, $dst",
               Pattern> {
  bits<3> mode;
  bits<2> size;
  bits<4> src;
  bits<4> dst;

  let Inst{63-61} = mode;
  let Inst{60-59} = size;
  let Inst{51-48} = dst;
  let Inst{55-52} = src;
  //let Inst{47-32} = addr{15-0}; // offset

  let mode = 3;     // ELVM_MEM
  let size = SizeOp;
  let ELVMClass = 3; // ELVM_STX
}

class STOREi32<bits<2> Opc, PatFrag OpNode>
    : STORE<Opc, [(OpNode i32:$src, addr:$dst)]>;

//def STW : STOREi32<0x0, truncstorei32>;
def STH : STOREi32<0x1, truncstorei16>;
def STB : STOREi32<0x2, truncstorei8>;
def STD : STOREi32<0x3, store>;

// LOAD instructions
class LOAD<bits<2> SizeOp, list<dag> Pattern>
    : InstELVM<(outs GPR:$dst), (ins GPR:$src),
               "load $dst, $src",
               Pattern> {
  bits<3> mode;
  bits<2> size;
  bits<4> dst;
  bits<20> addr;

  let Inst{63-61} = mode;
  let Inst{60-59} = size;
  let Inst{51-48} = dst;
  let Inst{55-52} = addr{19-16};
  let Inst{47-32} = addr{15-0};

  let mode = 3;     // ELVM_MEM
  let size = SizeOp;
  let ELVMClass = 1; // ELVM_LDX
}

class LOADi32<bits<2> SizeOp, PatFrag OpNode>
    : LOAD<SizeOp, [(set i32:$dst, (OpNode addr:$src))]>;

def LDW : LOADi32<0x0, zextloadi32>;
def LDH : LOADi32<0x1, zextloadi16>;
def LDB : LOADi32<0x2, zextloadi8>;
def LDD : LOADi32<0x3, load>;

let isCodeGenOnly = 1 in {
def STOREFI : Pseudo<(outs), (ins GPR:$src, MEMri:$addr),
                      "STOREFI!",
//                     "sub BP, $fi    # STOREFI\n\t"
//                     "store $src, BP   # STOREFI\n\t"
//                     "add BP, $fi    # STOREFI\n",
                     [(store i32:$src, FIri:$addr)]>;
def LOADFI : Pseudo<(outs GPR:$dst), (ins MEMri:$addr),
                      "LOADFI!",
//                    "sub BP, $fi    # LOADFI\n\t"
//                    "load $dst, BP    # LOADFI\n\t"
//                    "add BP, $fi    # LOADFI\n",
                    [(set i32:$dst, (load FIri:$addr))]>;
}

class BRANCH<bits<4> Opc, string OpcodeStr, list<dag> Pattern>
    : InstELVM<(outs), (ins brtarget:$BrDst),
              !strconcat(OpcodeStr, " $BrDst"), Pattern> {
  bits<4> op;
  bits<16> BrDst;
  bits<1> ELVMSrc;

  let Inst{63-60} = op;
  let Inst{59} = ELVMSrc;
  let Inst{47-32} = BrDst;

  let op = Opc;
  let ELVMSrc = 0;
  let ELVMClass = 5; // ELVM_JMP
}

class CALL<string OpcodeStr>
    : InstELVM<(outs), (ins calltarget:$BrDst),
               !strconcat(OpcodeStr, " $BrDst"), []> {
  bits<4> op;
  bits<32> BrDst;
  bits<1> ELVMSrc;

  let Inst{63-60} = op;
  let Inst{59} = ELVMSrc;
  let Inst{31-0} = BrDst;

  let op = 8;       // ELVM_CALL
  let ELVMSrc = 0;
  let ELVMClass = 5; // ELVM_JMP
}

class CALL2<string OpcodeStr>
    : InstELVM<(outs), (ins GPR:$BrDst),
               !strconcat(OpcodeStr, " $BrDst"),
               [(ELVMcall GPR:$BrDst)]> {
  bits<4> op;
  bits<32> BrDst;
  bits<1> ELVMSrc;

  let Inst{63-60} = op;
  let Inst{59} = ELVMSrc;
  let Inst{31-0} = BrDst;

  let op = 9;       // ELVM_CALL
  let ELVMSrc = 0;
  let ELVMClass = 5; // ELVM_JMP
}

// Jump always
let isBranch = 1, isTerminator = 1, hasDelaySlot=0, isBarrier = 1 in {
  def JMP : BRANCH<0x0, "jmp", [(br bb:$BrDst)]>;
}

// Call
let isCall=1, hasDelaySlot=0 /*, Uses = [D]*/,
    // Potentially clobbered registers
    Defs = [A, B, C, D] in {
  def CALL_I : CALL<"call">;
  def CALL_R : CALL2<"call">;
}

class NOP_I<string OpcodeStr>
    : InstELVM<(outs), (ins i32imm:$imm),
              !strconcat(OpcodeStr, "\t$imm"), []> {
  // mov r0, r0 == nop
  bits<4> op;
  bits<1> ELVMSrc;
  bits<4> dst;
  bits<4> src;

  let Inst{63-60} = op;
  let Inst{59} = ELVMSrc;
  let Inst{55-52} = src;
  let Inst{51-48} = dst;

  let op = 0xb;     // ELVM_MOV
  let ELVMSrc = 1;   // ELVM_X
  let ELVMClass = 7; // ELVM_ALU32
  let src = 0;      // R0
  let dst = 0;      // R0
}

let hasSideEffects = 0 in
  def NOP : NOP_I<"nop">;

class RET<string OpcodeStr>
    : InstELVM<(outs), (ins),
              !strconcat(OpcodeStr, ""), [(ELVMretflag)]> {
  bits<4> op;

  let Inst{63-60} = op;
  let Inst{59} = 0;
  let Inst{31-0} = 0;

  let op = 9;       // ELVM_EXIT
  let ELVMClass = 5; // ELVM_JMP
}

let isReturn = 1, isTerminator = 1, hasDelaySlot=0, isBarrier = 1,
    isNotDuplicable = 1 in {
  def RET : RET<"ret">;
}

// ADJCALLSTACKDOWN/UP pseudo insns
//let Defs = [D], Uses = [D] in {
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt),
                              "#ADJCALLSTACKDOWN $amt",
                              [(ELVMcallseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              "#ADJCALLSTACKUP $amt1 $amt2",
                              [(ELVMcallseq_end timm:$amt1, timm:$amt2)]>;
//}

let usesCustomInserter = 1 in {
  def Select : Pseudo<(outs GPR:$dst),
                      (ins GPR:$lhs, GPR:$rhs, i32imm:$imm, GPR:$src, GPR:$src2),
                      "# Select PSEUDO $dst = $lhs $imm $rhs ? $src : $src2",
                      [(set i32:$dst,
                       (ELVMselectcc i32:$lhs, i32:$rhs, (i32 imm:$imm), i32:$src, i32:$src2))]>;
}

// load 32-bit global addr into register
def : Pat<(ELVMWrapper tglobaladdr:$in), (LD_imm32 tglobaladdr:$in)>;

// 0xffffFFFF doesn't fit into simm32, optimize common case
def : Pat<(i32 (and (i32 GPR:$src), 0xffffFFFF)),
          (SRL_ri (SLL_ri (i32 GPR:$src), 32), 32)>;

// Calls
def : Pat<(ELVMcall tglobaladdr:$dst), (CALL_I tglobaladdr:$dst)>;
def : Pat<(ELVMcall texternalsym:$dst), (CALL_I texternalsym:$dst)>;
def : Pat<(ELVMcall imm:$dst), (CALL_I imm:$dst)>;

// Loads
def : Pat<(extloadi8  GPR:$src), (i32 (LDB GPR:$src))>;
def : Pat<(extloadi16 GPR:$src), (i32 (LDH GPR:$src))>;
def : Pat<(extloadi32 GPR:$src), (i32 (LDW GPR:$src))>;

/*
// Atomics
class XADD<bits<2> SizeOp, string OpcodeStr, PatFrag OpNode>
    : InstELVM<(outs GPR:$dst), (ins MEMri:$addr, GPR:$val),
              "lock *("#OpcodeStr#" *)($addr) += $val",
              [(set GPR:$dst, (OpNode ADDRri:$addr, GPR:$val))]> {
  bits<3> mode;
  bits<2> size;
  bits<4> dst;
  bits<20> addr;

  let Inst{63-61} = mode;
  let Inst{60-59} = size;
  let Inst{51-48} = addr{19-16}; // base reg
  let Inst{55-52} = dst;
  let Inst{47-32} = addr{15-0}; // offset

  let mode = 6;     // ELVM_XADD
  let size = SizeOp;
  let ELVMClass = 3; // ELVM_STX
}

let Constraints = "$dst = $val" in {
def XADD32 : XADD<0, "u32e", atomic_load_add_32>;
}
*/

// bswap16, bswap32, bswap32
class BSWAP<bits<32> SizeOp, string OpcodeStr, list<dag> Pattern>
    : InstELVM<(outs GPR:$dst), (ins GPR:$src),
              !strconcat(OpcodeStr, "\t$dst"),
              Pattern> {
  bits<4> op;
  bits<1> ELVMSrc;
  bits<4> dst;
  bits<32> imm;

  let Inst{63-60} = op;
  let Inst{59} = ELVMSrc;
  let Inst{51-48} = dst;
  let Inst{31-0} = imm;

  let op = 0xd;     // ELVM_END
  let ELVMSrc = 1;   // ELVM_TO_BE (TODO: use ELVM_TO_LE for big-endian target)
  let ELVMClass = 4; // ELVM_ALU
  let imm = SizeOp;
}

let Constraints = "$dst = $src" in {
def BSWAP16 : BSWAP<16, "bswap16", [(set GPR:$dst, (srl (bswap GPR:$src), (i32 48)))]>;
def BSWAP32 : BSWAP<32, "bswap32", [(set GPR:$dst, (bswap GPR:$src))]>;
}

let Defs = [A, B, D], Uses = [C], hasSideEffects = 1,
    hasExtraDefRegAllocReq = 1, hasExtraSrcRegAllocReq = 1, mayLoad = 1 in {
class LOAD_ABS<bits<2> SizeOp, string OpcodeStr, Intrinsic OpNode>
    : InstELVM<(outs), (ins GPR:$skb, i32imm:$imm),
              "r0 = *("#OpcodeStr#" *)skb[$imm]",
              [(set A, (OpNode GPR:$skb, i32immSExt32:$imm))]> {
  bits<3> mode;
  bits<2> size;
  bits<32> imm;

  let Inst{63-61} = mode;
  let Inst{60-59} = size;
  let Inst{31-0} = imm;

  let mode = 1;     // ELVM_ABS
  let size = SizeOp;
  let ELVMClass = 0; // ELVM_LD
}

class LOAD_IND<bits<2> SizeOp, string OpcodeStr, Intrinsic OpNode>
    : InstELVM<(outs), (ins GPR:$skb, GPR:$val),
              "r0 = *("#OpcodeStr#" *)skb[$val]",
              [(set A, (OpNode GPR:$skb, GPR:$val))]> {
  bits<3> mode;
  bits<2> size;
  bits<4> val;

  let Inst{63-61} = mode;
  let Inst{60-59} = size;
  let Inst{55-52} = val;

  let mode = 2;     // ELVM_IND
  let size = SizeOp;
  let ELVMClass = 0; // ELVM_LD
}
}

/*
def LD_ABS_B : LOAD_ABS<2, "u8", int_bpf_load_byte>;
def LD_ABS_H : LOAD_ABS<1, "u16", int_bpf_load_half>;
def LD_ABS_W : LOAD_ABS<0, "u32", int_bpf_load_word>;

def LD_IND_B : LOAD_IND<2, "u8", int_bpf_load_byte>;
def LD_IND_H : LOAD_IND<1, "u16", int_bpf_load_half>;
def LD_IND_W : LOAD_IND<0, "u32", int_bpf_load_word>;
*/
