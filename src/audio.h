#pragma once

#include "archive.h"
#include "deps/miniaudio.h"

struct Audio {
  float *buf;
  ma_format format;
  u64 frames;
  u32 channels;
  u32 sample_rate;
};

struct Sound {
  ma_audio_buffer buffer;
  ma_sound ma;
};

bool audio_load(Audio *audio, Archive *ar, String filepath);
void audio_trash(Audio *audio);
bool sound_load(Sound *sound, Audio *audio);
void sound_trash(Sound *sound);