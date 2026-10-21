#include "ui_learn.h"
#include "ui_jawa.h"
#include "ui_bali.h"
#include <stdio.h>
#include <string.h>

// ---------- Data Aksara Hanacaraka Jawa & Bali ----------
struct AksaraItem {
  const char* latin;   // Transliterasi Latin bersih (Pure ASCII)
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
static int current_target_item_idx = 0;
static int current_stage = 0; // 0: Materi, 1: Pasangkan, 2: Tebak Aksara, 3: Selesai
static int earned_xp = 0;

// Sub-views
static lv_obj_t* view_lesson = nullptr;
static lv_obj_t* view_match = nullptr;
static lv_obj_t* view_guess = nullptr;
static lv_obj_t* view_complete = nullptr;

// Stage 0: Lesson Slide Tracker
static int current_lesson_slide = 0;

// Status Buka Kartu Misteri (Harus dibuka untuk lanjut)
static bool card_revealed[4] = { false, false, false, false };
static bool card_animating[4] = { false, false, false, false };

// Tombol Navigasi Kartu
static lv_obj_t* btn_tall_next = nullptr;
static lv_obj_t* btn_start_practice = nullptr;

// Data Konteks Animasi Flip Kartu
struct CardFlipCtx {
  int card_idx;
  lv_obj_t* card_inner;
  int target_w;
};
static CardFlipCtx flip_ctx[4];

// Stage 1: Match Pairs Data
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

// Stage 2: Guess Data
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
static void build_stage_guess();
static void build_stage_complete();
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

static int map_level_to_item_index(int level_idx) {
  if (level_idx == 4) return 3; // Ra
  if (level_idx == 5) return 4; // Ka
  if (level_idx == 6) return 5; // Da
  if (level_idx == 7) return 6; // Ta
  if (level_idx >= 8) return 0;
  return level_idx % 10;
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

  // Periksa apakah semua kartu di slide sudah terbuka
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

  // Bersihkan konten misteri & ganti dengan konten aksara native terbuka
  lv_obj_clean(ctx->card_inner);
  render_revealed_card_content(ctx->card_inner, ctx->card_idx);

  // Ubah border menjadi warna aksen aktif
  lv_obj_set_style_border_color(ctx->card_inner, (current_script == AKSARA_JAWA) ? C_JAWA_1 : C_BALI_1, 0);
  lv_obj_set_style_border_width(ctx->card_inner, 2, 0);

  // Fase 2: Melebar kembali dari 6 ke target_w (Efek Membalik Kartu)
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
    // Jika sudah terbuka, putar efek suara dan tampilkan toast nama aksara
    audio_play(SOUND_NODE);
    const AksaraItem* items = get_current_items();
    int item_idx = active_item_indices[card_idx];
    char msg[32];
    snprintf(msg, sizeof(msg), "Aksara: %s", items[item_idx].latin);
    ui_show_toast(msg);
    return;
  }

  // Mulai Buka Kartu dengan Animasi Flip
  card_revealed[card_idx] = true;
  card_animating[card_idx] = true;
  audio_play(SOUND_CLICK);

  CardFlipCtx* ctx = &flip_ctx[card_idx];

