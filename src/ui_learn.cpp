#include "ui_learn.h"
#include "ui_jawa.h"
#include "ui_bali.h"
#include <stdio.h>
#include <string.h>

// ---------- Data Aksara Hanacaraka Jawa & Bali ----------
struct AksaraItem {
  const char* latin;   // Transliterasi Latin
  const char* aksara;  // Karakter UTF-8 Native Aksara
};

// 10 Aksara Jawa Utama
static const AksaraItem JAWA_ITEMS[10] = {
  { "ha", "\xEA\xA6\xB2" }, // ꦲ
  { "na", "\xEA\xA6\xA4" }, // ꦤ
  { "ca", "\xEA\xA6\x95" }, // ꦕ
  { "ra", "\xEA\xA6\xAA" }, // ꦫ
  { "ka", "\xEA\xA6\x8F" }, // ꦏ
  { "da", "\xEA\xA6\xA2" }, // ꦢ
  { "ta", "\xEA\xA6\xA0" }, // ꦠ
  { "sa", "\xEA\xA6\xB1" }, // ꦱ
  { "wa", "\xEA\xA6\xAE" }, // ꦮ
  { "la", "\xEA\xA6\xAD" }  // ꦭ
};

// 10 Aksara Bali Utama
static const AksaraItem BALI_ITEMS[10] = {
  { "ha", "\xE1\xAC\xB3" }, // ᬳ
  { "na", "\xE1\xAC\xA6" }, // ᬦ
  { "ca", "\xE1\xAC\x98" }, // ᬘ
  { "ra", "\xE1\xAC\xAD" }, // ᬭ
  { "ka", "\xE1\xAC\x93" }, // ᬓ
  { "da", "\xE1\xAC\xA4" }, // ᬤ
  { "ta", "\xE1\xAC\xA2" }, // ᬢ
  { "sa", "\xE1\xAC\xB2" }, // ᬲ
  { "wa", "\xE1\xAC\xAF" }, // ᬯ
  { "la", "\xE1\xAC\xAE" }  // ᬮ
};

// Warna Status Interaktif Duolingo
#define C_SEL_BORDER  lv_color_hex(0x38BDF8) // Biru Cyan aktif
#define C_SEL_BG      lv_color_hex(0x133857) // Biru gelap
#define C_SEL_TXT     lv_color_hex(0x38BDF8)

#define C_OK_BORDER   lv_color_hex(0x22C55E) // Hijau berhasil
#define C_OK_BG       lv_color_hex(0x14532D)
#define C_OK_TXT      lv_color_hex(0x4ADE80)

#define C_ERR_BORDER  lv_color_hex(0xEF4444) // Merah salah
#define C_ERR_BG      lv_color_hex(0x450A0A)
#define C_ERR_TXT     lv_color_hex(0xF87171)

#define C_DONE_BORDER lv_color_hex(0x1F2430) // Selesai / pudar
#define C_DONE_BG     lv_color_hex(0x11141B)
#define C_DONE_TXT    lv_color_hex(0x565E6D)

// State Pembelajaran
static const AppCallbacks* learn_cbs = nullptr;
static lv_obj_t* scr_learn = nullptr;
static lv_obj_t* bar_progress = nullptr;
static lv_obj_t* lbl_badge_xp = nullptr;
static lv_obj_t* content_cont = nullptr;

static AksaraType current_script = AKSARA_JAWA;
static int current_level_idx = 0;
static int current_stage = 0; // 0: Pasangkan (Gbr 1), 1: Tebak Aksara (Gbr 2), 2: Selesai
static int earned_xp = 0;

// Sub-views
static lv_obj_t* view_match = nullptr;
static lv_obj_t* view_guess = nullptr;
static lv_obj_t* view_complete = nullptr;

