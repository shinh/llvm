//===-- ELVMTargetInfo.cpp - ELVM Target Implementation ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ELVM.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

namespace llvm {
Target &getTheELVMleTarget() {
  static Target TheELVMleTarget;
  return TheELVMleTarget;
}
Target &getTheELVMbeTarget() {
  static Target TheELVMbeTarget;
  return TheELVMbeTarget;
}
Target &getTheELVMTarget() {
  static Target TheELVMTarget;
  return TheELVMTarget;
}
} // namespace llvm

extern "C" void LLVMInitializeELVMTargetInfo() {
  TargetRegistry::RegisterTarget(getTheELVMTarget(), "elvm", "ELVM",
                                 [](Triple::ArchType) { return false; }, true);
  RegisterTarget<Triple::elvmel, /*HasJIT=*/true> X(getTheELVMleTarget(), "elvmel",
                                                   "ELVM (little endian)");
  RegisterTarget<Triple::elvmeb, /*HasJIT=*/true> Y(getTheELVMbeTarget(), "elvmeb",
                                                   "ELVM (big endian)");
}