  // Fase 1: Mengempis dari target_w ke 6
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

// Sub-builder Konten Kartu Tertutup (Misteri "?" ala Sketsa User)
static void render_mystery_card_content(lv_obj_t* parent, int card_idx) {
  // Pill Nomor
  lv_obj_t* pill = ui_make_card(parent, 10, 10, 32, 22, lv_color_hex(0x232938), lv_color_hex(0x232938), 6);
  lv_obj_clear_flag(pill, LV_OBJ_FLAG_CLICKABLE);
  char n_str[8];
  snprintf(n_str, sizeof(n_str), "#%d", card_idx + 1);
  lv_obj_t* n_lbl = ui_make_label(pill, n_str, &lv_font_montserrat_14, lv_color_hex(0xF59E0B));
  lv_obj_center(n_lbl);

  // Lingkaran Tanda Tanya Misteri "?" di Tengah
  lv_obj_t* circle = ui_make_card(parent, 0, 0, 68, 68, lv_color_hex(0x1F293D), lv_color_hex(0x1F293D), 34);
  lv_obj_set_style_border_color(circle, lv_color_hex(0x38BDF8), 0);
  lv_obj_set_style_border_width(circle, 2, 0);
  lv_obj_clear_flag(circle, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_center(circle);

  lv_obj_t* q_lbl = ui_make_label(circle, "?", &lv_font_montserrat_28, lv_color_hex(0x38BDF8));
  lv_obj_center(q_lbl);

  // Label Ajakan Buka di Bawah
  lv_obj_t* tap_lbl = ui_make_label(parent, "Ketuk Buka", &lv_font_montserrat_14, lv_color_hex(0xF2F4F8));
  lv_obj_align(tap_lbl, LV_ALIGN_BOTTOM_MID, 0, -28);

  lv_obj_t* sub_lbl = ui_make_label(parent, "Sentuh kartu", &lv_font_montserrat_14, lv_color_hex(0x8A93A6));
  lv_obj_align(sub_lbl, LV_ALIGN_BOTTOM_MID, 0, -10);
}

// Sub-builder Konten Kartu Terbuka (Aksara GEDE 38px & Latin 28px)
static void render_revealed_card_content(lv_obj_t* parent, int card_idx) {
  const AksaraItem* items = get_current_items();
  const lv_font_t* font_ak_jumbo = get_current_font_jumbo();
  int item_idx = active_item_indices[card_idx];

  // Pill Nomor
  lv_obj_t* pill = ui_make_card(parent, 10, 10, 32, 22, lv_color_hex(0x232938), lv_color_hex(0x232938), 6);
  lv_obj_clear_flag(pill, LV_OBJ_FLAG_CLICKABLE);
  char n_str[8];
  snprintf(n_str, sizeof(n_str), "#%d", card_idx + 1);
  lv_obj_t* n_lbl = ui_make_label(pill, n_str, &lv_font_montserrat_14, lv_color_hex(0xF59E0B));
  lv_obj_center(n_lbl);

  // Glif Aksara Native GEDE (Font Jumbo 38px)
  lv_obj_t* ak_lbl = ui_make_label(parent, items[item_idx].aksara, font_ak_jumbo, C_TEXT);
  lv_obj_align(ak_lbl, LV_ALIGN_TOP_MID, 0, 44);

  // Separator Garis Tipis
  lv_obj_t* line = ui_make_card(parent, 16, 122, 84, 1, lv_color_hex(0x2A3245), lv_color_hex(0x2A3245), 0);
  lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);

  // Nama Latin Besar (Montserrat 28 Bold)
  lv_obj_t* lat_lbl = ui_make_label(parent, items[item_idx].latin, &lv_font_montserrat_28, (current_script == AKSARA_JAWA) ? C_JAWA_1 : C_BALI_1);
  lv_obj_align(lat_lbl, LV_ALIGN_TOP_MID, 0, 136);

  // Subtitle
  lv_obj_t* sub_lbl = ui_make_label(parent, "Aksara", &lv_font_montserrat_14, C_SUB);
  lv_obj_align(sub_lbl, LV_ALIGN_TOP_MID, 0, 186);
}

// Pembaruan Status Tombol Aksi (Terkunci jika belum dibuka)
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

// Sub-builder: Slide Konsep Mandiri (Hanya Muncul di Node 0)
static void render_single_concept_slide(lv_obj_t* parent, int concept_idx) {
  struct SingleConcept {
    const char* badge_tag;
    const char* title;
    const char* detail;
    const char* note;
    lv_color_t theme_color;
  };

  SingleConcept concepts_jawa[4] = {
    {
      "KONSEP 1 DARI 4",
      "1. Aksara Carakan / Nglegena",
      "Aksara dasar yang secara alami berbunyi konsonan dan vokal 'a' utuh (Ha, Na, Ca, Ra, Ka, Da, Ta, Sa, Wa, La, dst).",
      "Setiap aksara sudah memiliki bunyi utuh tanpa memerlukan sandhangan tambahan apapun.",
      lv_color_hex(0xF59E0B) // Amber
    },
    {
      "KONSEP 2 DARI 4",
      "2. Sandhangan Swara & Panyigeg",
      "Tanda yang dipasang untuk mengubah vokal 'a' menjadi vokal lain: Wulu (i), Suku (u), Taling (e), Taling Tarung (o), dan Pepet (e).",
      "Serta tanda pemati suku kata: Layar (r), Cecak (ng), Wignyan (h), dan Pangkon (pemati umum).",
      lv_color_hex(0x10B981) // Emerald
    },
    {
      "KONSEP 3 DARI 4",
      "3. Pasangan Aksara",
      "Bentuk khusus aksara yang berfungsi untuk mematikan vokal suku kata sebelumnya ketika ada dua konsonan berurutan.",
      "Masing-masing dari 20 aksara Carakan memiliki bentuk pasangan spesifik (ditulis di bawah/samping).",
      lv_color_hex(0x38BDF8) // Cyan
    },
    {
      "KONSEP 4 DARI 4",
      "4. Aksara Rekan & Murda",
      "Aksara Rekan digunakan untuk melafalkan kata serapan asing (kh, f, dz, z, gh).",
      "Aksara Murda berfungsi khusus sebagai huruf kapital untuk penulisan nama orang terhormat, gelar, dan tempat.",
      lv_color_hex(0xA855F7) // Purple
    }
  };

  SingleConcept concepts_bali[4] = {
    {
      "KONSEP 1 DARI 4",
      "1. Aksara Wreastra",
      "Aksara pokok yang digunakan dalam penulisan bahasa Bali lumrah sehari-hari (Ha, Na, Ca, Ra, Ka, Da, Ta, Sa, Wa, La, dst).",
      "Secara alami setiap aksara Wreastra sudah berbunyi vokal 'a' secara mandiri dan utuh.",
      lv_color_hex(0xF59E0B) // Amber
    },
    {
      "KONSEP 2 DARI 4",
      "2. Pangangge Suara & Tengenan",
      "Tanda yang dipasang untuk mengubah vokal: Ulu (i), Suku (u), Taleng (e), Tedong (a panjang), dan Pepet (e).",
      "Serta tanda penutup konsonan (Tengenan): Surang (r), Cecek (ng), Bisah (h), dan Adeg-adeg (pemati).",
      lv_color_hex(0x10B981) // Emerald
    },
    {
      "KONSEP 3 DARI 4",
      "3. Gantungan & Gempelan",
      "Bentuk aksara gantung (Gantungan di bawah) dan tempel (Gempelan di kanan) untuk meredam vokal konsonan sebelumnya.",
      "Digunakan saat dua konsonan bertemu berturut-turut tanpa jeda vokal.",
      lv_color_hex(0x38BDF8) // Cyan
    },
    {
      "KONSEP 4 DARI 4",
      "4. Aksara Swalalita & Modre",
      "Aksara Swalalita digunakan khusus untuk penulisan kidung sastra Kawi, Jawa Kuno, dan Sansekerta.",
      "Aksara Modre digunakan dalam konteks keagamaan, doa suci, rajah, dan lambang mistis ke Tuhanan.",
      lv_color_hex(0xA855F7) // Purple
    }
  };

  SingleConcept* cur = (current_script == AKSARA_JAWA) ? &concepts_jawa[concept_idx] : &concepts_bali[concept_idx];

  // Header Badge Kategori
  lv_obj_t* badge = ui_make_card(parent, 16, 8, 180, 26, lv_color_hex(0x133857), lv_color_hex(0x133857), 6);
  lv_obj_clear_flag(badge, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* b_lbl = ui_make_label(badge, cur->badge_tag, &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_center(b_lbl);

  // Judul Besar Proporsional (Montserrat 20 Bold)
  lv_obj_t* title = ui_make_label(parent, cur->title, &lv_font_montserrat_20, cur->theme_color);
  lv_obj_set_pos(title, 16, 38);

  // Kartu Utama Penjelasan (Lebar 448px, Tinggi 144px) - Headline dihapus agar lebih lega
  lv_obj_t* main_card = ui_make_card(parent, 16, 66, 448, 144, C_CARD, C_CARD, 16);
  lv_obj_set_style_border_color(main_card, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(main_card, 1, 0);
  lv_obj_clear_flag(main_card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_flex_flow(main_card, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(main_card, 14, 0);
  lv_obj_set_style_pad_row(main_card, 10, 0);

  // Detail Paragraf (Montserrat 16 - Lebih besar & sangat jelas dibaca)
  lv_obj_t* d_lbl = ui_make_label(main_card, cur->detail, &lv_font_montserrat_16, lv_color_hex(0xF3F4F6));
  lv_obj_set_width(d_lbl, 416);
  lv_label_set_long_mode(d_lbl, LV_LABEL_LONG_WRAP);

  // Catatan Khusus (Montserrat 16 - Warna lembut pendukung)
  lv_obj_t* n_lbl = ui_make_label(main_card, cur->note, &lv_font_montserrat_16, lv_color_hex(0x94A3B8));
  lv_obj_set_width(n_lbl, 416);
  lv_label_set_long_mode(n_lbl, LV_LABEL_LONG_WRAP);

  // Navigasi Bawah (Tinggi 60px)
  lv_obj_t* bar = ui_make_card(parent, 0, 216, 480, 60, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(bar, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(bar, 1, 0);

  // Tombol Sebelumnya
  if (concept_idx > 0) {
    lv_obj_t* btn_prev = ui_make_card(bar, 16, 9, 134, 42, lv_color_hex(0x1E2433), lv_color_hex(0x1E2433), 12);
    lv_obj_set_style_border_color(btn_prev, lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(btn_prev, 1, 0);
    ui_set_pressed(btn_prev, lv_color_hex(0x141824), lv_color_hex(0x141824));
    lv_obj_add_event_cb(btn_prev, on_lesson_goto_slide, LV_EVENT_CLICKED, (void*)(intptr_t)(concept_idx - 1));
    lv_obj_t* lbl_prev = ui_make_label(btn_prev, LV_SYMBOL_LEFT " SEBELUM", &lv_font_montserrat_14, C_TEXT);
    lv_obj_center(lbl_prev);
  }

  // Tombol Berikutnya
  lv_obj_t* btn_next = ui_make_card(bar, 330, 9, 134, 42, lv_color_hex(0x3B82F6), lv_color_hex(0x2563EB), 12);
  ui_set_pressed(btn_next, lv_color_hex(0x1D4ED8), lv_color_hex(0x1D4ED8));
  lv_obj_add_event_cb(btn_next, on_lesson_goto_slide, LV_EVENT_CLICKED, (void*)(intptr_t)(concept_idx + 1));

  const char* next_str = (concept_idx < 3) ? "LANJUT " LV_SYMBOL_RIGHT : "KARTU " LV_SYMBOL_RIGHT;
  lv_obj_t* lbl_next = ui_make_label(btn_next, next_str, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(lbl_next);
}

// Sub-builder: Slide Kartu Aksara 1..3 Misteri/Interaktif
static void render_slide_cards_123(lv_obj_t* parent, bool show_back_concept) {
  // Header Baris Kecil
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

  // 3 Kartu Berdampingan (Lebar 116px, Tinggi 224px di y=38)
  for (int i = 0; i < 3; i++) {
    int x_pos = 16 + (i * 124);

    // Wadah Luar Tetap (Outer Card)
    lv_obj_t* card_outer = ui_make_card(parent, x_pos, 38, 116, 224, C_BG, C_BG, 16);
    lv_obj_clear_flag(card_outer, LV_OBJ_FLAG_SCROLLABLE);

    // Wadah Dalam Beranimasi (Inner Card)
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

  // Tombol Tinggi di Kanan ala Sketsa User (Lebar 76px, Tinggi 224px)
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

// Sub-builder: Slide Kartu 4 Misteri + Tombol Mulai Latihan ala Sketsa User
static void render_slide_cards_4_and_start(lv_obj_t* parent, int prev_slide_target) {
  // Header Tag
  lv_obj_t* badge = ui_make_card(parent, 16, 6, 210, 26, lv_color_hex(0x133857), lv_color_hex(0x133857), 6);
  lv_obj_clear_flag(badge, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* b_lbl = ui_make_label(badge, LV_SYMBOL_FILE " KARTU AKSARA (2/2)", &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_center(b_lbl);

  // Tombol Tinggi Kembali di Kiri ala Sketsa User (Lebar 76px, Tinggi 224px di y=38)
  lv_obj_t* btn_tall_back = ui_make_card(parent, 16, 38, 76, 224, lv_color_hex(0x1E2433), lv_color_hex(0x1E2433), 16);
  lv_obj_set_style_border_color(btn_tall_back, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(btn_tall_back, 1, 0);
  ui_set_pressed(btn_tall_back, lv_color_hex(0x141824), lv_color_hex(0x141824));
  lv_obj_add_event_cb(btn_tall_back, on_lesson_goto_slide, LV_EVENT_CLICKED, (void*)(intptr_t)prev_slide_target);

  lv_obj_t* ic_back = ui_make_label(btn_tall_back, LV_SYMBOL_LEFT, &lv_font_montserrat_20, C_TEXT);
  lv_obj_align(ic_back, LV_ALIGN_CENTER, 0, -12);

  lv_obj_t* lbl_back = ui_make_label(btn_tall_back, "KEMBALI", &lv_font_montserrat_14, C_TEXT);
  lv_obj_align(lbl_back, LV_ALIGN_CENTER, 0, 18);

  // Kartu Ke-4 (Lebar 140px, Tinggi 224px di x=102, y=38)
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

  // Kartu Kanan: Maskot & Tombol Mulai Latihan (Lebar 212px, Tinggi 224px di x=252)
  lv_obj_t* ready_card = ui_make_card(parent, 252, 38, 212, 224, C_CARD, C_CARD, 16);
  lv_obj_set_style_border_color(ready_card, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(ready_card, 1, 0);
  lv_obj_clear_flag(ready_card, LV_OBJ_FLAG_CLICKABLE);

  // Maskot Avatar
  lv_obj_t* av_card = ui_make_card(ready_card, 16, 14, 48, 48, lv_color_hex(0x1E2433), lv_color_hex(0x1E2433), 24);
  lv_obj_clear_flag(av_card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* av_img = lv_image_create(av_card);
  lv_image_set_src(av_img, (current_script == AKSARA_JAWA) ? &BuddyJawa : &BuddyBali);
  lv_obj_center(av_img);

  lv_obj_t* r_title = ui_make_label(ready_card, "4 Aksara Siap!", &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_set_pos(r_title, 72, 20);

  lv_obj_t* r_sub = ui_make_label(ready_card, "Buka kartu untuk kuis", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(r_sub, 72, 40);

  lv_obj_t* r_desc = ui_make_label(ready_card, "Siap uji ingatanmu di kuis pasangan & tebak aksara?", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(r_desc, 16, 76);
  lv_obj_set_width(r_desc, 180);
  lv_label_set_long_mode(r_desc, LV_LABEL_LONG_WRAP);

  // Tombol Mulai Latihan (Terkunci jika kartu 4 belum dibuka)
  bool unlocked4 = card_revealed[3];
  btn_start_practice = ui_make_card(ready_card, 16, 150, 180, 56,
                                    unlocked4 ? lv_color_hex(0x22C55E) : lv_color_hex(0x1A202C),
                                    unlocked4 ? lv_color_hex(0x16A34A) : lv_color_hex(0x141824), 14);
  lv_obj_set_style_border_color(btn_start_practice, unlocked4 ? lv_color_hex(0x4ADE80) : lv_color_hex(0x2D3748), 0);
  lv_obj_set_style_border_width(btn_start_practice, 1, 0);
  ui_set_pressed(btn_start_practice, lv_color_hex(0x15803D), lv_color_hex(0x15803D));
  lv_obj_add_event_cb(btn_start_practice, on_start_practice_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* start_lbl = ui_make_label(btn_start_practice, "LATIHAN " LV_SYMBOL_RIGHT, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(start_lbl);
}

static void update_lesson_slide() {
  if (!view_lesson) return;

  btn_tall_next = nullptr;
  btn_start_practice = nullptr;

  lv_obj_clean(view_lesson);

  if (current_level_idx == 0) {
    // Mode Node 0 (Ha): Tampilkan Konsep 1..4 lalu Kartu Aksara 1..3 dan 4
    if (current_lesson_slide == 0) {
      render_single_concept_slide(view_lesson, 0); // Konsep 1: Carakan / Wreastra
    } else if (current_lesson_slide == 1) {
      render_single_concept_slide(view_lesson, 1); // Konsep 2: Sandhangan / Pangangge
    } else if (current_lesson_slide == 2) {
      render_single_concept_slide(view_lesson, 2); // Konsep 3: Pasangan / Gantungan
    } else if (current_lesson_slide == 3) {
      render_single_concept_slide(view_lesson, 3); // Konsep 4: Rekan / Swalalita
    } else if (current_lesson_slide == 4) {
      render_slide_cards_123(view_lesson, true /* Ada tombol kembali ke konsep */);
    } else {
      render_slide_cards_4_and_start(view_lesson, 4 /* Kembali ke slide 4 */);
    }

    if (bar_progress) {
      int p = 8 + (current_lesson_slide * 8);
      lv_bar_set_value(bar_progress, p, LV_ANIM_ON);
    }
  } else {
    // Mode Node 1..N: LANGSUNG Ke Kartu Aksara Interaktif (Tanpa Konsep)
    if (current_lesson_slide == 0) {
      render_slide_cards_123(view_lesson, false /* Tidak ada konsep */);
    } else {
      render_slide_cards_4_and_start(view_lesson, 0 /* Kembali ke slide 0 */);
    }

    if (bar_progress) {
      int p = 20 + (current_lesson_slide * 20);
      lv_bar_set_value(bar_progress, p, LV_ANIM_ON);
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
      snprintf(b, sizeof(b), LV_SYMBOL_CHARGE " +%d XP", earned_xp);
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
  show_stage(2); // Ke Stage 2 (Tebak Aksara)
}

// ---------- Stage 2: Logika Tebak Aksara (Multiple Choice) ----------
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
      // Benar
      audio_play(SOUND_SUCCESS);
      earned_xp += 15;
      char b[24];
      snprintf(b, sizeof(b), LV_SYMBOL_CHARGE " +%d XP", earned_xp);
      lv_label_set_text(lbl_badge_xp, b);

      lv_obj_set_style_bg_color(guess_btns[guess_selected], C_OK_BG, 0);
      lv_obj_set_style_border_color(guess_btns[guess_selected], C_OK_BORDER, 0);
      lv_obj_set_style_border_width(guess_btns[guess_selected], 2, 0);
      lv_obj_set_style_text_color(guess_lbls[guess_selected], C_OK_TXT, 0);

      lv_label_set_text(guess_feedback_lbl, LV_SYMBOL_OK " Luar biasa! Jawabanmu benar (+15 XP)");
      lv_obj_set_style_text_color(guess_feedback_lbl, lv_color_hex(0x4ADE80), 0);
    } else {
      // Salah
      audio_play(SOUND_ERROR);
      lv_obj_set_style_bg_color(guess_btns[guess_selected], C_ERR_BG, 0);
      lv_obj_set_style_border_color(guess_btns[guess_selected], C_ERR_BORDER, 0);
      lv_obj_set_style_border_width(guess_btns[guess_selected], 2, 0);
      lv_obj_set_style_text_color(guess_lbls[guess_selected], C_ERR_TXT, 0);

      // Tunjukkan yang benar
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

    lv_label_set_text(guess_action_lbl, "LANJUT " LV_SYMBOL_RIGHT);
    lv_obj_set_style_bg_color(guess_action_btn, lv_color_hex(0x22C55E), 0);
  } else {
    // Pindah ke Stage Complete
    audio_play(SOUND_CLICK);
    show_stage(3);
  }
}

static void on_audio_hint_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_NODE);
  ui_show_toast("Mendengarkan bunyi aksara...");
}

// ---------- Stage 3: Selesai & Kembali ----------
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

static void on_mascot_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_BUDDY);
  ui_show_toast("Semangat belajarmu luar biasa!");
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

  // Judul pertanyaan ala Duolingo (Besar, jelas, dan proporsional)
  lv_obj_t* title = ui_make_label(view_match, "Pilih Pasangan yang Cocok", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 20, 6);

  lv_obj_t* sub = ui_make_label(view_match, "Sentuh aksara dan transliterasi latinnya", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(sub, 20, 28);

  // Buat 4 Pasang Tombol Kiri (Aksara) dan Kanan (Latin)
  for (int i = 0; i < PAIR_COUNT; i++) {
    int y_pos = 60 + (i * 54);

    // --- Tombol Kiri (Aksara) ---
    left_btns[i] = ui_make_card(view_match, 18, y_pos, 216, 46, C_CARD, C_CARD, 12);
    lv_obj_set_style_border_color(left_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(left_btns[i], 1, 0);
    ui_set_pressed(left_btns[i], C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(left_btns[i], on_left_card_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    // Badge Angka 1..4
    lv_obj_t* num_bg_l = ui_make_card(left_btns[i], 10, 10, 26, 26, lv_color_hex(0x232938), lv_color_hex(0x232938), 8);
    lv_obj_clear_flag(num_bg_l, LV_OBJ_FLAG_CLICKABLE);
    char num_str_l[8];
    snprintf(num_str_l, sizeof(num_str_l), "%d", i + 1);
    lv_obj_t* num_lbl_l = ui_make_label(num_bg_l, num_str_l, &lv_font_montserrat_14, C_SUB);
    lv_obj_center(num_lbl_l);

    // Karakter Aksara Native Font (22px)
    int item_idx = active_item_indices[i];
    left_lbls[i] = ui_make_label(left_btns[i], items[item_idx].aksara, font_ak, C_TEXT);
    lv_obj_align(left_lbls[i], LV_ALIGN_CENTER, 8, 0);

    // --- Tombol Kanan (Latin) ---
    right_btns[i] = ui_make_card(view_match, 246, y_pos, 216, 46, C_CARD, C_CARD, 12);
    lv_obj_set_style_border_color(right_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(right_btns[i], 1, 0);
    ui_set_pressed(right_btns[i], C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(right_btns[i], on_right_card_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    // Badge Angka 5..8
    lv_obj_t* num_bg_r = ui_make_card(right_btns[i], 10, 10, 26, 26, lv_color_hex(0x232938), lv_color_hex(0x232938), 8);
    lv_obj_clear_flag(num_bg_r, LV_OBJ_FLAG_CLICKABLE);
    char num_str_r[8];
    snprintf(num_str_r, sizeof(num_str_r), "%d", i + 5);
    lv_obj_t* num_lbl_r = ui_make_label(num_bg_r, num_str_r, &lv_font_montserrat_14, C_SUB);
    lv_obj_center(num_lbl_r);

    // Bunyi Latin
    int r_idx = active_item_indices[right_shuffle[i]];
    right_lbls[i] = ui_make_label(right_btns[i], items[r_idx].latin, &lv_font_montserrat_20, C_TEXT);
    lv_obj_align(right_lbls[i], LV_ALIGN_CENTER, 8, 0);
  }

  // Baris Kemenangan Bawah ala Duolingo
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

  // Maskot Avatar Kiri (Circular 52x52)
  lv_obj_t* avatar_card = ui_make_card(view_guess, 18, 10, 52, 52, C_CARD, C_CARD, 26);
  lv_obj_clear_flag(avatar_card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* avatar_img = lv_image_create(avatar_card);
  lv_image_set_src(avatar_img, (current_script == AKSARA_JAWA) ? &BuddyJawa : &BuddyBali);
  lv_obj_center(avatar_img);

  // Balon Ucapan Maskot (Speech Bubble Duolingo)
  lv_obj_t* bubble = ui_make_card(view_guess, 76, 8, 386, 56, C_CARD, C_CARD, 14);
  lv_obj_set_style_border_color(bubble, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(bubble, 1, 0);

  // Tombol Audio
  lv_obj_t* btn_audio = ui_make_card(bubble, 7, 7, 42, 42, C_CARD_PR, C_CARD_PR, 21);
  ui_set_pressed(btn_audio, lv_color_hex(0x133857), lv_color_hex(0x133857));
  lv_obj_add_event_cb(btn_audio, on_audio_hint_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* ic_audio = ui_make_label(btn_audio, LV_SYMBOL_VOLUME_MAX, &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_center(ic_audio);

  lv_obj_t* prompt = ui_make_label(bubble, "Manakah aksara yang berbunyi:", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(prompt, 58, 8);

  // Target Kata Latin
  char target_str[24];
  snprintf(target_str, sizeof(target_str), "\" %s \"", items[guess_correct_idx].latin);
  lv_color_t accent_color = (current_script == AKSARA_JAWA) ? C_JAWA_1 : C_BALI_1;
  lv_obj_t* target_lbl = ui_make_label(bubble, target_str, &lv_font_montserrat_20, accent_color);
  lv_obj_set_pos(target_lbl, 58, 28);

  // 4 Pilihan Ganda (Grid 2x2 JUMBO: Lebar 216px, Tinggi 60px)
  for (int i = 0; i < 4; i++) {
    int x_pos = (i % 2 == 0) ? 18 : 246;
    int y_pos = 76 + (i / 2) * 68;

    guess_btns[i] = ui_make_card(view_guess, x_pos, y_pos, 216, 60, C_CARD, C_CARD, 14);
    lv_obj_set_style_border_color(guess_btns[i], lv_color_hex(0x2A3245), 0);
    lv_obj_set_style_border_width(guess_btns[i], 1, 0);
    ui_set_pressed(guess_btns[i], C_CARD_PR, C_CARD_PR);
    lv_obj_add_event_cb(guess_btns[i], on_guess_choice_click, LV_EVENT_CLICKED, (void*)(intptr_t)i);

    // Option Tag A, B, C, D
    lv_obj_t* opt_bg = ui_make_card(guess_btns[i], 12, 16, 28, 28, lv_color_hex(0x232938), lv_color_hex(0x232938), 8);
    lv_obj_clear_flag(opt_bg, LV_OBJ_FLAG_CLICKABLE);
    static const char* OPTS[4] = { "A", "B", "C", "D" };
    lv_obj_t* opt_lbl = ui_make_label(opt_bg, OPTS[i], &lv_font_montserrat_14, C_SUB);
    lv_obj_center(opt_lbl);

    int c_idx = guess_choices[i];
    guess_lbls[i] = ui_make_label(guess_btns[i], items[c_idx].aksara, font_ak, C_TEXT);
    lv_obj_align(guess_lbls[i], LV_ALIGN_CENTER, 8, 0);
  }

  // Action Bar Bawah (Tinggi 60px)
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

static void build_stage_complete() {
  if (view_complete) {
    lv_obj_clean(view_complete);
  } else {
    view_complete = lv_obj_create(content_cont);
    lv_obj_remove_style_all(view_complete);
    lv_obj_set_size(view_complete, 480, 276);
    lv_obj_clear_flag(view_complete, LV_OBJ_FLAG_SCROLLABLE);
  }

  // Layout Landscape SIMETRIS & DI-TENGAHIN (Centered Harmonis)
  // Judul & Subtitle di Tengah Atas
  lv_obj_t* title = ui_make_label(view_complete, "Hore! Pelajaran Selesai!", &lv_font_montserrat_20, lv_color_hex(0x22C55E));
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

  lv_obj_t* sub = ui_make_label(view_complete, "Kamu telah menuntaskan materi & kuis aksara ini!", &lv_font_montserrat_14, C_SUB);
  lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, 44);

  // --- KIRI: Frame Maskot AksaraBuddy (x=32, y=74) ---
  lv_obj_t* mascot_frame = ui_make_card(view_complete, 32, 74, 120, 146, lv_color_hex(0x141824), lv_color_hex(0x141824), 16);
  lv_obj_set_style_border_color(mascot_frame, lv_color_hex(0x22324A), 0);
  lv_obj_set_style_border_width(mascot_frame, 1, 0);
  lv_obj_add_event_cb(mascot_frame, on_mascot_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* mascot_img = lv_image_create(mascot_frame);
  lv_image_set_src(mascot_img, &BuddyMaskot);
  lv_obj_align(mascot_img, LV_ALIGN_TOP_MID, 0, 4);

  lv_obj_t* win_tag = ui_make_card(mascot_frame, 10, 114, 100, 24, lv_color_hex(0x241D10), lv_color_hex(0x241D10), 8);
  lv_obj_clear_flag(win_tag, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* tag_lbl = ui_make_label(win_tag, LV_SYMBOL_OK " HEBAT!", &lv_font_montserrat_14, lv_color_hex(0xF59E0B));
  lv_obj_center(tag_lbl);

  // --- KANAN: Statistik & Tombol Kembali (x=168, lebar=280) ---
  // Stat 1: XP
  lv_obj_t* stat1 = ui_make_card(view_complete, 168, 74, 134, 56, lv_color_hex(0x241D10), lv_color_hex(0x241D10), 12);
  lv_obj_set_style_border_color(stat1, lv_color_hex(0x78350F), 0);
  lv_obj_set_style_border_width(stat1, 1, 0);
  lv_obj_clear_flag(stat1, LV_OBJ_FLAG_CLICKABLE);

  char stat_xp_str[24];
  snprintf(stat_xp_str, sizeof(stat_xp_str), LV_SYMBOL_CHARGE " +%d XP", earned_xp);
  lv_obj_t* lbl_xp = ui_make_label(stat1, stat_xp_str, &lv_font_montserrat_20, lv_color_hex(0xF59E0B));
  lv_obj_set_pos(lbl_xp, 12, 6);
  lv_obj_t* lbl_xp_sub = ui_make_label(stat1, "Poin Belajar", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(lbl_xp_sub, 12, 32);

  // Stat 2: Akurasi
  lv_obj_t* stat2 = ui_make_card(view_complete, 314, 74, 134, 56, lv_color_hex(0x0F291E), lv_color_hex(0x0F291E), 12);
  lv_obj_set_style_border_color(stat2, lv_color_hex(0x065F46), 0);
  lv_obj_set_style_border_width(stat2, 1, 0);
  lv_obj_clear_flag(stat2, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t* lbl_acc = ui_make_label(stat2, LV_SYMBOL_OK " 100%", &lv_font_montserrat_20, lv_color_hex(0x10B981));
  lv_obj_set_pos(lbl_acc, 12, 6);
  lv_obj_t* lbl_acc_sub = ui_make_label(stat2, "Akurasi Jawaban", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(lbl_acc_sub, 12, 32);

  // Tombol CTA Utama (Lebar 280px, Tinggi 50px)
  lv_obj_t* btn_finish = ui_make_card(view_complete, 168, 144, 280, 50, lv_color_hex(0x22C55E), lv_color_hex(0x16A34A), 14);
  ui_set_pressed(btn_finish, lv_color_hex(0x15803D), lv_color_hex(0x15803D));
  lv_obj_add_event_cb(btn_finish, on_finish_learn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* btn_lbl = ui_make_label(btn_finish, "KEMBALI KE PETA AKSARA " LV_SYMBOL_RIGHT, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(btn_lbl);
}

static void show_stage(int stage) {
  current_stage = stage;

  // Sembunyikan semua sub-views
  if (view_lesson) lv_obj_add_flag(view_lesson, LV_OBJ_FLAG_HIDDEN);
  if (view_match) lv_obj_add_flag(view_match, LV_OBJ_FLAG_HIDDEN);
  if (view_guess) lv_obj_add_flag(view_guess, LV_OBJ_FLAG_HIDDEN);
  if (view_complete) lv_obj_add_flag(view_complete, LV_OBJ_FLAG_HIDDEN);

  if (stage == 0) {
    current_lesson_slide = 0;
    build_stage_lesson();
    lv_obj_clear_flag(view_lesson, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 1) {
    // Stage 1: Pasangkan Aksara (Match Pairs)
    lv_bar_set_value(bar_progress, 70, LV_ANIM_ON);
    build_stage_match();
    lv_obj_clear_flag(view_match, LV_OBJ_FLAG_HIDDEN);
  } else if (stage == 2) {
    // Stage 2: Tebak Aksara (Quiz Multiple Choice)
    lv_bar_set_value(bar_progress, 88, LV_ANIM_ON);
    build_stage_guess();
    lv_obj_clear_flag(view_guess, LV_OBJ_FLAG_HIDDEN);
  } else {
    // Stage 3: Rekap Kemenangan Selesai
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

  // Tombol Close "X" (Lebih besar: 38x38 px lingkaran)
  lv_obj_t* btn_close = ui_make_card(header, 14, 3, 38, 38, C_CARD, C_CARD, 19);
  lv_obj_set_style_border_color(btn_close, lv_color_hex(0x2A3245), 0);
  lv_obj_set_style_border_width(btn_close, 1, 0);
  ui_set_pressed(btn_close, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btn_close, on_close_learn_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* ic_close = ui_make_label(btn_close, LV_SYMBOL_CLOSE, &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(ic_close);

  // Progress Bar ala Duolingo (Tebal 12px)
  bar_progress = lv_bar_create(header);
  lv_obj_set_pos(bar_progress, 62, 16);
  lv_obj_set_size(bar_progress, 266, 12);
  lv_obj_set_style_radius(bar_progress, 6, 0);
  lv_obj_set_style_bg_color(bar_progress, lv_color_hex(0x1E2433), 0);
  lv_obj_set_style_bg_color(bar_progress, lv_color_hex(0x22C55E), LV_PART_INDICATOR);
  lv_obj_set_style_radius(bar_progress, 6, LV_PART_INDICATOR);
  lv_bar_set_range(bar_progress, 0, 100);
  lv_bar_set_value(bar_progress, 8, LV_ANIM_OFF);

  // XP Badge (Pill Card berkelas dengan border Amber, ikon LV_SYMBOL_CHARGE resmi)
  lv_obj_t* badge_card = ui_make_card(header, 342, 5, 124, 34, lv_color_hex(0x241D10), lv_color_hex(0x241D10), 17);
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

  // Reset status buka kartu interaktif
  for (int i = 0; i < 4; i++) {
    card_revealed[i] = false;
    card_animating[i] = false;
  }

  char b[24];
  snprintf(b, sizeof(b), LV_SYMBOL_CHARGE " +%d XP", earned_xp);
  if (lbl_badge_xp) lv_label_set_text(lbl_badge_xp, b);

  // Pilih 4 item berurutan dari level
  int base_idx = current_target_item_idx;
  for (int i = 0; i < PAIR_COUNT; i++) {
    active_item_indices[i] = (base_idx + i) % 10;
  }

  // Acak tombol kanan (shuffle deterministik agar seru)
  right_shuffle[0] = 2;
  right_shuffle[1] = 0;
  right_shuffle[2] = 3;
  right_shuffle[3] = 1;

  // Siapkan Stage 2 (Tebak Aksara)
  guess_correct_idx = active_item_indices[0];
  guess_choices[0] = active_item_indices[2];
  guess_choices[1] = active_item_indices[0]; // Jawaban benar di posisi 1
  guess_choices[2] = active_item_indices[3];
  guess_choices[3] = active_item_indices[1];

  // Mulai dari Stage 0 (Materi Pembelajaran)
  show_stage(0);
  ui_learn_show();
}

void ui_learn_show() {
  if (scr_learn) {
    lv_scr_load_anim(scr_learn, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
  }
}
