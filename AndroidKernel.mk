KERNEL_PATH := $(LOCAL_PATH)/../../../kernel

ifeq ($(TARGET_PREBUILT_KERNEL),)

KERNEL_MODULES_OUT := $(TARGET_OUT)/lib/modules
TARGET_PREBUILT_INT_KERNEL := $(KERNEL_PATH)/arch/arm/boot/zImage
TARGET_PREBUILT_KERNEL := $(TARGET_PREBUILT_INT_KERNEL)

define mv-modules
ko=`find $(KERNEL_PATH) -type f -name *.ko`;\
for i in $$ko; do cp $$i $(KERNEL_MODULES_OUT)/; done;
endef


$(TARGET_PREBUILT_INT_KERNEL): $(KERNEL_MODULES_OUT)
	$(info PATH := ../prebuilts/gcc/$(HOST_PREBUILT_TAG)/arm/arm-eabi-4.6/bin:$(PATH))
	$(info $(shell cd $(KERNEL_PATH); ./build.sh -p sun6i_fiber))
	$(mv-modules)

endif #/TARGET_PREBUILT_KERNEL

$(KERNEL_MODULES_OUT):
	mkdir -p $(KERNEL_MODULES_OUT)

file := $(INSTALLED_KERNEL_TARGET)
PREBUILT_ALL += $(file)
$(file) : $(TARGET_PREBUILT_KERNEL) | $(ACP)
	$(transform-prebuilt-to-target)

