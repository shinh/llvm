//===- ELVMDisassembler.cpp - Disassembler for ELVM ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is part of the ELVM Disassembler.
//
//===----------------------------------------------------------------------===//

#include "ELVM.h"
#include "ELVMRegisterInfo.h"
#include "ELVMSubtarget.h"
#include "MCTargetDesc/ELVMMCTargetDesc.h"

#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCFixedLenDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "elvm-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

/// A disassembler class for ELVM.
class ELVMDisassembler : public MCDisassembler {
public:
  ELVMDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  virtual ~ELVMDisassembler() {}

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &VStream,
                              raw_ostream &CStream) const override;
};
}

static MCDisassembler *createELVMDisassembler(const Target &T,
                                             const MCSubtargetInfo &STI,
                                             MCContext &Ctx) {
  return new ELVMDisassembler(STI, Ctx);
}


extern "C" void LLVMInitializeELVMDisassembler() {
  // Register the disassembler.
  TargetRegistry::RegisterMCDisassembler(getTheELVMTarget(),
                                         createELVMDisassembler);
  TargetRegistry::RegisterMCDisassembler(getTheELVMleTarget(),
                                         createELVMDisassembler);
  TargetRegistry::RegisterMCDisassembler(getTheELVMbeTarget(),
                                         createELVMDisassembler);
}

static const unsigned GPRDecoderTable[] = {
    ELVM::A,  ELVM::B,  ELVM::C,  ELVM::D,  ELVM::BP,  ELVM::SP
};

static DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t /*Address*/,
                                           const void * /*Decoder*/) {
  if (RegNo > 11)
    return MCDisassembler::Fail;

  unsigned Reg = GPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus decodeMemoryOpValue(MCInst &Inst, unsigned Insn,
                                        uint64_t Address, const void *Decoder) {
  unsigned Register = (Insn >> 16) & 0xf;
  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[Register]));
  unsigned Offset = (Insn & 0xffff);
  Inst.addOperand(MCOperand::createImm(SignExtend32<16>(Offset)));

  return MCDisassembler::Success;
}

#include "ELVMGenDisassemblerTables.inc"

static DecodeStatus readInstruction64(ArrayRef<uint8_t> Bytes, uint64_t Address,
                                      uint64_t &Size, uint64_t &Insn) {
  uint64_t Lo, Hi;

  if (Bytes.size() < 8) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Size = 8;
  Hi = (Bytes[0] << 24) | (Bytes[1] << 16) | (Bytes[2] << 0) | (Bytes[3] << 8);
  Lo = (Bytes[4] << 0) | (Bytes[5] << 8) | (Bytes[6] << 16) | (Bytes[7] << 24);
  Insn = Make_64(Hi, Lo);

  return MCDisassembler::Success;
}

DecodeStatus ELVMDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                             ArrayRef<uint8_t> Bytes,
                                             uint64_t Address,
                                             raw_ostream &VStream,
                                             raw_ostream &CStream) const {
  uint64_t Insn;
  DecodeStatus Result;

  Result = readInstruction64(Bytes, Address, Size, Insn);
  if (Result == MCDisassembler::Fail) return MCDisassembler::Fail;

  Result = decodeInstruction(DecoderTableELVM64, Instr, Insn,
                             Address, this, STI);
  if (Result == MCDisassembler::Fail) return MCDisassembler::Fail;

  switch (Instr.getOpcode()) {
  case ELVM::LD_imm32: {
    if (Bytes.size() < 16) {
      Size = 0;
      return MCDisassembler::Fail;
    }
    Size = 16;
    uint64_t Hi = (Bytes[12] << 0) | (Bytes[13] << 8) | (Bytes[14] << 16) | (Bytes[15] << 24);
    auto& Op = Instr.getOperand(1);
    Op.setImm(Make_64(Hi, Op.getImm()));
    break;
  }
#if 0
  case ELVM::LD_ABS_B:
  case ELVM::LD_ABS_H:
  case ELVM::LD_ABS_W:
  case ELVM::LD_IND_B:
  case ELVM::LD_IND_H:
  case ELVM::LD_IND_W: {
    auto Op = Instr.getOperand(0);
    Instr.clear();
    Instr.addOperand(MCOperand::createReg(ELVM::R6));
    Instr.addOperand(Op);
    break;
  }
#endif
  }

  return Result;
}

typedef DecodeStatus (*DecodeFunc)(MCInst &MI, unsigned insn, uint64_t Address,
                                   const void *Decoder);
