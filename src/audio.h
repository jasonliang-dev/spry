#pragma once

#include "deps/miniaudio.h"
#include "prelude.h"

struct Audio {
  float *buf;
  u64 frames;
  u32 channels;
  u32 sample_rate;
  i32 ref_count;
  ma_format format;
};

bool audio_load(Audio *audio, String filepath);
void audio_unref(Audio *audio);

struct Sound {
  ma_audio_buffer buffer;
  ma_sound ma;
  Audio *audio;
  bool zombie;
  bool dead_end;
};

Sound *sound_load(Audio *audio);
void sound_trash(Sound *sound);