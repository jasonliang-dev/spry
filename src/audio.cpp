#include "audio.h"
#include "app.h"
#include "profile.h"

bool audio_load(Audio *audio, Archive *ar, String filepath) {
  PROFILE_FUNC();

  String contents = {};
  bool ok = ar->read_entire_file(&contents, filepath);
  if (!ok) {
    return false;
  }
  defer(mem_free(contents.data));

  ma_result res = MA_SUCCESS;

  ma_decoder decoder = {};

  {
    PROFILE_BLOCK("ma_decoder_init_memory");

    res =
        ma_decoder_init_memory(contents.data, contents.len, nullptr, &decoder);
    if (res != MA_SUCCESS) {
      return false;
    }
  }

  defer(ma_decoder_uninit(&decoder));

  ma_uint64 frames = 0;
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

  {
    PROFILE_BLOCK("ma_data_source_read_pcm_frames");

    res = ma_data_source_read_pcm_frames(&decoder, buf, frames, nullptr);
    if (res != MA_SUCCESS) {
      mem_free(buf);
      return false;
    }
  }

  audio->buf = buf;
  audio->frames = (u64)frames;
  audio->format = format;
  audio->channels = channels;
  audio->sample_rate = sample_rate;
  audio->ref_count = 1;

  printf("created audio with %llu frames, %d channels, %d sample rate\n",
         (unsigned long long)frames, channels, sample_rate);
  return true;
}

void audio_unref(Audio *audio) {
  audio->ref_count--;
  if (audio->ref_count == 0) {
    mem_free(audio->buf);
    mem_free(audio);
  }
}

static void on_sound_end(void *udata, ma_sound *ma) {
  Sound *sound = (Sound *)udata;
  if (sound->zombie) {
    sound->dead_end = true;
  }
}

Sound *sound_load(Audio *audio) {
  PROFILE_FUNC();

  ma_result res = MA_SUCCESS;

  Sound *sound = (Sound *)mem_alloc(sizeof(Sound));

  ma_audio_buffer_config config = {};
  config.format = audio->format;
  config.channels = audio->channels;
  config.sampleRate = audio->sample_rate;
  config.sizeInFrames = audio->frames;
  config.pData = audio->buf;
  res = ma_audio_buffer_init(&config, &sound->buffer);
  if (res != MA_SUCCESS) {
    mem_free(sound);
    return nullptr;
  }

  res = ma_sound_init_from_data_source(&g_app->audio_engine, &sound->buffer, 0,
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

  audio->ref_count++;
  sound->audio = audio;
  sound->zombie = false;
  sound->dead_end = false;
  return sound;
}

void sound_trash(Sound *sound) {
  ma_sound_uninit(&sound->ma);
  ma_audio_buffer_uninit(&sound->buffer);
  audio_unref(sound->audio);
}
