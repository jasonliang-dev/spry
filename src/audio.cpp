#include "audio.h"

#define STB_VORBIS_HEADER_ONLY
#include "deps/stb_vorbis.c"

bool audio_wave_load(AudioWave *wave, Archive *ar, String filepath) {
  String contents = {};
  bool ok = ar->read_entire_file(ar, &contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  i32 channels;
  i32 rate;
  i16 *data16;
  i32 len = stb_vorbis_decode_memory((u8 *)contents.data, (i32)contents.len,
                                     &channels, &rate, &data16);
  if (len < 0) {
    return false;
  }
  defer(mem_free(data16));

  float *data32 = (float *)mem_alloc(len * sizeof(float));
  for (i32 i = 0; i < len; i++) {
    data32[i] = data16[i] / (float)0x7fff;
  }

  AudioWave aw;
  aw.data = data32;
  aw.len = len;
  aw.channels = channels;
  aw.sample_rate = rate;
  *wave = aw;

  return true;
}

void audio_playback(AudioSources *srcs, float *buf, i32 frames, i32 channels) {
  i32 samples = frames * channels;
  memset(buf, 0, samples * sizeof(float));

  for (AudioSource &src : srcs->playing) {
    AudioWave wave = srcs->waves[src.index];

    i32 write = min(samples, wave.len - src.cursor);
    for (i32 i = 0; i < write; i++) {
      buf[i] += wave.data[src.cursor + i];
    }

    src.cursor += write;
  }

  for (u64 i = 0; i < srcs->playing.len; i++) {
    AudioSource src = srcs->playing[i];
    AudioWave wave = srcs->waves[src.index];

    if (src.cursor == wave.len) {
      srcs->playing[i] = srcs->playing[srcs->playing.len - 1];
      srcs->playing.len--;
    }
  }
}

void drop(AudioSources *srcs) {
  for (AudioWave wave : srcs->waves) {
    drop(&wave);
  }
  drop(&srcs->waves);
  drop(&srcs->playing);
}
