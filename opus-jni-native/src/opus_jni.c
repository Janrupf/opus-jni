#include <opus.h>
#include <stdlib.h>

#include "net_labymod_opus_OpusCodec.h"
#include "opus_jni_util.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    OPUS_JNI_UNUSED(vm);
    OPUS_JNI_UNUSED(reserved);
    return JNI_VERSION_1_1;
}

JNIEXPORT jbyteArray JNICALL Java_net_labymod_opus_OpusCodec_createEncoder(
    JNIEnv *env, jclass caller, jint sample_rate, jint channels, jint bit_rate) {
    OPUS_JNI_UNUSED(caller);

    size_t encoder_size = opus_encoder_get_size(channels);
    if(!encoder_size) {
        throw_illegal_argument_exception(env, "Channels must be 1 or 2");
        return NULL;
    }

    int opus_error;

    OpenedJavaByteArray encoder_memory = create_and_open_java_byte_array(env, encoder_size);
    if((opus_error = opus_encoder_init((OpusEncoder *) encoder_memory.data,
                                       sample_rate,
                                       channels,
                                       OPUS_APPLICATION_AUDIO)) != OPUS_OK) {
        if(opus_error == OPUS_BAD_ARG) {
            throw_illegal_argument_exception(
                env, "Invalid sample rate, can only be one of 48000, 24000, 16000, 12000 or 8000");
        } else {
            throw_opus_error(env, opus_error, opus_strerror(opus_error));
        }
        return NULL;
    }

    opus_error = opus_encoder_ctl((OpusEncoder *) encoder_memory.data, OPUS_SET_BITRATE(bit_rate));
    if(opus_error != OPUS_OK) {
        throw_opus_error(env, opus_error, opus_strerror(opus_error));
        return NULL;
    }

    return close_java_byte_array(env, &encoder_memory);
}

JNIEXPORT jbyteArray JNICALL Java_net_labymod_opus_OpusCodec_createDecoder(JNIEnv *env,
                                                                           jclass caller,
                                                                           jint sample_rate,
                                                                           jint channels) {
    OPUS_JNI_UNUSED(caller);

    size_t decoder_size = opus_decoder_get_size(channels);
    if(!decoder_size) {
        throw_illegal_argument_exception(env, "Channels must be 1 or 2");
        return NULL;
    }

    int opus_error;

    OpenedJavaByteArray decoder_memory = create_and_open_java_byte_array(env, decoder_size);
    if((opus_error = opus_decoder_init((OpusDecoder *) decoder_memory.data, sample_rate, channels)) != OPUS_OK) {
        if(opus_error == OPUS_BAD_ARG) {
            throw_illegal_argument_exception(
                env, "Invalid sample rate, can only be one of 48000, 24000, 16000, 12000 or 8000");
        } else {
            throw_opus_error(env, opus_error, opus_strerror(opus_error));
        }
        return NULL;
    }

    return close_java_byte_array(env, &decoder_memory);
}

JNIEXPORT jbyteArray JNICALL Java_net_labymod_opus_OpusCodec_encodeFrame0(JNIEnv *env,
                                                                          jclass caller,
                                                                          jbyteArray encoder,
                                                                          jbyteArray data,
                                                                          jint offset,
                                                                          jint length,
                                                                          jint max_packet_size,
                                                                          jint channels,
                                                                          jint frame_size) {
    OPUS_JNI_UNUSED(caller);

    OpenedJavaByteArray opened_encoder = open_java_byte_array(env, encoder, JNI_FALSE);
    OpenedJavaByteArray opened_data = open_java_byte_array(env, data, JNI_TRUE);

    opus_int16 *input = malloc(sizeof(opus_int16) * frame_size * channels * 2);

    for(uint32_t i = 0; i < length / 2; i++) {
        input[i] = opened_data.data[offset + 2 * i + 1] << 8 | opened_data.data[offset + 2 * i];
    }

    unsigned char *output = malloc(max_packet_size);

    opus_int32 num_bytes = opus_encode((OpusEncoder *) opened_encoder.data,
                                       input,
                                       frame_size,
                                       output,
                                       max_packet_size);

    free(input);
    close_java_byte_array(env, &opened_data);
    close_java_byte_array(env, &opened_encoder);

    if(num_bytes < 0) {
        free(output);
        throw_opus_error(env, num_bytes, "Failed to encode frame");
        return NULL;
    }

    jbyteArray java_output = (*env)->NewByteArray(env, num_bytes);
    (*env)->SetByteArrayRegion(env, java_output, 0, num_bytes, (const jbyte *) output);
    free(output);

    return java_output;
}

JNIEXPORT jbyteArray JNICALL Java_net_labymod_opus_OpusCodec_decodeFrame0(JNIEnv *env,
                                                                          jclass caller,
                                                                          jbyteArray decoder,
                                                                          jbyteArray data,
                                                                          jint offset,
                                                                          jint length,
                                                                          jint max_frame_size,
                                                                          jint channels) {
    OPUS_JNI_UNUSED(caller);

    OpenedJavaByteArray opened_decoder = open_java_byte_array(env, decoder, JNI_FALSE);
    OpenedJavaByteArray opened_data = open_java_byte_array(env, data, JNI_TRUE);

    opus_int16 *output = malloc(sizeof(opus_uint16) * max_frame_size * channels);

    int frame_size = opus_decode((OpusDecoder *) opened_decoder.data,
                                (const unsigned char *) opened_data.data + offset,
                                length,
                                output,
                                max_frame_size,
                                0);

    close_java_byte_array(env, &opened_data);
    close_java_byte_array(env, &opened_decoder);

    if(frame_size < 0) {
        if(frame_size == OPUS_INVALID_PACKET) {
            throw_illegal_argument_exception(env, "Input data is not a valid packet");
        } else {
            throw_opus_error(env, frame_size, "Failed to decode data");
        }
        return NULL;
    }

    size_t raw_output_size = channels * frame_size * 2;
    OpenedJavaByteArray raw_output = create_and_open_java_byte_array(env, raw_output_size);

    for(size_t i = 0; i < channels * frame_size; i++) {
        raw_output.data[2 * i] = output[i] & 0xFF;
        raw_output.data[2 * i + 1] = (output[i] >> 8) & 0xFF;
    }

    free(output);

    return close_java_byte_array(env, &raw_output);
}