set(LLVM_TARGET_DEFINITIONS ELVM.td)

tablegen(LLVM ELVMGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM ELVMGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM ELVMGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM ELVMGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM X86GenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM ELVMGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM ELVMGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM ELVMGenCallingConv.inc -gen-callingconv)
tablegen(LLVM ELVMGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(ELVMCommonTableGen)

add_llvm_target(ELVMCodeGen
  ELVMAsmPrinter.cpp
  ELVMFrameLowering.cpp
  ELVMInstrInfo.cpp
  ELVMISelDAGToDAG.cpp
  ELVMISelLowering.cpp
  ELVMMCInstLower.cpp
  ELVMRegisterInfo.cpp
  ELVMSubtarget.cpp
  ELVMTargetMachine.cpp
  )

add_subdirectory(Disassembler)
add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
