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
  i32 index; // index into waves
  i32 cursor;
  bool looping;
};

struct AudioSources {
  Array<AudioWave> waves;
  Array<AudioSource> playing;
};

i32 audio_load(AudioSources *srcs, Archive *ar, String filepath);
void audio_playback(AudioSources *srcs, float *buf, i32 frames, i32 channels);
void audio_play(AudioSources *srcs, i32 wave);
void audio_play_loop(AudioSources *srcs, i32 wave);
void drop(AudioSources *srcs, i32 wave);
void drop(AudioSources *srcs);