//===-- ELVMFrameLowering.h - Define frame lowering for ELVM -----*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class implements ELVM-specific bits of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ELVM_ELVMFRAMELOWERING_H
#define LLVM_LIB_TARGET_ELVM_ELVMFRAMELOWERING_H

#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
class ELVMSubtarget;

class ELVMFrameLowering : public TargetFrameLowering {
public:
  explicit ELVMFrameLowering(const ELVMSubtarget &sti)
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 8, 0) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasFP(const MachineFunction &MF) const override;
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override {
    return MBB.erase(MI);
  }
};
}
#endif
