ASSETS_DIR ?= assets
ASEPRITE_PATH ?= ~/.steam/debian-installation/steamapps/common/Aseprite/aseprite
ASEPRITE_SRCS=$(wildcard $(ASSETS_DIR)/*.aseprite)
GIF_SRCS ?= $(ASEPRITE_SRCS:.aseprite=.gif)

.PHONY: all

all: $(GIF_SRCS)
	@echo > /dev/null


%.gif: %.aseprite
	@if [ -f $(ASEPRITE_PATH) ]; then $(ASEPRITE_PATH) -b --sheet $@ $<; fi
