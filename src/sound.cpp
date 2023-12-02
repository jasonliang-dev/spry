#include "sound.h"
#include "app.h"
#include "profile.h"

static void on_sound_end(void *udata, ma_sound *ma) {
  Sound *sound = (Sound *)udata;
  if (sound->zombie) {
    sound->dead_end = true;
  }
}

Sound *sound_load(String filepath) {
  PROFILE_FUNC();

  ma_result res = MA_SUCCESS;

  Sound *sound = (Sound *)mem_alloc(sizeof(Sound));

  String cpath = to_cstr(filepath);
  defer(mem_free(cpath.data));

  res = ma_sound_init_from_file(&g_app->audio_engine, cpath.data, 0, nullptr,
                                nullptr, &sound->ma);
  if (res != MA_SUCCESS) {
    mem_free(sound);
    return nullptr;
  }

  res = ma_sound_set_end_callback(&sound->ma, on_sound_end, sound);
  if (res != MA_SUCCESS) {
    mem_free(sound);
    return nullptr;
  }

  sound->zombie = false;
  sound->dead_end = false;
  return sound;
}

void Sound::trash() {
  ma_sound_uninit(&ma);
  mem_free(this);
}
