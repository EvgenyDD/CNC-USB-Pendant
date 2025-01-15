EXE_NAME=cnc_pend_preldr
PRODUCT =cnc_pend
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
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

PPDEFS += STM32H503xx FW_TYPE=FW_PRELDR

INCDIR += .
INCDIR += ../common
INCDIR += ../common/CMSIS
INCDIR += ../common/crc
INCDIR += ../common/fw_header
SOURCES += preldr.c
SOURCES += ../common/platform.c
SOURCES += ../common/ret_mem.c
SOURCES += ../common/system_stm32h5xx.c
SOURCES += $(wildcard ../common/crc/*.c)
SOURCES += $(wildcard ../common/fw_header/*.c)
SOURCES += ../common/startup_stm32h503xx.s
LDSCRIPT += ../ldscript/preldr.ld

BINARY_SIGNED = $(BUILDDIR)/$(EXE_NAME)_signed.bin
SIGN = $(BUILDDIR)/sign
ARTEFACTS += $(BINARY_SIGNED)

include ../common/core.mk

$(SIGN): ../sign/sign.c
	@gcc $< -o $@

$(BINARY_SIGNED): $(BINARY) $(SIGN)
	@$(SIGN) $(BINARY) $@ 8192 \
		prod=$(PRODUCT) \
		prod_name=$(EXE_NAME) \
		ver_maj=$(VER_MAJ) \
		ver_min=$(VER_MIN) \
		ver_pat=$(VER_PATCH) \
		build_ts=$$(date -u +'%Y%m%d_%H%M%S')

#####################
### FLASH & DEBUG ###
#####################

flash: $(BINARY_SIGNED)
	@openocd_h5 -f ../target/stm32h5.cfg -c "program $< 0x08000000 verify reset exit" 

ds:
	@openocd_h5 -d0 -f ../target/stm32h5.cfg

debug:
	@set _NO_DEBUG_HEAP=1
	@echo "file $(EXECUTABLE)" > .gdbinit
	@echo "set auto-load safe-path /" >> .gdbinit
	@echo "set confirm off" >> .gdbinit
	@echo "target extended-remote :3333" >> .gdbinit
	@arm-none-eabi-gdb -q -x .gdbinit