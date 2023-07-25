#include "audio.h"

#define STB_VORBIS_HEADER_ONLY
#include "deps/stb_vorbis.c"

i32 audio_load(AudioSources *srcs, Archive *ar, String filepath) {
  String contents = {};
  bool ok = ar->read_entire_file(ar, &contents, filepath);
  if (!ok) {
    return -1;
  }
  defer(mem_free(contents.data));

  i32 channels = 0;
  i32 rate = 0;
  i16 *data16 = nullptr;
  i32 len = stb_vorbis_decode_memory((u8 *)contents.data, (i32)contents.len,
                                     &channels, &rate, &data16);
  if (len < 0) {
    return -1;
  }
  defer(free(data16));

  float *data32 = nullptr;
  if (channels == 1) {
    data32 = (float *)mem_alloc(len * sizeof(float) * 2);
    for (i32 i = 0; i < len; i++) {
      data32[i * 2 + 0] = data16[i] / (float)0x7fff;
      data32[i * 2 + 1] = data16[i] / (float)0x7fff;
    }
    channels = 2;
  } else if (channels == 2) {
    data32 = (float *)mem_alloc(len * 2 * sizeof(float));
    for (i32 i = 0; i < len * 2; i++) {
      data32[i] = data16[i] / (float)0x7fff;
    }
  }

  AudioWave wave = {};
  wave.data = data32;
  wave.len = len * channels;
  wave.channels = 2;
  wave.sample_rate = rate;

  push(&srcs->waves, wave);
  return srcs->waves.len - 1;
}

void audio_playback(AudioSources *srcs, float *buf, i32 frames, i32 channels) {
  i32 samples = frames * channels;
  memset(buf, 0, samples * sizeof(float));

  for (AudioSource &src : srcs->playing) {
    float *cursor = buf;
    i32 remain = samples;
    AudioWave wave = srcs->waves[src.index];
    assert(channels == wave.channels);

    if (wave.data == nullptr) {
      src.cursor = wave.len;
      continue;
    }

    while (remain > 0) {
      i32 write = min(remain, wave.len - src.cursor);
      for (i32 i = 0; i < write; i++) {
        cursor[i] += wave.data[src.cursor + i];
      }

      src.cursor += write;
      remain -= write;
      cursor += write;

      if (src.cursor == wave.len) {
        if (src.looping) {
          src.cursor = 0;
        } else {
          remain = 0;
        }
      }
    }
  }

  for (u64 i = 0; i < srcs->playing.len;) {
    AudioSource src = srcs->playing[i];
    AudioWave wave = srcs->waves[src.index];

    if (src.cursor == wave.len) {
      srcs->playing[i] = srcs->playing[srcs->playing.len - 1];
      srcs->playing.len--;
    } else {
      i++;
    }
  }
}

void audio_play(AudioSources *srcs, i32 wave) {
  AudioSource src = {};
  src.index = wave;
  push(&srcs->playing, src);
}

void audio_play_loop(AudioSources *srcs, i32 wave) {
  AudioSource src = {};
  src.index = wave;
  src.looping = true;
  push(&srcs->playing, src);
}

void drop(AudioSources *srcs, i32 wave) {
  Array<AudioWave> waves = srcs->waves;
  if (waves[wave].data != nullptr) {
    mem_free(waves[wave].data);
    waves[wave] = {};
  }
}

void drop(AudioSources *srcs) {
  for (AudioWave wave : srcs->waves) {
    if (wave.data != nullptr) {
      mem_free(wave.data);
    }
  }
  drop(&srcs->waves);
  drop(&srcs->playing);
}
