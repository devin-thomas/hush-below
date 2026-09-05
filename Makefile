DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM
CC := $(DEVKITARM)/bin/arm-none-eabi-gcc
OBJCOPY := $(DEVKITARM)/bin/arm-none-eabi-objcopy
GBAFIX := $(DEVKITPRO)/tools/bin/gbafix
TARGET := hush-below
BUILD := build
CFLAGS := -mthumb -mthumb-interwork -O2 -Wall -Wextra -Werror -std=gnu11 -ffunction-sections -fdata-sections -Iinclude -Iassets/generated -I$(DEVKITPRO)/libgba/include
LDFLAGS := -mthumb -mthumb-interwork -specs=gba.specs -Wl,--gc-sections -Wl,--undefined=hb_debug_offsets -L$(DEVKITPRO)/libgba/lib
GAME := source/main.c source/sim.c source/video.c source/audio.c
ART := $(wildcard assets/generated/*.c)
SOURCES := $(GAME) $(ART)
OBJECTS := $(patsubst %.c,$(BUILD)/%.o,$(SOURCES))
.PHONY: all test verify size site site-test clean
all: $(TARGET).gba
$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -O binary "$<" "$@"
	$(GBAFIX) "$@" "-tHUSH BELOW!" -cHSHB -m00
$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o "$@" $(OBJECTS) -lgba
$(BUILD)/%.o: %.c
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -MMD -MP -c "$<" -o "$@"
test:
	mkdir -p $(BUILD)
	cc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude source/sim.c tests/test_sim.c -o $(BUILD)/test-sim
	$(BUILD)/test-sim
	python3 -m unittest discover -s tests -p 'test_*.py' -v
verify: $(TARGET).gba
	python3 tools/verify_rom.py $(TARGET).gba $(TARGET).elf
size: $(TARGET).elf
	$(DEVKITARM)/bin/arm-none-eabi-size $(TARGET).elf
site: verify
	mkdir -p site/downloads site/assets/screens
	cp $(TARGET).gba site/downloads/hush-below.gba
	chmod 644 site/downloads/hush-below.gba
site-test: site
	python3 -m unittest tests.test_site -v
clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).gba
-include $(OBJECTS:.o=.d)
