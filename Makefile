CROSS   ?= arm-none-eabi-
CC       = $(CROSS)gcc
OBJCOPY  = $(CROSS)objcopy
OBJDUMP  = $(CROSS)objdump
PYTHON  ?= python3
SPEC_DIR ?= ../../spec
EMULATOR_DIR ?= ../../emulator

VERSION_MAJOR = 1
VERSION_MINOR = 0
FIRMWARE_NAME = jungle-sq64-bloom

# Built as the ARMv6-M (Thumb-1) subset: it executes unchanged on the
# Cortex-M7, and producing no IT blocks keeps interrupt delivery working on
# the native emulator's stock Unicorn build (see the emulator's engine notes
# about the 2.1.3 IT-state patch). Use ARCH=-mcpu=cortex-m7 for Thumb-2.
ARCH ?= -march=armv6-m
ARM_LIBS = -lgcc
LD_SELECT =

# A freestanding Clang toolchain is a reproducible fallback on hosts without
# GNU Arm Embedded. Clang 23 ships the matching bare-metal compiler-rt archive.
ifeq ($(shell command -v $(CC) 2>/dev/null),)
CC := clang
OBJCOPY := llvm-objcopy
OBJDUMP := llvm-objdump
ARCH := --target=arm-none-eabi -march=armv6-m
LD_SELECT := -fuse-ld=lld
CLANG_RT := $(shell $(CC) $(ARCH) -print-libgcc-file-name)
ARM_LIBS := $(if $(wildcard $(CLANG_RT)),$(CLANG_RT),)
endif
CFLAGS = $(ARCH) -mthumb -mfloat-abi=soft -O2 -g \
         -Wall -Wextra -ffreestanding -fno-builtin -nostdlib \
         -Wl,--build-id=none

SRCS = src/startup.c src/main.c src/panel.c src/display.c src/outputs.c \
       src/midi.c src/usb.c src/storage.c src/updater.c $(ENGINE_SRCS)
HDRS = $(wildcard src/*.h)

HOST_CC ?= cc
HOST_CFLAGS ?= -std=c11 -O2 -g -Wall -Wextra -Werror -pedantic
ENGINE_SRCS = firmware/core/prng.c firmware/core/scheduler.c firmware/music/quantizer.c \
              firmware/engines/euclidean.c firmware/engines/turing.c \
              firmware/engines/fractal.c firmware/engines/generative.c \
              firmware/storage/project_codec.c

all: build/$(FIRMWARE_NAME).vsb

build/$(FIRMWARE_NAME).elf: $(SRCS) $(HDRS) linker.ld tools/check_blocks.py
	@mkdir -p build
	$(CC) $(CFLAGS) $(LD_SELECT) -T linker.ld $(SRCS) $(ARM_LIBS) -o $@
	$(PYTHON) tools/check_blocks.py $@ --objdump $(OBJDUMP)

build/$(FIRMWARE_NAME).bin: build/$(FIRMWARE_NAME).elf
	$(OBJCOPY) -O binary $< $@

build/$(FIRMWARE_NAME).vsb: build/$(FIRMWARE_NAME).bin tools/make_vsb.py
	$(PYTHON) tools/make_vsb.py $< $@ --major $(VERSION_MAJOR) --minor $(VERSION_MINOR)

# Run the pinned conformance spec's seven image-profile checks.
check: build/$(FIRMWARE_NAME).vsb
	cd $(SPEC_DIR) && $(PYTHON) -m sq64_spec $(abspath $<) --profile image

# Headless smoke run in the native emulator (requires `pip install -e` there).
smoke: build/$(FIRMWARE_NAME).vsb
	cd $(EMULATOR_DIR) && $(PYTHON) -m sq64_emulator $(abspath $<) \
	  --headless --instructions 2000000 --report $(abspath build/smoke.json)

# Behavioral tests. The Python harness needs the emulator's dependencies
# (unicorn 2.1.3, capstone, pyelftools); the Node tests use the emulator's
# vendored WASM engine and need no installation.
test: build/$(FIRMWARE_NAME).vsb
	$(PYTHON) tools/headless_test.py --emulator $(EMULATOR_DIR) --image $<
	$(PYTHON) tests/emulator/test_generative.py --emulator $(EMULATOR_DIR) --image $<
	node tools/browser_update_test.mjs $(EMULATOR_DIR)/web $<
	node tools/browser_panel_test.mjs $(EMULATOR_DIR)/web $<
	node tests/emulator/browser_generative_test.mjs $(EMULATOR_DIR)/web $<

build/host_tests: tests/unit/test_main.c $(ENGINE_SRCS)
	@mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -Ifirmware $^ -o $@

host-test: build/host_tests
	./build/host_tests

build/bloom-diagnostic: tools/bloom_diagnostic.c $(ENGINE_SRCS)
	@mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -Ifirmware $^ -o $@

diagnostic: build/bloom-diagnostic
	./build/bloom-diagnostic --steps 128

clean:
	rm -rf build

.PHONY: all check smoke test host-test diagnostic clean
