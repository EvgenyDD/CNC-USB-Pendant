EXE_NAME=pendant
MAKE_BINARY=yes

TCHAIN = arm-none-eabi-
MCPU += -mcpu=cortex-m33 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
CDIALECT = gnu99
OPT_LVL = 2
DBG_OPTS = -gdwarf-2 -ggdb -g

CFLAGS   += -fvisibility=hidden -funsafe-math-optimizations -fdata-sections -ffunction-sections -fno-move-loop-invariants
CFLAGS   += -fmessage-length=0 -fno-exceptions -fno-common -fno-builtin -ffreestanding
CFLAGS   += -fsingle-precision-constant
CFLAGS   += $(C_FULL_FLAGS)
CFLAGS   += -Werror

CXXFLAGS += -fvisibility=hidden -funsafe-math-optimizations -fdata-sections -ffunction-sections -fno-move-loop-invariants
CXXFLAGS += -fmessage-length=0 -fno-exceptions -fno-common -fno-builtin -ffreestanding
CXXFLAGS += -fvisibility-inlines-hidden -fuse-cxa-atexit -felide-constructors -fno-rtti
CXXFLAGS += -fsingle-precision-constant
CXXFLAGS += $(CXX_FULL_FLAGS)
CXXFLAGS += -Werror

LDFLAGS  += -specs=nano.specs
LDFLAGS  += -Wl,--gc-sections
LDFLAGS  += -Wl,--print-memory-usage

EXT_LIBS +=c m nosys

PPDEFS += USE_HAL_DRIVER STM32H503xx

INCDIR += .
INCDIR += base/Core/Inc
INCDIR += base/Drivers/STM32H5xx_HAL_Driver/Inc
INCDIR += base/Drivers/STM32H5xx_HAL_Driver/Inc/Legacy
INCDIR += base/Drivers/CMSIS/Device/ST/STM32H5xx/Include
INCDIR += base/Drivers/CMSIS/Include
INCDIR += usb/Core
INCDIR += display
INCDIR += md5

SOURCES += $(call rwildcard, base/Core/Src base/Drivers/CMSIS base/Drivers/STM32H5xx_HAL_Driver usb/Core, *.c *.S *.s)
SOURCES += pendant.c
SOURCES += platform.c
SOURCES += usbd_pend_hid.c
SOURCES += usbd_core_dfu.c
SOURCES += usbd_desc.c
SOURCES += $(wildcard display/*.c)
SOURCES += $(wildcard md5/*.c)
SOURCES += base/startup_stm32h503xx.s

LDSCRIPT += base/STM32H503xx_FLASH.ld

include core.mk

#####################
### FLASH & DEBUG ###
#####################

flash: $(BINARY)
	@openocd_h5 -f target/stm32h5.cfg -c "program $< 0x08000000 verify reset exit" 

ds:
	@openocd_h5 -d0 -f target/stm32h5.cfg

debug:
	@set _NO_DEBUG_HEAP=1
	@echo "file $(EXECUTABLE)" > .gdbinit
	@echo "set auto-load safe-path /" >> .gdbinit
	@echo "set confirm off" >> .gdbinit
	@echo "target extended-remote :3333" >> .gdbinit
	@arm-none-eabi-gdb -q -x .gdbinit