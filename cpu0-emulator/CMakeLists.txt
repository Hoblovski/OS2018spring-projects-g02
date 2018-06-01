project (cpu0emulator)

add_executable(cpu0emu
  cpu0machine.c
  loader.h loader.c
  uart.h uart.c
  mem.c
  machine.h
  common.h
	main.c)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -m32")
