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

ELVMFrameLowering::ELVMFrameLowering(const ELVMSubtarget &STI)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 1, 0),
      STI(STI), TII(*STI.getInstrInfo()), TRI(STI.getRegisterInfo()) {
}

bool ELVMFrameLowering::hasFP(const MachineFunction &MF) const { return true; }

void ELVMFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.begin();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  DebugLoc DL = MBB.findDebugLoc(MBBI);
  BuildMI(MBB, MBBI, DL, TII.get(ELVM::SUB_ri), ELVM::SP)
      .addReg(ELVM::SP)
      .addImm(1)
      .setMIFlag(MachineInstr::FrameSetup);
  BuildMI(MBB, MBBI, DL, TII.get(ELVM::STD), ELVM::BP)
      .addReg(ELVM::SP)
      .setMIFlag(MachineInstr::FrameSetup);
  BuildMI(MBB, MBBI, DL, TII.get(ELVM::MOV_rr), ELVM::BP)
      .addReg(ELVM::SP)
      .setMIFlag(MachineInstr::FrameSetup);
  if (MFI.getStackSize()) {
    BuildMI(MBB, MBBI, DL, TII.get(ELVM::SUB_ri), ELVM::SP)
        .addReg(ELVM::SP)
        .addImm(MFI.getStackSize())
        .setMIFlag(MachineInstr::FrameSetup);
  }
}

void ELVMFrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getFirstTerminator();
  DebugLoc DL = MBB.findDebugLoc(MBBI);
  BuildMI(MBB, MBBI, DL, TII.get(ELVM::MOV_rr), ELVM::SP)
      .addReg(ELVM::BP)
      .setMIFlag(MachineInstr::FrameDestroy);
  BuildMI(MBB, MBBI, DL, TII.get(ELVM::LDD), ELVM::BP)
      .addReg(ELVM::SP)
      .setMIFlag(MachineInstr::FrameDestroy);
  BuildMI(MBB, MBBI, DL, TII.get(ELVM::ADD_ri), ELVM::SP)
      .addReg(ELVM::SP)
      .addImm(1)
      .setMIFlag(MachineInstr::FrameDestroy);
}

void ELVMFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                            BitVector &SavedRegs,
                                            RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
}
