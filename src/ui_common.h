#pragma once
#include <lvgl.h>
#include "audio_player.h"

// ---------- Palet Warna (Modern Dark Theme) ----------
#define C_BG         lv_color_hex(0x0B0D12)
#define C_CARD       lv_color_hex(0x161A22)
#define C_CARD_PR    lv_color_hex(0x232938)
#define C_TEXT       lv_color_hex(0xF2F4F8)
#define C_SUB        lv_color_hex(0x8A93A6)
#define C_MUTED      lv_color_hex(0x565E6D)

// Aksara Jawa Theme (Indigo / Purple)
#define C_JAWA_1     lv_color_hex(0x7C5CFF)
#define C_JAWA_2     lv_color_hex(0x5B3FE0)

// Aksara Bali Theme (Warm Amber / Gold)
#define C_BALI_1     lv_color_hex(0xF59E0B)
#define C_BALI_2     lv_color_hex(0xD97706)

// Accent tambahan
#define C_PINK       lv_color_hex(0xFF5C8A)
#define C_PINK_D     lv_color_hex(0xD93F6C)

// Jenis aksara
enum AksaraType {
  AKSARA_JAWA = 0,
  AKSARA_BALI = 1
};

// Callback aplikasi
struct AppCallbacks {
  void (*onSelectScript)(AksaraType type);           // Saat memilih Jawa / Bali
  void (*onStartPractice)(AksaraType type);          // Kartu besar "Mulai Latihan"
  void (*onSelectAksara)(AksaraType type, int index);// Memilih salah satu aksara (0-4)
  void (*onAllAksara)(AksaraType type);              // Tombol "Semua Aksara"
  void (*onSettings)();                              // Tombol "Setelan"
};

// Helper UI umum (dioptimasi: gradien hanya jika warna beda)
inline lv_obj_t* ui_make_card(lv_obj_t* parent, int x, int y, int w, int h,
                              lv_color_t c1, lv_color_t c2, int radius) {
  lv_obj_t* o = lv_obj_create(parent);
  if (!o) return nullptr;
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_radius(o, radius, 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(o, c1, 0);
  if (c1.blue != c2.blue || c1.green != c2.green || c1.red != c2.red) {
    lv_obj_set_style_bg_grad_color(o, c2, 0);
    lv_obj_set_style_bg_grad_dir(o, LV_GRAD_DIR_VER, 0);
  } else {
    lv_obj_set_style_bg_grad_dir(o, LV_GRAD_DIR_NONE, 0);
  }
  lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
  return o;
}

inline lv_obj_t* ui_make_label(lv_obj_t* parent, const char* text,
                               const lv_font_t* font, lv_color_t color) {
  lv_obj_t* l = lv_label_create(parent);
  if (!l) return nullptr;
  if (font) lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, color, 0);
  if (text) lv_label_set_text(l, text);
  return l;
}

inline void ui_set_pressed(lv_obj_t* o, lv_color_t c1, lv_color_t c2) {
  if (!o) return;
  lv_obj_set_style_bg_color(o, c1, LV_PART_MAIN | LV_STATE_PRESSED);
  if (c1.blue != c2.blue || c1.green != c2.green || c1.red != c2.red) {
    lv_obj_set_style_bg_grad_color(o, c2, LV_PART_MAIN | LV_STATE_PRESSED);
  }
  lv_obj_set_style_translate_y(o, 2, LV_PART_MAIN | LV_STATE_PRESSED);
}

// Toast global
void ui_toast_init();
void ui_show_toast(const char* text);

// Maskot Buddy (Varian Babak 1, 2, 3 & Home Avatar)
#ifdef __cplusplus
extern "C" {
#endif
LV_IMAGE_DECLARE(BuddyMaskot);
LV_IMAGE_DECLARE(Buddy3);
LV_IMAGE_DECLARE(Buddy4);
LV_IMAGE_DECLARE(BuddyJawa);
LV_IMAGE_DECLARE(BuddyBali);

// Font Aksara Daerah Native
LV_FONT_DECLARE(font_jawa_22);
LV_FONT_DECLARE(font_bali_22);
#ifdef __cplusplus
}
#endif

