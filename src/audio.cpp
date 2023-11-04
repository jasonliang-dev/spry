#include "audio.h"
#include "app.h"

bool audio_load(Audio *audio, Archive *ar, String filepath) {
  String contents = {};
  bool ok = ar->read_entire_file(&contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  ma_result res = MA_SUCCESS;

  ma_decoder decoder = {};
  res = ma_decoder_init_memory(contents.data, contents.len, nullptr, &decoder);
  if (res != MA_SUCCESS) {
    return false;
  }
  defer(ma_decoder_uninit(&decoder));

  u64 frames = 0;
  res = ma_data_source_get_length_in_pcm_frames(&decoder, &frames);
  if (res != MA_SUCCESS) {
    return false;
  }

  ma_format format = ma_format_unknown;
  u32 channels = 0;
  u32 sample_rate = 0;
  res = ma_data_source_get_data_format(&decoder, &format, &channels,
                                       &sample_rate, nullptr, 0);
  if (res != MA_SUCCESS) {
    return false;
  }

  float *buf = (float *)mem_alloc(sizeof(float) * frames * channels);
  res = ma_data_source_read_pcm_frames(&decoder, buf, frames, nullptr);
  if (res != MA_SUCCESS && res != MA_AT_END) {
    return false;
  }

  audio->buf = buf;
  audio->frames = frames;
  audio->format = format;
  audio->channels = channels;
  audio->sample_rate = sample_rate;
  return true;
}

void audio_trash(Audio *audio) { mem_free(audio->buf); }

bool sound_load(Sound *sound, Audio *audio) {
  ma_result res = MA_SUCCESS;

  ma_audio_buffer_config config = {};
  config.format = audio->format;
  config.channels = audio->channels;
  config.sampleRate = audio->sample_rate;
  config.sizeInFrames = audio->frames;
  config.pData = audio->buf;
  res = ma_audio_buffer_init(&config, &sound->buffer);
  if (res != MA_SUCCESS) {
    return false;
  }

  res = ma_sound_init_from_data_source(&g_app->audio_engine, &sound->buffer, 0,
                                       nullptr, &sound->ma);
  if (res != MA_SUCCESS) {
    return false;
  }

  return true;
}

void sound_trash(Sound *sound) {
  ma_sound_uninit(&sound->ma);
  ma_audio_buffer_uninit(&sound->buffer);
}