// Stage 0: Match Pairs Data
static const int PAIR_COUNT = 4;
static int active_item_indices[4];
static int right_shuffle[4]; // Pemetaan tombol kanan ke indeks item
static lv_obj_t* left_btns[4];
static lv_obj_t* right_btns[4];
static lv_obj_t* left_lbls[4];
static lv_obj_t* right_lbls[4];
static bool left_matched[4];
static bool right_matched[4];
static int sel_left = -1;
static int sel_right = -1;
static int matched_pairs_count = 0;
static lv_obj_t* match_bottom_bar = nullptr;

// Stage 1: Guess Data
static int guess_correct_idx = 0;
static int guess_choices[4];
static lv_obj_t* guess_btns[4];
static lv_obj_t* guess_lbls[4];
static int guess_selected = -1;
static bool guess_checked = false;
static lv_obj_t* guess_feedback_lbl = nullptr;
static lv_obj_t* guess_action_btn = nullptr;
static lv_obj_t* guess_action_lbl = nullptr;

// Forward declarations
static void build_stage_match();
static void build_stage_guess();
static void build_stage_complete();
static void show_stage(int stage);

// ---------- Helper Font & Data Aksara ----------
static const lv_font_t* get_current_font() {
  return (current_script == AKSARA_JAWA) ? &font_jawa_22 : &font_bali_22;
}

static const AksaraItem* get_current_items() {
  return (current_script == AKSARA_JAWA) ? JAWA_ITEMS : BALI_ITEMS;
}

// ---------- Event: Tutup / Kembali ----------
static void on_close_learn_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  if (current_script == AKSARA_JAWA) {
    ui_jawa_show();
  } else {
    ui_bali_show();
  }
}

