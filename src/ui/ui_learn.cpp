#include "ui_learn.h"
#include "ui_jawa.h"
#include "ui_bali.h"
#include <stdio.h>
#include <string.h>
#include <esp_system.h>

// ---------- Data Aksara Hanacaraka Jawa & Bali ----------
struct AksaraItem {
  const char* latin;   // Transliterasi Latin bersih (Pure ASCII)
  const char* aksara;  // Karakter UTF-8 Native Aksara
};

// 20 Aksara Jawa Carakan (Nglegena Lengkap)
static const AksaraItem JAWA_ITEMS[20] = {
  { "ha", "\xEA\xA6\xB2" }, // ꦲ (0)
  { "na", "\xEA\xA6\xA4" }, // ꦤ (1)
  { "ca", "\xEA\xA6\x95" }, // ꦕ (2)
  { "ra", "\xEA\xA6\xAB" }, // ꦫ (3)
  { "ka", "\xEA\xA6\x8F" }, // ꦏ (4)
  { "da", "\xEA\xA6\xA2" }, // ꦢ (5)
  { "ta", "\xEA\xA6\xA0" }, // ꦠ (6)
  { "sa", "\xEA\xA6\xB1" }, // ꦱ (7)
  { "wa", "\xEA\xA6\xAE" }, // ꦮ (8)
  { "la", "\xEA\xA6\xAD" }, // ꦭ (9)
  { "pa", "\xEA\xA6\xA5" }, // ꦥ (10)
  { "dha", "\xEA\xA6\x9D" }, // ꦝ (11)
  { "ja", "\xEA\xA6\x97" }, // ꦗ (12)
  { "ya", "\xEA\xA6\xAA" }, // ꦪ (13)
  { "nya", "\xEA\xA6\x9A" }, // ꦚ (14)
  { "ma", "\xEA\xA6\xA9" }, // ꦩ (15)
  { "ga", "\xEA\xA6\x92" }, // ꦒ (16)
  { "ba", "\xEA\xA6\xA7" }, // ꦧ (17)
  { "tha", "\xEA\xA6\x9B" }, // ꦛ (18)
  { "nga", "\xEA\xA6\x94" }  // ꦔ (19)
};

// 18 Aksara Bali Wreastra Lengkap
static const AksaraItem BALI_ITEMS[18] = {
  { "ha", "\xE1\xAC\xB3" }, // ᬳ (0)
  { "na", "\xE1\xAC\xA6" }, // ᬦ (1)
  { "ca", "\xE1\xAC\x98" }, // ᬘ (2)
  { "ra", "\xE1\xAC\xAD" }, // ᬭ (3)
  { "ka", "\xE1\xAC\x93" }, // ᬓ (4)
  { "da", "\xE1\xAC\xA4" }, // ᬤ (5)
  { "ta", "\xE1\xAC\xA2" }, // ᬢ (6)
  { "sa", "\xE1\xAC\xB2" }, // ᬲ (7)
  { "wa", "\xE1\xAC\xAF" }, // ᬯ (8)
  { "la", "\xE1\xAC\xAE" }, // ᬮ (9)
  { "ma", "\xE1\xAC\xAB" }, // ᬫ (10)
  { "ga", "\xE1\xAC\x95" }, // ᬕ (11)
  { "ba", "\xE1\xAC\xA9" }, // ᬩ (12)
  { "nga", "\xE1\xAC\x97" }, // ᬗ (13)
  { "pa", "\xE1\xAC\xA7" }, // ᬧ (14)
  { "ja", "\xE1\xAC\x9A" }, // ᬚ (15)
  { "ya", "\xE1\xAC\xAC" }, // ᬬ (16)
  { "nya", "\xE1\xAC\x9C" }  // ᬜ (17)
};

// Daftar 20 Kata Bermakna (2 Suku Kata Terbuka) untuk Mode "Susun Kata"
struct WordEntry {
  const char* syl1;
  const char* syl2;
  const char* word_display;
  const char* meaning;
};

static const WordEntry WORD_BANK[20] = {
  { "ba", "ca", "ba - ca", "Membaca" },
  { "ma", "ta", "ma - ta", "Mata" },
  { "na", "ga", "na - ga", "Ular Naga" },
  { "ra", "ja", "ra - ja", "Raja" },
  { "ka", "ca", "ka - ca", "Cermin / Kaca" },
  { "sa", "ya", "sa - ya", "Diri Sendiri" },
  { "da", "ya", "da - ya", "Kekuatan" },
  { "la", "ra", "la - ra", "Sedih / Sakit" },
  { "pa", "da", "pa - da", "Sama / Setara" },
  { "ja", "la", "ja - la", "Jaring Ikan" },
  { "wa", "na", "wa - na", "Hutan Rimba" },
  { "ga", "ya", "ga - ya", "Penampilan" },
  { "ha", "na", "ha - na", "Ada / Berada" },
  { "ca", "ra", "ca - ra", "Metode / Jalan" },
  { "ta", "la", "ta - la", "Nada / Selaras" },
  { "ba", "ta", "ba - ta", "Batu Bata" },
  { "ma", "na", "ma - na", "Hati / Tempat" },
  { "sa", "pa", "sa - pa", "Menyapa" },
  { "ka", "ya", "ka - ya", "Makmur" },
  { "na", "ma", "na - ma", "Sebutan Diri" }
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
static lv_obj_t* lbl_stage_info = nullptr;
static lv_obj_t* lbl_badge_xp = nullptr;
static lv_obj_t* content_cont = nullptr;

static AksaraType current_script = AKSARA_JAWA;
static int current_level_idx = 0;
static int current_target_item_idx = 0;
// Stage:
// 0: Kartu Misteri & Konsep
// 1: Pasangkan Aksara (Match Pairs)
// 2: Susun Kata Aksara (Word Builder)
// 3: Lab Vokal Sandhangan Swara / Pangangge Suara
// 4: Kilat Benar / Salah (True or False)
// 5: Tebak Aksara (Multiple Choice)
// 6: Selesai (Complete)
// 8: Pengenalan Sandhangan Swara / Pangangge Suara (sebelum Lab Vokal)
// 9: Memori Kartu Aksara
// 10: Baca Kata Aksara
// 7: Menu Pilih Mode Pembelajaran
static int current_stage = 0;
static int earned_xp = 0;

// Sub-views
static lv_obj_t* view_lesson = nullptr;
static lv_obj_t* view_match = nullptr;
static lv_obj_t* view_word = nullptr;
static lv_obj_t* view_vowel = nullptr;
static lv_obj_t* view_tf = nullptr;
static lv_obj_t* view_guess = nullptr;
static lv_obj_t* view_complete = nullptr;
static lv_obj_t* view_mode_menu = nullptr;
static lv_obj_t* view_vowel_intro = nullptr;
static lv_obj_t* view_memory = nullptr;
static lv_obj_t* view_readword = nullptr;

// Stage 0: Lesson Slide Tracker
static int current_lesson_slide = 0;
static bool card_revealed[4] = { false, false, false, false };
static bool card_animating[4] = { false, false, false, false };
static lv_obj_t* btn_tall_next = nullptr;
static lv_obj_t* btn_start_practice = nullptr;

struct CardFlipCtx {
  int card_idx;
  lv_obj_t* card_inner;
  int target_w;
};
static CardFlipCtx flip_ctx[4];

// Stage 1: Match Pairs Data
static const int PAIR_COUNT = 4;
static int active_item_indices[4];
static int match_left_item[4];  // Urutan item kolom kiri (diacak tiap kali)
static int match_right_item[4]; // Urutan item kolom kanan (diacak tiap kali)
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

// Stage 2: Susun Kata (Word Builder) Data
static int current_word_idx = 0;
static int word_target_items[2] = { 0, 1 };
static int word_tile_items[6] = { 0, 1, 2, 3, 4, 5 };
static bool word_tile_used[6] = { false, false, false, false, false, false };
static int word_slot_tile_idx[2] = { -1, -1 }; // Menyimpan indeks tile (0..5) di tiap slot
static bool word_checked = false;
static lv_obj_t* word_slot_btns[2];
static lv_obj_t* word_slot_lbls[2];
static lv_obj_t* word_slot_sub_lbls[2];
static lv_obj_t* word_tile_btns[6];
static lv_obj_t* word_tile_lbls[6];
static lv_obj_t* word_feedback_lbl = nullptr;
static lv_obj_t* word_action_btn = nullptr;
static lv_obj_t* word_action_lbl = nullptr;

// Stage 3: Lab Vokal Sandhangan Swara / Pangangge Suara Data
// 0: Wulu/Ulu (-i), 1: Suku (-u), 2: Taling/Taleng (-é), 3: Pepet (-e), 4: Taling Tarung/Taleng Tedung (-o)
static int vowel_base_item_idx = 0;
static int vowel_target_type = 0; // 0..4
static int vowel_choice_types[4] = { 0, 1, 2, 4 };
static int vowel_selected = -1;
static bool vowel_checked = false;
static char vowel_choice_glyphs[4][24];
static lv_obj_t* vowel_btns[4];
static lv_obj_t* vowel_lbls[4];
static lv_obj_t* vowel_feedback_lbl = nullptr;
static lv_obj_t* vowel_action_btn = nullptr;
static lv_obj_t* vowel_action_lbl = nullptr;

// Stage 4: Kilat Benar / Salah (True or False) Data
static int tf_shown_item_idx = 0;
static int tf_shown_latin_idx = 0;
static bool tf_is_true_pair = true;
static bool tf_answered = false;
static lv_obj_t* tf_btn_false = nullptr;
static lv_obj_t* tf_btn_true = nullptr;
static lv_obj_t* tf_bottom_bar = nullptr;
static lv_obj_t* tf_feedback_lbl = nullptr;

// Stage 9: Memori Kartu Aksara Data (8 kartu = 4 pasang aksara <-> latin)
static int mem_card_item[8];
static int mem_card_kind[8]; // 0 = aksara, 1 = latin
static bool mem_face_up[8];
static bool mem_matched[8];
static int mem_first = -1;
static bool mem_lock = false;
static int mem_pairs_found = 0;
static int mem_moves = 0;
static int mem_pending_a = -1;
static int mem_pending_b = -1;
static uint32_t mem_gen = 0;
static lv_obj_t* mem_btns[8];
static lv_obj_t* mem_lbls[8];
static lv_obj_t* mem_moves_lbl = nullptr;
static lv_obj_t* mem_bottom_bar = nullptr;

// Stage 10: Baca Kata Aksara Data
static int rw_word_idx = 0;
static int rw_opts[4];
static bool rw_answered = false;
static lv_obj_t* rw_btns[4];
static lv_obj_t* rw_lbls[4];
static lv_obj_t* rw_bottom_bar = nullptr;
static lv_obj_t* rw_feedback_lbl = nullptr;

// Stage 5: Guess Data (Multiple Choice)
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
static void build_stage_lesson();
static void build_stage_match();
static void build_stage_word();
static void build_stage_vowel();
static void build_stage_tf();
static void build_stage_guess();
static void build_stage_complete();
static void build_stage_mode_menu();
static void build_stage_vowel_intro();
static void build_stage_memory();
static void build_stage_readword();
static void show_stage(int stage);
static void update_lesson_slide();
static void render_mystery_card_content(lv_obj_t* parent, int card_idx);
static void render_revealed_card_content(lv_obj_t* parent, int card_idx);
static void update_cards_action_state();

// ---------- Helper Font & Data Aksara ----------
static const lv_font_t* get_current_font() {
  return (current_script == AKSARA_JAWA) ? &font_jawa_22 : &font_bali_22;
}

static const lv_font_t* get_current_font_jumbo() {
  return (current_script == AKSARA_JAWA) ? &font_jawa_38 : &font_bali_38;
}

static const AksaraItem* get_current_items() {
  return (current_script == AKSARA_JAWA) ? JAWA_ITEMS : BALI_ITEMS;
}

static int get_total_items() {
  return (current_script == AKSARA_JAWA) ? 20 : 18;
}

static int map_level_to_item_index(int level_idx) {
  int max_items = get_total_items();
  if (level_idx < 0) return 0;
  if (level_idx >= max_items) return max_items - 1;
  return level_idx;
}

static int find_item_by_latin(const char* lat) {
  const AksaraItem* items = get_current_items();
  int n = get_total_items();
  for (int i = 0; i < n; i++) {
    if (strcmp(items[i].latin, lat) == 0) return i;
  }
  return 0;
}

static void add_xp(int amount) {
  earned_xp += amount;
  if (lbl_badge_xp) {
    char b[24];
    snprintf(b, sizeof(b), LV_SYMBOL_CHARGE " +%d XP", earned_xp);
    lv_label_set_text(lbl_badge_xp, b);
  }
}

// Helper merakit string UTF-8 Aksara + Sandhangan Swara (Jawa) / Pangangge Suara (Bali)
// vowel_type: 0 = i (Wulu/Ulu), 1 = u (Suku), 2 = é (Taling/Taleng), 3 = e (Pepet), 4 = o (Taling Tarung/Taleng Tedung)
static void build_vowel_glyph_utf8(int item_idx, int vowel_type, char* out_buf, size_t buf_size) {
  const AksaraItem* items = get_current_items();
  const char* base = items[item_idx].aksara;

  if (current_script == AKSARA_JAWA) {
    if (vowel_type == 0) {
      // Wulu (i) -> U+A9B6 (\xEA\xA6\xB6)
      snprintf(out_buf, buf_size, "%s\xEA\xA6\xB6", base);
    } else if (vowel_type == 1) {
      // Suku (u) -> U+A9B8 (\xEA\xA6\xB8)
      snprintf(out_buf, buf_size, "%s\xEA\xA6\xB8", base);
    } else if (vowel_type == 2) {
      // Taling (é) -> U+A9BA (\xEA\xA6\xBA) ditaruh di depan konsonan secara visual
      snprintf(out_buf, buf_size, "\xEA\xA6\xBA%s", base);
    } else if (vowel_type == 3) {
      // Pepet (e) -> U+A9BC (\xEA\xA6\xBC)
      snprintf(out_buf, buf_size, "%s\xEA\xA6\xBC", base);
    } else {
      // Taling Tarung (o) -> Taling (\xEA\xA6\xBA) + Konsonan + Tarung (\xEA\xA6\xB4)
      snprintf(out_buf, buf_size, "\xEA\xA6\xBA%s\xEA\xA6\xB4", base);
    }
  } else {
    if (vowel_type == 0) {
      // Ulu (i) -> U+1B36 (\xE1\xAC\xB6)
      snprintf(out_buf, buf_size, "%s\xE1\xAC\xB6", base);
    } else if (vowel_type == 1) {
      // Suku (u) -> U+1B38 (\xE1\xAC\xB8)
      snprintf(out_buf, buf_size, "%s\xE1\xAC\xB8", base);
    } else if (vowel_type == 2) {
      // Taleng (é) -> U+1B3E (\xE1\xAC\xBE) ditaruh di depan konsonan secara visual
      snprintf(out_buf, buf_size, "\xE1\xAC\xBE%s", base);
    } else if (vowel_type == 3) {
      // Pepet (e) -> U+1B42 (\xE1\xAD\x82)
      snprintf(out_buf, buf_size, "%s\xE1\xAD\x82", base);
    } else {
      // Taleng Tedung (o) -> Taleng (\xE1\xAC\xBE) + Konsonan + Tedung (\xE1\xAC\xB5)
      snprintf(out_buf, buf_size, "\xE1\xAC\xBE%s\xE1\xAC\xB5", base);
    }
  }
}

// Helper mengubah bunyi konsonan dasar (misal "ha", "na", "nya") menjadi vokal tertentu ("hi", "nu", "nyo")
static void build_vowel_latin_str(int item_idx, int vowel_type, char* out_buf, size_t buf_size) {
  const AksaraItem* items = get_current_items();
  const char* base = items[item_idx].latin;
  size_t len = strlen(base);
  char stem[8];
  if (len > 1 && base[len - 1] == 'a') {
    memcpy(stem, base, len - 1);
    stem[len - 1] = '\0';
  } else {
    snprintf(stem, sizeof(stem), "%s", base);
  }

  const char* v_char = "a";
  if (vowel_type == 0) v_char = "i";
  else if (vowel_type == 1) v_char = "u";
  else if (vowel_type == 2) v_char = "e (terang)";
  else if (vowel_type == 3) v_char = "e (pepet)";
  else if (vowel_type == 4) v_char = "o";

  snprintf(out_buf, buf_size, "%s%s", stem, v_char);
}

static const char* get_vowel_diacritic_name(int vowel_type) {
  if (current_script == AKSARA_JAWA) {
    switch (vowel_type) {
      case 0: return "Wulu (vokal i)";
      case 1: return "Suku (vokal u)";
      case 2: return "Taling (vokal e)";
      case 3: return "Pepet (vokal e)";
      default: return "Taling Tarung (vokal o)";
    }
  } else {
    switch (vowel_type) {
      case 0: return "Ulu (vokal i)";
      case 1: return "Suku (vokal u)";
      case 2: return "Taleng (vokal e)";
      case 3: return "Pepet (vokal e)";
      default: return "Taleng Tedung (vokal o)";
    }
  }
}

// ---------- Helper Acak (Random) ----------
static int rnd_n(int n) {
  return (n <= 1) ? 0 : (int)(esp_random() % (uint32_t)n);
}

static void shuffle_ints(int* a, int n) {
  for (int i = n - 1; i > 0; i--) {
    int j = rnd_n(i + 1);
    int t = a[i];
    a[i] = a[j];
    a[j] = t;
  }
}

// Pilih `count` item unik secara acak dari `total` item aksara
static void pick_random_unique_items(int* out, int count, int total) {
  int pool[20];
  if (total > 20) total = 20;
  for (int i = 0; i < total; i++) pool[i] = i;
  shuffle_ints(pool, total);
  for (int i = 0; i < count && i < total; i++) out[i] = pool[i];
}

// ---------- Preparation Data Setiap Stage ----------
static void prepare_session_stages() {
  int total_items = get_total_items();
  int base_idx = current_target_item_idx % total_items;

  // Kartu 1..4 = materi node ini (berurutan), seluruh soal di bawah diacak dari 4 item ini
  for (int i = 0; i < PAIR_COUNT; i++) {
    active_item_indices[i] = (base_idx + i) % total_items;
  }

  // Stage 2: Susun Kata (kata acak, posisi keping acak)
  current_word_idx = rnd_n(20);
  word_target_items[0] = find_item_by_latin(WORD_BANK[current_word_idx].syl1);
  word_target_items[1] = find_item_by_latin(WORD_BANK[current_word_idx].syl2);

  int raw_tiles[6];
  raw_tiles[0] = word_target_items[0];
  raw_tiles[1] = word_target_items[1];
  int pool[20];
  int pool_n = 0;
  for (int i = 0; i < total_items; i++) {
    if (i != word_target_items[0] && i != word_target_items[1]) pool[pool_n++] = i;
  }
  shuffle_ints(pool, pool_n);
  for (int i = 0; i < 4; i++) raw_tiles[2 + i] = pool[i];
  shuffle_ints(raw_tiles, 6);
  for (int i = 0; i < 6; i++) word_tile_items[i] = raw_tiles[i];

  // Stage 3: Lab Vokal (konsonan, target, dan posisi jawaban acak)
  vowel_base_item_idx = active_item_indices[rnd_n(PAIR_COUNT)];
  int v_types[4] = { 0, 1, 3, 4 }; // i, u, e(pepet), o
  shuffle_ints(v_types, 4);
  vowel_target_type = v_types[rnd_n(4)];
  for (int i = 0; i < 4; i++) {
    vowel_choice_types[i] = v_types[i];
    build_vowel_glyph_utf8(vowel_base_item_idx, vowel_choice_types[i],
                           vowel_choice_glyphs[i], sizeof(vowel_choice_glyphs[i]));
  }

  // Stage 4: Benar / Salah (item & status benar/salah acak)
  tf_shown_item_idx = active_item_indices[rnd_n(PAIR_COUNT)];
  tf_is_true_pair = (rnd_n(2) == 0);
  if (tf_is_true_pair) {
    tf_shown_latin_idx = tf_shown_item_idx;
  } else {
    int j;
    do {
      j = active_item_indices[rnd_n(PAIR_COUNT)];
    } while (j == tf_shown_item_idx);
    tf_shown_latin_idx = j;
  }

  // Stage 5: Tebak Aksara (target & posisi jawaban acak)
  guess_correct_idx = active_item_indices[rnd_n(PAIR_COUNT)];
  int ch[4];
  for (int i = 0; i < 4; i++) ch[i] = active_item_indices[i];
  shuffle_ints(ch, 4);
  for (int i = 0; i < 4; i++) guess_choices[i] = ch[i];
}

// ---------- Event: Tutup / Kembali & Mode Menu ----------
static void on_close_learn_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  if (current_script == AKSARA_JAWA) {
    ui_jawa_show();
  } else {
    ui_bali_show();
  }
}

