#
# SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
#
# SPDX-License-Identifier: Apache-2.0
#

SHELL := /bin/bash

SRC_DIR = src
BUILD_DIR = build
LIB_DIR = lib
LIB_FILE = zgdk.lib

C_SRCS = $(shell find $(SRC_DIR) -name '*.c')
ASM_SRCS = $(shell find $(SRC_DIR) -name '*.asm')
RELS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.rel,$(C_SRCS)) \
       $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.rel,$(ASM_SRCS))

ifndef ZOS_PATH
$(error "Please define ZOS_PATH environment variable. It must point to Zeal 8-bit OS source code path.")
endif

ifndef ZVB_SDK_PATH
$(error "Please define ZVB_SDK_PATH environment variable. It must point to Zeal Video Board SDK path.")
endif

ZVB_INCLUDE=$(ZVB_SDK_PATH)/include/
ZOS_INCLUDE=$(ZOS_PATH)/kernel_headers/sdcc/include/
ZVB_LIB_PATH=$(ZVB_SDK_PATH)/lib/
ZOS_LIB_PATH=$(ZOS_PATH)/kernel_headers/sdcc/lib
ZGDK_INCLUDE=include/


CC = sdcc
AR = sdar
AS = sdasz80
# Specify Z80 as the target, compile without linking, and place all the code in TEXT section
# (_CODE must be replace).
CFLAGS := -mz80 -c --codeseg TEXT -I$(ZOS_INCLUDE) -I$(ZVB_INCLUDE) -I$(ZGDK_INCLUDE) --opt-code-speed

.PHONY: all clean

all: $(BUILD_DIR) $(RELS) $(LIB_DIR)/$(LIB_FILE)
	@bash -c 'echo -e "\x1b[32;1mSuccess, library generated\x1b[0m"'

$(BUILD_DIR)/%.rel: $(SRC_DIR)/%.c | $(BUILD_DIR) $(BUILD_DIR)/$(dir $<)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.rel: $(SRC_DIR)/%.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) -o $@ $<

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(LIB_DIR)/$(LIB_FILE): $(RELS) Makefile | $(LIB_DIR)
	echo "Archiving library..."
	@rm -f $@.tmp
	$(AR) -rc $@.tmp $(RELS)
	mv $@.tmp $@

clean:
	rm -rf $(BUILD_DIR)

cleanall: clean
	rm -rf lib/*
