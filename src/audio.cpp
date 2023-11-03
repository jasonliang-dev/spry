#include "audio.h"
#include "app.h"

bool audio_load(Audio *audio, Archive *ar, String filepath) {
  String contents = {};
  bool ok = ar->read_entire_file(&contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  ma_result res = 0;

  res = ma_decoder_init_memory(contents.data, contents.len, nullptr,
                               &audio->decoder);
  if (res != MA_SUCCESS) {
    return false;
  }

  res = ma_sound_init_from_data_source(&g_app->audio_engine, &audio->decoder, 0,
                                       nullptr, &audio->sound);
  if (res != MA_SUCCESS) {
    return false;
  }

  return true;
}

void audio_trash(Audio *audio) {
  ma_sound_uninit(&audio->sound);
  ma_decoder_uninit(&audio->decoder);
}