// ---------- Stage 0: Logika Pasangkan Aksara (Match Pairs) ----------
static void reset_pair_button_style(lv_obj_t* btn, lv_obj_t* lbl, bool is_matched) {
  if (is_matched) {
    lv_obj_set_style_bg_color(btn, C_DONE_BG, 0);
    lv_obj_set_style_border_color(btn, C_DONE_BORDER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_text_color(lbl, C_DONE_TXT, 0);
  } else {
    lv_obj_set_style_bg_color(btn, C_CARD, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_text_color(lbl, C_TEXT, 0);
  }
}

static void highlight_selected_button(lv_obj_t* btn, lv_obj_t* lbl) {
  lv_obj_set_style_bg_color(btn, C_SEL_BG, 0);
  lv_obj_set_style_border_color(btn, C_SEL_BORDER, 0);
  lv_obj_set_style_border_width(btn, 2, 0);
  lv_obj_set_style_text_color(lbl, C_SEL_TXT, 0);
}

static void mark_success_buttons(int l, int r) {
  lv_obj_set_style_bg_color(left_btns[l], C_OK_BG, 0);
  lv_obj_set_style_border_color(left_btns[l], C_OK_BORDER, 0);
  lv_obj_set_style_text_color(left_lbls[l], C_OK_TXT, 0);

  lv_obj_set_style_bg_color(right_btns[r], C_OK_BG, 0);
  lv_obj_set_style_border_color(right_btns[r], C_OK_BORDER, 0);
  lv_obj_set_style_text_color(right_lbls[r], C_OK_TXT, 0);
}

static void mark_error_buttons(int l, int r) {
  lv_obj_set_style_bg_color(left_btns[l], C_ERR_BG, 0);
  lv_obj_set_style_border_color(left_btns[l], C_ERR_BORDER, 0);
  lv_obj_set_style_text_color(left_lbls[l], C_ERR_TXT, 0);

  lv_obj_set_style_bg_color(right_btns[r], C_ERR_BG, 0);
  lv_obj_set_style_border_color(right_btns[r], C_ERR_BORDER, 0);
  lv_obj_set_style_text_color(right_lbls[r], C_ERR_TXT, 0);
}

// Timer untuk mereset tombol jika salah
static void wrong_reset_timer_cb(lv_timer_t* t) {
  int* pair = (int*)lv_timer_get_user_data(t);
  int l = pair[0];
  int r = pair[1];
  lv_free(pair);

  if (l >= 0 && l < PAIR_COUNT) reset_pair_button_style(left_btns[l], left_lbls[l], left_matched[l]);
  if (r >= 0 && r < PAIR_COUNT) reset_pair_button_style(right_btns[r], right_lbls[r], right_matched[r]);
}

static void check_match() {
  if (sel_left < 0 || sel_right < 0) return;

  int item_left = active_item_indices[sel_left];
  int item_right = active_item_indices[right_shuffle[sel_right]];

  if (item_left == item_right) {
    // Cocok!
    left_matched[sel_left] = true;
    right_matched[sel_right] = true;
    mark_success_buttons(sel_left, sel_right);
    audio_play(SOUND_SUCCESS);
    matched_pairs_count++;

    sel_left = -1;
    sel_right = -1;

    if (matched_pairs_count >= PAIR_COUNT) {
      earned_xp += 20;
      char b[24];
      snprintf(b, sizeof(b), "⚡ +%d XP", earned_xp);
      lv_label_set_text(lbl_badge_xp, b);
      audio_play(SOUND_REWARD);
      lv_obj_clear_flag(match_bottom_bar, LV_OBJ_FLAG_HIDDEN);
    }
  } else {
    // Salah!
    mark_error_buttons(sel_left, sel_right);
    audio_play(SOUND_ERROR);

    int* pair = (int*)lv_malloc(sizeof(int) * 2);
    pair[0] = sel_left;
    pair[1] = sel_right;
    lv_timer_t* tmr = lv_timer_create(wrong_reset_timer_cb, 350, pair);
    lv_timer_set_repeat_count(tmr, 1);

    sel_left = -1;
    sel_right = -1;
  }
}

static void on_left_card_click(lv_event_t* e) {
  int idx = (int)(intptr_t)lv_event_get_user_data(e);
  if (left_matched[idx]) return;

  audio_play(SOUND_CLICK);
  if (sel_left >= 0 && sel_left < PAIR_COUNT) {
    reset_pair_button_style(left_btns[sel_left], left_lbls[sel_left], left_matched[sel_left]);
  }
  sel_left = idx;
  highlight_selected_button(left_btns[sel_left], left_lbls[sel_left]);

  if (sel_right >= 0) {
    check_match();
  }
}

static void on_right_card_click(lv_event_t* e) {
  int idx = (int)(intptr_t)lv_event_get_user_data(e);
  if (right_matched[idx]) return;

  audio_play(SOUND_CLICK);
  if (sel_right >= 0 && sel_right < PAIR_COUNT) {
    reset_pair_button_style(right_btns[sel_right], right_lbls[sel_right], right_matched[sel_right]);
  }
  sel_right = idx;
  highlight_selected_button(right_btns[sel_right], right_lbls[sel_right]);

  if (sel_left >= 0) {
    check_match();
  }
}

static void on_match_next_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  show_stage(1); // Ke Stage 1 (Tebak Aksara)
}

// ---------- Stage 1: Logika Tebak Aksara (Multiple Choice) ----------
static void on_guess_choice_click(lv_event_t* e) {
  if (guess_checked) return;

  int idx = (int)(intptr_t)lv_event_get_user_data(e);
  audio_play(SOUND_CLICK);

  for (int i = 0; i < 4; i++) {
    lv_obj_set_style_bg_color(guess_btns[i], C_CARD, 0);
    lv_obj_set_style_border_color(guess_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(guess_btns[i], 1, 0);
    lv_obj_set_style_text_color(guess_lbls[i], C_TEXT, 0);
  }

  guess_selected = idx;
  lv_obj_set_style_bg_color(guess_btns[idx], C_SEL_BG, 0);
  lv_obj_set_style_border_color(guess_btns[idx], C_SEL_BORDER, 0);
  lv_obj_set_style_border_width(guess_btns[idx], 2, 0);
  lv_obj_set_style_text_color(guess_lbls[idx], C_SEL_TXT, 0);

  lv_label_set_text(guess_feedback_lbl, "Tekan PERIKSA untuk mengecek jawaban");
  lv_obj_set_style_text_color(guess_feedback_lbl, C_TEXT, 0);
  lv_obj_set_style_bg_color(guess_action_btn, lv_color_hex(0x22C55E), 0);
}

static void on_guess_action_click(lv_event_t* e) {
  (void)e;
  if (!guess_checked) {
    if (guess_selected < 0) {
      audio_play(SOUND_CLICK);
      ui_show_toast("Pilih salah satu aksara terlebih dahulu!");
      return;
    }

    guess_checked = true;
    int chosen_item = guess_choices[guess_selected];

    if (chosen_item == guess_correct_idx) {
      // Benar
      audio_play(SOUND_SUCCESS);
      earned_xp += 15;
      char b[24];
      snprintf(b, sizeof(b), "⚡ +%d XP", earned_xp);
      lv_label_set_text(lbl_badge_xp, b);

      lv_obj_set_style_bg_color(guess_btns[guess_selected], C_OK_BG, 0);
      lv_obj_set_style_border_color(guess_btns[guess_selected], C_OK_BORDER, 0);
      lv_obj_set_style_text_color(guess_lbls[guess_selected], C_OK_TXT, 0);

      lv_label_set_text(guess_feedback_lbl, "Luar biasa! Jawabanmu benar (+15 XP)");
      lv_obj_set_style_text_color(guess_feedback_lbl, lv_color_hex(0x4ADE80), 0);
    } else {
      // Salah
      audio_play(SOUND_ERROR);
      lv_obj_set_style_bg_color(guess_btns[guess_selected], C_ERR_BG, 0);
      lv_obj_set_style_border_color(guess_btns[guess_selected], C_ERR_BORDER, 0);
      lv_obj_set_style_text_color(guess_lbls[guess_selected], C_ERR_TXT, 0);

      // Tunjukkan yang benar
      for (int i = 0; i < 4; i++) {
        if (guess_choices[i] == guess_correct_idx) {
          lv_obj_set_style_bg_color(guess_btns[i], C_OK_BG, 0);
          lv_obj_set_style_border_color(guess_btns[i], C_OK_BORDER, 0);
          lv_obj_set_style_text_color(guess_lbls[i], C_OK_TXT, 0);
        }
      }

      const AksaraItem* items = get_current_items();
      char err_str[64];
      snprintf(err_str, sizeof(err_str), "Kurang tepat! Yang benar adalah: %s", items[guess_correct_idx].latin);
      lv_label_set_text(guess_feedback_lbl, err_str);
      lv_obj_set_style_text_color(guess_feedback_lbl, lv_color_hex(0xF87171), 0);
    }

    lv_label_set_text(guess_action_lbl, "LANJUT " LV_SYMBOL_RIGHT);
  } else {
    // Pindah ke Stage Complete
    audio_play(SOUND_CLICK);
    show_stage(2);
  }
}

static void on_audio_hint_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_NODE);
  ui_show_toast("Mendengarkan bunyi aksara...");
}

