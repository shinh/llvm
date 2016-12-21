//===-- ELVMMCTargetDesc.h - ELVM Target Descriptions -------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_ELVM_MCTARGETDESC_ELVMMCTARGETDESC_H
#define LLVM_LIB_TARGET_ELVM_MCTARGETDESC_ELVMMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"
#include "llvm/Config/config.h"

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class StringRef;
class Target;
class Triple;
class raw_ostream;
class raw_pwrite_stream;

Target &getTheELVMleTarget();
Target &getTheELVMbeTarget();
Target &getTheELVMTarget();

MCCodeEmitter *createELVMMCCodeEmitter(const MCInstrInfo &MCII,
                                      const MCRegisterInfo &MRI,
                                      MCContext &Ctx);
MCCodeEmitter *createELVMbeMCCodeEmitter(const MCInstrInfo &MCII,
                                        const MCRegisterInfo &MRI,
                                        MCContext &Ctx);

MCAsmBackend *createELVMAsmBackend(const Target &T, const MCRegisterInfo &MRI,
                                  const Triple &TT, StringRef CPU,
                                  const MCTargetOptions &Options);
MCAsmBackend *createELVMbeAsmBackend(const Target &T, const MCRegisterInfo &MRI,
                                    const Triple &TT, StringRef CPU,
                                    const MCTargetOptions &Options);

MCObjectWriter *createELVMELFObjectWriter(raw_pwrite_stream &OS,
                                         uint8_t OSABI, bool IsLittleEndian);
}

// Defines symbolic names for ELVM registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "ELVMGenRegisterInfo.inc"

// Defines symbolic names for the ELVM instructions.
//
#define GET_INSTRINFO_ENUM
#include "ELVMGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "ELVMGenSubtargetInfo.inc"

#endif
