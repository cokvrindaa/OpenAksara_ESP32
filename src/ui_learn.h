#pragma once
#include "ui_common.h"

// Inisialisasi layar pembelajaran & kuis interaktif ala Duolingo
void ui_learn_init(const AppCallbacks* callbacks);

// Memulai sesi pembelajaran untuk aksara daerah dan level tertentu
void ui_learn_start(AksaraType script, int level_index);

// Menampilkan layar pembelajaran
void ui_learn_show();

