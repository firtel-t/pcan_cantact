# ------------------------------------------------
# Generic Makefile for PCAN USB firmware
# Supports STM32F042 (CANtact/CANable) and STM32G431 (MKS CANable V2.0)
# ------------------------------------------------

######################################
# target
######################################
TARGET = pcan_$(BOARD)_hw
TARGET_VARIANT = $(shell echo $(BOARD) | tr '[:lower:]' '[:upper:]')

#######################################
# paths
#######################################
BUILD_DIR = build-$(BOARD)

######################################
# source (selected per target)
######################################

# Common application sources (shared between F042 and G431)
APP_SOURCES = \
Src/main.c \
Src/usbd_conf.c \
Src/usbd_desc.c \
Src/pcan_usb.c \
Src/pcan_can.c \
Src/pcan_led.c \
Src/pcan_protocol.c \
Src/pcan_timestamp.c

# USB Middleware (shared)
USB_MW_SOURCES = \
Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c

# --- STM32G431 specific ---
ifeq ($(BOARD),mks_canable2)

C_SOURCES = $(APP_SOURCES) \
Src/system_stm32g4xx.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_fdcan.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_usb.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.c \
$(USB_MW_SOURCES)

ASM_SOURCES = startup_stm32g431xx.s

CPU = -mcpu=cortex-m4
FPU = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=hard

C_DEFS = \
-DUSE_HAL_DRIVER \
-DSTM32G431xx \
-DG431_BOARD \
-DNDEBUG

C_INCLUDES = \
-ISrc \
-IDrivers/STM32G4xx_HAL_Driver/Inc \
-IDrivers/STM32G4xx_HAL_Driver/Inc/Legacy \
-IMiddlewares/ST/STM32_USB_Device_Library/Core/Inc \
-IDrivers/CMSIS/Device/ST/STM32G4xx/Include \
-IDrivers/CMSIS/Include

LDSCRIPT = STM32G431CBTx_FLASH.ld

# --- STM32F042 specific (all other boards) ---
else

C_SOURCES = $(APP_SOURCES) \
Src/system_stm32f0xx.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_usb.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pcd.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pcd_ex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_can.c \
$(USB_MW_SOURCES)

ASM_SOURCES = startup_stm32f042x6.s

CPU = -mcpu=cortex-m0
FPU =
FLOAT-ABI =

C_DEFS = \
-DUSE_HAL_DRIVER \
-DSTM32F042x6 \
-DNDEBUG \
$(BOARD_DEFS)

C_INCLUDES = \
-ISrc \
-IDrivers/STM32F0xx_HAL_Driver/Inc \
-IMiddlewares/ST/STM32_USB_Device_Library/Core/Inc \
-IDrivers/CMSIS/Device/ST/STM32F0xx/Include \
-IDrivers/CMSIS/Include

LDSCRIPT = STM32F042C6Tx_FLASH.ld

endif

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

AS_DEFS =
AS_INCLUDES =

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fno-common -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Wpedantic -Wextra -fno-common -fdata-sections -ffunction-sections -std=c99 \
$(BOARD_FLAGS) \
-D$(TARGET_VARIANT)

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

.PHONY : all clean

# default action: build all F042 targets
all: cantact_16 cantact_8 entree canable

# --- F042 targets ---
cantact_16:
	$(MAKE) BOARD=cantact_16 DEBUG=0 OPT=-Os BOARD_FLAGS='-DHSE_VALUE=16000000' elf hex bin

cantact_8:
	$(MAKE) BOARD=cantact_8 DEBUG=0 OPT=-Os BOARD_FLAGS='-DHSE_VALUE=8000000' elf hex bin

entree:
	$(MAKE) BOARD=entree DEBUG=0 OPT=-Os BOARD_FLAGS='-DHSE_VALUE=0' elf hex bin

canable:
	$(MAKE) BOARD=canable DEBUG=0 OPT=-Os BOARD_FLAGS='-DHSE_VALUE=0' elf hex bin

ollie:
	$(MAKE) BOARD=ollie DEBUG=0 OPT=-Os BOARD_FLAGS='-DHSE_VALUE=16000000' elf hex bin

# --- G431 target ---
mks_canable2:
	$(MAKE) BOARD=mks_canable2 DEBUG=0 OPT=-Os BOARD_FLAGS='' BOARD_DEFS='' elf hex bin

#######################################
# build the application
#######################################
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

ELF_TARGET = $(BUILD_DIR)/$(TARGET).elf
BIN_TARGET = $(BUILD_DIR)/$(TARGET).bin
HEX_TARGET = $(BUILD_DIR)/$(TARGET).hex

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

bin: $(BIN_TARGET)
elf: $(ELF_TARGET)
hex: $(HEX_TARGET)

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)*

clean_obj:
	-rm -f $(BUILD_DIR)*/*.o $(BUILD_DIR)*/*.d $(BUILD_DIR)*/*.lst

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
