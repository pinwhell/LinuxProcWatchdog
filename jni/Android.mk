# lets define my current working dir
LOCAL_PATH := $(call my-dir)

# lets clear all main vars
include $(CLEAR_VARS)

# lets define the name of our bin/lib
LOCAL_MODULE := memw

# lets define the source code of our project
LOCAL_SRC_FILES := main.cpp \
crc.cpp \
LinuxProcess/LinuxProcess.cpp \
LinuxProcess/ElfUtils.cpp \

#define flags
LOCAL_CFLAGS := -fexceptions

# lets tell Android.mk what type of thing to build
include $(BUILD_EXECUTABLE)