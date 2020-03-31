package net.labymod.opus;

public class OpusCodecBuilder {
  private int frameSize = 960;
  private int sampleRate = 48000;
  private int channels = 1;
  private int bitRate = 64000;
  private int maxFrameSize = 6 * 960;
  private int maxPacketSize = 3 * 1276;

  OpusCodecBuilder() {
  }

  public int frameSize() {
    return this.frameSize;
  }

  public OpusCodecBuilder frameSize(int frameSize) {
    this.frameSize = frameSize;
    return this;
  }

  public int sampleRate() {
    return this.sampleRate;
  }

  /**
   * @param sampleRate The sample rate to use in the codec instance.
   *                   8, 12, 16, 24 and 48khz are supported.
   * @return this
   */
  public OpusCodecBuilder sampleRate(int sampleRate) {
    this.sampleRate = sampleRate;
    return this;
  }

  public int channels() {
    return this.channels;
  }

  public OpusCodecBuilder channels(int channels) {
    this.channels = channels;
    return this;
  }

  public int bitRate() {
    return this.bitRate;
  }

  public OpusCodecBuilder bitRate(int bitrate) {
    this.bitRate = bitrate;
    return this;
  }

  public int maxFrameSize() {
    return this.maxFrameSize;
  }

  public OpusCodecBuilder maxFrameSize(int maxFrameSize) {
    this.maxFrameSize = maxFrameSize;
    return this;
  }

  public int maxPacketSize() {
    return this.maxPacketSize;
  }

  public OpusCodecBuilder maxPacketSize(int maxPacketSize) {
    this.maxPacketSize = maxPacketSize;
    return this;
  }

  public OpusCodec build() {
    return new OpusCodec(OpusCodecOptions.of(frameSize, sampleRate, channels, bitRate, maxFrameSize, maxPacketSize));
  }
}
