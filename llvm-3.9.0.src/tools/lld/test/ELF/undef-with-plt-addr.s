// REQUIRES: x86
// RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %s -o %t.o
// RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %p/Inputs/undef-with-plt-addr.s -o %t2.o
// RUN: ld.lld %t2.o -o %t2.so -shared
// RUN: ld.lld %t.o %t2.so -o %t3
// RUN: llvm-readobj -t -s -r %t3 | FileCheck %s

.globl _start
_start:
movabsq	$set_data, %rax

.data
.quad foo
// Test that set_data has an address in the .plt, but foo is not

// CHECK:      Name: .plt
// CHECK-NEXT: Type: SHT_PROGBITS
// CHECK-NEXT: Flags [
// CHECK-NEXT:   SHF_ALLOC
// CHECK-NEXT:   SHF_EXECINSTR
// CHECK-NEXT: ]
// CHECK-NEXT: Address: 0x11010

// CHECK:      Section ({{.*}}) .rela.dyn {
// CHECK-NEXT:   0x13000 R_X86_64_64 foo 0x0
// CHECK-NEXT: }
// CHECK-NEXT: Section ({{.*}}) .rela.plt {
// CHECK-NEXT:   0x13020 R_X86_64_JUMP_SLOT set_data 0x0
// CHECK-NEXT: }

// CHECK:      Name: foo
// CHECK-NEXT: Value: 0x0
// CHECK-NEXT: Size: 0
// CHECK-NEXT: Binding: Global
// CHECK-NEXT: Type: Function
// CHECK-NEXT: Other: 0
// CHECK-NEXT: Section: Undefined

// CHECK:      Name:    set_data
// CHECK-NEXT: Value:   0x11020
// CHECK-NEXT: Size: 0
// CHECK-NEXT: Binding: Global
// CHECK-NEXT: Type: Function
// CHECK-NEXT: Other: 0
// CHECK-NEXT: Section: Undefined
