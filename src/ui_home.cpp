#include "ui_home.h"
#include "ui_jawa.h"
#include "ui_bali.h"

static const AppCallbacks* home_cbs = nullptr;
static lv_obj_t* scr_home = nullptr;

// Event Handlers
static void on_select_jawa_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  ui_show_toast("Membuka Aksara Jawa");
  if (home_cbs && home_cbs->onSelectScript) {
    home_cbs->onSelectScript(AKSARA_JAWA);
  }
  ui_jawa_show();
}

static void on_select_bali_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  ui_show_toast("Membuka Aksara Bali");
  if (home_cbs && home_cbs->onSelectScript) {
    home_cbs->onSelectScript(AKSARA_BALI);
  }
  ui_bali_show();
}

static void on_settings_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  ui_show_toast("Buka Setelan");
  if (home_cbs && home_cbs->onSettings) {
    home_cbs->onSettings();
  }
}

static void build_home_screen() {
  scr_home = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_home, C_BG, 0);
  lv_obj_set_style_bg_opa(scr_home, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr_home, LV_OBJ_FLAG_SCROLLABLE);

  // Header Title
  lv_obj_t* title = ui_make_label(scr_home, "OpenAksara", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 20, 14);

  lv_obj_t* sub = ui_make_label(scr_home, "Pilih aksara daerah untuk dipelajari",
                                &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(sub, 20, 38);

  // Tombol Setelan Header Kanan
  lv_obj_t* btnSet = ui_make_card(scr_home, 360, 14, 100, 36, C_CARD, C_CARD, 18);
  ui_set_pressed(btnSet, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btnSet, on_settings_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* btnSetLbl = ui_make_label(btnSet, LV_SYMBOL_SETTINGS " Setelan", &lv_font_montserrat_14, C_SUB);
  lv_obj_center(btnSetLbl);

  // ===== KARTU 1: AKSARA JAWA =====
  lv_obj_t* cardJawa = ui_make_card(scr_home, 20, 72, 212, 204, C_CARD, C_CARD, 20);
  ui_set_pressed(cardJawa, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(cardJawa, on_select_jawa_click, LV_EVENT_CLICKED, NULL);

  // Badge Tag "20 AKSARA"
  lv_obj_t* tagJawa = ui_make_card(cardJawa, 14, 14, 94, 22, C_JAWA_1, C_JAWA_2, 11);
  lv_obj_clear_flag(tagJawa, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* tagJawaLbl = ui_make_label(tagJawa, "20 AKSARA", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(tagJawaLbl);

  // Avatar Maskot Buddy Jawa (Pengganti icon "Jw")
  lv_obj_t* icJawa = lv_image_create(cardJawa);
  lv_image_set_src(icJawa, &BuddyJawa);
  lv_obj_set_pos(icJawa, 148, 12);
  lv_obj_clear_flag(icJawa, LV_OBJ_FLAG_CLICKABLE);

  // Judul & Keterangan
  lv_obj_t* tJawa = ui_make_label(cardJawa, "Aksara Jawa", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(tJawa, 14, 52);

  lv_obj_t* descJawa = ui_make_label(cardJawa, "Carakan / Nglegena\nHa - Na - Ca - Ra - Ka",
                                    &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(descJawa, 14, 82);

  // Tombol aksi di kartu
  lv_obj_t* btnJawa = ui_make_card(cardJawa, 14, 150, 184, 38, C_JAWA_1, C_JAWA_2, 14);
  ui_set_pressed(btnJawa, C_JAWA_2, C_JAWA_2);
  lv_obj_add_event_cb(btnJawa, on_select_jawa_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* btnJawaLbl = ui_make_label(btnJawa, "Pilih Jawa  " LV_SYMBOL_RIGHT,
                                      &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(btnJawaLbl);

  // ===== KARTU 2: AKSARA BALI =====
  lv_obj_t* cardBali = ui_make_card(scr_home, 248, 72, 212, 204, C_CARD, C_CARD, 20);
  ui_set_pressed(cardBali, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(cardBali, on_select_bali_click, LV_EVENT_CLICKED, NULL);

  // Badge Tag "18 AKSARA"
  lv_obj_t* tagBali = ui_make_card(cardBali, 14, 14, 94, 22, C_BALI_1, C_BALI_2, 11);
  lv_obj_clear_flag(tagBali, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* tagBaliLbl = ui_make_label(tagBali, "18 AKSARA", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(tagBaliLbl);

  // Avatar Maskot Buddy Bali (Pengganti icon "Bl")
  lv_obj_t* icBali = lv_image_create(cardBali);
  lv_image_set_src(icBali, &BuddyBali);
  lv_obj_set_pos(icBali, 148, 12);
  lv_obj_clear_flag(icBali, LV_OBJ_FLAG_CLICKABLE);

  // Judul & Keterangan
  lv_obj_t* tBali = ui_make_label(cardBali, "Aksara Bali", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(tBali, 14, 52);

  lv_obj_t* descBali = ui_make_label(cardBali, "Wreastra Sastra Bali\nHa - Na - Ca - Ra - Ka",
                                    &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(descBali, 14, 82);

  // Tombol aksi di kartu
  lv_obj_t* btnBali = ui_make_card(cardBali, 14, 150, 184, 38, C_BALI_1, C_BALI_2, 14);
  ui_set_pressed(btnBali, C_BALI_2, C_BALI_2);
  lv_obj_add_event_cb(btnBali, on_select_bali_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* btnBaliLbl = ui_make_label(btnBali, "Pilih Bali  " LV_SYMBOL_RIGHT,
                                      &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(btnBaliLbl);

  // Footer Note
  lv_obj_t* footer = ui_make_label(scr_home, "OpenAksara ESP32  •  Sentuh kartu untuk mulai",
                                  &lv_font_montserrat_14, C_MUTED);
  lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void ui_init(const AppCallbacks* callbacks) {
  home_cbs = callbacks;

  // Inisialisasi Toast Global
  ui_toast_init();

  // Bangun Layar Home
  build_home_screen();

  // Inisialisasi Layar Khusus Aksara Jawa dan Aksara Bali secara terpisah
  ui_jawa_init(callbacks);
  ui_bali_init(callbacks);

  // Tampilkan Layar Home
  lv_scr_load(scr_home);
}

void ui_home_show() {
  if (scr_home) {
    lv_scr_load_anim(scr_home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, false);
  }
}