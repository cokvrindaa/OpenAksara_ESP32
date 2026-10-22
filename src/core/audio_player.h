#pragma once
#include <Arduino.h>

enum SoundEffect {
  SOUND_CLICK = 1,     // Suara klik tombol umum
  SOUND_NODE = 2,      // Suara pilih level aksara
  SOUND_REWARD = 3,    // Suara peti / bonus / piala
  SOUND_BUDDY = 4,     // Suara sapaan maskot Buddy
  SOUND_PAGE = 5,      // Suara ganti babak (halaman naik/turun)
  SOUND_SUCCESS = 6,   // Suara goresan berhasil / cek canvas
  SOUND_ERROR = 7      // Suara jawaban salah / gagal
};

void audio_init();
void audio_update();
void audio_play(int track);
void audio_set_volume(int vol); // 0 - 30
