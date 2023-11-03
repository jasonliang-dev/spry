#include "audio.h"
#include "app.h"

bool audio_load(Audio *audio, Archive *ar, String filepath) {
  String contents = {};
  bool ok = ar->read_entire_file(&contents, filepath);
  if (!ok) {
    return false;
  }
  audio->buf = contents;

  return true;
}

void audio_trash(Audio *audio) { mem_free(audio->buf.data); }

bool sound_load(Sound *sound, Audio *audio) {
  ma_result res = MA_SUCCESS;

  res = ma_decoder_init_memory(audio->buf.data, audio->buf.len, nullptr,
                               &sound->decoder);
  if (res != MA_SUCCESS) {
    return false;
  }

  res = ma_sound_init_from_data_source(&g_app->audio_engine, &sound->decoder, 0,
                                       nullptr, &sound->ma);
  if (res != MA_SUCCESS) {
    return false;
  }

  return true;
}

void sound_trash(Sound *sound) {
  ma_sound_uninit(&sound->ma);
  ma_decoder_uninit(&sound->decoder);
}
