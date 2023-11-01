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
  float volume;
  bool looping;
};

struct AudioSources {
  Array<AudioWave> waves;
  Array<AudioSource> playing;
};

i32 audio_load(AudioSources *srcs, Archive *ar, String filepath);
void audio_playback(AudioSources *srcs, float *buf, i32 frames, i32 channels,
                    float master_volume);
void audio_play(AudioSources *srcs, i32 wave, float vol, bool loop);
void audio_wave_trash(AudioSources *srcs, i32 wave);
void audio_sources_trash(AudioSources *srcs);