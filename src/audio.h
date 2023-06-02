#pragma once

#include "archive.h"
#include "array.h"

struct AudioWave {
  float *data;
  i32 len;
  i32 channels;
  i32 sample_rate;
};

struct AudioSource {
  i32 index;
  i32 cursor;
};

struct AudioSources {
  Array<AudioWave> waves;
  Array<AudioSource> playing;
};

bool audio_wave_load(AudioWave *wave, Archive *ar, String filepath);
void audio_playback(AudioSources *srcs, float *buf, i32 frames, i32 channels);
inline void drop(AudioWave *wave) { mem_free(wave->data); }
void drop(AudioSources *srcs);