EXE_NAME=cnc_pend
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

EXT_LIBS +=c nosys

PPDEFS += USE_HAL_DRIVER STM32H503xx FW_TYPE=FW_APP DEV=\"CNC_PEND\"
PPDEFS += DFU_READS_CFG_SECTION CFG_SYSTEM_SAVES_NON_NATIVE_DATA

INCDIR += app
INCDIR += app/display
INCDIR += common
INCDIR += common/CMSIS
INCDIR += common/config_system
INCDIR += common/crc
INCDIR += common/fw_header
INCDIR += common/md5
INCDIR += common/usb/Core
INCDIR += app/base/Core/Inc
INCDIR += app/base/Drivers/STM32H5xx_HAL_Driver/Inc

SOURCES += app/base/Core/Src/main.c
SOURCES += app/base/Core/Src/stm32h5xx_hal_msp.c
SOURCES += app/base/Core/Src/stm32h5xx_it.c
SOURCES += $(wildcard app/base/Drivers/STM32H5xx_HAL_Driver/Src/*.c)
SOURCES += $(wildcard app/*.c)
SOURCES += $(wildcard app/display/*.c)
SOURCES += $(wildcard common/*.c)
SOURCES += $(wildcard common/config_system/*.c)
SOURCES += $(wildcard common/crc/*.c)
SOURCES += $(wildcard common/fw_header/*.c)
SOURCES += $(wildcard common/md5/*.c)
SOURCES += $(wildcard common/usb/Core/*.c)
SOURCES += common/startup_stm32h503xx.s
LDSCRIPT += ldscript/app.ld

BINARY_SIGNED = $(BUILDDIR)/$(EXE_NAME)_app_signed.bin
BINARY_MERGED = $(BUILDDIR)/$(EXE_NAME)_full.bin
SIGN = $(BUILDDIR)/sign
ARTEFACTS += $(BINARY_SIGNED)

PRELDR_SIGNED = preldr/build/$(EXE_NAME)_preldr_signed.bin
LDR_SIGNED = ldr/build/$(EXE_NAME)_ldr_signed.bin 
EXT_MAKE_TARGETS = $(PRELDR_SIGNED) $(LDR_SIGNED)

include common/core.mk

$(SIGN): sign/sign.c
	@gcc $< -o $@

$(BINARY_SIGNED): $(BINARY) $(SIGN)
	@$(SIGN) $(BINARY) $@ 81920 \
		prod=$(EXE_NAME) \
		prod_name=$(EXE_NAME)_app \
		ver_maj=$(VER_MAJ) \
		ver_min=$(VER_MIN) \
		ver_pat=$(VER_PATCH) \
		build_ts=$$(date -u +'%Y%m%d_%H%M%S')

$(BINARY_MERGED): $(EXT_MAKE_TARGETS) $(BINARY_SIGNED)
	@echo " [Merging binaries] ..." {$@}
	@cp -f $(PRELDR_SIGNED) $@
	@dd if=$(LDR_SIGNED) of=$@ bs=1 oflag=append seek=8192 status=none
	@dd if=$(BINARY_SIGNED) of=$@ bs=1 oflag=append seek=49152 status=none

.PHONY: composite
composite: $(BINARY_MERGED)

.PHONY: clean
clean: clean_ext_targets

.PHONY: $(EXT_MAKE_TARGETS)
$(EXT_MAKE_TARGETS):
	@$(MAKE) -C $(subst build/,,$(dir $@)) --no-print-directory

.PHONY: clean_ext_targets
clean_ext_targets:
	$(foreach var,$(EXT_MAKE_TARGETS),$(MAKE) -C $(subst build/,,$(dir $(var))) clean;)

#####################
### FLASH & DEBUG ###
#####################

flash: $(BINARY_SIGNED)
	@openocd_h5 -d0 -f target/stm32h5.cfg -c "program $< 0x0800C000 verify reset exit" 

flash_all: $(BINARY_MERGED)
	@openocd_h5 -d0 -f target/stm32h5.cfg -c "program $< 0x08000000 verify reset exit" 

program: $(BINARY_SIGNED)
	@usb_dfu_flasher w a $< CNC_PEND

ds:
	@openocd_h5 -d0 -f target/stm32h5.cfg
	
debug:
	@set _NO_DEBUG_HEAP=1
	@echo "file $(EXECUTABLE)" > .gdbinit
	@echo "set auto-load safe-path /" >> .gdbinit
	@echo "set confirm off" >> .gdbinit
	@echo "target extended-remote :3333" >> .gdbinit
	@arm-none-eabi-gdb -q -x .gdbinit