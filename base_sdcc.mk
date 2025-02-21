
# This file is means to be included by programs based on SDCC.
# This will ease writing a Makefile for a new project that is meant to use ZGDK.
# This file can included by adding this line to any Makefile:
#	include $(ZGDK_PATH)/base_sdcc.mk

ifndef ZOS_PATH
    $(error "Failure: ZOS_PATH variable not found. It must point to Zeal 8-bit OS path.")
endif
ifndef ZVB_SDK_PATH
    $(error "Failure: ZVB_SDK_PATH variable not found. It must point to Zeal Video Board SDK path.")
endif

STAT_BYTES = stat
ifeq ($(detected_OS),Darwin)
	STAT_BYTES += -f %z
# TODO: Support Windows?
else
	STAT_BYTES += -c %s
endif

ASSETS_DIR ?= assets
ASEPRITE_PATH ?= ~/.steam/debian-installation/steamapps/common/Aseprite/aseprite
ASEPRITE_SRCS=$(wildcard $(ASSETS_DIR)/*.aseprite)
TILEMAP_SRCS=$(wildcard $(ASSETS_DIR)/*.tmx)
GIF_SRCS ?= $(ASEPRITE_SRCS:.aseprite=.gif)
ZTS_SRCS ?= $(GIF_SRCS:.gif=.zts)
ZTM_SRCS ?= $(TILEMAP_SRCS:.tmx=.ztm)

# Specify Z80 as the target, compile without linking, and place all the code in TEXT section
ZOS_CFLAGS += -I$(ZGDK_PATH)/include $(EXTRA_CFLAGS)
ZOS_LDFLAGS += -k $(ZGDK_PATH)/lib -l zgdk $(EXTRA_LDFLAGS)

ENABLE_GFX ?= 1
ENABLE_SOUND ?= 1
ENABLE_CRC32 ?= 0
ENABLE_ZAR ?= 0

ifeq ($(ENABLE_ZAR), 1)
ZOS_CFLAGS += -I$(ZAR_PATH)/include/
ZOS_LDFLAGS += -k $(ZAR_PATH)/lib/ -l zar
endif

EMULATOR ?= 0
FRAMELOCK ?= 0
DEBUG ?= 0
ifeq ($(EMULATOR), 1)
ZOS_CFLAGS += -DEMULATOR=1
endif
ifeq ($(FRAMELOCK), 1)
ZOS_CFLAGS += -DFRAMELOCK=1
endif
ifeq ($(DEBUG), 1)
ZOS_CFLAGS += -DDEBUG=1
endif

ASSETS_OUTPUT ?= 0
ifneq ($(ASSETS_OUTPUT), 0)
# ASSETS_OUTPUT_DEST=$(ASSETS_OUTPUT_DEST)
GIF2ZEAL_ARGS += -o $(ASSETS_OUTPUT)
TILED2ZEAL_ARGS += -o $(ASSETS_OUTPUT)
endif

GFX_ALLOWED_BITS := 1 4 8
GFX_BITS ?= 0
ifneq ($(filter $(GFX_BITS), $(GFX_ALLOWED_BITS)),)
GIF2ZEAL_ARGS += -b $(GFX_BITS)
endif

GFX_COMPRESSED ?= 0
ifeq ($(GFX_COMPRESSED), 1)
GIF2ZEAL_ARGS += -z
endif

GFX_STRIP ?= 0
ifneq ($(GFX_STRIP), 0)
GIF2ZEAL_ARGS += -s $(GFX_STRIP)
endif

GFX_COLORS ?= 0
ifneq ($(GFX_COLORS), 0)
GIF2ZEAL_ARGS += -c $(GFX_COLORS)
endif

TILED_SIZE ?= 0
ifneq ($(TILED_SIZE), 0)
TILED2ZEAL_ARGS += --size $(TILED_SIZE)
endif

TILED_LAYER ?= 0
ifneq ($(TILED_LAYER), 0)
TILED2ZEAL_ARGS += --layer $(TILED_LAYER)
endif

TILED_OUTPUT ?= 0
ifneq ($(TILED_OUTPUT), 0)
TILED2ZEAL_ARGS += -o $(TILED_OUTPUT)
endif

all:: $(GIF_SRCS) $(ZTS_SRCS) $(ZTM_SRCS)
	$(if $(ZGDK_DEBUG),@echo "Enable GFX: $(ENABLE_GFX)")
	$(if $(ZGDK_DEBUG),@echo "Enable Sound: $(ENABLE_SOUND)")
	$(if $(ZGDK_DEBUG),@echo "Enable CRC32: $(ENABLE_CRC32)")
	$(if $(ZGDK_DEBUG),@echo "Enable ZAR: $(ENABLE_ZAR)")
	$(if $(ZGDK_DEBUG),@echo "Emulator: $(EMULATOR)")
	$(if $(ZGDK_DEBUG),@echo "Frame Lock: $(FRAMELOCK)")
	$(if $(ZGDK_DEBUG),@echo "Debug: $(DEBUG)")
	$(if $(ZGDK_DEBUG),@echo "Gfx Compressed: $(GFX_COMPRESSED)")
	$(if $(ZGDK_DEBUG),@echo "Gfx Colors: $(GFX_COLORS)")
	$(if $(ZGDK_DEBUG),@echo "Gfx Bits: $(GFX_BITS)")

%.gif: %.aseprite
	@if [ -f $(ASEPRITE_PATH) ]; then $(ASEPRITE_PATH) -b --sheet $@ $<; fi

%.zts: %.gif
	$(ZVB_SDK_PATH)/tools/zeal2gif/gif2zeal.py -i $< $(GIF2ZEAL_ARGS)

%.ztm: %.tmx
	$(ZVB_SDK_PATH)/tools/tiled2zeal/tiled2zeal.py -i $< $(TILED2ZEAL_ARGS)


# include $(ZOS_PATH)/kernel_headers/sdcc/base_sdcc.mk
include $(ZVB_SDK_PATH)/sdcc/base_sdcc.mk

all::
	@echo "Binary Size" $$($(STAT_BYTES) $(OUTPUT_DIR)/$(BIN)) $(BIN)
