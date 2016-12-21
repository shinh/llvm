//===-- ELVMTargetMachine.h - Define TargetMachine for ELVM --- C++ ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the ELVM specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ELVM_ELVMTARGETMACHINE_H
#define LLVM_LIB_TARGET_ELVM_ELVMTARGETMACHINE_H

#include "ELVMSubtarget.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class ELVMTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  ELVMSubtarget Subtarget;

public:
  ELVMTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                   StringRef FS, const TargetOptions &Options,
                   Optional<Reloc::Model> RM, CodeModel::Model CM,
                   CodeGenOpt::Level OL);

  const ELVMSubtarget *getSubtargetImpl() const { return &Subtarget; }
  const ELVMSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};
}

#endif
