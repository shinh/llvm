//===-- ELVMMachineFunctionInfo.h - ELVM machine function info ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares ELVM-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ELVM_ELVMMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_ELVM_ELVMMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineValueType.h"

namespace llvm {

/// ELVMMachineFunctionInfo - This class is derived from MachineFunction and
/// contains private ELVM target-specific information for each MachineFunction.
class ELVMMachineFunctionInfo : public MachineFunctionInfo {
  virtual void anchor() {}

  /// VarArgsFrameIndex - FrameIndex for start of varargs area.
  int VarArgsFrameIndex = 0;

public:
  ELVMMachineFunctionInfo() = default;

  explicit ELVMMachineFunctionInfo(MachineFunction &MF) {}

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Idx) { VarArgsFrameIndex = Idx; }
};

} // End llvm namespace

#endif