// ---------- Stage 2: Selesai & Kembali ----------
static void on_finish_learn_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_REWARD);
  ui_show_toast("Selamat! Latihan Selesai, XP Bertambah.");

  if (current_script == AKSARA_JAWA) {
    ui_jawa_show();
  } else {
    ui_bali_show();
  }
}

// ---------- Pembangunan Tampilan Tiap Stage ----------
static void build_stage_match() {
  if (view_match) {
    lv_obj_clean(view_match);
  } else {
    view_match = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_match);
    lv_obj_set_size(view_match, 480, 276);
    lv_obj_clear_flag(view_match, LV_OBJ_FLAG_SCROLLABLE);
  }

  const AksaraItem* items = get_current_items();
  const lv_font_t* font_ak = get_current_font();

  // Reset state match
  matched_pairs_count = 0;
  sel_left = -1;
  sel_right = -1;
  for (int i = 0; i < PAIR_COUNT; i++) {
    left_matched[i] = false;
    right_matched[i] = false;
  }

  // Judul pertanyaan ala Duolingo (Gambar 1)
  lv_obj_t* title = ui_make_label(view_match, "Pilih pasangan yang cocok", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 24, 6);

  lv_obj_t* sub = ui_make_label(view_match, "Cocokkan aksara daerah dengan bunyi latinnya", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(sub, 24, 30);

  // Buat 4 Pasang Tombol Kiri (Aksara) dan Kanan (Latin)
  for (int i = 0; i < PAIR_COUNT; i++) {
    int y_pos = 54 + (i * 44);

    // --- Tombol Kiri (Aksara) ---
    left_btns[i] = ui_make_card(view_match, 24, y_pos, 210, 38, C_CARD, C_CARD, 10);
    lv_obj_set_style_border_color(left_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(left_btns[i], 1, 0);
    lv_obj_add_event_cb(left_btns[i], on_left_card_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    // Badge Angka 1..4
    lv_obj_t* num_bg_l = ui_make_card(left_btns[i], 8, 7, 24, 24, lv_color_hex(0x232938), lv_color_hex(0x232938), 6);
    lv_obj_clear_flag(num_bg_l, LV_OBJ_FLAG_CLICKABLE);
    char num_str_l[8];
    snprintf(num_str_l, sizeof(num_str_l), "%d", i + 1);
    lv_obj_t* num_lbl_l = ui_make_label(num_bg_l, num_str_l, &lv_font_montserrat_14, C_SUB);
    lv_obj_center(num_lbl_l);

    // Karakter Aksara Native Font
    int item_idx = active_item_indices[i];
    left_lbls[i] = ui_make_label(left_btns[i], items[item_idx].aksara, font_ak, C_TEXT);
    lv_obj_align(left_lbls[i], LV_ALIGN_CENTER, 8, 0);

    // --- Tombol Kanan (Latin) ---
    right_btns[i] = ui_make_card(view_match, 246, y_pos, 210, 38, C_CARD, C_CARD, 10);
    lv_obj_set_style_border_color(right_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(right_btns[i], 1, 0);
    lv_obj_add_event_cb(right_btns[i], on_right_card_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    // Badge Angka 5..8
    lv_obj_t* num_bg_r = ui_make_card(right_btns[i], 8, 7, 24, 24, lv_color_hex(0x232938), lv_color_hex(0x232938), 6);
    lv_obj_clear_flag(num_bg_r, LV_OBJ_FLAG_CLICKABLE);
    char num_str_r[8];
    snprintf(num_str_r, sizeof(num_str_r), "%d", i + 5);
    lv_obj_t* num_lbl_r = ui_make_label(num_bg_r, num_str_r, &lv_font_montserrat_14, C_SUB);
    lv_obj_center(num_lbl_r);

    // Bunyi Latin
    int r_idx = active_item_indices[right_shuffle[i]];
    right_lbls[i] = ui_make_label(right_btns[i], items[r_idx].latin, &lv_font_montserrat_14, C_TEXT);
    lv_obj_align(right_lbls[i], LV_ALIGN_CENTER, 8, 0);
  }

  // Baris Kemenangan Bawah (Disembunyikan awalnya)
  match_bottom_bar = ui_make_card(view_match, 0, 222, 480, 54, lv_color_hex(0x14532D), lv_color_hex(0x14532D), 0);
  lv_obj_set_style_border_color(match_bottom_bar, lv_color_hex(0x22C55E), 0);
  lv_obj_set_style_border_width(match_bottom_bar, 2, 0);
  lv_obj_add_flag(match_bottom_bar, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t* win_lbl = ui_make_label(match_bottom_bar, "Luar Biasa! Pasangan Cocok! +20 XP", &lv_font_montserrat_14, lv_color_hex(0x4ADE80));
  lv_obj_set_pos(win_lbl, 24, 18);

  lv_obj_t* btn_next = ui_make_card(match_bottom_bar, 340, 8, 120, 38, lv_color_hex(0x22C55E), lv_color_hex(0x16A34A), 10);
  ui_set_pressed(btn_next, lv_color_hex(0x15803D), lv_color_hex(0x15803D));
  lv_obj_add_event_cb(btn_next, on_match_next_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* next_lbl = ui_make_label(btn_next, "LANJUT " LV_SYMBOL_RIGHT, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(next_lbl);
}

static void build_stage_guess() {
  if (view_guess) {
    lv_obj_clean(view_guess);
  } else {
    view_guess = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_guess);
    lv_obj_set_size(view_guess, 480, 276);
    lv_obj_clear_flag(view_guess, LV_OBJ_FLAG_SCROLLABLE);
  }

  const AksaraItem* items = get_current_items();
  const lv_font_t* font_ak = get_current_font();

  guess_selected = -1;
  guess_checked = false;

  // Header Judul (Gambar 2: Translate / Tebak)
  lv_obj_t* title = ui_make_label(view_guess, "Tebak Aksara Ini", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 24, 6);

  // Maskot Avatar Kiri
  lv_obj_t* avatar_card = ui_make_card(view_guess, 24, 34, 52, 52, C_CARD, C_CARD, 26);
  lv_obj_clear_flag(avatar_card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* avatar_img = lv_image_create(avatar_card);
  lv_image_set_src(avatar_img, (current_script == AKSARA_JAWA) ? &BuddyJawa : &BuddyBali);
  lv_obj_center(avatar_img);

  // Balon Ucapan Maskot (Speech Bubble ala Duolingo Gambar 2)
  lv_obj_t* bubble = ui_make_card(view_guess, 86, 32, 370, 56, C_CARD, C_CARD, 16);
  lv_obj_set_style_border_color(bubble, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(bubble, 1, 0);

  // Tombol Audio
  lv_obj_t* btn_audio = ui_make_card(bubble, 10, 8, 40, 40, C_CARD_PR, C_CARD_PR, 20);
  ui_set_pressed(btn_audio, lv_color_hex(0x133857), lv_color_hex(0x133857));
  lv_obj_add_event_cb(btn_audio, on_audio_hint_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* ic_audio = ui_make_label(btn_audio, LV_SYMBOL_VOLUME_MAX, &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_center(ic_audio);

  lv_obj_t* prompt = ui_make_label(bubble, "Manakah aksara yang berbunyi:", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(prompt, 60, 10);

  // Target Kata Latin
  char target_str[24];
  snprintf(target_str, sizeof(target_str), "\" %s \"", items[guess_correct_idx].latin);
  lv_color_t accent_color = (current_script == AKSARA_JAWA) ? C_JAWA_1 : C_BALI_1;
  lv_obj_t* target_lbl = ui_make_label(bubble, target_str, &lv_font_montserrat_20, accent_color);
  lv_obj_set_pos(target_lbl, 60, 28);

  // 4 Pilihan Ganda (2x2 Grid)
  for (int i = 0; i < 4; i++) {
    int x_pos = (i % 2 == 0) ? 24 : 246;
    int y_pos = 96 + (i / 2) * 54;

    guess_btns[i] = ui_make_card(view_guess, x_pos, y_pos, 210, 46, C_CARD, C_CARD, 12);
    lv_obj_set_style_border_color(guess_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(guess_btns[i], 1, 0);
    ui_set_pressed(guess_btns[i], C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(guess_btns[i], on_guess_choice_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    int c_idx = guess_choices[i];
    guess_lbls[i] = ui_make_label(guess_btns[i], items[c_idx].aksara, font_ak, C_TEXT);
    lv_obj_center(guess_lbls[i]);
  }

  // Action Bar Bawah
  lv_obj_t* action_bar = ui_make_card(view_guess, 0, 218, 480, 58, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(action_bar, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(action_bar, 1, 0);

  guess_feedback_lbl = ui_make_label(action_bar, "Pilih salah satu aksara di atas", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(guess_feedback_lbl, 24, 20);

  guess_action_btn = ui_make_card(action_bar, 340, 10, 120, 38, lv_color_hex(0x3B82F6), lv_color_hex(0x2563EB), 10);
  ui_set_pressed(guess_action_btn, lv_color_hex(0x1D4ED8), lv_color_hex(0x1D4ED8));
  lv_obj_add_event_cb(guess_action_btn, on_guess_action_click, LV_EVENT_CLICKED, NULL);

  guess_action_lbl = ui_make_label(guess_action_btn, "PERIKSA", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(guess_action_lbl);
}

static void build_stage_complete() {
  if (view_complete) {
    lv_obj_clean(view_complete);
  } else {
    view_complete = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_complete);
    lv_obj_set_size(view_complete, 480, 276);
    lv_obj_clear_flag(view_complete, LV_OBJ_FLAG_SCROLLABLE);
  }

  // Maskot di tengah
  lv_obj_t* mascot_img = lv_image_create(view_complete);
  lv_image_set_src(mascot_img, &BuddyMaskot);
  lv_obj_align(mascot_img, LV_ALIGN_TOP_MID, 0, 6);

  // Judul Selesai
  lv_obj_t* title = ui_make_label(view_complete, "Hore! Pelajaran Selesai!", &lv_font_montserrat_20, lv_color_hex(0x22C55E));
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 134);

  lv_obj_t* sub = ui_make_label(view_complete, "Kamu telah menyelesaikan kuis aksara ini dengan sangat baik!",
                                &lv_font_montserrat_14, C_SUB);
  lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, 160);

  // Statistik Hasil
  lv_obj_t* stat_cont = ui_make_card(view_complete, 70, 184, 340, 36, C_CARD, C_CARD, 10);
  lv_obj_clear_flag(stat_cont, LV_OBJ_FLAG_CLICKABLE);

  char stat_xp_str[32];
  snprintf(stat_xp_str, sizeof(stat_xp_str), "⚡ +%d XP Diperoleh", earned_xp);
  lv_obj_t* stat1 = ui_make_label(stat_cont, stat_xp_str, &lv_font_montserrat_14, lv_color_hex(0xF59E0B));
  lv_obj_set_pos(stat1, 24, 10);

  lv_obj_t* stat2 = ui_make_label(stat_cont, "🎯 Akurasi Sempurna!", &lv_font_montserrat_14, lv_color_hex(0x22C55E));
  lv_obj_set_pos(stat2, 180, 10);

  // Tombol Selesai
  lv_obj_t* btn_finish = ui_make_card(view_complete, 110, 228, 260, 40, lv_color_hex(0x22C55E), lv_color_hex(0x16A34A), 12);
  ui_set_pressed(btn_finish, lv_color_hex(0x15803D), lv_color_hex(0x15803D));
  lv_obj_add_event_cb(btn_finish, on_finish_learn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* btn_lbl = ui_make_label(btn_finish, "KEMBALI KE PETA AKSARA", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(btn_lbl);
}

static void show_stage(int stage) {
  current_stage = stage;

  // Sembunyikan semua
  if (view_match) lv_obj_add_flag(view_match, LV_OBJ_FLAG_HIDDEN);
  if (view_guess) lv_obj_add_flag(view_guess, LV_OBJ_FLAG_HIDDEN);
  if (view_complete) lv_obj_add_flag(view_complete, LV_OBJ_FLAG_HIDDEN);

  if (stage == 0) {
    lv_bar_set_value(bar_progress, 33, LV_ANIM_ON);
    build_stage_match();
    lv_obj_clear_flag(view_match, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 1) {
    lv_bar_set_value(bar_progress, 66, LV_ANIM_ON);
    build_stage_guess();
    lv_obj_clear_flag(view_guess, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_bar_set_value(bar_progress, 100, LV_ANIM_ON);
    build_stage_complete();
    lv_obj_clear_flag(view_complete, LV_OBJ_FLAG_HIDDEN);
  }
}

// ---------- Inisialisasi Layar Pembelajaran ----------
void ui_learn_init(const AppCallbacks* callbacks) {
  learn_cbs = callbacks;

  scr_learn = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_learn, C_BG, 0);
  lv_obj_set_style_bg_opa(scr_learn, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr_learn, LV_OBJ_FLAG_SCROLLABLE);

  // Top Header Bar
  lv_obj_t* header = ui_make_card(scr_learn, 0, 0, 480, 44, C_BG, C_BG, 0);
  lv_obj_clear_flag(header, LV_OBJ_FLAG_CLICKABLE);

  // Tombol Close "X"
  lv_obj_t* btn_close = ui_make_card(header, 14, 6, 32, 32, C_CARD, C_CARD, 16);
  ui_set_pressed(btn_close, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btn_close, on_close_learn_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* ic_close = ui_make_label(btn_close, LV_SYMBOL_CLOSE, &lv_font_montserrat_14, C_SUB);
  lv_obj_center(ic_close);

  // Progress Bar ala Duolingo
  bar_progress = lv_bar_create(header);
  lv_obj_set_pos(bar_progress, 56, 17);
  lv_obj_set_size(bar_progress, 280, 10);
  lv_obj_set_style_radius(bar_progress, 5, 0);
  lv_obj_set_style_bg_color(bar_progress, lv_color_hex(0x232938), 0);
  lv_obj_set_style_bg_color(bar_progress, lv_color_hex(0x22C55E), LV_PART_INDICATOR);
  lv_obj_set_style_radius(bar_progress, 5, LV_PART_INDICATOR);
  lv_bar_set_range(bar_progress, 0, 100);
  lv_bar_set_value(bar_progress, 33, LV_ANIM_OFF);

  // XP Badge
  lv_obj_t* badge_card = ui_make_card(header, 350, 6, 116, 32, C_CARD, C_CARD, 16);
  lv_obj_clear_flag(badge_card, LV_OBJ_FLAG_CLICKABLE);
  lbl_badge_xp = ui_make_label(badge_card, "⚡ +0 XP", &lv_font_montserrat_14, lv_color_hex(0xF59E0B));
  lv_obj_center(lbl_badge_xp);

  // Container Konten Dinamis
  content_cont = lv_obj_create(scr_learn);
  lv_obj_remove_style_all(content_cont);
  lv_obj_set_pos(content_cont, 0, 44);
  lv_obj_set_size(content_cont, 480, 276);
  lv_obj_clear_flag(content_cont, LV_OBJ_FLAG_SCROLLABLE);
}

void ui_learn_start(AksaraType script, int level_index) {
  current_script = script;
  current_level_idx = level_index;
  earned_xp = 0;

  char b[24];
  snprintf(b, sizeof(b), "⚡ +%d XP", earned_xp);
  if (lbl_badge_xp) lv_label_set_text(lbl_badge_xp, b);

  // Pilih 4 item berurutan dari level
  int base_idx = (level_index * 2) % 6; // Agar selalu ada variasi per level
  for (int i = 0; i < PAIR_COUNT; i++) {
    active_item_indices[i] = (base_idx + i) % 10;
  }

  // Acak tombol kanan (shuffle deterministik agar seru)
  right_shuffle[0] = 2;
  right_shuffle[1] = 0;
  right_shuffle[2] = 3;
  right_shuffle[3] = 1;

  // Siapkan Stage 1 (Tebak Aksara)
  guess_correct_idx = active_item_indices[0];
  guess_choices[0] = active_item_indices[2];
  guess_choices[1] = active_item_indices[0]; // Jawaban benar di posisi 1
  guess_choices[2] = active_item_indices[3];
  guess_choices[3] = active_item_indices[1];

  // Mulai dari Stage 0 (Pasangan Aksara)
  show_stage(0);
  ui_learn_show();
}

void ui_learn_show() {
  if (scr_learn) {
    lv_scr_load_anim(scr_learn, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
  }
}

