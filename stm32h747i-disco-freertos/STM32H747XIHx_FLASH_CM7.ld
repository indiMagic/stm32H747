/* Linker script: STM32H747XI, Cortex-M7 core, runs from Flash bank 1.
 * Flash bank 2 (0x08100000) is left for the Cortex-M4 image. */

ENTRY(Reset_Handler)

_Min_Heap_Size  = 0x200;   /* newlib heap (FreeRTOS uses its own heap_4 array) */
_Min_Stack_Size = 0x800;   /* main/ISR stack (MSP) */

_estack = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM);

MEMORY
{
  FLASH   (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
  DTCMRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  RAM_D1  (xrw) : ORIGIN = 0x24000000, LENGTH = 512K   /* AXI SRAM */
  RAM_D2  (xrw) : ORIGIN = 0x30000000, LENGTH = 288K   /* SRAM1-3, DMA-capable */
  RAM_D3  (xrw) : ORIGIN = 0x38000000, LENGTH = 64K    /* SRAM4 */
  ITCMRAM (xrw) : ORIGIN = 0x00000000, LENGTH = 64K
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab : { . = ALIGN(4); *(.ARM.extab* .gnu.linkonce.armextab.*) . = ALIGN(4); } >FLASH
  .ARM : { . = ALIGN(4); __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; . = ALIGN(4); } >FLASH

  .preinit_array : { . = ALIGN(4); PROVIDE_HIDDEN(__preinit_array_start = .); KEEP(*(.preinit_array*)) PROVIDE_HIDDEN(__preinit_array_end = .); . = ALIGN(4); } >FLASH
  .init_array :    { . = ALIGN(4); PROVIDE_HIDDEN(__init_array_start = .);    KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array*)) PROVIDE_HIDDEN(__init_array_end = .); . = ALIGN(4); } >FLASH
  .fini_array :    { . = ALIGN(4); PROVIDE_HIDDEN(__fini_array_start = .);    KEEP(*(SORT(.fini_array.*))) KEEP(*(.fini_array*)) PROVIDE_HIDDEN(__fini_array_end = .); . = ALIGN(4); } >FLASH

  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    *(.RamFunc)
    *(.RamFunc*)
    . = ALIGN(4);
    _edata = .;
  } >RAM_D1 AT> FLASH

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM_D1

  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    . = . + _Min_Heap_Size;
    . = ALIGN(8);
  } >RAM_D1

  /* Stack lives in DTCM (fastest RAM; NOT reachable by DMA) */
  ._stack_check (NOLOAD) :
  {
    . = ALIGN(8);
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >DTCMRAM

  /* DMA buffers: place with __attribute__((section(".dma_buffer"))) */
  .dma_buffer (NOLOAD) :
  {
    . = ALIGN(32);
    *(.dma_buffer)
    *(.dma_buffer*)
    . = ALIGN(32);
  } >RAM_D2

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
