//===-- ELVMCallingConv.td - Calling Conventions ELVM --------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the ELVM architecture.
//
//===----------------------------------------------------------------------===//

// ELVM C return-value convention.
def RetCC_ELVM : CallingConv<[CCIfType<[i32], CCAssignToReg<[A]>>]>;

// ELVM C Calling convention.
def CC_ELVM : CallingConv<[
  CCAssignToStack<4, 4>
]>;
