//===-- ELVM.h - Top-level interface for ELVM representation ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ELVM_ELVM_H
#define LLVM_LIB_TARGET_ELVM_ELVM_H

#include "MCTargetDesc/ELVMMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class ELVMTargetMachine;

FunctionPass *createELVMISelDag(ELVMTargetMachine &TM);
}

#endif
