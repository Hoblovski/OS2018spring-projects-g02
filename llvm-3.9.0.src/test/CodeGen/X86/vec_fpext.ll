; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown -mattr=+sse4.1 | FileCheck %s --check-prefix=X32-SSE
; RUN: llc < %s -mtriple=i686-unknown -mattr=+avx | FileCheck %s --check-prefix=X32-AVX
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+sse4.1 | FileCheck %s --check-prefix=X64-SSE
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+avx | FileCheck %s --check-prefix=X64-AVX

; PR11674
define void @fpext_frommem(<2 x float>* %in, <2 x double>* %out) {
; X32-SSE-LABEL: fpext_frommem:
; X32-SSE:       # BB#0: # %entry
; X32-SSE-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-SSE-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-SSE-NEXT:    cvtps2pd (%ecx), %xmm0
; X32-SSE-NEXT:    movups %xmm0, (%eax)
; X32-SSE-NEXT:    retl
;
; X32-AVX-LABEL: fpext_frommem:
; X32-AVX:       # BB#0: # %entry
; X32-AVX-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-AVX-NEXT:    vcvtps2pd (%ecx), %xmm0
; X32-AVX-NEXT:    vmovups %xmm0, (%eax)
; X32-AVX-NEXT:    retl
;
; X64-SSE-LABEL: fpext_frommem:
; X64-SSE:       # BB#0: # %entry
; X64-SSE-NEXT:    cvtps2pd (%rdi), %xmm0
; X64-SSE-NEXT:    movups %xmm0, (%rsi)
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: fpext_frommem:
; X64-AVX:       # BB#0: # %entry
; X64-AVX-NEXT:    vcvtps2pd (%rdi), %xmm0
; X64-AVX-NEXT:    vmovups %xmm0, (%rsi)
; X64-AVX-NEXT:    retq
entry:
  %0 = load <2 x float>, <2 x float>* %in, align 8
  %1 = fpext <2 x float> %0 to <2 x double>
  store <2 x double> %1, <2 x double>* %out, align 1
  ret void
}

define void @fpext_frommem4(<4 x float>* %in, <4 x double>* %out) {
; X32-SSE-LABEL: fpext_frommem4:
; X32-SSE:       # BB#0: # %entry
; X32-SSE-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-SSE-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-SSE-NEXT:    cvtps2pd (%ecx), %xmm0
; X32-SSE-NEXT:    cvtps2pd 8(%ecx), %xmm1
; X32-SSE-NEXT:    movups %xmm1, 16(%eax)
; X32-SSE-NEXT:    movups %xmm0, (%eax)
; X32-SSE-NEXT:    retl
;
; X32-AVX-LABEL: fpext_frommem4:
; X32-AVX:       # BB#0: # %entry
; X32-AVX-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-AVX-NEXT:    vcvtps2pd (%ecx), %ymm0
; X32-AVX-NEXT:    vmovups %ymm0, (%eax)
; X32-AVX-NEXT:    vzeroupper
; X32-AVX-NEXT:    retl
;
; X64-SSE-LABEL: fpext_frommem4:
; X64-SSE:       # BB#0: # %entry
; X64-SSE-NEXT:    cvtps2pd (%rdi), %xmm0
; X64-SSE-NEXT:    cvtps2pd 8(%rdi), %xmm1
; X64-SSE-NEXT:    movups %xmm1, 16(%rsi)
; X64-SSE-NEXT:    movups %xmm0, (%rsi)
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: fpext_frommem4:
; X64-AVX:       # BB#0: # %entry
; X64-AVX-NEXT:    vcvtps2pd (%rdi), %ymm0
; X64-AVX-NEXT:    vmovups %ymm0, (%rsi)
; X64-AVX-NEXT:    vzeroupper
; X64-AVX-NEXT:    retq
entry:
  %0 = load <4 x float>, <4 x float>* %in
  %1 = fpext <4 x float> %0 to <4 x double>
  store <4 x double> %1, <4 x double>* %out, align 1
  ret void
}

