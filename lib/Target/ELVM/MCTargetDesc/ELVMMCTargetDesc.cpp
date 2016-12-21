//===-- ELVMMCTargetDesc.cpp - ELVM Target Descriptions ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides ELVM specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "ELVM.h"
#include "ELVMMCTargetDesc.h"
#include "ELVMMCAsmInfo.h"
#include "InstPrinter/ELVMInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "ELVMGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "ELVMGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "ELVMGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createELVMMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitELVMMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createELVMMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitELVMMCRegisterInfo(X, ELVM::D /* RAReg doesn't exist */);
  return X;
}

static MCSubtargetInfo *createELVMMCSubtargetInfo(const Triple &TT,
                                                 StringRef CPU, StringRef FS) {
  return createELVMMCSubtargetInfoImpl(TT, CPU, FS);
}

static MCStreamer *createELVMMCStreamer(const Triple &T,
                                       MCContext &Ctx, MCAsmBackend &MAB,
                                       raw_pwrite_stream &OS, MCCodeEmitter *Emitter,
                                       bool RelaxAll) {
  return createELFStreamer(Ctx, MAB, OS, Emitter, RelaxAll);
}

static MCInstPrinter *createELVMMCInstPrinter(const Triple &T,
                                             unsigned SyntaxVariant,
                                             const MCAsmInfo &MAI,
                                             const MCInstrInfo &MII,
                                             const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new ELVMInstPrinter(MAI, MII, MRI);
  return 0;
}

extern "C" void LLVMInitializeELVMTargetMC() {
  for (Target *T :
       {&getTheELVMleTarget(), &getTheELVMbeTarget(), &getTheELVMTarget()}) {
    // Register the MC asm info.
    RegisterMCAsmInfo<ELVMMCAsmInfo> X(*T);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createELVMMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createELVMMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T,
                                            createELVMMCSubtargetInfo);

    // Register the object streamer
    TargetRegistry::RegisterELFStreamer(*T, createELVMMCStreamer);

    // Register the MCInstPrinter.
    TargetRegistry::RegisterMCInstPrinter(*T, createELVMMCInstPrinter);
  }

  // Register the MC code emitter
  TargetRegistry::RegisterMCCodeEmitter(getTheELVMleTarget(),
                                        createELVMMCCodeEmitter);
  TargetRegistry::RegisterMCCodeEmitter(getTheELVMbeTarget(),
                                        createELVMbeMCCodeEmitter);

  // Register the ASM Backend
  TargetRegistry::RegisterMCAsmBackend(getTheELVMleTarget(),
                                       createELVMAsmBackend);
  TargetRegistry::RegisterMCAsmBackend(getTheELVMbeTarget(),
                                       createELVMbeAsmBackend);

  if (sys::IsLittleEndianHost) {
    TargetRegistry::RegisterMCCodeEmitter(getTheELVMTarget(),
                                          createELVMMCCodeEmitter);
    TargetRegistry::RegisterMCAsmBackend(getTheELVMTarget(),
                                         createELVMAsmBackend);
  } else {
    TargetRegistry::RegisterMCCodeEmitter(getTheELVMTarget(),
                                          createELVMbeMCCodeEmitter);
    TargetRegistry::RegisterMCAsmBackend(getTheELVMTarget(),
                                         createELVMbeAsmBackend);
  }
}
