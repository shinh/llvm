//===-- ELVMFrameLowering.cpp - ELVM Frame Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the ELVM implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "ELVMFrameLowering.h"
#include "ELVMInstrInfo.h"
#include "ELVMSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

bool ELVMFrameLowering::hasFP(const MachineFunction &MF) const { return true; }

void ELVMFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {}

void ELVMFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {}

void ELVMFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                            BitVector &SavedRegs,
                                            RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  SavedRegs.reset(ELVM::R6);
  SavedRegs.reset(ELVM::R7);
  SavedRegs.reset(ELVM::R8);
  SavedRegs.reset(ELVM::R9);
}
