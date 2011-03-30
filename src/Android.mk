LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	shjpeg_common.c \
	shjpeg_config.c \
	shjpeg_decode.c \
	shjpeg_encode.c \
	shjpeg_softhelper.c \
	shjpeg_jpu.c  \
	shjpeg_veu.c


LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include \
			external/jpeg

LOCAL_SHARED_LIBRARIES := libcutils \
			  libjpeg

LOCAL_MODULE:= libshjpeg
LOCAL_PRELINK_MODULE:= false
LOCAL_MODULE_TAGS:= optional
include $(BUILD_SHARED_LIBRARY)
