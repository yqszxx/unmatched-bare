UART = 0x10010000;
STACK = 0x81000000;

SECTIONS
{
  . = 0x80000000;
  .text : {
    *(.text.init)
    *(.text)
  }
  .data : { *(.data) }
  .bss : {
    *(.bss)
  }
}
