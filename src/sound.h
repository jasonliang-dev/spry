#pragma once

#include "deps/miniaudio.h"
#include "prelude.h"

struct Sound {
  ma_sound ma;
  bool zombie;
  bool dead_end;

  void trash();
};

Sound *sound_load(String filepath);