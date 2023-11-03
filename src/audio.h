#pragma once

#include "archive.h"
#include "deps/miniaudio.h"

struct Audio {
  String buf;
};

struct Sound {
  ma_decoder decoder;
  ma_sound ma;
};

bool audio_load(Audio *audio, Archive *ar, String filepath);
void audio_trash(Audio *audio);
bool sound_load(Sound *sound, Audio *audio);
void sound_trash(Sound *sound);