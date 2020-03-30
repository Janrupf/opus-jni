#pragma once

#include <jni.h>
#include <stdint.h>

#define OPUS_JNI_UNUSED(x) ((void) x)

typedef struct OpusJNIRuntimeData_ {
    jclass runtime_exception_class;
    jclass illegal_argument_exception_class;
    jclass buffer_overflow_exception_class;
    jclass out_of_memory_error_class;
} OpusJNIRuntimeData;

typedef struct OpenedJavaByteArray_ {
    jboolean read_only;
    jboolean is_copy;
    jbyteArray array;
    jbyte *data;
} OpenedJavaByteArray;

OpenedJavaByteArray open_java_byte_array(JNIEnv *env, jbyteArray array, jboolean read_only);
OpenedJavaByteArray create_and_open_java_byte_array(JNIEnv *env, size_t bytes);
jbyteArray close_java_byte_array(JNIEnv *env, OpenedJavaByteArray *opened);

void throw_illegal_argument_exception(JNIEnv *env, const char *message);
void throw_opus_error(JNIEnv *env, int32_t opus_error, const char *message);
