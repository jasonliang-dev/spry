#pragma once

#include "archive.h"
#include "deps/miniaudio.h"

struct Audio {
  ma_decoder decoder;
  ma_sound sound;
};

bool audio_load(Audio *audio, Archive *ar, String filepath);
void audi_trash(Audio *audio);