static void on_open_mode_menu_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  show_stage(7); // Buka Menu Pilih Mode Pembelajaran
}

static void on_select_mode_card_click(lv_event_t* e) {
  int target_stage = (int)(intptr_t)lv_event_get_user_data(e);
  audio_play(SOUND_NODE);
  show_stage(target_stage);
}

// ---------- Stage 0: Event Handlers Materi Pembelajaran ----------
static void on_lesson_goto_slide(lv_event_t* e) {
  int target_slide = (int)(intptr_t)lv_event_get_user_data(e);
  audio_play(SOUND_CLICK);
  current_lesson_slide = target_slide;
  update_lesson_slide();
}

static void on_tall_next_click(lv_event_t* e) {
  (void)e;
  if (!card_revealed[0] || !card_revealed[1] || !card_revealed[2]) {
    audio_play(SOUND_ERROR);
    ui_show_toast("Buka semua 3 kartu di atas terlebih dahulu!");
    return;
  }
  audio_play(SOUND_CLICK);
  current_lesson_slide = (current_level_idx == 0) ? 5 : 1;
  update_lesson_slide();
}

static void on_start_practice_click(lv_event_t* e) {
  (void)e;
  if (!card_revealed[3]) {
    audio_play(SOUND_ERROR);
    ui_show_toast("Ketuk kartu ke-4 untuk membukanya!");
    return;
  }
  audio_play(SOUND_CLICK);
  show_stage(1); // Masuk ke Stage 1 (Match Pairs)
}

// ---------- Animasi Flip Kartu Interaktif ----------
static void card_flip_anim_exec_cb(void* var, int32_t val) {
  lv_obj_set_width((lv_obj_t*)var, val);
  lv_obj_center((lv_obj_t*)var);
}

static void card_flip_phase2_ready_cb(lv_anim_t* a) {
  CardFlipCtx* ctx = (CardFlipCtx*)a->user_data;
  card_animating[ctx->card_idx] = false;
  audio_play(SOUND_REWARD);

  update_cards_action_state();

  bool is_slide_cards_123 = (current_level_idx == 0) ? (current_lesson_slide == 4) : (current_lesson_slide == 0);
  bool is_slide_cards_4 = (current_level_idx == 0) ? (current_lesson_slide == 5) : (current_lesson_slide == 1);

  if (is_slide_cards_123) {
    if (card_revealed[0] && card_revealed[1] && card_revealed[2]) {
      audio_play(SOUND_SUCCESS);
      ui_show_toast("Sempurna! Semua 3 kartu terbuka. Tekan LANJUT.");
    }
  } else if (is_slide_cards_4) {
    if (card_revealed[3]) {
      audio_play(SOUND_SUCCESS);
      ui_show_toast("Aksara #4 Terbuka! Tekan LATIHAN untuk mulai kuis.");
    }
  }
}

static void card_flip_phase1_ready_cb(lv_anim_t* a) {
  CardFlipCtx* ctx = (CardFlipCtx*)a->user_data;

  lv_obj_clean(ctx->card_inner);
  render_revealed_card_content(ctx->card_inner, ctx->card_idx);

  lv_obj_set_style_border_color(ctx->card_inner, (current_script == AKSARA_JAWA) ? C_JAWA_1 : C_BALI_1, 0);
  lv_obj_set_style_border_width(ctx->card_inner, 2, 0);

  lv_anim_t a2;
  lv_anim_init(&a2);
  lv_anim_set_var(&a2, ctx->card_inner);
  lv_anim_set_values(&a2, 6, ctx->target_w);
  lv_anim_set_duration(&a2, 110);
  lv_anim_set_exec_cb(&a2, card_flip_anim_exec_cb);
  lv_anim_set_completed_cb(&a2, card_flip_phase2_ready_cb);
  lv_anim_set_user_data(&a2, ctx);
  lv_anim_start(&a2);
}

static void on_interactive_card_click(lv_event_t* e) {
  int card_idx = (int)(intptr_t)lv_event_get_user_data(e);
  if (card_animating[card_idx]) return;

  if (card_revealed[card_idx]) {
    audio_play(SOUND_NODE);
    const AksaraItem* items = get_current_items();
    int item_idx = active_item_indices[card_idx];
    char msg[32];
    snprintf(msg, sizeof(msg), "Aksara: %s", items[item_idx].latin);
    ui_show_toast(msg);
    return;
  }

  card_revealed[card_idx] = true;
  card_animating[card_idx] = true;
  audio_play(SOUND_CLICK);

  CardFlipCtx* ctx = &flip_ctx[card_idx];

  lv_anim_t a1;
  lv_anim_init(&a1);
  lv_anim_set_var(&a1, ctx->card_inner);
  lv_anim_set_values(&a1, ctx->target_w, 6);
  lv_anim_set_duration(&a1, 110);
  lv_anim_set_exec_cb(&a1, card_flip_anim_exec_cb);
  lv_anim_set_completed_cb(&a1, card_flip_phase1_ready_cb);
  lv_anim_set_user_data(&a1, ctx);
  lv_anim_start(&a1);
}

