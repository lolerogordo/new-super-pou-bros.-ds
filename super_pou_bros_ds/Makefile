# Simple Nintendo DS homebrew build for Super Pou Bros (ARM9 only)

ifeq ($(strip $(DEVKITPRO)),)
$(error Please set DEVKITPRO in your environment. e.g. export DEVKITPRO=/opt/devkitpro)
endif

TARGET := super_pou_bros_ds
BUILD := build
SOURCES := source
INCLUDES := include

ARCH := -mthumb -mthumb-interwork
CFLAGS := -g -Wall -O2 -ffast-math -mtune=arm946e-s $(ARCH)
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS := -g $(ARCH)
LDFLAGS := -specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS := -lnds9
LIBDIRS :=

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)
export CFLAGS += $(INCLUDE)
export CXXFLAGS += $(INCLUDE)
export ASFLAGS += $(INCLUDE)

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export LD := $(CC)

export OFILES := $(addsuffix .o,$(BINFILES)) \
  $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
  -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: all clean

all:
	$(MAKE) -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).ds.gba

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).nds: $(OUTPUT).elf

$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

endif

include $(DEVKITPRO)/ds_rules
