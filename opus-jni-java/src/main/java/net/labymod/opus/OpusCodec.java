package net.labymod.opus;

import java.io.*;
import java.nio.file.Files;
import java.util.*;

/*
opus-jni, a simple Opus JNI Wrapper.
Copyright (C) 2020 LabyMedia GmbH This program is free software:
you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
public class OpusCodec {
  private final OpusCodecOptions options;

  /*
   * The following 2 byte arrays directly contain the data the native
   * opus library uses to store the state of the decoder. Since destroying
   * the decoder does nothing more than freeing it, we can skip this step
   * and let the java garbage collector clean up the byte arrays for us.
   */
  private final byte[] encoderMemory;
  private final byte[] decoderMemory;

  /**
   * Creates a new {@link OpusCodec} instance.
   *
   * @param options The options to create the instance with
   * @throws IllegalArgumentException If some options are invalid
   */
  public OpusCodec(OpusCodecOptions options) {
    this.options = options;
    this.encoderMemory = createEncoder(options.getSampleRate(), options.getChannels(), options.getBitRate());
    this.decoderMemory = createDecoder(options.getSampleRate(), options.getChannels());
  }

  private static native byte[] createEncoder(int sampleRate, int channels, int bitrate);

  private static native byte[] createDecoder(int sampleRate, int channels);

  /**
   * Encodes a chunk of raw PCM data.
   *
   * @param frame The chunk to encode
   * @return The encoded chunk
   * @throws IllegalArgumentException If the length of the chunk is not CHANNELS * FRAME_SIZE * 2
   */
  public byte[] encodeFrame(byte[] frame) {
    return encodeFrame(frame, 0, frame.length);
  }

  /**
   * Encodes a chunk of raw PCM data from a specific area of a byte array.
   *
   * @param data   The byte array to extract the chunk from
   * @param offset The offset where the chunk can be found
   * @param length The length of the chunk
   * @return The encoded chunk
   * @throws IllegalArgumentException  If the length of the chunk is not CHANNELS * FRAME_SIZE * 2
   * @throws IndexOutOfBoundsException If {@code data.length < length + offset}
   */
  public byte[] encodeFrame(byte[] data, int offset, int length) {
    if (length != options.getChannels() * options.getFrameSize() * 2) {
      throw new IllegalArgumentException(String.format("Expected length to be %d, but got %d",
          options.getChannels() * options.getFrameSize() * 2, length));
    } else if (data.length < offset + length) {
      throw new IndexOutOfBoundsException("data.length is less than length + offset");
    }

    return encodeFrame0(encoderMemory, data, offset, length,
        options.getMaxPacketSize(), options.getChannels(), options.getFrameSize());
  }

  private static native byte[] encodeFrame0(
      byte[] encoderMemory, byte[] data, int offset, int length, int maxPacketSize, int channels, int frameSize);

  /**
   * Decodes a chunk of encoded pcm data.
   *
   * @param data The data to decode
   * @return The decoded data
   * @throws IllegalArgumentException If the data has an invalid format
   */
  public byte[] decodeFrame(byte[] data) {
    return decodeFrame(data, 0, data.length);
  }

  /**
   * Decodes a chunk of encoded pcm data from a specific area of a byte array.
   *
   * @param data   The byte array to extract the chunk from
   * @param offset The offset where the chunk can be found
   * @param length The length of the chunk
   * @return The decoded data
   * @throws IllegalArgumentException  If the data has an invalid format
   * @throws IndexOutOfBoundsException If {@code data.length < length + offset}
   */
  public byte[] decodeFrame(byte[] data, int offset, int length) {
    if (data.length < offset + length) {
      throw new IndexOutOfBoundsException("data.length is less than length + offset");
    }

    return decodeFrame0(decoderMemory, data, offset, length, options.getMaxFrameSize(), options.getChannels());
  }

  private static native byte[] decodeFrame0(
      byte[] decoderMemory, byte[] data, int offset, int length, int maxFrameSize, int channels);


  /**
   * Retrieves the options this codec was created with.
   *
   * @return The options this codec was created with
   */
  public OpusCodecOptions getOptions() {
    return options;
  }

  private static String getNativeLibraryName() {
    String systemName = System.getProperty("os.name", "bare-metal?").toLowerCase();
    if (systemName.contains("nux") || systemName.contains("nix")) {
      return "libopus-jni-native.so";
    } else if (systemName.contains("mac")) {
      return "libopus-jni-native.dylib";
    } else if (systemName.contains("windows")) {
      return "opus-jni-native.dll";
    } else {
      throw new NoSuchElementException("No native library for system " + systemName);
    }
  }

  /**
   * Extracts the native library to the specified directory but does not load them.
   *
   * @param directory    The directory to extract the natives to
   * @throws IOException If an error occurs while loading the native library
   */
  public static void extractNatives(File directory) throws IOException {
    String nativeLibraryName = getNativeLibraryName();
    Files.copy(OpusCodec.class.getResourceAsStream("/native-binaries/" + nativeLibraryName),
        directory.toPath().resolve(nativeLibraryName));
  }

  /**
   * Loads the native library from the specified directory
   * (which should have been set up with {@link OpusCodec#extractNatives(File directory)}).
   *
   * @param directory             The directory to load the native library from
   * @throws UnsatisfiedLinkError In case the native libraries fail to load
   */
  public static void loadNatives(File directory) {
    System.load(new File(directory, getNativeLibraryName()).getAbsolutePath());
  }

  /**
   * Extract the native library and load it
   *
   * @throws IOException          In case an error occurs while extracting the native library
   * @throws UnsatisfiedLinkError In case the native libraries fail to load
   */
  public static void setupWithTemporaryFolder() throws IOException {
    File temporaryDir = Files.createTempDirectory("opus-jni").toFile();
    temporaryDir.deleteOnExit();
    extractNatives(temporaryDir);
    loadNatives(temporaryDir);
  }

  /**
   * Retrieves a builder filled with default values.
   *
   * @return A builder which can be used to create an {@link OpusCodec}
   */
  public static OpusCodecBuilder builder() {
    return new OpusCodecBuilder();
  }
}