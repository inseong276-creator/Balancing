PROJECT := nucleo-f103rb
BUILD_DIR := build
BIN_DIR := bin

CC := arm-none-eabi-gcc
AS := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE := arm-none-eabi-size

CFLAGS := -mcpu=cortex-m3 -mthumb -Os -ffunction-sections -fdata-sections -g3
CFLAGS += -std=c11
CFLAGS += -Wall -Wextra -Werror -pedantic
CFLAGS += -Iinclude

ASFLAGS := -mcpu=cortex-m3 -mthumb -x assembler-with-cpp

LDFLAGS := -T linker/stm32f103xb.ld -Wl,--gc-sections -Wl,-Map,$(BUILD_DIR)/$(PROJECT).map
LDLIBS := -lm

SRCS := \
    src/main.c \
    src/system_stm32f103xb.c \
	src/motor.c \
	src/sensor.c \
	src/serial.c \
    startup/startup_stm32f103xb.s \

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

ELF := $(BIN_DIR)/$(PROJECT).elf
HEX := $(BIN_DIR)/$(PROJECT).hex
BIN := $(BIN_DIR)/$(PROJECT).bin


all: $(ELF) $(HEX) $(BIN)

$(BUILD_DIR):
	if not exist $@ mkdir $@

$(BIN_DIR):
	if not exist $@ mkdir $@

$(BUILD_DIR)/%.c.o: %.c | $(BUILD_DIR)
	if not exist $(dir $@) mkdir "$(dir $@)"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.s.o: %.s | $(BUILD_DIR)
	if not exist $(dir $@) mkdir "$(dir $@)"
	$(AS) $(ASFLAGS) -c $< -o $@

$(ELF): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@ $(LDLIBS)
	$(SIZE) $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)

flash: $(ELF)
	openocd -f board/st_nucleo_f103rb.cfg -c "program $< verify reset exit"

.PHONY: all clean flash
