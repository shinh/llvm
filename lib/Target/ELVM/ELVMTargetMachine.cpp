//===-- ELVMTargetMachine.cpp - Define TargetMachine for ELVM ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about ELVM target spec.
//
//===----------------------------------------------------------------------===//

#include "ELVM.h"
#include "ELVMTargetMachine.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
using namespace llvm;

extern "C" void LLVMInitializeELVMTarget() {
  // Register the target.
  RegisterTargetMachine<ELVMTargetMachine> X(getTheELVMleTarget());
  RegisterTargetMachine<ELVMTargetMachine> Y(getTheELVMbeTarget());
  RegisterTargetMachine<ELVMTargetMachine> Z(getTheELVMTarget());
}

// DataLayout: little or big endian
static std::string computeDataLayout(const Triple &TT) {
  return "ELVM-e-m:e-p:32:32-i32:32-n32-S32";
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::PIC_;
  return *RM;
}

ELVMTargetMachine::ELVMTargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   Optional<Reloc::Model> RM,
                                   CodeModel::Model CM, CodeGenOpt::Level OL)
    : LLVMTargetMachine(T, computeDataLayout(TT), TT, CPU, FS, Options,
                        getEffectiveRelocModel(RM), CM, OL),
      TLOF(make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}
namespace {
// ELVM Code Generator Pass Configuration Options.
class ELVMPassConfig : public TargetPassConfig {
public:
  ELVMPassConfig(ELVMTargetMachine *TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  ELVMTargetMachine &getELVMTargetMachine() const {
    return getTM<ELVMTargetMachine>();
  }

  bool addInstSelector() override;
};
}

TargetPassConfig *ELVMTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new ELVMPassConfig(this, PM);
}

// Install an instruction selector pass using
// the ISelDag to gen ELVM code.
bool ELVMPassConfig::addInstSelector() {
  addPass(createELVMISelDag(getELVMTargetMachine()));

  return false;
}
