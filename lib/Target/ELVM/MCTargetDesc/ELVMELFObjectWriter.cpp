//===-- ELVMELFObjectWriter.cpp - ELVM ELF Writer ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/ELVMMCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class ELVMELFObjectWriter : public MCELFObjectTargetWriter {
public:
  ELVMELFObjectWriter(uint8_t OSABI);

  ~ELVMELFObjectWriter() override;

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};
}

ELVMELFObjectWriter::ELVMELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(/*Is64Bit*/ true, OSABI, /*ELF::EM_ELVM*/ -42,
                              /*HasRelocationAddend*/ false) {}

ELVMELFObjectWriter::~ELVMELFObjectWriter() {}

unsigned ELVMELFObjectWriter::getRelocType(MCContext &Ctx, const MCValue &Target,
                                          const MCFixup &Fixup,
                                          bool IsPCRel) const {
  // determine the type of the relocation
  switch ((unsigned)Fixup.getKind()) {
  default:
    llvm_unreachable("invalid fixup kind!");
#if 0
  case FK_SecRel_8:
    return ELF::R_ELVM_64_64;
  case FK_SecRel_4:
    return ELF::R_ELVM_64_32;
  case FK_Data_8:
    return ELF::R_ELVM_64_64;
  case FK_Data_4:
    return ELF::R_ELVM_64_32;
#endif
  }
}

MCObjectWriter *llvm::createELVMELFObjectWriter(raw_pwrite_stream &OS,
                                               uint8_t OSABI, bool IsLittleEndian) {
  MCELFObjectTargetWriter *MOTW = new ELVMELFObjectWriter(OSABI);
  return createELFObjectWriter(MOTW, OS, IsLittleEndian);
}
