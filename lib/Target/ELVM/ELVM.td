//===-- ELVM.td - Describe the ELVM Target Machine -----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

include "ELVMRegisterInfo.td"
include "ELVMCallingConv.td"
include "ELVMInstrInfo.td"

def ELVMInstrInfo : InstrInfo;

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def ELVMInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def ELVMAsmParser : AsmParser {
  bit HasMnemonicFirst = 0;
}

def ELVMAsmParserVariant : AsmParserVariant {
  int Variant = 0;
  string Name = "ELVM";
  string BreakCharacters = ".";
  string TokenizingCharacters = "#()[]=:.<>!+*";
}

def ELVM : Target {
  let InstructionSet = ELVMInstrInfo;
  let AssemblyWriters = [ELVMInstPrinter];
  let AssemblyParsers = [ELVMAsmParser];
  let AssemblyParserVariants = [ELVMAsmParserVariant];
}
