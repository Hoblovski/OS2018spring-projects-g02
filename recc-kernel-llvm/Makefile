# position of codes and objects
BUILD_DIR    := build
SRC_DIR      := kernel
ASMSRC_DIR   := kernel/l2
SINCLUDE_DIR := include

# FIXME: terrible hack, placing main at 0x0 is done by specifing `entry` 
# 	as the first one.
# I've tried the usual .startup way to put main at 0x0, but the linker
# 	then puts main and schedule_next_task both at 0x0
# I've tried the input section  entry.o(.text),
# 	but the linker does not know this command. (llvm 3.9's problem?)
OBJS := entry kernel_impl kernel_state my_printf queue user_proc memman\
	getchar_nobusy iehandling kernel_interface putchar_busy putchar_nobusy
OBJS := $(addprefix ${BUILD_DIR}/, ${OBJS})
OBJS := $(addsuffix .o, ${OBJS})

# tools
LLVM_BUILD_DIR := ../llvm-3.9.0.src/build/bin
LD             := ${LLVM_BUILD_DIR}/ld.lld
LD_FLAGS       := -T script.ld -e main -o a.out
CLANG          := clang # use system clang, we don't use custom frontend
CLANG_FLAGS    := -target mips-unknown-linux-gnu -O0 -emit-llvm -isystem${SINCLUDE_DIR} -nostdinc -Wno-incompatible-library-redeclaration
LLC            := ${LLVM_BUILD_DIR}/llc
LLC_FLAGS      := -march=cpu0el -relocation-model=static
LLAS           := ${LLVM_BUILD_DIR}/llvm-mc
LLAS_FLAGS     := -arch=cpu0el -filetype=obj
OBJCOPY        := objcopy
OBJCOPY_FLAGS  := -I elf32-little -O binary -j .text -j .rodata

.PHONY: all clean

all: ${BUILD_DIR}/ ${BUILD_DIR}/a.out ${BUILD_DIR}/a.img

clean:
	rm build/ -rf

${BUILD_DIR}/:
	mkdir ${BUILD_DIR}

${BUILD_DIR}/a.out: ${OBJS}
	${LD} ${LD_FLAGS} $^ -o $@

${BUILD_DIR}/a.img: ${BUILD_DIR}/a.out
	${OBJCOPY} ${OBJCOPY_FLAGS} $< $@

${BUILD_DIR}/%.o: ${BUILD_DIR}/%.bc
	${LLC} ${LLC_FLAGS} $< -o $@ -filetype=obj
	${LLC} ${LLC_FLAGS} $< -o $@.s -filetype=asm

${BUILD_DIR}/%.bc: ${SRC_DIR}/%.c ${SRC_DIR}/*.h # FIXME: obj depends on all headers?!
	${CLANG} ${CLANG_FLAGS} $< -o $@ -c

${BUILD_DIR}/%.o: ${ASMSRC_DIR}/%.s
	${LLAS} ${LLAS_FLAGS} $< -o $@
