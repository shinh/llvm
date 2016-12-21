//===-- ELVMRegisterInfo.td - ELVM Register defs -------------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the ELVM register file
//===----------------------------------------------------------------------===//

class ELVMReg<bits<16> Enc, string n> : Register<n> {
  let Namespace = "ELVM";
  let HWEncoding = Enc;
}

// Integer registers
def A : ELVMReg< 0, "A">, DwarfRegNum<[0]>;
def B : ELVMReg< 1, "B">, DwarfRegNum<[1]>;
def C : ELVMReg< 2, "C">, DwarfRegNum<[2]>;
def D : ELVMReg< 3, "D">, DwarfRegNum<[3]>;
def BP : ELVMReg< 4, "BP">, DwarfRegNum<[4]>;
def SP : ELVMReg< 5, "SP">, DwarfRegNum<[5]>;

// Register classes.
def GPR : RegisterClass<"ELVM", [i32], 24, (add B, C, D,
                                            A,  // return value
                                            SP, // stack ptr
                                            BP  // frame ptr
                                           )>;
