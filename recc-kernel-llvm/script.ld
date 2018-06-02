SECTIONS
{
  . = 0xC0000000;
  .text : {
    *(.text)
  }
  . = ALIGN(0x10);
  .data : {
    *(.data)
  }
}