static void render_mystery_card_content(lv_obj_t* parent, int card_idx) {
  lv_obj_t* pill = ui_make_card(parent, 10, 10, 32, 22, lv_color_hex(0x232938), lv_color_hex(0x232938), 6);
  lv_obj_clear_flag(pill, LV_OBJ_FLAG_CLICKABLE);
  char n_str[8];
  snprintf(n_str, sizeof(n_str), "#%d", card_idx + 1);
  lv_obj_t* n_lbl = ui_make_label(pill, n_str, &lv_font_montserrat_14, lv_color_hex(0xF59E0B));
  lv_obj_center(n_lbl);

  lv_obj_t* circle = ui_make_card(parent, 0, 0, 68, 68, lv_color_hex(0x1F293D), lv_color_hex(0x1F293D), 34);
  lv_obj_set_style_border_color(circle, lv_color_hex(0x38BDF8), 0);
  lv_obj_set_style_border_width(circle, 2, 0);
  lv_obj_clear_flag(circle, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_center(circle);

  lv_obj_t* q_lbl = ui_make_label(circle, "?", &lv_font_montserrat_28, lv_color_hex(0x38BDF8));
  lv_obj_center(q_lbl);

  lv_obj_t* tap_lbl = ui_make_label(parent, "Ketuk Buka", &lv_font_montserrat_14, lv_color_hex(0xF2F4F8));
  lv_obj_align(tap_lbl, LV_ALIGN_BOTTOM_MID, 0, -28);

  lv_obj_t* sub_lbl = ui_make_label(parent, "Sentuh kartu", &lv_font_montserrat_14, lv_color_hex(0x8A93A6));
  lv_obj_align(sub_lbl, LV_ALIGN_BOTTOM_MID, 0, -10);
}

static void render_revealed_card_content(lv_obj_t* parent, int card_idx) {
  const AksaraItem* items = get_current_items();
  const lv_font_t* font_ak_jumbo = get_current_font_jumbo();
  int item_idx = active_item_indices[card_idx];

  lv_obj_t* pill = ui_make_card(parent, 10, 10, 32, 22, lv_color_hex(0x232938), lv_color_hex(0x232938), 6);
  lv_obj_clear_flag(pill, LV_OBJ_FLAG_CLICKABLE);
  char n_str[8];
  snprintf(n_str, sizeof(n_str), "#%d", card_idx + 1);
  lv_obj_t* n_lbl = ui_make_label(pill, n_str, &lv_font_montserrat_14, lv_color_hex(0xF59E0B));
  lv_obj_center(n_lbl);

  lv_obj_t* ak_lbl = ui_make_label(parent, items[item_idx].aksara, font_ak_jumbo, C_TEXT);
  lv_obj_align(ak_lbl, LV_ALIGN_TOP_MID, 0, 44);

  lv_obj_t* line = ui_make_card(parent, 16, 122, 84, 1, lv_color_hex(0x2A3245), lv_color_hex(0x2A3245), 0);
  lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t* lat_lbl = ui_make_label(parent, items[item_idx].latin, &lv_font_montserrat_28, (current_script == AKSARA_JAWA) ? C_JAWA_1 : C_BALI_1);
  lv_obj_align(lat_lbl, LV_ALIGN_TOP_MID, 0, 136);

  lv_obj_t* sub_lbl = ui_make_label(parent, "Aksara", &lv_font_montserrat_14, C_SUB);
  lv_obj_align(sub_lbl, LV_ALIGN_TOP_MID, 0, 186);
}

static void update_cards_action_state() {
  bool is_slide_cards_123 = (current_level_idx == 0) ? (current_lesson_slide == 4) : (current_lesson_slide == 0);
  bool is_slide_cards_4 = (current_level_idx == 0) ? (current_lesson_slide == 5) : (current_lesson_slide == 1);

  if (is_slide_cards_123 && btn_tall_next) {
    bool unlocked = card_revealed[0] && card_revealed[1] && card_revealed[2];
    if (unlocked) {
      lv_obj_set_style_bg_color(btn_tall_next, lv_color_hex(0x2563EB), 0);
      lv_obj_set_style_bg_grad_color(btn_tall_next, lv_color_hex(0x1D4ED8), 0);
      lv_obj_set_style_border_color(btn_tall_next, lv_color_hex(0x3B82F6), 0);
    } else {
      lv_obj_set_style_bg_color(btn_tall_next, lv_color_hex(0x1A202C), 0);
      lv_obj_set_style_bg_grad_color(btn_tall_next, lv_color_hex(0x141824), 0);
      lv_obj_set_style_border_color(btn_tall_next, lv_color_hex(0x2D3748), 0);
    }
  } else if (is_slide_cards_4 && btn_start_practice) {
    bool unlocked = card_revealed[3];
    if (unlocked) {
      lv_obj_set_style_bg_color(btn_start_practice, lv_color_hex(0x22C55E), 0);
      lv_obj_set_style_bg_grad_color(btn_start_practice, lv_color_hex(0x16A34A), 0);
      lv_obj_set_style_border_color(btn_start_practice, lv_color_hex(0x4ADE80), 0);
    } else {
      lv_obj_set_style_bg_color(btn_start_practice, lv_color_hex(0x1A202C), 0);
      lv_obj_set_style_bg_grad_color(btn_start_practice, lv_color_hex(0x141824), 0);
      lv_obj_set_style_border_color(btn_start_practice, lv_color_hex(0x2D3748), 0);
    }
  }
}

static void render_single_concept_slide(lv_obj_t* parent, int concept_idx) {
  struct SingleConcept {
    const char* title;
    const char* detail;
    const char* note;
    lv_color_t theme_color;
  };

  SingleConcept concepts_jawa[4] = {
    {
      "1. Aksara Carakan / Nglegena",
      "Aksara dasar yang secara alami berbunyi konsonan dan vokal 'a' utuh (Ha, Na, Ca, Ra, Ka, Da, Ta, Sa, Wa, La, dst).",
      "Setiap aksara sudah memiliki bunyi utuh tanpa memerlukan sandhangan tambahan apapun.",
      lv_color_hex(0xF59E0B)
    },
    {
      "2. Sandhangan Swara & Panyigeg",
      "Tanda yang dipasang untuk mengubah vokal 'a' menjadi vokal lain: Wulu (i), Suku (u), Taling (e), Taling Tarung (o), dan Pepet (e).",
      "Serta tanda pemati suku kata: Layar (r), Cecak (ng), Wignyan (h), dan Pangkon (pemati umum).",
      lv_color_hex(0x10B981)
    },
    {
      "3. Pasangan Aksara",
      "Bentuk khusus aksara yang berfungsi untuk mematikan vokal suku kata sebelumnya ketika ada dua konsonan berurutan.",
      "Masing-masing dari 20 aksara Carakan memiliki bentuk pasangan spesifik (ditulis di bawah/samping).",
      lv_color_hex(0x38BDF8)
    },
    {
      "4. Aksara Rekan & Murda",
      "Aksara Rekan digunakan untuk melafalkan kata serapan asing (kh, f, dz, z, gh).",
      "Aksara Murda berfungsi khusus sebagai huruf kapital untuk penulisan nama orang terhormat, gelar, dan tempat.",
      lv_color_hex(0xA855F7)
    }
  };

  SingleConcept concepts_bali[4] = {
    {
      "1. Aksara Wreastra",
      "Aksara pokok yang digunakan dalam penulisan bahasa Bali lumrah sehari-hari (Ha, Na, Ca, Ra, Ka, Da, Ta, Sa, Wa, La, dst).",
      "Secara alami setiap aksara Wreastra sudah berbunyi vokal 'a' secara mandiri dan utuh.",
      lv_color_hex(0xF59E0B)
    },
    {
      "2. Pangangge Suara & Tengenan",
      "Tanda yang dipasang untuk mengubah vokal: Ulu (i), Suku (u), Taleng (e), Taleng Tedung (o), dan Pepet (e).",
      "Serta tanda penutup konsonan (Tengenan): Surang (r), Cecek (ng), Bisah (h), dan Adeg-adeg (pemati).",
      lv_color_hex(0x10B981)
    },
    {
      "3. Gantungan & Gempelan",
      "Bentuk aksara gantung (Gantungan di bawah) dan tempel (Gempelan di kanan) untuk meredam vokal konsonan sebelumnya.",
      "Digunakan saat dua konsonan bertemu berturut-turut tanpa jeda vokal.",
      lv_color_hex(0x38BDF8)
    },
    {
      "4. Aksara Swalalita & Modre",
      "Aksara Swalalita digunakan khusus untuk penulisan kidung sastra Kawi, Jawa Kuno, dan Sansekerta.",
      "Aksara Modre digunakan dalam konteks keagamaan, doa suci, rajah, dan lambang mistis ke-Tuhanan.",
      lv_color_hex(0xA855F7)
    }
  };

  SingleConcept* cur = (current_script == AKSARA_JAWA) ? &concepts_jawa[concept_idx] : &concepts_bali[concept_idx];

  lv_obj_t* title = ui_make_label(parent, cur->title, &lv_font_montserrat_20, cur->theme_color);
  lv_obj_set_pos(title, 16, 12);

  lv_obj_t* main_card = ui_make_card(parent, 16, 44, 448, 156, C_CARD, C_CARD, 16);
  lv_obj_set_style_border_color(main_card, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(main_card, 1, 0);
  lv_obj_clear_flag(main_card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_flex_flow(main_card, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(main_card, 16, 0);
  lv_obj_set_style_pad_row(main_card, 12, 0);

  lv_obj_t* d_lbl = ui_make_label(main_card, cur->detail, &lv_font_montserrat_16, lv_color_hex(0xF3F4F6));
  lv_obj_set_width(d_lbl, 416);
  lv_label_set_long_mode(d_lbl, LV_LABEL_LONG_WRAP);

  lv_obj_t* n_lbl = ui_make_label(main_card, cur->note, &lv_font_montserrat_16, lv_color_hex(0x94A3B8));
  lv_obj_set_width(n_lbl, 416);
  lv_label_set_long_mode(n_lbl, LV_LABEL_LONG_WRAP);

  lv_obj_t* bar = ui_make_card(parent, 0, 216, 480, 60, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(bar, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(bar, 1, 0);

  if (concept_idx > 0) {
    lv_obj_t* btn_prev = ui_make_card(bar, 16, 9, 134, 42, lv_color_hex(0x1E2433), lv_color_hex(0x1E2433), 12);
    lv_obj_set_style_border_color(btn_prev, lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(btn_prev, 1, 0);
    ui_set_pressed(btn_prev, lv_color_hex(0x141824), lv_color_hex(0x141824));
    lv_obj_add_event_cb(btn_prev, on_lesson_goto_slide, LV_EVENT_CLICKED, (void*)(intptr_t)(concept_idx - 1));
    lv_obj_t* lbl_prev = ui_make_label(btn_prev, LV_SYMBOL_LEFT " SEBELUM", &lv_font_montserrat_14, C_TEXT);
    lv_obj_center(lbl_prev);
  }

  lv_obj_t* btn_next = ui_make_card(bar, 330, 9, 134, 42, lv_color_hex(0x3B82F6), lv_color_hex(0x2563EB), 12);
  ui_set_pressed(btn_next, lv_color_hex(0x1D4ED8), lv_color_hex(0x1D4ED8));
  lv_obj_add_event_cb(btn_next, on_lesson_goto_slide, LV_EVENT_CLICKED, (void*)(intptr_t)(concept_idx + 1));

  const char* next_str = (concept_idx < 3) ? "LANJUT " LV_SYMBOL_RIGHT : "KARTU " LV_SYMBOL_RIGHT;
  lv_obj_t* lbl_next = ui_make_label(btn_next, next_str, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(lbl_next);
}

static void render_slide_cards_123(lv_obj_t* parent, bool show_back_concept) {
  if (show_back_concept) {
    lv_obj_t* btn_back = ui_make_card(parent, 16, 6, 88, 26, lv_color_hex(0x1E2433), lv_color_hex(0x1E2433), 6);
    ui_set_pressed(btn_back, lv_color_hex(0x141824), lv_color_hex(0x141824));
    lv_obj_add_event_cb(btn_back, on_lesson_goto_slide, LV_EVENT_CLICKED, (void*)(intptr_t)3);
    lv_obj_t* lbl_back = ui_make_label(btn_back, LV_SYMBOL_LEFT " Konsep", &lv_font_montserrat_14, lv_color_hex(0x8A93A6));
    lv_obj_center(lbl_back);
  }

  int badge_x = show_back_concept ? 114 : 16;
  lv_obj_t* badge = ui_make_card(parent, badge_x, 6, 210, 26, lv_color_hex(0x133857), lv_color_hex(0x133857), 6);
  lv_obj_clear_flag(badge, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* b_lbl = ui_make_label(badge, LV_SYMBOL_FILE " KARTU AKSARA (1/2)", &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_center(b_lbl);

  for (int i = 0; i < 3; i++) {
    int x_pos = 16 + (i * 124);

    lv_obj_t* card_outer = ui_make_card(parent, x_pos, 38, 116, 224, C_BG, C_BG, 16);
    lv_obj_clear_flag(card_outer, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* card_inner = ui_make_card(card_outer, 0, 0, 116, 224, C_CARD, C_CARD, 16);
    lv_obj_set_style_border_color(card_inner, card_revealed[i] ? ((current_script == AKSARA_JAWA) ? C_JAWA_1 : C_BALI_1) : lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(card_inner, card_revealed[i] ? 2 : 1, 0);
    ui_set_pressed(card_inner, C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(card_inner, on_interactive_card_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    lv_obj_center(card_inner);

    flip_ctx[i].card_idx = i;
    flip_ctx[i].card_inner = card_inner;
    flip_ctx[i].target_w = 116;

    if (card_revealed[i]) {
      render_revealed_card_content(card_inner, i);
    } else {
      render_mystery_card_content(card_inner, i);
    }
  }

  bool unlocked = card_revealed[0] && card_revealed[1] && card_revealed[2];
  btn_tall_next = ui_make_card(parent, 388, 38, 76, 224,
                               unlocked ? lv_color_hex(0x2563EB) : lv_color_hex(0x1A202C),
                               unlocked ? lv_color_hex(0x1D4ED8) : lv_color_hex(0x141824), 16);
  lv_obj_set_style_border_color(btn_tall_next, unlocked ? lv_color_hex(0x3B82F6) : lv_color_hex(0x2D3748), 0);
  lv_obj_set_style_border_width(btn_tall_next, 1, 0);
  ui_set_pressed(btn_tall_next, lv_color_hex(0x1E40AF), lv_color_hex(0x1E40AF));
  lv_obj_add_event_cb(btn_tall_next, on_tall_next_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* ic_tall = ui_make_label(btn_tall_next, LV_SYMBOL_RIGHT, &lv_font_montserrat_20, lv_color_hex(0xFFFFFF));
  lv_obj_align(ic_tall, LV_ALIGN_CENTER, 0, -12);

  lv_obj_t* lbl_tall = ui_make_label(btn_tall_next, "LANJUT", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_align(lbl_tall, LV_ALIGN_CENTER, 0, 18);
}

static void render_slide_cards_4_and_start(lv_obj_t* parent, int prev_slide_target) {
  lv_obj_t* badge = ui_make_card(parent, 16, 6, 210, 26, lv_color_hex(0x133857), lv_color_hex(0x133857), 6);
  lv_obj_clear_flag(badge, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* b_lbl = ui_make_label(badge, LV_SYMBOL_FILE " KARTU AKSARA (2/2)", &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_center(b_lbl);

  lv_obj_t* btn_tall_back = ui_make_card(parent, 16, 38, 76, 224, lv_color_hex(0x1E2433), lv_color_hex(0x1E2433), 16);
  lv_obj_set_style_border_color(btn_tall_back, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(btn_tall_back, 1, 0);
  ui_set_pressed(btn_tall_back, lv_color_hex(0x141824), lv_color_hex(0x141824));
  lv_obj_add_event_cb(btn_tall_back, on_lesson_goto_slide, LV_EVENT_CLICKED, (void*)(intptr_t)prev_slide_target);

  lv_obj_t* ic_back = ui_make_label(btn_tall_back, LV_SYMBOL_LEFT, &lv_font_montserrat_20, C_TEXT);
  lv_obj_align(ic_back, LV_ALIGN_CENTER, 0, -12);

  lv_obj_t* lbl_back = ui_make_label(btn_tall_back, "KEMBALI", &lv_font_montserrat_14, C_TEXT);
  lv_obj_align(lbl_back, LV_ALIGN_CENTER, 0, 18);

  lv_obj_t* card_outer4 = ui_make_card(parent, 102, 38, 140, 224, C_BG, C_BG, 16);
  lv_obj_clear_flag(card_outer4, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t* card_inner4 = ui_make_card(card_outer4, 0, 0, 140, 224, C_CARD, C_CARD, 16);
  lv_obj_set_style_border_color(card_inner4, card_revealed[3] ? ((current_script == AKSARA_JAWA) ? C_JAWA_1 : C_BALI_1) : lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(card_inner4, card_revealed[3] ? 2 : 1, 0);
  ui_set_pressed(card_inner4, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(card_inner4, on_interactive_card_click, LV_EVENT_CLICKED, (void*)(intptr_t)3);
  lv_obj_center(card_inner4);

  flip_ctx[3].card_idx = 3;
  flip_ctx[3].card_inner = card_inner4;
  flip_ctx[3].target_w = 140;

  if (card_revealed[3]) {
    render_revealed_card_content(card_inner4, 3);
  } else {
    render_mystery_card_content(card_inner4, 3);
  }

  lv_obj_t* ready_card = ui_make_card(parent, 252, 38, 212, 224, C_CARD, C_CARD, 16);
  lv_obj_set_style_border_color(ready_card, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(ready_card, 1, 0);
  lv_obj_clear_flag(ready_card, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t* av_card = ui_make_card(ready_card, 16, 14, 48, 48, lv_color_hex(0x1E2433), lv_color_hex(0x1E2433), 24);
  lv_obj_clear_flag(av_card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* av_img = lv_image_create(av_card);
  lv_image_set_src(av_img, (current_script == AKSARA_JAWA) ? &BuddyJawa : &BuddyBali);
  lv_obj_center(av_img);

  lv_obj_t* r_title = ui_make_label(ready_card, "8 Tantangan Seru!", &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_set_pos(r_title, 72, 20);

  lv_obj_t* r_sub = ui_make_label(ready_card, "Kata, Vokal, Memori", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(r_sub, 72, 40);

  lv_obj_t* r_desc = ui_make_label(ready_card, "Uji kemampuanmu lewat 8 permainan interaktif berbeda!", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(r_desc, 16, 76);
  lv_obj_set_width(r_desc, 180);
  lv_label_set_long_mode(r_desc, LV_LABEL_LONG_WRAP);

  bool unlocked4 = card_revealed[3];
  btn_start_practice = ui_make_card(ready_card, 16, 150, 180, 56,
                                    unlocked4 ? lv_color_hex(0x22C55E) : lv_color_hex(0x1A202C),
                                    unlocked4 ? lv_color_hex(0x16A34A) : lv_color_hex(0x141824), 14);
  lv_obj_set_style_border_color(btn_start_practice, unlocked4 ? lv_color_hex(0x4ADE80) : lv_color_hex(0x2D3748), 0);
  lv_obj_set_style_border_width(btn_start_practice, 1, 0);
  ui_set_pressed(btn_start_practice, lv_color_hex(0x15803D), lv_color_hex(0x15803D));
  lv_obj_add_event_cb(btn_start_practice, on_start_practice_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* start_lbl = ui_make_label(btn_start_practice, "MULAI KUIS " LV_SYMBOL_RIGHT, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(start_lbl);
}

static void update_lesson_slide() {
  if (!view_lesson) return;

  btn_tall_next = nullptr;
  btn_start_practice = nullptr;

  lv_obj_clean(view_lesson);

  if (current_level_idx == 0) {
    if (current_lesson_slide < 4) {
      render_single_concept_slide(view_lesson, current_lesson_slide);
    } else if (current_lesson_slide == 4) {
      render_slide_cards_123(view_lesson, true);
    } else {
      render_slide_cards_4_and_start(view_lesson, 4);
    }
    if (bar_progress) {
      lv_bar_set_value(bar_progress, 5 + (current_lesson_slide * 3), LV_ANIM_ON);
    }
  } else {
    if (current_lesson_slide == 0) {
      render_slide_cards_123(view_lesson, false);
    } else {
      render_slide_cards_4_and_start(view_lesson, 0);
    }
    if (bar_progress) {
      lv_bar_set_value(bar_progress, 10 + (current_lesson_slide * 8), LV_ANIM_ON);
    }
  }
}

static void build_stage_lesson() {
  if (view_lesson) {
    lv_obj_clean(view_lesson);
  } else {
    view_lesson = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_lesson);
    lv_obj_set_size(view_lesson, 480, 276);
    lv_obj_clear_flag(view_lesson, LV_OBJ_FLAG_SCROLLABLE);
  }
  update_lesson_slide();
}

// ---------- Stage 1: Logika Pasangkan Aksara (Match Pairs) ----------
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
  lv_obj_set_style_border_width(left_btns[l], 2, 0);
  lv_obj_set_style_text_color(left_lbls[l], C_OK_TXT, 0);

  lv_obj_set_style_bg_color(right_btns[r], C_OK_BG, 0);
  lv_obj_set_style_border_color(right_btns[r], C_OK_BORDER, 0);
  lv_obj_set_style_border_width(right_btns[r], 2, 0);
  lv_obj_set_style_text_color(right_lbls[r], C_OK_TXT, 0);
}

static void mark_error_buttons(int l, int r) {
  lv_obj_set_style_bg_color(left_btns[l], C_ERR_BG, 0);
  lv_obj_set_style_border_color(left_btns[l], C_ERR_BORDER, 0);
  lv_obj_set_style_border_width(left_btns[l], 2, 0);
  lv_obj_set_style_text_color(left_lbls[l], C_ERR_TXT, 0);

  lv_obj_set_style_bg_color(right_btns[r], C_ERR_BG, 0);
  lv_obj_set_style_border_color(right_btns[r], C_ERR_BORDER, 0);
  lv_obj_set_style_border_width(right_btns[r], 2, 0);
  lv_obj_set_style_text_color(right_lbls[r], C_ERR_TXT, 0);
}

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

  int item_left = match_left_item[sel_left];
  int item_right = match_right_item[sel_right];

  if (item_left == item_right) {
    left_matched[sel_left] = true;
    right_matched[sel_right] = true;
    mark_success_buttons(sel_left, sel_right);
    audio_play(SOUND_SUCCESS);
    matched_pairs_count++;

    sel_left = -1;
    sel_right = -1;

    if (matched_pairs_count >= PAIR_COUNT) {
      add_xp(20);
      audio_play(SOUND_REWARD);
      lv_obj_clear_flag(match_bottom_bar, LV_OBJ_FLAG_HIDDEN);
    }
  } else {
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
  show_stage(2); // Lanjut ke Stage 2: Susun Kata
}

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

  matched_pairs_count = 0;
  sel_left = -1;
  sel_right = -1;
  for (int i = 0; i < PAIR_COUNT; i++) {
    left_matched[i] = false;
    right_matched[i] = false;
  }

  // Acak urutan kolom kiri & kanan setiap kali soal dibuka (kanan tidak boleh sejajar dengan kiri)
  for (int i = 0; i < PAIR_COUNT; i++) {
    match_left_item[i] = active_item_indices[i];
    match_right_item[i] = active_item_indices[i];
  }
  shuffle_ints(match_left_item, PAIR_COUNT);
  for (int tries = 0; tries < 30; tries++) {
    shuffle_ints(match_right_item, PAIR_COUNT);
    bool aligned = false;
    for (int i = 0; i < PAIR_COUNT; i++) {
      if (match_right_item[i] == match_left_item[i]) { aligned = true; break; }
    }
    if (!aligned) break;
  }

  lv_obj_t* title = ui_make_label(view_match, "Pilih Pasangan yang Cocok", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 20, 6);

  lv_obj_t* sub = ui_make_label(view_match, "Sentuh aksara di kiri dan bacaan latin di kanan", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(sub, 20, 28);

  for (int i = 0; i < PAIR_COUNT; i++) {
    int y_pos = 60 + (i * 54);

    left_btns[i] = ui_make_card(view_match, 18, y_pos, 216, 46, C_CARD, C_CARD, 12);
    lv_obj_set_style_border_color(left_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(left_btns[i], 1, 0);
    ui_set_pressed(left_btns[i], C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(left_btns[i], on_left_card_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    lv_obj_t* num_bg_l = ui_make_card(left_btns[i], 10, 10, 26, 26, lv_color_hex(0x232938), lv_color_hex(0x232938), 8);
    lv_obj_clear_flag(num_bg_l, LV_OBJ_FLAG_CLICKABLE);
    char num_str_l[8];
    snprintf(num_str_l, sizeof(num_str_l), "%d", i + 1);
    lv_obj_t* num_lbl_l = ui_make_label(num_bg_l, num_str_l, &lv_font_montserrat_14, C_SUB);
    lv_obj_center(num_lbl_l);

    int item_idx = match_left_item[i];
    left_lbls[i] = ui_make_label(left_btns[i], items[item_idx].aksara, font_ak, C_TEXT);
    lv_obj_align(left_lbls[i], LV_ALIGN_CENTER, 8, 0);

    right_btns[i] = ui_make_card(view_match, 246, y_pos, 216, 46, C_CARD, C_CARD, 12);
    lv_obj_set_style_border_color(right_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(right_btns[i], 1, 0);
    ui_set_pressed(right_btns[i], C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(right_btns[i], on_right_card_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    lv_obj_t* num_bg_r = ui_make_card(right_btns[i], 10, 10, 26, 26, lv_color_hex(0x232938), lv_color_hex(0x232938), 8);
    lv_obj_clear_flag(num_bg_r, LV_OBJ_FLAG_CLICKABLE);
    char num_str_r[8];
    snprintf(num_str_r, sizeof(num_str_r), "%d", i + 5);
    lv_obj_t* num_lbl_r = ui_make_label(num_bg_r, num_str_r, &lv_font_montserrat_14, C_SUB);
    lv_obj_center(num_lbl_r);

    int r_idx = match_right_item[i];
    right_lbls[i] = ui_make_label(right_btns[i], items[r_idx].latin, &lv_font_montserrat_20, C_TEXT);
    lv_obj_align(right_lbls[i], LV_ALIGN_CENTER, 8, 0);
  }

  match_bottom_bar = ui_make_card(view_match, 0, 216, 480, 60, lv_color_hex(0x14532D), lv_color_hex(0x14532D), 0);
  lv_obj_set_style_border_color(match_bottom_bar, lv_color_hex(0x22C55E), 0);
  lv_obj_set_style_border_width(match_bottom_bar, 2, 0);
  lv_obj_add_flag(match_bottom_bar, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t* win_lbl = ui_make_label(match_bottom_bar, LV_SYMBOL_OK " Sempurna! Pasangan Cocok (+20 XP)", &lv_font_montserrat_14, lv_color_hex(0x4ADE80));
  lv_obj_set_pos(win_lbl, 20, 20);

  lv_obj_t* btn_next = ui_make_card(match_bottom_bar, 330, 8, 132, 42, lv_color_hex(0x22C55E), lv_color_hex(0x16A34A), 12);
  ui_set_pressed(btn_next, lv_color_hex(0x15803D), lv_color_hex(0x15803D));
  lv_obj_add_event_cb(btn_next, on_match_next_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* next_lbl = ui_make_label(btn_next, "LANJUT " LV_SYMBOL_RIGHT, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(next_lbl);
}

// ---------- Stage 2: Logika Susun Kata Aksara (Word Builder) ----------
static void update_word_builder_visuals() {
  const AksaraItem* items = get_current_items();
  const lv_font_t* font_ak = get_current_font();

  for (int s = 0; s < 2; s++) {
    int t_idx = word_slot_tile_idx[s];
    if (t_idx >= 0) {
      int item_idx = word_tile_items[t_idx];
      lv_obj_set_style_text_font(word_slot_lbls[s], font_ak, 0);
      lv_label_set_text(word_slot_lbls[s], items[item_idx].aksara);
      lv_obj_set_style_text_color(word_slot_lbls[s], C_SEL_TXT, 0);
      lv_obj_set_style_bg_color(word_slot_btns[s], C_SEL_BG, 0);
      lv_obj_set_style_border_color(word_slot_btns[s], C_SEL_BORDER, 0);
      lv_obj_set_style_border_width(word_slot_btns[s], 2, 0);
      lv_label_set_text(word_slot_sub_lbls[s], "Ketuk hapus");
    } else {
      lv_obj_set_style_text_font(word_slot_lbls[s], &lv_font_montserrat_20, 0);
      lv_label_set_text(word_slot_lbls[s], "?");
      lv_obj_set_style_text_color(word_slot_lbls[s], C_SUB, 0);
      lv_obj_set_style_bg_color(word_slot_btns[s], lv_color_hex(0x141824), 0);
      lv_obj_set_style_border_color(word_slot_btns[s], lv_color_hex(0x2A3245), 0);
      lv_obj_set_style_border_width(word_slot_btns[s], 1, 0);
      lv_label_set_text(word_slot_sub_lbls[s], (s == 0) ? "Suku Kata 1" : "Suku Kata 2");
    }
  }

  for (int i = 0; i < 6; i++) {
    if (word_tile_used[i]) {
      lv_obj_set_style_bg_color(word_tile_btns[i], C_DONE_BG, 0);
      lv_obj_set_style_border_color(word_tile_btns[i], C_DONE_BORDER, 0);
      lv_obj_set_style_text_color(word_tile_lbls[i], C_DONE_TXT, 0);
    } else {
      lv_obj_set_style_bg_color(word_tile_btns[i], C_CARD, 0);
      lv_obj_set_style_border_color(word_tile_btns[i], lv_color_hex(0x2A3245), 0);
      lv_obj_set_style_text_color(word_tile_lbls[i], C_TEXT, 0);
    }
  }
}

static void on_word_tile_click(lv_event_t* e) {
  if (word_checked) return;
  int tile_idx = (int)(intptr_t)lv_event_get_user_data(e);
  if (word_tile_used[tile_idx]) return;

  int target_slot = -1;
  if (word_slot_tile_idx[0] < 0) target_slot = 0;
  else if (word_slot_tile_idx[1] < 0) target_slot = 1;

  if (target_slot < 0) {
    audio_play(SOUND_ERROR);
    ui_show_toast("Kotak penuh! Ketuk kotak atau tombol Hapus.");
    return;
  }

  audio_play(SOUND_CLICK);
  word_tile_used[tile_idx] = true;
  word_slot_tile_idx[target_slot] = tile_idx;
  update_word_builder_visuals();

  if (word_slot_tile_idx[0] >= 0 && word_slot_tile_idx[1] >= 0) {
    lv_label_set_text(word_feedback_lbl, "Tekan PERIKSA untuk cek rangkaian kata!");
    lv_obj_set_style_text_color(word_feedback_lbl, C_TEXT, 0);
    lv_obj_set_style_bg_color(word_action_btn, lv_color_hex(0x22C55E), 0);
  }
}

static void on_word_slot_click(lv_event_t* e) {
  if (word_checked) return;
  int slot_idx = (int)(intptr_t)lv_event_get_user_data(e);
  int t_idx = word_slot_tile_idx[slot_idx];
  if (t_idx < 0) return;

  audio_play(SOUND_CLICK);
  word_tile_used[t_idx] = false;
  word_slot_tile_idx[slot_idx] = -1;
  update_word_builder_visuals();
}

static void on_word_clear_click(lv_event_t* e) {
  (void)e;
  if (word_checked) return;
  audio_play(SOUND_CLICK);
  for (int i = 0; i < 6; i++) word_tile_used[i] = false;
  word_slot_tile_idx[0] = -1;
  word_slot_tile_idx[1] = -1;
  update_word_builder_visuals();
  lv_label_set_text(word_feedback_lbl, "Ketuk 2 keping aksara secara berurutan");
  lv_obj_set_style_text_color(word_feedback_lbl, C_SUB, 0);
}

static void on_word_action_click(lv_event_t* e) {
  (void)e;
  if (!word_checked) {
    if (word_slot_tile_idx[0] < 0 || word_slot_tile_idx[1] < 0) {
      audio_play(SOUND_ERROR);
      ui_show_toast("Pilih 2 keping aksara terlebih dahulu!");
      return;
    }

    word_checked = true;
    int chosen0 = word_tile_items[word_slot_tile_idx[0]];
    int chosen1 = word_tile_items[word_slot_tile_idx[1]];
    bool ok0 = (chosen0 == word_target_items[0]);
    bool ok1 = (chosen1 == word_target_items[1]);

    lv_obj_set_style_bg_color(word_slot_btns[0], ok0 ? C_OK_BG : C_ERR_BG, 0);
    lv_obj_set_style_border_color(word_slot_btns[0], ok0 ? C_OK_BORDER : C_ERR_BORDER, 0);
    lv_obj_set_style_text_color(word_slot_lbls[0], ok0 ? C_OK_TXT : C_ERR_TXT, 0);

    lv_obj_set_style_bg_color(word_slot_btns[1], ok1 ? C_OK_BG : C_ERR_BG, 0);
    lv_obj_set_style_border_color(word_slot_btns[1], ok1 ? C_OK_BORDER : C_ERR_BORDER, 0);
    lv_obj_set_style_text_color(word_slot_lbls[1], ok1 ? C_OK_TXT : C_ERR_TXT, 0);

    if (ok0 && ok1) {
      audio_play(SOUND_SUCCESS);
      add_xp(25);
      char msg[64];
      snprintf(msg, sizeof(msg), LV_SYMBOL_OK " Hebat! Kata \"%s\" benar (+25 XP)", WORD_BANK[current_word_idx].word_display);
      lv_label_set_text(word_feedback_lbl, msg);
      lv_obj_set_style_text_color(word_feedback_lbl, lv_color_hex(0x4ADE80), 0);
    } else {
      audio_play(SOUND_ERROR);
      const AksaraItem* items = get_current_items();
      char msg[64];
      snprintf(msg, sizeof(msg), LV_SYMBOL_CLOSE " Urutan yang tepat: %s + %s",
               items[word_target_items[0]].latin, items[word_target_items[1]].latin);
      lv_label_set_text(word_feedback_lbl, msg);
      lv_obj_set_style_text_color(word_feedback_lbl, lv_color_hex(0xF87171), 0);
    }

    lv_label_set_text(word_action_lbl, "LANJUT " LV_SYMBOL_RIGHT);
    lv_obj_set_style_bg_color(word_action_btn, lv_color_hex(0x22C55E), 0);
  } else {
    audio_play(SOUND_CLICK);
    show_stage(8); // Lanjut ke Stage 8: Pengenalan Sandhangan/Pangangge dulu
  }
}

static void build_stage_word() {
  if (view_word) {
    lv_obj_clean(view_word);
  } else {
    view_word = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_word);
    lv_obj_set_size(view_word, 480, 276);
    lv_obj_clear_flag(view_word, LV_OBJ_FLAG_SCROLLABLE);
  }

  const AksaraItem* items = get_current_items();
  const lv_font_t* font_ak = get_current_font();

  word_checked = false;
  word_slot_tile_idx[0] = -1;
  word_slot_tile_idx[1] = -1;
  for (int i = 0; i < 6; i++) word_tile_used[i] = false;

  // Header Prompt Card
  lv_obj_t* prompt_card = ui_make_card(view_word, 18, 6, 444, 52, C_CARD, C_CARD, 12);
  lv_obj_set_style_border_color(prompt_card, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(prompt_card, 1, 0);
  lv_obj_clear_flag(prompt_card, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t* p_sub = ui_make_label(prompt_card, "Susun Keping Aksara Menjadi Kata:", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(p_sub, 14, 5);

  char w_title[64];
  snprintf(w_title, sizeof(w_title), "\" %s \"  (%s)",
           WORD_BANK[current_word_idx].word_display, WORD_BANK[current_word_idx].meaning);
  lv_color_t accent = (current_script == AKSARA_JAWA) ? C_JAWA_1 : C_BALI_1;
  lv_obj_t* p_word = ui_make_label(prompt_card, w_title, &lv_font_montserrat_20, accent);
  lv_obj_set_pos(p_word, 14, 24);

  lv_obj_t* btn_clr = ui_make_card(prompt_card, 350, 8, 82, 36, lv_color_hex(0x2A1820), lv_color_hex(0x2A1820), 8);
  lv_obj_set_style_border_color(btn_clr, lv_color_hex(0xEF4444), 0);
  lv_obj_set_style_border_width(btn_clr, 1, 0);
  ui_set_pressed(btn_clr, lv_color_hex(0x450A0A), lv_color_hex(0x450A0A));
  lv_obj_add_event_cb(btn_clr, on_word_clear_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* clr_lbl = ui_make_label(btn_clr, LV_SYMBOL_BACKSPACE " Reset", &lv_font_montserrat_14, lv_color_hex(0xF87171));
  lv_obj_center(clr_lbl);

  // 2 Slot Jawaban di Tengah
  for (int s = 0; s < 2; s++) {
    int sx = (s == 0) ? 94 : 254;
    word_slot_btns[s] = ui_make_card(view_word, sx, 64, 132, 68, lv_color_hex(0x141824), lv_color_hex(0x141824), 14);
    lv_obj_set_style_border_color(word_slot_btns[s], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(word_slot_btns[s], 1, 0);
    lv_obj_add_event_cb(word_slot_btns[s], on_word_slot_click, LV_EVENT_CLICKED, (void*)(intptr_t)s);

    word_slot_lbls[s] = ui_make_label(word_slot_btns[s], "?", &lv_font_montserrat_20, C_SUB);
    lv_obj_align(word_slot_lbls[s], LV_ALIGN_TOP_MID, 0, 10);

    word_slot_sub_lbls[s] = ui_make_label(word_slot_btns[s], (s == 0) ? "Suku Kata 1" : "Suku Kata 2", &lv_font_montserrat_14, C_SUB);
    lv_obj_align(word_slot_sub_lbls[s], LV_ALIGN_BOTTOM_MID, 0, -6);
  }

  lv_obj_t* plus_lbl = ui_make_label(view_word, "+", &lv_font_montserrat_28, lv_color_hex(0x38BDF8));
  lv_obj_set_pos(plus_lbl, 233, 82);

  // 6 Keping Pilihan Aksara di Bawah
  for (int i = 0; i < 6; i++) {
    int tx = 18 + (i * 75);
    word_tile_btns[i] = ui_make_card(view_word, tx, 142, 68, 64, C_CARD, C_CARD, 12);
    lv_obj_set_style_border_color(word_tile_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(word_tile_btns[i], 1, 0);
    ui_set_pressed(word_tile_btns[i], C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(word_tile_btns[i], on_word_tile_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    int item_idx = word_tile_items[i];
    word_tile_lbls[i] = ui_make_label(word_tile_btns[i], items[item_idx].aksara, font_ak, C_TEXT);
    lv_obj_center(word_tile_lbls[i]);
  }

  // Action Bar Bawah
  lv_obj_t* action_bar = ui_make_card(view_word, 0, 216, 480, 60, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(action_bar, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(action_bar, 1, 0);

  word_feedback_lbl = ui_make_label(action_bar, "Ketuk 2 keping aksara secara berurutan", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(word_feedback_lbl, 18, 22);

  word_action_btn = ui_make_card(action_bar, 320, 9, 142, 42, lv_color_hex(0x3B82F6), lv_color_hex(0x2563EB), 12);
  ui_set_pressed(word_action_btn, lv_color_hex(0x1D4ED8), lv_color_hex(0x1D4ED8));
  lv_obj_add_event_cb(word_action_btn, on_word_action_click, LV_EVENT_CLICKED, NULL);

  word_action_lbl = ui_make_label(word_action_btn, "PERIKSA", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(word_action_lbl);
}

// ---------- Stage 3: Lab Vokal Sandhangan Swara / Pangangge Suara ----------
static void on_vowel_choice_click(lv_event_t* e) {
  if (vowel_checked) return;

  int idx = (int)(intptr_t)lv_event_get_user_data(e);
  audio_play(SOUND_CLICK);

  for (int i = 0; i < 4; i++) {
    lv_obj_set_style_bg_color(vowel_btns[i], C_CARD, 0);
    lv_obj_set_style_border_color(vowel_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(vowel_btns[i], 1, 0);
    lv_obj_set_style_text_color(vowel_lbls[i], C_TEXT, 0);
  }

  vowel_selected = idx;
  lv_obj_set_style_bg_color(vowel_btns[idx], C_SEL_BG, 0);
  lv_obj_set_style_border_color(vowel_btns[idx], C_SEL_BORDER, 0);
  lv_obj_set_style_border_width(vowel_btns[idx], 2, 0);
  lv_obj_set_style_text_color(vowel_lbls[idx], C_SEL_TXT, 0);

  lv_label_set_text(vowel_feedback_lbl, "Tekan PERIKSA untuk cek vokal");
  lv_obj_set_style_text_color(vowel_feedback_lbl, C_TEXT, 0);
  lv_obj_set_style_bg_color(vowel_action_btn, lv_color_hex(0x22C55E), 0);
}

static void on_vowel_action_click(lv_event_t* e) {
  (void)e;
  if (!vowel_checked) {
    if (vowel_selected < 0) {
      audio_play(SOUND_ERROR);
      ui_show_toast("Pilih salah satu aksara ber-vokal terlebih dahulu!");
      return;
    }

    vowel_checked = true;
    int chosen_vtype = vowel_choice_types[vowel_selected];

    if (chosen_vtype == vowel_target_type) {
      audio_play(SOUND_SUCCESS);
      add_xp(20);

      lv_obj_set_style_bg_color(vowel_btns[vowel_selected], C_OK_BG, 0);
      lv_obj_set_style_border_color(vowel_btns[vowel_selected], C_OK_BORDER, 0);
      lv_obj_set_style_border_width(vowel_btns[vowel_selected], 2, 0);
      lv_obj_set_style_text_color(vowel_lbls[vowel_selected], C_OK_TXT, 0);

      char msg[64];
      snprintf(msg, sizeof(msg), LV_SYMBOL_OK " Tepat! %s (+20 XP)", get_vowel_diacritic_name(vowel_target_type));
      lv_label_set_text(vowel_feedback_lbl, msg);
      lv_obj_set_style_text_color(vowel_feedback_lbl, lv_color_hex(0x4ADE80), 0);
    } else {
      audio_play(SOUND_ERROR);
      lv_obj_set_style_bg_color(vowel_btns[vowel_selected], C_ERR_BG, 0);
      lv_obj_set_style_border_color(vowel_btns[vowel_selected], C_ERR_BORDER, 0);
      lv_obj_set_style_border_width(vowel_btns[vowel_selected], 2, 0);
      lv_obj_set_style_text_color(vowel_lbls[vowel_selected], C_ERR_TXT, 0);

      for (int i = 0; i < 4; i++) {
        if (vowel_choice_types[i] == vowel_target_type) {
          lv_obj_set_style_bg_color(vowel_btns[i], C_OK_BG, 0);
          lv_obj_set_style_border_color(vowel_btns[i], C_OK_BORDER, 0);
          lv_obj_set_style_border_width(vowel_btns[i], 2, 0);
          lv_obj_set_style_text_color(vowel_lbls[i], C_OK_TXT, 0);
        }
      }

      char msg[64];
      snprintf(msg, sizeof(msg), LV_SYMBOL_CLOSE " Yang benar memakai %s", get_vowel_diacritic_name(vowel_target_type));
      lv_label_set_text(vowel_feedback_lbl, msg);
      lv_obj_set_style_text_color(vowel_feedback_lbl, lv_color_hex(0xF87171), 0);
    }

    lv_label_set_text(vowel_action_lbl, "LANJUT " LV_SYMBOL_RIGHT);
    lv_obj_set_style_bg_color(vowel_action_btn, lv_color_hex(0x22C55E), 0);
  } else {
    audio_play(SOUND_CLICK);
    show_stage(4); // Lanjut ke Stage 4: Kilat Benar / Salah
  }
}

static void build_stage_vowel() {
  if (view_vowel) {
    lv_obj_clean(view_vowel);
  } else {
    view_vowel = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_vowel);
    lv_obj_set_size(view_vowel, 480, 276);
    lv_obj_clear_flag(view_vowel, LV_OBJ_FLAG_SCROLLABLE);
  }

  const lv_font_t* font_ak = get_current_font();
  vowel_selected = -1;
  vowel_checked = false;

  lv_obj_t* prompt_card = ui_make_card(view_vowel, 18, 6, 444, 58, C_CARD, C_CARD, 14);
  lv_obj_set_style_border_color(prompt_card, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(prompt_card, 1, 0);
  lv_obj_clear_flag(prompt_card, LV_OBJ_FLAG_CLICKABLE);

  const char* mode_tag = (current_script == AKSARA_JAWA)
    ? "Sandhangan Swara (Pengubah Vokal Jawa)"
    : "Pangangge Suara (Pengubah Vokal Bali)";
  lv_obj_t* tag_lbl = ui_make_label(prompt_card, mode_tag, &lv_font_montserrat_14, lv_color_hex(0x10B981));
  lv_obj_set_pos(tag_lbl, 14, 6);

  char target_sound[24];
  build_vowel_latin_str(vowel_base_item_idx, vowel_target_type, target_sound, sizeof(target_sound));
  char q_str[80];
  snprintf(q_str, sizeof(q_str), "Pilih aksara \"%s\" dengan %s:", target_sound, get_vowel_diacritic_name(vowel_target_type));
  lv_obj_t* q_lbl = ui_make_label(prompt_card, q_str, &lv_font_montserrat_16, C_TEXT);
  lv_obj_set_pos(q_lbl, 14, 28);

  for (int i = 0; i < 4; i++) {
    int x_pos = (i % 2 == 0) ? 18 : 246;
    int y_pos = 72 + (i / 2) * 68;

    vowel_btns[i] = ui_make_card(view_vowel, x_pos, y_pos, 216, 62, C_CARD, C_CARD, 14);
    lv_obj_set_style_border_color(vowel_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(vowel_btns[i], 1, 0);
    lv_obj_add_flag(vowel_btns[i], LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    ui_set_pressed(vowel_btns[i], C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(vowel_btns[i], on_vowel_choice_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    lv_obj_t* opt_bg = ui_make_card(vowel_btns[i], 10, 17, 28, 28, lv_color_hex(0x232938), lv_color_hex(0x232938), 8);
    lv_obj_clear_flag(opt_bg, LV_OBJ_FLAG_CLICKABLE);
    static const char* OPTS[4] = { "A", "B", "C", "D" };
    lv_obj_t* opt_lbl = ui_make_label(opt_bg, OPTS[i], &lv_font_montserrat_14, C_SUB);
    lv_obj_center(opt_lbl);

    vowel_lbls[i] = ui_make_label(vowel_btns[i], vowel_choice_glyphs[i], font_ak, C_TEXT);
    lv_obj_align(vowel_lbls[i], LV_ALIGN_CENTER, 10, 2);
  }

  lv_obj_t* action_bar = ui_make_card(view_vowel, 0, 216, 480, 60, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(action_bar, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(action_bar, 1, 0);

  vowel_feedback_lbl = ui_make_label(action_bar, "Perhatikan letak tanda vokal di atas/bawah/samping", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(vowel_feedback_lbl, 18, 22);

  vowel_action_btn = ui_make_card(action_bar, 320, 9, 142, 42, lv_color_hex(0x3B82F6), lv_color_hex(0x2563EB), 12);
  ui_set_pressed(vowel_action_btn, lv_color_hex(0x1D4ED8), lv_color_hex(0x1D4ED8));
  lv_obj_add_event_cb(vowel_action_btn, on_vowel_action_click, LV_EVENT_CLICKED, NULL);

  vowel_action_lbl = ui_make_label(vowel_action_btn, "PERIKSA", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(vowel_action_lbl);
}

// ---------- Stage 4: Tantangan Kilat Benar / Salah (True or False) ----------
static void on_tf_answer_click(lv_event_t* e) {
  if (tf_answered) return;
  tf_answered = true;

  bool user_said_true = ((int)(intptr_t)lv_event_get_user_data(e) == 1);
  bool correct = (user_said_true == tf_is_true_pair);
  const AksaraItem* items = get_current_items();

  if (correct) {
    audio_play(SOUND_SUCCESS);
    add_xp(15);
    if (tf_is_true_pair) {
      char msg[64];
      snprintf(msg, sizeof(msg), LV_SYMBOL_OK " Tepat! Aksara itu memang \"%s\" (+15 XP)", items[tf_shown_item_idx].latin);
      lv_label_set_text(tf_feedback_lbl, msg);
    } else {
      char msg[64];
      snprintf(msg, sizeof(msg), LV_SYMBOL_OK " Tepat! Itu aksara \"%s\", bukan \"%s\" (+15 XP)",
               items[tf_shown_item_idx].latin, items[tf_shown_latin_idx].latin);
      lv_label_set_text(tf_feedback_lbl, msg);
    }
    lv_obj_set_style_text_color(tf_feedback_lbl, lv_color_hex(0x4ADE80), 0);
  } else {
    audio_play(SOUND_ERROR);
    char msg[64];
    snprintf(msg, sizeof(msg), LV_SYMBOL_CLOSE " Kurang tepat! Aksara itu berbunyi \"%s\"", items[tf_shown_item_idx].latin);
    lv_label_set_text(tf_feedback_lbl, msg);
    lv_obj_set_style_text_color(tf_feedback_lbl, lv_color_hex(0xF87171), 0);
  }

  lv_obj_clear_flag(tf_bottom_bar, LV_OBJ_FLAG_HIDDEN);
}

static void on_tf_next_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  show_stage(9); // Lanjut ke Stage 9: Memori Kartu
}

static void build_stage_tf() {
  if (view_tf) {
    lv_obj_clean(view_tf);
  } else {
    view_tf = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_tf);
    lv_obj_set_size(view_tf, 480, 276);
    lv_obj_clear_flag(view_tf, LV_OBJ_FLAG_SCROLLABLE);
  }

  const AksaraItem* items = get_current_items();
  const lv_font_t* font_ak_jumbo = get_current_font_jumbo();
  tf_answered = false;

  lv_obj_t* title = ui_make_label(view_tf, "Tantangan Kilat: Benar atau Salah?", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 18, 6);

  lv_obj_t* sub = ui_make_label(view_tf, "Apakah pasangan aksara dan bacaan latin di bawah ini cocok?", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(sub, 18, 30);

  // Kartu Showcase Besar di Tengah
  lv_obj_t* show_card = ui_make_card(view_tf, 18, 54, 444, 92, C_CARD, C_CARD, 16);
  lv_obj_set_style_border_color(show_card, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(show_card, 1, 0);
  lv_obj_clear_flag(show_card, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t* ak_box = ui_make_card(show_card, 20, 10, 164, 72, lv_color_hex(0x141824), lv_color_hex(0x141824), 12);
  lv_obj_clear_flag(ak_box, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* ak_lbl = ui_make_label(ak_box, items[tf_shown_item_idx].aksara, font_ak_jumbo, C_TEXT);
  lv_obj_center(ak_lbl);

  lv_obj_t* eq_lbl = ui_make_label(show_card, "=", &lv_font_montserrat_28, C_SUB);
  lv_obj_align(eq_lbl, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t* lat_box = ui_make_card(show_card, 260, 10, 164, 72, lv_color_hex(0x141824), lv_color_hex(0x141824), 12);
  lv_obj_clear_flag(lat_box, LV_OBJ_FLAG_CLICKABLE);
  char lat_str[24];
  snprintf(lat_str, sizeof(lat_str), "\" %s \"", items[tf_shown_latin_idx].latin);
  lv_obj_t* lat_lbl = ui_make_label(lat_box, lat_str, &lv_font_montserrat_28, lv_color_hex(0x38BDF8));
  lv_obj_center(lat_lbl);

  // 2 Tombol Besar: SALAH (Merah) & BENAR (Hijau)
  tf_btn_false = ui_make_card(view_tf, 18, 156, 214, 52, lv_color_hex(0x3B151E), lv_color_hex(0x270B13), 14);
  lv_obj_set_style_border_color(tf_btn_false, lv_color_hex(0xEF4444), 0);
  lv_obj_set_style_border_width(tf_btn_false, 2, 0);
  ui_set_pressed(tf_btn_false, lv_color_hex(0x450A0A), lv_color_hex(0x450A0A));
  lv_obj_add_event_cb(tf_btn_false, on_tf_answer_click, LV_EVENT_CLICKED, (void*)(intptr_t)0);
  lv_obj_t* f_lbl = ui_make_label(tf_btn_false, LV_SYMBOL_CLOSE "  SALAH", &lv_font_montserrat_20, lv_color_hex(0xF87171));
  lv_obj_center(f_lbl);

  tf_btn_true = ui_make_card(view_tf, 248, 156, 214, 52, lv_color_hex(0x0F291E), lv_color_hex(0x091C14), 14);
  lv_obj_set_style_border_color(tf_btn_true, lv_color_hex(0x22C55E), 0);
  lv_obj_set_style_border_width(tf_btn_true, 2, 0);
  ui_set_pressed(tf_btn_true, lv_color_hex(0x14532D), lv_color_hex(0x14532D));
  lv_obj_add_event_cb(tf_btn_true, on_tf_answer_click, LV_EVENT_CLICKED, (void*)(intptr_t)1);
  lv_obj_t* t_lbl = ui_make_label(tf_btn_true, LV_SYMBOL_OK "  BENAR", &lv_font_montserrat_20, lv_color_hex(0x4ADE80));
  lv_obj_center(t_lbl);

  // Bar Hasil di Bawah
  tf_bottom_bar = ui_make_card(view_tf, 0, 216, 480, 60, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(tf_bottom_bar, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(tf_bottom_bar, 1, 0);
  lv_obj_add_flag(tf_bottom_bar, LV_OBJ_FLAG_HIDDEN);

  tf_feedback_lbl = ui_make_label(tf_bottom_bar, "", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(tf_feedback_lbl, 18, 22);

  lv_obj_t* btn_next = ui_make_card(tf_bottom_bar, 330, 9, 132, 42, lv_color_hex(0x22C55E), lv_color_hex(0x16A34A), 12);
  ui_set_pressed(btn_next, lv_color_hex(0x15803D), lv_color_hex(0x15803D));
  lv_obj_add_event_cb(btn_next, on_tf_next_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* next_lbl = ui_make_label(btn_next, "LANJUT " LV_SYMBOL_RIGHT, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(next_lbl);
}

// ---------- Stage 5: Logika Tebak Aksara (Multiple Choice) ----------
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

  lv_label_set_text(guess_feedback_lbl, "Tekan PERIKSA untuk cek jawaban");
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
      audio_play(SOUND_SUCCESS);
      add_xp(15);

      lv_obj_set_style_bg_color(guess_btns[guess_selected], C_OK_BG, 0);
      lv_obj_set_style_border_color(guess_btns[guess_selected], C_OK_BORDER, 0);
      lv_obj_set_style_border_width(guess_btns[guess_selected], 2, 0);
      lv_obj_set_style_text_color(guess_lbls[guess_selected], C_OK_TXT, 0);

      lv_label_set_text(guess_feedback_lbl, LV_SYMBOL_OK " Luar biasa! Jawabanmu benar (+15 XP)");
      lv_obj_set_style_text_color(guess_feedback_lbl, lv_color_hex(0x4ADE80), 0);
    } else {
      audio_play(SOUND_ERROR);
      lv_obj_set_style_bg_color(guess_btns[guess_selected], C_ERR_BG, 0);
      lv_obj_set_style_border_color(guess_btns[guess_selected], C_ERR_BORDER, 0);
      lv_obj_set_style_border_width(guess_btns[guess_selected], 2, 0);
      lv_obj_set_style_text_color(guess_lbls[guess_selected], C_ERR_TXT, 0);

      for (int i = 0; i < 4; i++) {
        if (guess_choices[i] == guess_correct_idx) {
          lv_obj_set_style_bg_color(guess_btns[i], C_OK_BG, 0);
          lv_obj_set_style_border_color(guess_btns[i], C_OK_BORDER, 0);
          lv_obj_set_style_border_width(guess_btns[i], 2, 0);
          lv_obj_set_style_text_color(guess_lbls[i], C_OK_TXT, 0);
        }
      }

      const AksaraItem* items = get_current_items();
      char err_str[64];
      snprintf(err_str, sizeof(err_str), LV_SYMBOL_CLOSE " Kurang tepat! Benar: %s", items[guess_correct_idx].latin);
      lv_label_set_text(guess_feedback_lbl, err_str);
      lv_obj_set_style_text_color(guess_feedback_lbl, lv_color_hex(0xF87171), 0);
    }

    lv_label_set_text(guess_action_lbl, "SELESAI " LV_SYMBOL_RIGHT);
    lv_obj_set_style_bg_color(guess_action_btn, lv_color_hex(0x22C55E), 0);
  } else {
    audio_play(SOUND_CLICK);
    show_stage(6); // Pindah ke Stage Complete
  }
}

static void on_audio_hint_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_NODE);
  ui_show_toast("Mendengarkan bunyi aksara...");
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

  lv_obj_t* avatar_card = ui_make_card(view_guess, 18, 10, 52, 52, C_CARD, C_CARD, 26);
  lv_obj_clear_flag(avatar_card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* avatar_img = lv_image_create(avatar_card);
  lv_image_set_src(avatar_img, (current_script == AKSARA_JAWA) ? &BuddyJawa : &BuddyBali);
  lv_obj_center(avatar_img);

  lv_obj_t* bubble = ui_make_card(view_guess, 76, 8, 386, 56, C_CARD, C_CARD, 14);
  lv_obj_set_style_border_color(bubble, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(bubble, 1, 0);

  lv_obj_t* btn_audio = ui_make_card(bubble, 7, 7, 42, 42, C_CARD_PR, C_CARD_PR, 21);
  ui_set_pressed(btn_audio, lv_color_hex(0x133857), lv_color_hex(0x133857));
  lv_obj_add_event_cb(btn_audio, on_audio_hint_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* ic_audio = ui_make_label(btn_audio, LV_SYMBOL_VOLUME_MAX, &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_center(ic_audio);

  lv_obj_t* prompt = ui_make_label(bubble, "Manakah aksara yang berbunyi:", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(prompt, 58, 8);

  char target_str[24];
  snprintf(target_str, sizeof(target_str), "\" %s \"", items[guess_correct_idx].latin);
  lv_color_t accent_color = (current_script == AKSARA_JAWA) ? C_JAWA_1 : C_BALI_1;
  lv_obj_t* target_lbl = ui_make_label(bubble, target_str, &lv_font_montserrat_20, accent_color);
  lv_obj_set_pos(target_lbl, 58, 28);

  for (int i = 0; i < 4; i++) {
    int x_pos = (i % 2 == 0) ? 18 : 246;
    int y_pos = 76 + (i / 2) * 68;

    guess_btns[i] = ui_make_card(view_guess, x_pos, y_pos, 216, 60, C_CARD, C_CARD, 14);
    lv_obj_set_style_border_color(guess_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(guess_btns[i], 1, 0);
    ui_set_pressed(guess_btns[i], C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(guess_btns[i], on_guess_choice_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    lv_obj_t* opt_bg = ui_make_card(guess_btns[i], 12, 16, 28, 28, lv_color_hex(0x232938), lv_color_hex(0x232938), 8);
    lv_obj_clear_flag(opt_bg, LV_OBJ_FLAG_CLICKABLE);
    static const char* OPTS[4] = { "A", "B", "C", "D" };
    lv_obj_t* opt_lbl = ui_make_label(opt_bg, OPTS[i], &lv_font_montserrat_14, C_SUB);
    lv_obj_center(opt_lbl);

    int c_idx = guess_choices[i];
    guess_lbls[i] = ui_make_label(guess_btns[i], items[c_idx].aksara, font_ak, C_TEXT);
    lv_obj_align(guess_lbls[i], LV_ALIGN_CENTER, 8, 0);
  }

  lv_obj_t* action_bar = ui_make_card(view_guess, 0, 216, 480, 60, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(action_bar, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(action_bar, 1, 0);

  guess_feedback_lbl = ui_make_label(action_bar, "Pilih salah satu aksara di atas", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(guess_feedback_lbl, 18, 22);

  guess_action_btn = ui_make_card(action_bar, 320, 9, 142, 42, lv_color_hex(0x3B82F6), lv_color_hex(0x2563EB), 12);
  ui_set_pressed(guess_action_btn, lv_color_hex(0x1D4ED8), lv_color_hex(0x1D4ED8));
  lv_obj_add_event_cb(guess_action_btn, on_guess_action_click, LV_EVENT_CLICKED, NULL);

  guess_action_lbl = ui_make_label(guess_action_btn, "PERIKSA", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(guess_action_lbl);
}

// ---------- Stage 6: Selesai & Main Lagi ----------
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

static void on_replay_random_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_NODE);
  prepare_session_stages();
  ui_show_toast("Soal baru diacak! Pilih mode atau mulai kuis.");
  show_stage(7); // Buka menu mode dengan soal yang sudah diacak baru
}

static void on_mascot_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_BUDDY);
  ui_show_toast("Semangat belajarmu luar biasa!");
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

  lv_obj_t* title = ui_make_label(view_complete, "Hore! Rangkaian Latihan Selesai!", &lv_font_montserrat_20, lv_color_hex(0x22C55E));
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

  lv_obj_t* sub = ui_make_label(view_complete, "Kamu telah menaklukkan semua mode tantangan aksara ini!", &lv_font_montserrat_14, C_SUB);
  lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, 38);

  lv_obj_t* mascot_frame = ui_make_card(view_complete, 28, 68, 120, 152, lv_color_hex(0x141824), lv_color_hex(0x141824), 16);
  lv_obj_set_style_border_color(mascot_frame, lv_color_hex(0x22324A), 0);
  lv_obj_set_style_border_width(mascot_frame, 1, 0);
  lv_obj_add_event_cb(mascot_frame, on_mascot_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* mascot_img = lv_image_create(mascot_frame);
  lv_image_set_src(mascot_img, &BuddyMaskot);
  lv_obj_align(mascot_img, LV_ALIGN_TOP_MID, 0, 6);

  lv_obj_t* win_tag = ui_make_card(mascot_frame, 10, 118, 100, 24, lv_color_hex(0x241D10), lv_color_hex(0x241D10), 8);
  lv_obj_clear_flag(win_tag, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* tag_lbl = ui_make_label(win_tag, LV_SYMBOL_OK " JUARA!", &lv_font_montserrat_14, lv_color_hex(0xF59E0B));
  lv_obj_center(tag_lbl);

  // Stat 1: XP
  lv_obj_t* stat1 = ui_make_card(view_complete, 164, 68, 138, 56, lv_color_hex(0x241D10), lv_color_hex(0x241D10), 12);
  lv_obj_set_style_border_color(stat1, lv_color_hex(0x78350F), 0);
  lv_obj_set_style_border_width(stat1, 1, 0);
  lv_obj_clear_flag(stat1, LV_OBJ_FLAG_CLICKABLE);

  char stat_xp_str[24];
  snprintf(stat_xp_str, sizeof(stat_xp_str), LV_SYMBOL_CHARGE " +%d XP", earned_xp);
  lv_obj_t* lbl_xp = ui_make_label(stat1, stat_xp_str, &lv_font_montserrat_20, lv_color_hex(0xF59E0B));
  lv_obj_set_pos(lbl_xp, 12, 6);
  lv_obj_t* lbl_xp_sub = ui_make_label(stat1, "Total Poin", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(lbl_xp_sub, 12, 32);

  // Stat 2: Mode
  lv_obj_t* stat2 = ui_make_card(view_complete, 314, 68, 138, 56, lv_color_hex(0x0F291E), lv_color_hex(0x0F291E), 12);
  lv_obj_set_style_border_color(stat2, lv_color_hex(0x065F46), 0);
  lv_obj_set_style_border_width(stat2, 1, 0);
  lv_obj_clear_flag(stat2, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t* lbl_acc = ui_make_label(stat2, LV_SYMBOL_OK " 8 Mode", &lv_font_montserrat_20, lv_color_hex(0x10B981));
  lv_obj_set_pos(lbl_acc, 12, 6);
  lv_obj_t* lbl_acc_sub = ui_make_label(stat2, "Tuntas Semua", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(lbl_acc_sub, 12, 32);

  // Tombol Latihan Soal Baru (Acak)
  lv_obj_t* btn_replay = ui_make_card(view_complete, 164, 132, 288, 40, lv_color_hex(0x1E293B), lv_color_hex(0x1E293B), 12);
  lv_obj_set_style_border_color(btn_replay, lv_color_hex(0x38BDF8), 0);
  lv_obj_set_style_border_width(btn_replay, 1, 0);
  ui_set_pressed(btn_replay, lv_color_hex(0x133857), lv_color_hex(0x133857));
  lv_obj_add_event_cb(btn_replay, on_replay_random_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* rep_lbl = ui_make_label(btn_replay, LV_SYMBOL_REFRESH "  ACAK SOAL & PILIH MODE LAIN", &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_center(rep_lbl);

  // Tombol Kembali ke Peta
  lv_obj_t* btn_finish = ui_make_card(view_complete, 164, 180, 288, 44, lv_color_hex(0x22C55E), lv_color_hex(0x16A34A), 12);
  ui_set_pressed(btn_finish, lv_color_hex(0x15803D), lv_color_hex(0x15803D));
  lv_obj_add_event_cb(btn_finish, on_finish_learn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* btn_lbl = ui_make_label(btn_finish, "KEMBALI KE PETA AKSARA " LV_SYMBOL_RIGHT, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(btn_lbl);
}

// ---------- Helper Vokal Singkat untuk Pengenalan ----------
static void build_vowel_short_latin(int item_idx, int vowel_type, char* out_buf, size_t buf_size) {
  const AksaraItem* items = get_current_items();
  const char* base = items[item_idx].latin;
  size_t len = strlen(base);
  char stem[8];
  if (len > 1 && base[len - 1] == 'a') {
    memcpy(stem, base, len - 1);
    stem[len - 1] = '\0';
  } else {
    snprintf(stem, sizeof(stem), "%s", base);
  }
  static const char* V[5] = { "i", "u", "e", "e", "o" };
  snprintf(out_buf, buf_size, "%s%s", stem, V[vowel_type]);
}

// ---------- Stage 8: Pengenalan Sandhangan Swara / Pangangge Suara ----------
static void on_intro_tile_click(lv_event_t* e) {
  int vt = (int)(intptr_t)lv_event_get_user_data(e);
  audio_play(SOUND_NODE);
  char latin[24];
  build_vowel_latin_str(vowel_base_item_idx, vt, latin, sizeof(latin));
  char msg[80];
  snprintf(msg, sizeof(msg), "%s: bunyi \"%s\"", get_vowel_diacritic_name(vt), latin);
  ui_show_toast(msg);
}

static void on_intro_start_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  show_stage(3);
}

static void build_stage_vowel_intro() {
  if (view_vowel_intro) {
    lv_obj_clean(view_vowel_intro);
  } else {
    view_vowel_intro = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_vowel_intro);
    lv_obj_set_size(view_vowel_intro, 480, 276);
    lv_obj_clear_flag(view_vowel_intro, LV_OBJ_FLAG_SCROLLABLE);
  }

  const AksaraItem* items = get_current_items();
  const lv_font_t* font_big = get_current_font_jumbo();
  bool jawa = (current_script == AKSARA_JAWA);
  lv_color_t accent = jawa ? C_JAWA_1 : C_BALI_1;

  lv_obj_t* title = ui_make_label(view_vowel_intro,
                                  jawa ? "Tahap Baru: Sandhangan Swara" : "Tahap Baru: Pangangge Suara",
                                  &lv_font_montserrat_20, lv_color_hex(0x10B981));
  lv_obj_set_pos(title, 18, 6);

  const char* base_latin = items[vowel_base_item_idx].latin;
  char desc[160];
  snprintf(desc, sizeof(desc),
           "Aksara dasar \"%s\" berbunyi \"%s\". Pasang tanda vokal berikut untuk mengubah bunyinya. Ketuk kartu untuk detail!",
           base_latin, base_latin);
  lv_obj_t* d_lbl = ui_make_label(view_vowel_intro, desc, &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(d_lbl, 18, 32);
  lv_obj_set_width(d_lbl, 444);
  lv_label_set_long_mode(d_lbl, LV_LABEL_LONG_WRAP);

  static const char* JAWA_SHORT[5] = { "Wulu", "Suku", "Taling", "Pepet", "Tarung" };
  static const char* BALI_SHORT[5] = { "Ulu", "Suku", "Taleng", "Pepet", "Tedung" };

  for (int i = 0; i < 5; i++) {
    int x = 18 + i * 90;
    lv_obj_t* tile = ui_make_card(view_vowel_intro, x, 80, 84, 128, C_CARD, C_CARD, 14);
    lv_obj_set_style_border_color(tile, accent, 0);
    lv_obj_set_style_border_width(tile, 1, 0);
    lv_obj_add_flag(tile, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    ui_set_pressed(tile, C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(tile, on_intro_tile_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    char glyph[24];
    build_vowel_glyph_utf8(vowel_base_item_idx, i, glyph, sizeof(glyph));
    lv_obj_t* g_lbl = ui_make_label(tile, glyph, font_big, C_TEXT);
    lv_obj_align(g_lbl, LV_ALIGN_CENTER, 0, -22);

    char latin[16];
    build_vowel_short_latin(vowel_base_item_idx, i, latin, sizeof(latin));
    lv_obj_t* l_lbl = ui_make_label(tile, latin, &lv_font_montserrat_20, accent);
    lv_obj_align(l_lbl, LV_ALIGN_TOP_MID, 0, 84);

    lv_obj_t* n_lbl = ui_make_label(tile, jawa ? JAWA_SHORT[i] : BALI_SHORT[i], &lv_font_montserrat_14, C_SUB);
    lv_obj_align(n_lbl, LV_ALIGN_TOP_MID, 0, 107);
  }

  lv_obj_t* bar = ui_make_card(view_vowel_intro, 0, 216, 480, 60, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(bar, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(bar, 1, 0);

  lv_obj_t* hint = ui_make_label(bar, "Siap uji pemahaman vokal?", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(hint, 18, 22);

  lv_obj_t* btn = ui_make_card(bar, 280, 9, 182, 42, lv_color_hex(0x22C55E), lv_color_hex(0x16A34A), 12);
  ui_set_pressed(btn, lv_color_hex(0x15803D), lv_color_hex(0x15803D));
  lv_obj_add_event_cb(btn, on_intro_start_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* b_lbl = ui_make_label(btn, "MULAI LATIHAN " LV_SYMBOL_RIGHT, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(b_lbl);
}

// ---------- Stage 9: Memori Kartu Aksara ----------
static void mem_style_card(int i, lv_color_t bg, lv_color_t border, int bw, lv_color_t txt) {
  lv_obj_set_style_bg_color(mem_btns[i], bg, 0);
  lv_obj_set_style_border_color(mem_btns[i], border, 0);
  lv_obj_set_style_border_width(mem_btns[i], bw, 0);
  lv_obj_set_style_text_color(mem_lbls[i], txt, 0);
}

static void mem_render_card(int i) {
  const AksaraItem* items = get_current_items();
  bool show = mem_face_up[i] || mem_matched[i];

  if (!show) {
    lv_obj_set_style_text_font(mem_lbls[i], &lv_font_montserrat_28, 0);
    lv_label_set_text(mem_lbls[i], "?");
    mem_style_card(i, C_CARD, lv_color_hex(0x2A3245), 1, C_SUB);
  } else {
    if (mem_card_kind[i] == 0) {
      lv_obj_set_style_text_font(mem_lbls[i], get_current_font_jumbo(), 0);
      lv_label_set_text(mem_lbls[i], items[mem_card_item[i]].aksara);
    } else {
      lv_obj_set_style_text_font(mem_lbls[i], &lv_font_montserrat_28, 0);
      lv_label_set_text(mem_lbls[i], items[mem_card_item[i]].latin);
    }
    if (mem_matched[i]) {
      mem_style_card(i, C_OK_BG, C_OK_BORDER, 2, C_OK_TXT);
    } else {
      mem_style_card(i, C_SEL_BG, C_SEL_BORDER, 2, C_SEL_TXT);
    }
  }
  lv_obj_center(mem_lbls[i]);
}

static void mem_flip_back_cb(lv_timer_t* t) {
  uint32_t gen = (uint32_t)(uintptr_t)lv_timer_get_user_data(t);
  if (gen != mem_gen) return; // Permainan sudah dibangun ulang, abaikan
  int a = mem_pending_a;
  int b = mem_pending_b;
  if (a >= 0) { mem_face_up[a] = false; mem_render_card(a); }
  if (b >= 0) { mem_face_up[b] = false; mem_render_card(b); }
  mem_pending_a = -1;
  mem_pending_b = -1;
  mem_first = -1;
  mem_lock = false;
}

static void on_mem_card_click(lv_event_t* e) {
  int i = (int)(intptr_t)lv_event_get_user_data(e);
  if (mem_lock || mem_face_up[i] || mem_matched[i]) return;

  audio_play(SOUND_CLICK);
  mem_face_up[i] = true;
  mem_render_card(i);

  if (mem_first < 0) {
    mem_first = i;
    return;
  }

  int a = mem_first;
  mem_first = -1;
  mem_moves++;
  char mv[24];
  snprintf(mv, sizeof(mv), "Langkah: %d", mem_moves);
  lv_label_set_text(mem_moves_lbl, mv);

  if (mem_card_item[a] == mem_card_item[i]) {
    mem_matched[a] = true;
    mem_matched[i] = true;
    mem_render_card(a);
    mem_render_card(i);
    audio_play(SOUND_SUCCESS);
    mem_pairs_found++;
    if (mem_pairs_found >= 4) {
      add_xp(30);
      audio_play(SOUND_REWARD);
      lv_obj_clear_flag(mem_bottom_bar, LV_OBJ_FLAG_HIDDEN);
    }
  } else {
    audio_play(SOUND_ERROR);
    mem_style_card(a, C_ERR_BG, C_ERR_BORDER, 2, C_ERR_TXT);
    mem_style_card(i, C_ERR_BG, C_ERR_BORDER, 2, C_ERR_TXT);
    mem_lock = true;
    mem_pending_a = a;
    mem_pending_b = i;
    lv_timer_t* tmr = lv_timer_create(mem_flip_back_cb, 800, (void*)(uintptr_t)mem_gen);
    lv_timer_set_repeat_count(tmr, 1);
  }
}

static void on_mem_next_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  show_stage(10);
}

static void build_stage_memory() {
  if (view_memory) {
    lv_obj_clean(view_memory);
  } else {
    view_memory = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_memory);
    lv_obj_set_size(view_memory, 480, 276);
    lv_obj_clear_flag(view_memory, LV_OBJ_FLAG_SCROLLABLE);
  }

  mem_gen++;
  mem_first = -1;
  mem_lock = false;
  mem_pairs_found = 0;
  mem_moves = 0;
  mem_pending_a = -1;
  mem_pending_b = -1;

  // 4 item acak -> 8 kartu (aksara + latin) diacak posisinya
  int pick[4];
  pick_random_unique_items(pick, 4, get_total_items());
  int order[8];
  for (int i = 0; i < 8; i++) order[i] = i;
  shuffle_ints(order, 8);
  for (int s = 0; s < 8; s++) {
    int c = order[s];
    mem_card_item[s] = pick[c / 2];
    mem_card_kind[s] = c % 2;
    mem_face_up[s] = false;
    mem_matched[s] = false;
  }

  lv_obj_t* title = ui_make_label(view_memory, "Memori Kartu Aksara", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 18, 6);

  lv_obj_t* sub = ui_make_label(view_memory, "Buka 2 kartu, temukan pasangan aksara & latinnya", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(sub, 18, 30);

  mem_moves_lbl = ui_make_label(view_memory, "Langkah: 0", &lv_font_montserrat_14, lv_color_hex(0xF59E0B));
  lv_obj_set_pos(mem_moves_lbl, 380, 10);

  for (int i = 0; i < 8; i++) {
    int col = i % 4;
    int row = i / 4;
    int x = 18 + col * 112;
    int y = 56 + row * 82;
    mem_btns[i] = ui_make_card(view_memory, x, y, 104, 74, C_CARD, C_CARD, 14);
    ui_set_pressed(mem_btns[i], C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(mem_btns[i], on_mem_card_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    mem_lbls[i] = ui_make_label(mem_btns[i], "?", &lv_font_montserrat_28, C_SUB);
    mem_render_card(i);
  }

  mem_bottom_bar = ui_make_card(view_memory, 0, 216, 480, 60, lv_color_hex(0x14532D), lv_color_hex(0x14532D), 0);
  lv_obj_set_style_border_color(mem_bottom_bar, lv_color_hex(0x22C55E), 0);
  lv_obj_set_style_border_width(mem_bottom_bar, 2, 0);
  lv_obj_add_flag(mem_bottom_bar, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t* win_lbl = ui_make_label(mem_bottom_bar, LV_SYMBOL_OK " Semua pasangan ketemu! (+30 XP)", &lv_font_montserrat_14, lv_color_hex(0x4ADE80));
  lv_obj_set_pos(win_lbl, 20, 20);

  lv_obj_t* btn_next = ui_make_card(mem_bottom_bar, 330, 8, 132, 42, lv_color_hex(0x22C55E), lv_color_hex(0x16A34A), 12);
  ui_set_pressed(btn_next, lv_color_hex(0x15803D), lv_color_hex(0x15803D));
  lv_obj_add_event_cb(btn_next, on_mem_next_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* next_lbl = ui_make_label(btn_next, "LANJUT " LV_SYMBOL_RIGHT, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(next_lbl);
}

// ---------- Stage 10: Baca Kata Aksara ----------
static void on_rw_choice_click(lv_event_t* e) {
  if (rw_answered) return;
  rw_answered = true;

  int idx = (int)(intptr_t)lv_event_get_user_data(e);
  bool ok = (rw_opts[idx] == rw_word_idx);

  if (ok) {
    audio_play(SOUND_SUCCESS);
    add_xp(20);
    lv_obj_set_style_bg_color(rw_btns[idx], C_OK_BG, 0);
    lv_obj_set_style_border_color(rw_btns[idx], C_OK_BORDER, 0);
    lv_obj_set_style_border_width(rw_btns[idx], 2, 0);
    lv_obj_set_style_text_color(rw_lbls[idx], C_OK_TXT, 0);

    char msg[80];
    snprintf(msg, sizeof(msg), LV_SYMBOL_OK " Benar! %s = %s (+20 XP)",
             WORD_BANK[rw_word_idx].word_display, WORD_BANK[rw_word_idx].meaning);
    lv_label_set_text(rw_feedback_lbl, msg);
    lv_obj_set_style_text_color(rw_feedback_lbl, lv_color_hex(0x4ADE80), 0);
  } else {
    audio_play(SOUND_ERROR);
    lv_obj_set_style_bg_color(rw_btns[idx], C_ERR_BG, 0);
    lv_obj_set_style_border_color(rw_btns[idx], C_ERR_BORDER, 0);
    lv_obj_set_style_border_width(rw_btns[idx], 2, 0);
    lv_obj_set_style_text_color(rw_lbls[idx], C_ERR_TXT, 0);

    for (int i = 0; i < 4; i++) {
      if (rw_opts[i] == rw_word_idx) {
        lv_obj_set_style_bg_color(rw_btns[i], C_OK_BG, 0);
        lv_obj_set_style_border_color(rw_btns[i], C_OK_BORDER, 0);
        lv_obj_set_style_border_width(rw_btns[i], 2, 0);
        lv_obj_set_style_text_color(rw_lbls[i], C_OK_TXT, 0);
      }
    }

    char msg[80];
    snprintf(msg, sizeof(msg), LV_SYMBOL_CLOSE " Yang benar: %s (%s)",
             WORD_BANK[rw_word_idx].word_display, WORD_BANK[rw_word_idx].meaning);
    lv_label_set_text(rw_feedback_lbl, msg);
    lv_obj_set_style_text_color(rw_feedback_lbl, lv_color_hex(0xF87171), 0);
  }

  lv_obj_clear_flag(rw_bottom_bar, LV_OBJ_FLAG_HIDDEN);
}

static void on_rw_next_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  show_stage(5);
}

static void build_stage_readword() {
  if (view_readword) {
    lv_obj_clean(view_readword);
  } else {
    view_readword = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_readword);
    lv_obj_set_size(view_readword, 480, 276);
    lv_obj_clear_flag(view_readword, LV_OBJ_FLAG_SCROLLABLE);
  }

  const AksaraItem* items = get_current_items();
  rw_answered = false;

  // Kata acak + 3 pengecoh acak, posisi jawaban diacak
  rw_word_idx = rnd_n(20);
  int others[19];
  int n = 0;
  for (int i = 0; i < 20; i++) {
    if (i != rw_word_idx) others[n++] = i;
  }
  shuffle_ints(others, n);
  rw_opts[0] = rw_word_idx;
  for (int i = 0; i < 3; i++) rw_opts[1 + i] = others[i];
  shuffle_ints(rw_opts, 4);

  lv_obj_t* title = ui_make_label(view_readword, "Baca Kata Aksara", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 18, 6);

  lv_obj_t* show_card = ui_make_card(view_readword, 18, 34, 444, 78, C_CARD, C_CARD, 16);
  lv_obj_set_style_border_color(show_card, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(show_card, 1, 0);
  lv_obj_clear_flag(show_card, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t* q_lbl = ui_make_label(show_card, "Bagaimana cara membaca kata ini?", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(q_lbl, 14, 6);

  char word_ak[24];
  snprintf(word_ak, sizeof(word_ak), "%s%s",
           items[find_item_by_latin(WORD_BANK[rw_word_idx].syl1)].aksara,
           items[find_item_by_latin(WORD_BANK[rw_word_idx].syl2)].aksara);
  lv_obj_t* w_lbl = ui_make_label(show_card, word_ak, get_current_font_jumbo(), C_TEXT);
  lv_obj_align(w_lbl, LV_ALIGN_CENTER, 0, 8);

  for (int i = 0; i < 4; i++) {
    int x = (i % 2 == 0) ? 18 : 246;
    int y = 122 + (i / 2) * 48;
    rw_btns[i] = ui_make_card(view_readword, x, y, 216, 42, C_CARD, C_CARD, 12);
    lv_obj_set_style_border_color(rw_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(rw_btns[i], 1, 0);
    ui_set_pressed(rw_btns[i], C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(rw_btns[i], on_rw_choice_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    lv_obj_t* opt_bg = ui_make_card(rw_btns[i], 8, 7, 28, 28, lv_color_hex(0x232938), lv_color_hex(0x232938), 8);
    lv_obj_clear_flag(opt_bg, LV_OBJ_FLAG_CLICKABLE);
    static const char* OPTS[4] = { "A", "B", "C", "D" };
    lv_obj_t* opt_lbl = ui_make_label(opt_bg, OPTS[i], &lv_font_montserrat_14, C_SUB);
    lv_obj_center(opt_lbl);

    rw_lbls[i] = ui_make_label(rw_btns[i], WORD_BANK[rw_opts[i]].word_display, &lv_font_montserrat_20, C_TEXT);
    lv_obj_align(rw_lbls[i], LV_ALIGN_CENTER, 10, 0);
  }

  rw_bottom_bar = ui_make_card(view_readword, 0, 216, 480, 60, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(rw_bottom_bar, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(rw_bottom_bar, 1, 0);
  lv_obj_add_flag(rw_bottom_bar, LV_OBJ_FLAG_HIDDEN);

  rw_feedback_lbl = ui_make_label(rw_bottom_bar, "", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(rw_feedback_lbl, 18, 10);
  lv_obj_set_width(rw_feedback_lbl, 300);
  lv_label_set_long_mode(rw_feedback_lbl, LV_LABEL_LONG_WRAP);

  lv_obj_t* btn_next = ui_make_card(rw_bottom_bar, 330, 9, 132, 42, lv_color_hex(0x22C55E), lv_color_hex(0x16A34A), 12);
  ui_set_pressed(btn_next, lv_color_hex(0x15803D), lv_color_hex(0x15803D));
  lv_obj_add_event_cb(btn_next, on_rw_next_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* next_lbl = ui_make_label(btn_next, "LANJUT " LV_SYMBOL_RIGHT, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(next_lbl);
}

// ---------- Stage 7: Menu Pilih Mode Pembelajaran Langsung ----------
static void on_shuffle_questions_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_NODE);
  prepare_session_stages();
  ui_show_toast("Semua soal diacak ulang!");
}

static void build_stage_mode_menu() {
  if (view_mode_menu) {
    lv_obj_clean(view_mode_menu);
  } else {
    view_mode_menu = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_mode_menu);
    lv_obj_set_size(view_mode_menu, 480, 276);
    lv_obj_clear_flag(view_mode_menu, LV_OBJ_FLAG_SCROLLABLE);
  }

  lv_obj_t* title = ui_make_label(view_mode_menu, "Pilih Mode Pembelajaran", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 18, 4);

  lv_obj_t* sub = ui_make_label(view_mode_menu, "Mainkan latihan mana saja, kapan saja:", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(sub, 18, 28);

  struct ModeCardInfo {
    const char* name;
    const char* desc;
    lv_color_t color;
    int target_stage; // -1 = acak soal
  };

  static const ModeCardInfo MODES[9] = {
    { "Kartu Misteri", "Buka & kenali",    lv_color_hex(0x38BDF8), 0 },
    { "Pasangkan",     "Cocokkan latin",   lv_color_hex(0x22C55E), 1 },
    { "Susun Kata",    "Rangkai kata",     lv_color_hex(0xF59E0B), 2 },
    { "Lab Vokal",     "Tanda vokal",      lv_color_hex(0xA855F7), 8 },
    { "Benar / Salah", "Tantangan kilat",  lv_color_hex(0xF43F5E), 4 },
    { "Memori Kartu",  "Cari pasangan",    lv_color_hex(0x14B8A6), 9 },
    { "Baca Kata",     "Baca aksara",      lv_color_hex(0xEAB308), 10 },
    { "Tebak Aksara",  "Pilihan ganda",    lv_color_hex(0x06B6D4), 5 },
    { "Acak Soal",     "Soal baru acak",   lv_color_hex(0xFB923C), -1 }
  };

  for (int i = 0; i < 9; i++) {
    int col = i % 3;
    int row = i / 3;
    int x_pos = 18 + col * 151;
    int y_pos = 52 + row * 74;

    lv_obj_t* card = ui_make_card(view_mode_menu, x_pos, y_pos, 142, 66, C_CARD, C_CARD, 14);
    lv_obj_set_style_border_color(card, MODES[i].color, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    ui_set_pressed(card, C_CARD_PR, C_CARD_PR);
    if (MODES[i].target_stage < 0) {
      lv_obj_add_event_cb(card, on_shuffle_questions_click, LV_EVENT_CLICKED, NULL);
    } else {
      lv_obj_add_event_cb(card, on_select_mode_card_click, LV_EVENT_CLICKED, (void*)(intptr_t)MODES[i].target_stage);
    }

    lv_obj_t* m_title = ui_make_label(card, MODES[i].name, &lv_font_montserrat_16, MODES[i].color);
    lv_obj_set_pos(m_title, 10, 8);

    lv_obj_t* m_desc = ui_make_label(card, MODES[i].desc, &lv_font_montserrat_14, C_SUB);
    lv_obj_set_pos(m_desc, 10, 36);
    lv_obj_set_width(m_desc, 124);
    lv_label_set_long_mode(m_desc, LV_LABEL_LONG_DOT);
  }
}

static void hide_all_stage_views() {
  if (view_lesson) lv_obj_add_flag(view_lesson, LV_OBJ_FLAG_HIDDEN);
  if (view_match) lv_obj_add_flag(view_match, LV_OBJ_FLAG_HIDDEN);
  if (view_word) lv_obj_add_flag(view_word, LV_OBJ_FLAG_HIDDEN);
  if (view_vowel) lv_obj_add_flag(view_vowel, LV_OBJ_FLAG_HIDDEN);
  if (view_tf) lv_obj_add_flag(view_tf, LV_OBJ_FLAG_HIDDEN);
  if (view_guess) lv_obj_add_flag(view_guess, LV_OBJ_FLAG_HIDDEN);
  if (view_complete) lv_obj_add_flag(view_complete, LV_OBJ_FLAG_HIDDEN);
  if (view_mode_menu) lv_obj_add_flag(view_mode_menu, LV_OBJ_FLAG_HIDDEN);
  if (view_vowel_intro) lv_obj_add_flag(view_vowel_intro, LV_OBJ_FLAG_HIDDEN);
  if (view_memory) lv_obj_add_flag(view_memory, LV_OBJ_FLAG_HIDDEN);
  if (view_readword) lv_obj_add_flag(view_readword, LV_OBJ_FLAG_HIDDEN);
}

// Alur: 0 Kartu -> 1 Pasangkan -> 2 Susun Kata -> 8 Kenali Vokal -> 3 Lab Vokal
//       -> 4 Benar/Salah -> 9 Memori -> 10 Baca Kata -> 5 Tebak Aksara -> 6 Selesai
static void set_stage_header(const char* info, int progress) {
  if (lbl_stage_info) lv_label_set_text(lbl_stage_info, info);
  if (bar_progress && progress >= 0) lv_bar_set_value(bar_progress, progress, LV_ANIM_ON);
}

static void show_stage(int stage) {
  current_stage = stage;
  hide_all_stage_views();

  if (stage == 0) {
    set_stage_header("1/9 \xE2\x80\xA2 Kartu Aksara", -1);
    current_lesson_slide = 0;
    build_stage_lesson();
    lv_obj_clear_flag(view_lesson, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 1) {
    set_stage_header("2/9 \xE2\x80\xA2 Pasangkan", 24);
    build_stage_match();
    lv_obj_clear_flag(view_match, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 2) {
    set_stage_header("3/9 \xE2\x80\xA2 Susun Kata", 36);
    build_stage_word();
    lv_obj_clear_flag(view_word, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 8) {
    set_stage_header("4/9 \xE2\x80\xA2 Kenali Vokal", 48);
    build_stage_vowel_intro();
    lv_obj_clear_flag(view_vowel_intro, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 3) {
    set_stage_header("5/9 \xE2\x80\xA2 Lab Vokal", 58);
    build_stage_vowel();
    lv_obj_clear_flag(view_vowel, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 4) {
    set_stage_header("6/9 \xE2\x80\xA2 Benar / Salah", 68);
    build_stage_tf();
    lv_obj_clear_flag(view_tf, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 9) {
    set_stage_header("7/9 \xE2\x80\xA2 Memori Kartu", 78);
    build_stage_memory();
    lv_obj_clear_flag(view_memory, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 10) {
    set_stage_header("8/9 \xE2\x80\xA2 Baca Kata", 88);
    build_stage_readword();
    lv_obj_clear_flag(view_readword, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 5) {
    set_stage_header("9/9 \xE2\x80\xA2 Tebak Aksara", 95);
    build_stage_guess();
    lv_obj_clear_flag(view_guess, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 6) {
    set_stage_header("Selesai!", 100);
    build_stage_complete();
    lv_obj_clear_flag(view_complete, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 7) {
    set_stage_header("Pilih Mode Latihan", -1);
    build_stage_mode_menu();
    lv_obj_clear_flag(view_mode_menu, LV_OBJ_FLAG_HIDDEN);
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
  lv_obj_t* btn_close = ui_make_card(header, 12, 3, 38, 38, C_CARD, C_CARD, 19);
  lv_obj_set_style_border_color(btn_close, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(btn_close, 1, 0);
  ui_set_pressed(btn_close, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btn_close, on_close_learn_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* ic_close = ui_make_label(btn_close, LV_SYMBOL_CLOSE, &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(ic_close);

  // Label Info Stage di atas Progress Bar
  lbl_stage_info = ui_make_label(header, "1/9 • Kartu Aksara", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(lbl_stage_info, 58, 4);

  // Progress Bar ala Duolingo
  bar_progress = lv_bar_create(header);
  lv_obj_set_pos(bar_progress, 58, 25);
  lv_obj_set_size(bar_progress, 206, 10);
  lv_obj_set_style_radius(bar_progress, 5, 0);
  lv_obj_set_style_bg_color(bar_progress, lv_color_hex(0x1E2433), 0);
  lv_obj_set_style_bg_color(bar_progress, lv_color_hex(0x22C55E), LV_PART_INDICATOR);
  lv_obj_set_style_radius(bar_progress, 5, LV_PART_INDICATOR);
  lv_bar_set_range(bar_progress, 0, 100);
  lv_bar_set_value(bar_progress, 8, LV_ANIM_OFF);

  // Tombol Pintasan Pilih Mode ("MODE")
  lv_obj_t* btn_mode = ui_make_card(header, 272, 5, 72, 34, lv_color_hex(0x133857), lv_color_hex(0x133857), 12);
  lv_obj_set_style_border_color(btn_mode, lv_color_hex(0x38BDF8), 0);
  lv_obj_set_style_border_width(btn_mode, 1, 0);
  ui_set_pressed(btn_mode, lv_color_hex(0x1E293B), lv_color_hex(0x1E293B));
  lv_obj_add_event_cb(btn_mode, on_open_mode_menu_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* lbl_mode = ui_make_label(btn_mode, LV_SYMBOL_LIST " MODE", &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_center(lbl_mode);

  // XP Badge
  lv_obj_t* badge_card = ui_make_card(header, 350, 5, 118, 34, lv_color_hex(0x241D10), lv_color_hex(0x241D10), 17);
  lv_obj_set_style_border_color(badge_card, lv_color_hex(0xD97706), 0);
  lv_obj_set_style_border_width(badge_card, 1, 0);
  lv_obj_clear_flag(badge_card, LV_OBJ_FLAG_CLICKABLE);
  lbl_badge_xp = ui_make_label(badge_card, LV_SYMBOL_CHARGE " +0 XP", &lv_font_montserrat_14, lv_color_hex(0xF59E0B));
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
  current_target_item_idx = map_level_to_item_index(level_index);
  current_lesson_slide = 0;
  earned_xp = 0;

  for (int i = 0; i < 4; i++) {
    card_revealed[i] = false;
    card_animating[i] = false;
  }

  char b[24];
  snprintf(b, sizeof(b), LV_SYMBOL_CHARGE " +%d XP", earned_xp);
  if (lbl_badge_xp) lv_label_set_text(lbl_badge_xp, b);

  prepare_session_stages();

  show_stage(0);
  ui_learn_show();
}

void ui_learn_show() {
  if (scr_learn) {
    lv_scr_load_anim(scr_learn, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
  }
}
