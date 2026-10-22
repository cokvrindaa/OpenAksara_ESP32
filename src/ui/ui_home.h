#pragma once
#include "ui_common.h"

// Inisialisasi seluruh sistem UI (Home, Jawa, Bali, dan Toast)
void ui_init(const AppCallbacks* callbacks);

// Kompatibilitas mundur
inline void ui_home_create(const AppCallbacks* callbacks) {
  ui_init(callbacks);
}

// Tampilkan layar Home (pemilihan aksara)
void ui_home_show();