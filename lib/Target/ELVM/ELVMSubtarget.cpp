//===-- ELVMSubtarget.cpp - ELVM Subtarget Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the ELVM specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "ELVMSubtarget.h"
#include "ELVM.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "elvm-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "ELVMGenSubtargetInfo.inc"

void ELVMSubtarget::anchor() {}

ELVMSubtarget::ELVMSubtarget(const Triple &TT, const std::string &CPU,
                           const std::string &FS, const TargetMachine &TM)
    : ELVMGenSubtargetInfo(TT, CPU, FS), InstrInfo(), FrameLowering(*this),
      TLInfo(TM, *this) {}