define void @fpext_frommem8(<8 x float>* %in, <8 x double>* %out) {
; X32-SSE-LABEL: fpext_frommem8:
; X32-SSE:       # BB#0: # %entry
; X32-SSE-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-SSE-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-SSE-NEXT:    cvtps2pd (%ecx), %xmm0
; X32-SSE-NEXT:    cvtps2pd 8(%ecx), %xmm1
; X32-SSE-NEXT:    cvtps2pd 16(%ecx), %xmm2
; X32-SSE-NEXT:    cvtps2pd 24(%ecx), %xmm3
; X32-SSE-NEXT:    movups %xmm3, 48(%eax)
; X32-SSE-NEXT:    movups %xmm2, 32(%eax)
; X32-SSE-NEXT:    movups %xmm1, 16(%eax)
; X32-SSE-NEXT:    movups %xmm0, (%eax)
; X32-SSE-NEXT:    retl
;
; X32-AVX-LABEL: fpext_frommem8:
; X32-AVX:       # BB#0: # %entry
; X32-AVX-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-AVX-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-AVX-NEXT:    vcvtps2pd (%ecx), %ymm0
; X32-AVX-NEXT:    vcvtps2pd 16(%ecx), %ymm1
; X32-AVX-NEXT:    vmovups %ymm1, 32(%eax)
; X32-AVX-NEXT:    vmovups %ymm0, (%eax)
; X32-AVX-NEXT:    vzeroupper
; X32-AVX-NEXT:    retl
;
; X64-SSE-LABEL: fpext_frommem8:
; X64-SSE:       # BB#0: # %entry
; X64-SSE-NEXT:    cvtps2pd (%rdi), %xmm0
; X64-SSE-NEXT:    cvtps2pd 8(%rdi), %xmm1
; X64-SSE-NEXT:    cvtps2pd 16(%rdi), %xmm2
; X64-SSE-NEXT:    cvtps2pd 24(%rdi), %xmm3
; X64-SSE-NEXT:    movups %xmm3, 48(%rsi)
; X64-SSE-NEXT:    movups %xmm2, 32(%rsi)
; X64-SSE-NEXT:    movups %xmm1, 16(%rsi)
; X64-SSE-NEXT:    movups %xmm0, (%rsi)
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: fpext_frommem8:
; X64-AVX:       # BB#0: # %entry
; X64-AVX-NEXT:    vcvtps2pd (%rdi), %ymm0
; X64-AVX-NEXT:    vcvtps2pd 16(%rdi), %ymm1
; X64-AVX-NEXT:    vmovups %ymm1, 32(%rsi)
; X64-AVX-NEXT:    vmovups %ymm0, (%rsi)
; X64-AVX-NEXT:    vzeroupper
; X64-AVX-NEXT:    retq
entry:
  %0 = load <8 x float>, <8 x float>* %in
  %1 = fpext <8 x float> %0 to <8 x double>
  store <8 x double> %1, <8 x double>* %out, align 1
  ret void
}

define <2 x double> @fpext_fromconst() {
; X32-SSE-LABEL: fpext_fromconst:
; X32-SSE:       # BB#0: # %entry
; X32-SSE-NEXT:    movaps {{.*#+}} xmm0 = [1.000000e+00,-2.000000e+00]
; X32-SSE-NEXT:    retl
;
; X32-AVX-LABEL: fpext_fromconst:
; X32-AVX:       # BB#0: # %entry
; X32-AVX-NEXT:    vmovaps {{.*#+}} xmm0 = [1.000000e+00,-2.000000e+00]
; X32-AVX-NEXT:    retl
;
; X64-SSE-LABEL: fpext_fromconst:
; X64-SSE:       # BB#0: # %entry
; X64-SSE-NEXT:    movaps {{.*#+}} xmm0 = [1.000000e+00,-2.000000e+00]
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: fpext_fromconst:
; X64-AVX:       # BB#0: # %entry
; X64-AVX-NEXT:    vmovaps {{.*#+}} xmm0 = [1.000000e+00,-2.000000e+00]
; X64-AVX-NEXT:    retq
entry:
  %0  = insertelement <2 x float> undef, float 1.0, i32 0
  %1  = insertelement <2 x float> %0, float -2.0, i32 1
  %2  = fpext <2 x float> %1 to <2 x double>
  ret <2 x double> %2
}
