ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);

_Min_Heap_Size = 0x300;
_Min_Stack_Size = 0x500;

__header_offset = 0x258;
__header_size = 0x10;
__dev_serial_preldr_offset = __header_offset + __header_size;

__preldr_start = ORIGIN(PRELDR_FLASH);
__preldr_end = ORIGIN(PRELDR_FLASH) + LENGTH(PRELDR_FLASH);

__ldr_start = ORIGIN(LDR_FLASH);
__ldr_end = ORIGIN(LDR_FLASH) + LENGTH(LDR_FLASH);

__app_start = ORIGIN(APP_FLASH);
__app_end = ORIGIN(APP_FLASH) + LENGTH(APP_FLASH);

__cfg_start = ORIGIN(CFG_FLASH);
__cfg_end = ORIGIN(CFG_FLASH) + LENGTH(CFG_FLASH);

MEMORY
{
  PRELDR_FLASH (rx): ORIGIN = 0x08000000, LENGTH = 8K
  LDR_FLASH (rx)   : ORIGIN = 0x08002000, LENGTH = 32K
  CFG_FLASH (rx)   : ORIGIN = 0x0800A000, LENGTH = 8K
  APP_FLASH (rx)   : ORIGIN = 0x0800C000, LENGTH = 80K

  RAM (xrw)        : ORIGIN = 0x20000000, LENGTH = 32K
}