#include "opus_jni_util.h"

#include <opus_defines.h>

OpenedJavaByteArray open_java_byte_array(JNIEnv *env, jbyteArray array, jboolean read_only) {
    OpenedJavaByteArray opened;
    opened.read_only = read_only;
    opened.array = array;
    opened.data = (*env)->GetByteArrayElements(env, array, &opened.is_copy);
    return opened;
}

OpenedJavaByteArray create_and_open_java_byte_array(JNIEnv *env, size_t bytes) {
    jbyteArray array = (*env)->NewByteArray(env, bytes);
    return open_java_byte_array(env, array, JNI_FALSE);
}

jbyteArray close_java_byte_array(JNIEnv *env, OpenedJavaByteArray *opened) {
    jbyteArray array = opened->array;

    (*env)->ReleaseByteArrayElements(env, opened->array, opened->data, opened->read_only ? JNI_ABORT : 0);
    opened->array = NULL;
    opened->data = NULL;
    opened->is_copy = JNI_FALSE;

    return array;
}

static jclass find_exception_class(JNIEnv *env, const char *class_name) {
    jclass class = (*env)->FindClass(env, class_name);
    if(class) {
        return class;
    } else {
        return (*env)->FindClass(env, "java/lang/RuntimeError");
    }
}

void throw_illegal_argument_exception(JNIEnv *env, const char *message) {
    (*env)->ThrowNew(env, find_exception_class(env, "java/lang/IllegalArgumentException"), message);
}

void throw_opus_error(JNIEnv *env, int32_t opus_error, const char *message) {
    switch(opus_error) {
        case OPUS_OK: {
            throw_illegal_argument_exception(env, "BUG: throw_opus_error called with OPUS_OK");
            return;
        }

        case OPUS_INVALID_PACKET:
        case OPUS_BAD_ARG: {
            throw_illegal_argument_exception(env, message ? message : "OPUS_BAD_ARG");
            return;
        }

        case OPUS_BUFFER_TOO_SMALL: {
            // BufferOverflowException does not take a string
            (*env)->ThrowNew(env, find_exception_class(env, "java/nio/BufferOverflowException"), NULL);
            return;
        }

        case OPUS_UNIMPLEMENTED:
        case OPUS_INVALID_STATE:
        case OPUS_INTERNAL_ERROR: {
            (*env)->ThrowNew(env, find_exception_class(env, "java/lang/RuntimeException"), message);
            return;
        }

        case OPUS_ALLOC_FAIL: {
            (*env)->ThrowNew(env, find_exception_class(env, "java/lang/OutOfMemoryError"), message);
            return;
        }

        default: {
            (*env)->ThrowNew(env, find_exception_class(env, "java/lang/RuntimeException"), message);
            return;
        }
    }
}