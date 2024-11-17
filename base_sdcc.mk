
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

all:: $(GIF_SRCS) $(ZTS_SRCS) $(ZTM_SRCS)
	@echo "Enable GFX", $(ENABLE_GFX)
	@echo "Enable Sound", $(ENABLE_SOUND)
	@echo "Emulator", $(EMULATOR)
	@echo "Frame Lock", $(FRAMELOCK)
	@echo "Debug", $(DEBUG)

%.gif: %.aseprite
	@if [ -f $(ASEPRITE_PATH) ]; then $(ASEPRITE_PATH) -b --sheet $@ $<; fi

%.zts: %.gif
	$(ZVB_SDK_PATH)/tools/zeal2gif/gif2zeal.py -i $< -t $@ -p $(patsubst %.zts,%.ztp,$@) -c

%.ztm: %.tmx
	$(ZVB_SDK_PATH)/tools/tiled2zeal/tiled2zeal.py -i $< -m $@


# include $(ZOS_PATH)/kernel_headers/sdcc/base_sdcc.mk
include $(ZVB_SDK_PATH)/sdcc/base_sdcc.mk