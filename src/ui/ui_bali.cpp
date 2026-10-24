#include "ui_bali.h"
#include "ui_home.h"
#include "ui_learn.h"
#include <stdio.h>

static const AppCallbacks* bali_cbs = nullptr;
static lv_obj_t* scr_bali = nullptr;

// Container 3 Tampilan
static lv_obj_t* view_path = nullptr;
static lv_obj_t* view_write = nullptr;
static lv_obj_t* view_settings = nullptr;

// Tombol Tab Bawah
static lv_obj_t* tab_btn_learn = nullptr;
static lv_obj_t* tab_btn_write = nullptr;
static lv_obj_t* tab_btn_settings = nullptr;
static lv_obj_t* tab_lbl_learn = nullptr;
static lv_obj_t* tab_lbl_write = nullptr;
static lv_obj_t* tab_lbl_settings = nullptr;

// State Tab Aktif: 0 = Belajar, 1 = Nulis Aksara, 2 = Setelan
static int current_tab = 0;

// Pagination Jalur Belajar (Tanpa Scroll, Anti Nge-lag)
static int current_page = 0; // 0 = Babak 1, 1 = Babak 2, 2 = Babak 3
static lv_obj_t* path_nodes_cont = nullptr;
static lv_obj_t* btn_page_up = nullptr;
static lv_obj_t* btn_page_down = nullptr;
static lv_obj_t* lbl_page_ind = nullptr;
static lv_obj_t* header_sub = nullptr;

// Aksara Bali Data (Wreastra - 18 Konsonan Lengkap)
struct PathNodeData {
  const char* name;
  const char* symbol;
  int state; // 0 = Selesai, 1 = Aktif, 2 = Peti/Bonus, 3 = Terkunci
};

// 6 Halaman, masing-masing 3 node (total 18 node Wreastra lengkap)
static const PathNodeData BALI_PAGES[6][3] = {
  // Babak 1
  {
    { "Ha", "\xE1\xAC\xB3", 0 },
    { "Na", "\xE1\xAC\xA6", 0 },
    { "Ca", "\xE1\xAC\x98", 1 }
  },
  // Babak 2
  {
    { "Ra", "\xE1\xAC\xAD", 3 },
    { "Ka", "\xE1\xAC\x93", 3 },
    { "Da", "\xE1\xAC\xA4", 3 }
  },
  // Babak 3
  {
    { "Ta", "\xE1\xAC\xA2", 3 },
    { "Sa", "\xE1\xAC\xB2", 3 },
    { "Wa", "\xE1\xAC\xAF", 3 }
  },
  // Babak 4
  {
    { "La", "\xE1\xAC\xAE", 3 },
    { "Ma", "\xE1\xAC\xAB", 3 },
    { "Ga", "\xE1\xAC\x95", 3 }
  },
  // Babak 5
  {
    { "Ba", "\xE1\xAC\xA9", 3 },
    { "Nga", "\xE1\xAC\x97", 3 },
    { "Pa", "\xE1\xAC\xA7", 3 }
  },
  // Babak 6
  {
    { "Ja", "\xE1\xAC\x9A", 3 },
    { "Ya", "\xE1\xAC\xAC", 3 },
    { "Nya", "\xE1\xAC\x9C", 3 }
  }
};

// Canvas Nulis Aksara Bali
static lv_obj_t* write_target_title = nullptr;
static lv_obj_t* write_target_sub = nullptr;
static lv_obj_t* write_watermark_lbl = nullptr;
static lv_obj_t* write_guide_track = nullptr;
static lv_obj_t* write_feedback_lbl = nullptr;
static lv_obj_t* write_preview_lbl = nullptr;
static int active_write_index = 2; // Default ke aksara "Ca"
static TraceState bali_tab_trace;
static lv_point_precise_t bali_guide_pts[100];

// 18 Aksara Bali Wreastra Lengkap
static const char* BALI_NAMES[18] = {
  "Ha", "Na", "Ca", "Ra", "Ka", "Da",
  "Ta", "Sa", "Wa", "La", "Ma", "Ga",
  "Ba", "Nga", "Pa", "Ja", "Ya", "Nya"
};
static const char* BALI_AKSARA[18] = {
  "\xE1\xAC\xB3", "\xE1\xAC\xA6", "\xE1\xAC\x98", "\xE1\xAC\xAD", "\xE1\xAC\x93", "\xE1\xAC\xA4",
  "\xE1\xAC\xA2", "\xE1\xAC\xB2", "\xE1\xAC\xAF", "\xE1\xAC\xAE", "\xE1\xAC\xAB", "\xE1\xAC\x95",
  "\xE1\xAC\xA9", "\xE1\xAC\x97", "\xE1\xAC\xA7", "\xE1\xAC\x9A", "\xE1\xAC\xAC", "\xE1\xAC\x9C"
};

// Waypoint Jalur Aksara Bali (Tersinkronisasi Presisi dengan Font Asli 160px di Tengah Kanvas 308x160)
static const StrokeWaypoint BALI_GUIDE_HA[] = {
  { 80, 66 }, { 80, 59 }, { 82, 52 }, { 87, 47 }, { 93, 43 }, 
  { 100, 42 }, { 107, 42 }, { 108, 49 }, { 104, 56 }, { 103, 63 }, 
  { 101, 70 }, { 98, 77 }, { 96, 84 }, { 95, 91 }, { 95, 98 }, 
  { 97, 105 }, { 102, 110 }, { 107, 116 }, { 114, 118 }, { 121, 120 }, 
  { 128, 118 }, { 135, 117 }, { 138, 110 }, { 142, 104 }, { 144, 97 }, 
  { 146, 90 }, { 146, 83 }, { 146, 76 }, { 145, 69 }, { 145, 62 }, 
  { 145, 55 }, { 147, 48 }, { 153, 44 }, { 160, 41 }, { 167, 41 }, 
  { 174, 43 }, { 180, 47 }, { 184, 53 }, { 186, 60 }, { 188, 67 }, 
  { 190, 74 }, { 190, 81 }, { 192, 88 }, { 192, 95 }, { 193, 102 }, 
  { 194, 109 }, { 194, 116 }, { 194, 109 }, { 194, 102 }, { 194, 95 }, 
  { 194, 88 }, { 195, 81 }, { 196, 74 }, { 195, 67 }, { 195, 60 }, 
  { 194, 53 }, { 195, 46 }, { 202, 43 }, { 209, 42 }, { 216, 43 }, 
  { 223, 44 }, { 224, 51 }, { 226, 58 }, { 226, 65 }, { 227, 72 }, 
  { 227, 79 }, { 227, 86 }, { 227, 93 }, { 227, 100 }, { 227, 107 }, 
  { 228, 114 }
};
static const StrokeWaypoint BALI_GUIDE_NA[] = {
  { 122, 106 }, { 132, 56 }, { 146, 92 }, { 158, 58 }, { 172, 102 }, { 184, 78 }
};
static const StrokeWaypoint BALI_GUIDE_CA[] = {
  { 92, 80 }, { 90, 64 }, { 90, 49 }, { 101, 38 }, { 116, 44 }, 
  { 123, 60 }, { 125, 76 }, { 125, 91 }, { 127, 107 }, { 118, 120 }, 
  { 103, 122 }, { 98, 107 }, { 107, 93 }, { 123, 96 }, { 136, 104 }, 
  { 147, 116 }, { 163, 118 }, { 178, 118 }, { 190, 107 }, { 192, 91 }, 
  { 183, 78 }, { 170, 67 }, { 163, 51 }, { 178, 47 }, { 194, 49 }, 
  { 210, 56 }, { 216, 71 }, { 218, 87 }, { 218, 102 }, { 218, 118 }
};
static const StrokeWaypoint BALI_GUIDE_RA[] = {
  { 124, 108 }, { 138, 58 }, { 152, 98 }, { 166, 58 }, { 180, 108 }
};
static const StrokeWaypoint BALI_GUIDE_KA[] = {
  { 122, 85 }, { 134, 55 }, { 142, 106 }, { 154, 70 }, { 168, 56 }, { 182, 108 }
};
static const StrokeWaypoint BALI_GUIDE_DA[] = {
  { 124, 106 }, { 140, 56 }, { 154, 82 }, { 168, 62 }, { 182, 108 }
};
static const StrokeWaypoint BALI_GUIDE_TA[] = {
  { 122, 108 }, { 136, 56 }, { 150, 72 }, { 164, 56 }, { 178, 108 }
};

static const StrokeWaypoint BALI_GUIDE_DEF[] = {
  { 104, 115 }, { 120, 52 }, { 148, 100 }, { 178, 52 }, { 204, 115 }
};

static void get_bali_guide(int index, const StrokeWaypoint*& pts, uint8_t& count) {
  switch (index) {
    case 0: pts = BALI_GUIDE_HA; count = sizeof(BALI_GUIDE_HA)/sizeof(BALI_GUIDE_HA[0]); break;
    case 1: pts = BALI_GUIDE_NA; count = sizeof(BALI_GUIDE_NA)/sizeof(BALI_GUIDE_NA[0]); break;
    case 2: pts = BALI_GUIDE_CA; count = sizeof(BALI_GUIDE_CA)/sizeof(BALI_GUIDE_CA[0]); break;
    case 3: pts = BALI_GUIDE_RA; count = sizeof(BALI_GUIDE_RA)/sizeof(BALI_GUIDE_RA[0]); break;
    case 4: pts = BALI_GUIDE_KA; count = sizeof(BALI_GUIDE_KA)/sizeof(BALI_GUIDE_KA[0]); break;
    case 5: pts = BALI_GUIDE_DA; count = sizeof(BALI_GUIDE_DA)/sizeof(BALI_GUIDE_DA[0]); break;
    case 6: pts = BALI_GUIDE_TA; count = sizeof(BALI_GUIDE_TA)/sizeof(BALI_GUIDE_TA[0]); break;
    default: pts = BALI_GUIDE_DEF; count = sizeof(BALI_GUIDE_DEF)/sizeof(BALI_GUIDE_DEF[0]); break;
  }
}

static void update_bali_write_guide() {
  if (write_watermark_lbl) {
    if (active_write_index >= 0 && active_write_index < 18) {
      lv_label_set_text(write_watermark_lbl, BALI_AKSARA[active_write_index]);
      lv_obj_align(write_watermark_lbl, LV_ALIGN_CENTER, 0, 29);
    }
  }

  if (!write_guide_track) return;
  const StrokeWaypoint* guide_pts = nullptr;
  uint8_t count = 0;
  get_bali_guide(active_write_index, guide_pts, count);

  for (int i = 0; i < count && i < 100; i++) {
    bali_guide_pts[i].x = guide_pts[i].x;
    bali_guide_pts[i].y = guide_pts[i].y;
  }
  lv_line_set_points(write_guide_track, bali_guide_pts, count > 100 ? 100 : count);

  // Inisialisasi checkpoints urutan titik P1 .. Pn (R=18px, Koridor=22px, Batas Gagal Parah=34px)
  ui_trace_set_checkpoints(&bali_tab_trace, guide_pts, count, 18, 22, 34);
  ui_trace_clear(&bali_tab_trace);
}

// Deklarasi fungsi navigasi
static void switch_tab(int tab_index);
static void update_path_page();

// ---------- Event Handlers ----------
static void on_back_home_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  ui_home_show();
}

static void on_tab_click(lv_event_t* e) {
  int target_tab = (int)(intptr_t)lv_event_get_user_data(e);
  audio_play(SOUND_CLICK);
  switch_tab(target_tab);
}

static void on_page_up_click(lv_event_t* e) {
  (void)e;
  if (current_page > 0) {
    audio_play(SOUND_PAGE);
    current_page--;
    update_path_page();
  }
}

static void on_page_down_click(lv_event_t* e) {
  (void)e;
  if (current_page < 5) {
    audio_play(SOUND_PAGE);
    current_page++;
    update_path_page();
  }
}

static void on_node_click(lv_event_t* e) {
  int node_idx = (int)(intptr_t)lv_event_get_user_data(e);
  const PathNodeData& node = BALI_PAGES[current_page][node_idx];

  char buf[64];
  int active_idx = current_page * 3 + node_idx;
  active_write_index = active_idx;

  if (node.state == 0) {
    audio_play(SOUND_NODE);
    snprintf(buf, sizeof(buf), "Aksara Bali %s: Latihan Ulang!", node.name);
    ui_show_toast(buf);
    ui_learn_start(AKSARA_BALI, active_idx);
  } else if (node.state == 1) {
    audio_play(SOUND_NODE);
    snprintf(buf, sizeof(buf), "Level Aktif: Aksara %s!", node.name);
    ui_show_toast(buf);
    ui_learn_start(AKSARA_BALI, active_idx);
  } else if (node.state == 2) {
    audio_play(SOUND_REWARD);
    ui_show_toast("Peti Hadiah Sastra: +50 XP!");
  } else {
    audio_play(SOUND_NODE);
    snprintf(buf, sizeof(buf), "Mulai Belajar Aksara %s!", node.name);
    ui_show_toast(buf);
    ui_learn_start(AKSARA_BALI, active_idx);
  }

  if (bali_cbs && bali_cbs->onSelectAksara) {
    bali_cbs->onSelectAksara(AKSARA_BALI, active_idx);
  }
}

static void on_clear_canvas_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  ui_trace_clear(&bali_tab_trace);
  ui_show_toast("Kanvas dibersihkan");
}

static void on_check_write_click(lv_event_t* e) {
  (void)e;
  StrokeEvalResult eval = ui_eval_stroke_trace(&bali_tab_trace);
  if (eval.success) {
    ui_trace_set_completed_visuals(&bali_tab_trace);
    audio_play(SOUND_SUCCESS);
    audio_play(SOUND_REWARD);
    if (write_feedback_lbl) {
      lv_label_set_text(write_feedback_lbl, "BENAR!");
      lv_obj_set_style_text_color(write_feedback_lbl, lv_color_hex(0x22C55E), 0);
    }
    ui_show_toast(eval.message);
  } else {
    audio_play(SOUND_ERROR);
    if (write_feedback_lbl) {
      lv_label_set_text(write_feedback_lbl, "BELUM SELESAI");
      lv_obj_set_style_text_color(write_feedback_lbl, lv_color_hex(0xEF4444), 0);
    }
    ui_show_toast(eval.message);
  }
}

static void mascot_bounce_anim_cb(void* var, int32_t val) {
  lv_image_set_scale((lv_obj_t*)var, val);
}

static void on_mascot_click(lv_event_t* e) {
  audio_play(SOUND_BUDDY);
  lv_obj_t* target = (lv_obj_t*)lv_event_get_target(e);
  if (target) {
    lv_anim_del(target, mascot_bounce_anim_cb);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, target);
    lv_anim_set_exec_cb(&a, mascot_bounce_anim_cb);
    lv_anim_set_values(&a, 256, 292); // Sedikit membesar (~114%) lalu kembali ke 256 (100%)
    lv_anim_set_duration(&a, 120);
    lv_anim_set_reverse_duration(&a, 130);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
  }

  static int mascot_quote_idx = 0;
  const char* quotes[] = {
    "Buddy: Rahajeng! Semangat nulis Aksara Bali!",
    "Buddy: Luar biasa, yuk teruskan langkahmu!",
    "Buddy: Setiap aksara punya keindahan tersendiri!",
    "Buddy: Keren banget progress belajarmu!"
  };
  ui_show_toast(quotes[mascot_quote_idx % 4]);
  mascot_quote_idx++;
}

// Helper membuat Node 3D ala Duolingo
static lv_obj_t* create_duo_node(lv_obj_t* parent, int x, int y, int size,
                                 lv_color_t color_top, lv_color_t color_dark,
                                 const char* text, const lv_font_t* font,
                                 bool is_active, int node_idx) {
  lv_obj_t* cont = lv_obj_create(parent);
  lv_obj_remove_style_all(cont);
  lv_obj_set_pos(cont, x, y);
  lv_obj_set_size(cont, size, size + 8);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

  // Lapisan dasar 3D
  lv_obj_t* base = ui_make_card(cont, 0, 6, size, size, color_dark, color_dark, LV_RADIUS_CIRCLE);
  lv_obj_clear_flag(base, LV_OBJ_FLAG_CLICKABLE);

  // Lapisan tombol atas
  lv_obj_t* btn = ui_make_card(cont, 0, 0, size, size, color_top, color_top, LV_RADIUS_CIRCLE);
  ui_set_pressed(btn, color_dark, color_dark);
  lv_obj_add_event_cb(btn, on_node_click, LV_EVENT_CLICKED, (void*)(intptr_t)node_idx);

  // Icon atau Label aksara
  lv_obj_t* lbl = ui_make_label(btn, text, font, lv_color_hex(0xFFFFFF));
  lv_obj_center(lbl);

  // Floating pill "MULAI" di atas node aktif
  if (is_active) {
    lv_obj_t* pill = ui_make_card(parent, x - 2, y - 24, size + 4, 22,
                                 lv_color_hex(0x10B981), lv_color_hex(0x059669), 11);
    lv_obj_clear_flag(pill, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t* pillLbl = ui_make_label(pill, "MULAI", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
    lv_obj_center(pillLbl);
  }

  return cont;
}

// Titik penghubung jalur
static void create_trail_dots(lv_obj_t* parent, int x1, int y1, int x2, int y2) {
  for (int step = 1; step <= 2; step++) {
    float t = step / 3.0f;
    int px = (int)(x1 + t * (x2 - x1));
    int py = (int)(y1 + t * (y2 - y1));
    lv_obj_t* dot = ui_make_card(parent, px + 26, py + 26, 10, 10,
                                 lv_color_hex(0x475569), lv_color_hex(0x475569), LV_RADIUS_CIRCLE);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);
  }
}

// Perbarui Konten Halaman Aktif (Tanpa Scroll)
static void update_path_page() {
  if (!path_nodes_cont) return;

  // Hapus elemen halaman sebelumnya
  lv_obj_clean(path_nodes_cont);

  // Update teks indikator halaman
  char ind_buf[16];
  snprintf(ind_buf, sizeof(ind_buf), "%d / 6", current_page + 1);
  lv_label_set_text(lbl_page_ind, ind_buf);

  char sub_buf[32];
  snprintf(sub_buf, sizeof(sub_buf), "Jalur Belajar • Babak %d / 6", current_page + 1);
  if (header_sub) lv_label_set_text(header_sub, sub_buf);

  // Update tombol panah atas/bawah
  if (current_page == 0) {
    lv_obj_set_style_opa(btn_page_up, LV_OPA_30, 0);
  } else {
    lv_obj_set_style_opa(btn_page_up, LV_OPA_COVER, 0);
  }

  if (current_page == 5) {
    lv_obj_set_style_opa(btn_page_down, LV_OPA_30, 0);
  } else {
    lv_obj_set_style_opa(btn_page_down, LV_OPA_COVER, 0);
  }

  // Koordinat 3 Node berbeda untuk setiap babak (variasi jalur berliku dinamis melengkung kiri/kanan)
  static const int POS[6][3][2] = {
    // Babak 1: Melengkung ke Kiri (Node di X=70..165, Info Card di KANAN X=246)
    { { 165, 16 }, { 70,  82 }, { 155, 148 } },
    // Babak 2: Melengkung ke Kanan (Node di X=185..280, Info Card di KIRI X=14)
    { { 185, 16 }, { 280, 82 }, { 195, 148 } },
    // Babak 3: Jalur Gelombang Kiri
    { { 165, 16 }, { 70,  82 }, { 155, 148 } },
    // Babak 4: Jalur Gelombang Kanan
    { { 185, 16 }, { 280, 82 }, { 195, 148 } },
    // Babak 5: Jalur Gelombang Kiri
    { { 165, 16 }, { 70,  82 }, { 155, 148 } },
    // Babak 6: Jalur Gelombang Kanan
    { { 185, 16 }, { 280, 82 }, { 195, 148 } }
  };

  // Titik jalur penghubung (S-curve sesuai babak)
  create_trail_dots(path_nodes_cont, POS[current_page][0][0], POS[current_page][0][1], POS[current_page][1][0], POS[current_page][1][1]);
  create_trail_dots(path_nodes_cont, POS[current_page][1][0], POS[current_page][1][1], POS[current_page][2][0], POS[current_page][2][1]);

  // Gambar 3 Node halaman ini dengan posisi khusus babak tersebut
  for (int i = 0; i < 3; i++) {
    const PathNodeData& node = BALI_PAGES[current_page][i];
    lv_color_t c_top, c_dark;
    const char* txt = node.symbol;
    const lv_font_t* f = &font_bali_22;

    if (node.state == 0) {
      // Selesai (Emerald/Teal)
      c_top = lv_color_hex(0x059669);
      c_dark = lv_color_hex(0x047857);
    } else if (node.state == 1) {
      // Sedang Aktif (Emas/Amber Bali)
      c_top = C_BALI_1;
      c_dark = C_BALI_2;
    } else if (node.state == 2) {
      // Peti / Piala
      c_top = lv_color_hex(0xEA580C);
      c_dark = lv_color_hex(0xC2410C);
      f = &lv_font_montserrat_14;
      txt = "Win";
    } else {
      // Terkunci
      c_top = lv_color_hex(0x334155);
      c_dark = lv_color_hex(0x1E293B);
    }

    create_duo_node(path_nodes_cont, POS[current_page][i][0], POS[current_page][i][1], 62,
                    c_top, c_dark, txt, f, (node.state == 1), i);
  }

  // Info Card & Maskot menyesuaikan ruang kosong agar tidak tabrakan dengan node:
  bool is_right_curve = (current_page % 2 == 1);
  int card_x = is_right_curve ? 14 : 244;
  int card_y = 10;
  lv_obj_t* infoCard = ui_make_card(path_nodes_cont, card_x, card_y, 146, 70, C_CARD, C_CARD, 12);
  lv_obj_clear_flag(infoCard, LV_OBJ_FLAG_CLICKABLE);

  struct BabakMeta {
    const char* title;
    const char* letters;
    const char* desc;
    lv_color_t color;
  };

  static const BabakMeta BALI_META[6] = {
    { "Babak 1: Dasar", "Ha • Na • Ca", "Level 1 - 3", C_BALI_1 },
    { "Babak 2: Wreastra", "Ra • Ka • Da", "Level 4 - 6", lv_color_hex(0x10B981) },
    { "Babak 3: Lanjutan", "Ta • Sa • Wa", "Level 7 - 9", lv_color_hex(0x38BDF8) },
    { "Babak 4: Menengah", "La • Ma • Ga", "Level 10 - 12", lv_color_hex(0xA855F7) },
    { "Babak 5: Terampil", "Ba • Nga • Pa", "Level 13 - 15", lv_color_hex(0xEC4899) },
    { "Babak 6: Mahir", "Ja • Ya • Nya", "Tamat 18 Aksara!", lv_color_hex(0xF59E0B) }
  };

  const BabakMeta& meta = BALI_META[current_page];
  lv_obj_t* t1 = ui_make_label(infoCard, meta.title, &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(t1, 10, 6);
  lv_obj_t* t2 = ui_make_label(infoCard, meta.letters, &lv_font_montserrat_14, meta.color);
  lv_obj_set_pos(t2, 10, 26);
  lv_obj_t* t3 = ui_make_label(infoCard, meta.desc, &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(t3, 10, 46);

  // Maskot Buddy berposisi di sisi berlawanan dari node
  const lv_image_dsc_t* buddy_src = (current_page % 3 == 0) ? &BuddyMaskot : ((current_page % 3 == 1) ? &Buddy3 : &Buddy4);
  int mascot_x = is_right_curve ? 36 : 268;
  int mascot_y = 86;

  lv_obj_t* mascot = lv_image_create(path_nodes_cont);
  lv_image_set_src(mascot, buddy_src);
  lv_image_set_pivot(mascot, buddy_src->header.w / 2, buddy_src->header.h / 2);
  lv_obj_set_pos(mascot, mascot_x, mascot_y);
  lv_obj_add_flag(mascot, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(mascot, on_mascot_click, LV_EVENT_CLICKED, NULL);
}

// ---------- 1. Tampilan Jalur Belajar Berhalaman (Paging) ----------
static void build_path_view(lv_obj_t* parent) {
  view_path = lv_obj_create(parent);
  lv_obj_remove_style_all(view_path);
  lv_obj_set_pos(view_path, 0, 0);
  lv_obj_set_size(view_path, 480, 224);
  lv_obj_clear_flag(view_path, LV_OBJ_FLAG_SCROLLABLE); // Tanpa scroll
  lv_obj_set_style_bg_color(view_path, C_BG, 0);
  lv_obj_set_style_bg_opa(view_path, LV_OPA_COVER, 0);

  // Area Node Aksara
  path_nodes_cont = lv_obj_create(view_path);
  lv_obj_remove_style_all(path_nodes_cont);
  lv_obj_set_pos(path_nodes_cont, 0, 0);
  lv_obj_set_size(path_nodes_cont, 396, 224);
  lv_obj_clear_flag(path_nodes_cont, LV_OBJ_FLAG_SCROLLABLE);

  // Panel Tombol Navigasi Panah Atas & Bawah
  lv_obj_t* nav_side = ui_make_card(view_path, 400, 8, 70, 208, C_CARD, C_CARD, 16);
  lv_obj_clear_flag(nav_side, LV_OBJ_FLAG_SCROLLABLE);

  // Tombol Panah Atas (▲)
  btn_page_up = ui_make_card(nav_side, 8, 10, 54, 52, C_BALI_1, C_BALI_2, 14);
  ui_set_pressed(btn_page_up, C_BALI_2, C_BALI_2);
  lv_obj_add_event_cb(btn_page_up, on_page_up_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* upIc = ui_make_label(btn_page_up, LV_SYMBOL_UP, &lv_font_montserrat_20, lv_color_hex(0xFFFFFF));
  lv_obj_center(upIc);

  // Indikator Halaman ("1 / 6")
  lv_obj_t* ind_box = ui_make_card(nav_side, 6, 80, 58, 36, C_CARD_PR, C_CARD_PR, 10);
  lv_obj_clear_flag(ind_box, LV_OBJ_FLAG_CLICKABLE);
  lbl_page_ind = ui_make_label(ind_box, "1 / 6", &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(lbl_page_ind);

  // Tombol Panah Bawah (▼)
  btn_page_down = ui_make_card(nav_side, 8, 134, 54, 52, C_BALI_1, C_BALI_2, 14);
  ui_set_pressed(btn_page_down, C_BALI_2, C_BALI_2);
  lv_obj_add_event_cb(btn_page_down, on_page_down_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* downIc = ui_make_label(btn_page_down, LV_SYMBOL_DOWN, &lv_font_montserrat_20, lv_color_hex(0xFFFFFF));
  lv_obj_center(downIc);

  // Gambar konten halaman pertama
  update_path_page();
}

// ---------- 2. Tampilan Nulis Aksara Bali (Writing Canvas) ----------
static void build_write_view(lv_obj_t* parent) {
  view_write = lv_obj_create(parent);
  lv_obj_remove_style_all(view_write);
  lv_obj_set_pos(view_write, 0, 0);
  lv_obj_set_size(view_write, 480, 224);
  lv_obj_clear_flag(view_write, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(view_write, LV_OBJ_FLAG_HIDDEN);

  // Kartu Target Aksara Bali (Kiri)
  lv_obj_t* cardTarget = ui_make_card(view_write, 12, 8, 140, 208, C_CARD, C_CARD, 16);
  write_target_title = ui_make_label(cardTarget, "Aksara Ca", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(write_target_title, 14, 14);

  write_target_sub = ui_make_label(cardTarget, "Wreastra Bali\nBunyi: /tja/", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(write_target_sub, 14, 40);

  // Preview Kotak Huruf
  lv_obj_t* boxHuruf = ui_make_card(cardTarget, 14, 84, 112, 110, C_BALI_1, C_BALI_2, 14);
  write_preview_lbl = ui_make_label(boxHuruf, "Ca", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF));
  lv_obj_center(write_preview_lbl);

  // Area Kanvas Tracing ala Contoh Gambar User (308x160 px)
  lv_obj_t* canvasCard = ui_make_card(view_write, 160, 8, 308, 160, lv_color_hex(0x0A0F1D), lv_color_hex(0x0A0F1D), 16);
  lv_obj_set_style_border_color(canvasCard, C_BALI_1, 0);
  lv_obj_set_style_border_width(canvasCard, 2, 0);

  // Watermark Glif Aksara Asli Transparan (Background Guide 160px JUMBO)
  write_watermark_lbl = ui_make_label(canvasCard, "\xE1\xAC\x98", &font_bali_160, lv_color_hex(0x38BDF8));
  lv_obj_set_style_text_opa(write_watermark_lbl, LV_OPA_30, 0); // 30% opasitas lembut transparan
  lv_obj_align(write_watermark_lbl, LV_ALIGN_CENTER, 0, 29);    // Offset +29px agar tengah visual pas di Y=80
  lv_obj_clear_flag(write_watermark_lbl, LV_OBJ_FLAG_CLICKABLE);

  // Label Feedback di pojok kanan atas (seperti "BENAR!" pada gambar contoh user)
  write_feedback_lbl = ui_make_label(canvasCard, "Mulai di P1 -> Pn", &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_align(write_feedback_lbl, LV_ALIGN_TOP_RIGHT, -12, 8);

  // Jalur Biru Tipis & Halus (Line Track)
  write_guide_track = lv_line_create(canvasCard);
  lv_obj_clear_flag(write_guide_track, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_line_color(write_guide_track, lv_color_hex(0x2563EB), 0); // Biru panduan
  lv_obj_set_style_line_width(write_guide_track, 4, 0);                      // Tebal 4px rapi (tidak menutupi huruf)
  lv_obj_set_style_line_opa(write_guide_track, LV_OPA_60, 0);                // 60% opasitas
  lv_obj_set_style_line_rounded(write_guide_track, true, 0);

  ui_trace_attach(&bali_tab_trace, canvasCard);
  bali_tab_trace.feedback_lbl = write_feedback_lbl;
  bali_tab_trace.watermark_lbl = write_watermark_lbl;
  bali_tab_trace.guide_track = write_guide_track;

  // Tombol Bersihkan & Periksa
  lv_obj_t* btnClear = ui_make_card(view_write, 160, 174, 146, 42, C_CARD, C_CARD, 12);
  ui_set_pressed(btnClear, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btnClear, on_clear_canvas_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* btnClearLbl = ui_make_label(btnClear, LV_SYMBOL_TRASH "  Bersihkan", &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(btnClearLbl);

  lv_obj_t* btnCheck = ui_make_card(view_write, 316, 174, 152, 42, C_BALI_1, C_BALI_2, 12);
  ui_set_pressed(btnCheck, C_BALI_2, C_BALI_2);
  lv_obj_add_event_cb(btnCheck, on_check_write_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* btnCheckLbl = ui_make_label(btnCheck, LV_SYMBOL_OK "  Cek Goresan", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(btnCheckLbl);
}

// ---------- 3. Tampilan Setelan Aksara Bali ----------
static void build_settings_view(lv_obj_t* parent) {
  view_settings = lv_obj_create(parent);
  lv_obj_remove_style_all(view_settings);
  lv_obj_set_pos(view_settings, 0, 0);
  lv_obj_set_size(view_settings, 480, 224);
  lv_obj_clear_flag(view_settings, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(view_settings, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t* cardSet = ui_make_card(view_settings, 16, 8, 448, 208, C_CARD, C_CARD, 16);

  lv_obj_t* sTitle = ui_make_label(cardSet, "Setelan Pembelajaran Bali", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(sTitle, 16, 12);

  // Baris 1: Panduan Goresan
  lv_obj_t* row1 = ui_make_card(cardSet, 16, 46, 416, 44, C_CARD_PR, C_CARD_PR, 10);
  lv_obj_t* r1Lbl = ui_make_label(row1, "Tampilkan Garis Panduan Tipis", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(r1Lbl, 14, 12);
  lv_obj_t* sw1 = lv_switch_create(row1);
  lv_obj_set_pos(sw1, 350, 8);
  lv_obj_add_state(sw1, LV_STATE_CHECKED);

  // Baris 2: Efek Suara
  lv_obj_t* row2 = ui_make_card(cardSet, 16, 96, 416, 44, C_CARD_PR, C_CARD_PR, 10);
  lv_obj_t* r2Lbl = ui_make_label(row2, "Audio Pelafalan Aksara Bali", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(r2Lbl, 14, 12);
  lv_obj_t* sw2 = lv_switch_create(row2);
  lv_obj_set_pos(sw2, 350, 8);
  lv_obj_add_state(sw2, LV_STATE_CHECKED);

  // Tombol Reset Kemajuan
  lv_obj_t* btnReset = ui_make_card(cardSet, 16, 150, 416, 42, lv_color_hex(0x3B151E), lv_color_hex(0x270B13), 10);
  ui_set_pressed(btnReset, lv_color_hex(0x4C1D27), lv_color_hex(0x4C1D27));
  lv_obj_t* rstLbl = ui_make_label(btnReset, LV_SYMBOL_REFRESH "  Reset Ulang Kemajuan Belajar Bali",
                                   &lv_font_montserrat_14, lv_color_hex(0xFB7185));
  lv_obj_center(rstLbl);
}

// Navigasi Pergantian Tab Bawah (Belajar / Nulis / Setelan)
static void switch_tab(int tab_index) {
  current_tab = tab_index;

  // Sembunyikan semua view
  lv_obj_add_flag(view_path, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(view_write, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(view_settings, LV_OBJ_FLAG_HIDDEN);

  // Reset warna tombol tab bawah
  lv_obj_set_style_bg_color(tab_btn_learn, C_CARD, 0);
  lv_obj_set_style_bg_color(tab_btn_write, C_CARD, 0);
  lv_obj_set_style_bg_color(tab_btn_settings, C_CARD, 0);
  lv_obj_set_style_text_color(tab_lbl_learn, C_SUB, 0);
  lv_obj_set_style_text_color(tab_lbl_write, C_SUB, 0);
  lv_obj_set_style_text_color(tab_lbl_settings, C_SUB, 0);

  if (tab_index == 0) {
    // Tab Belajar
    lv_obj_clear_flag(view_path, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(tab_btn_learn, C_CARD_PR, 0);
    lv_obj_set_style_text_color(tab_lbl_learn, C_BALI_1, 0);
  } else if (tab_index == 1) {
    // Tab Nulis Aksara
    lv_obj_clear_flag(view_write, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(tab_btn_write, C_CARD_PR, 0);
    lv_obj_set_style_text_color(tab_lbl_write, C_BALI_1, 0);

    // Update target aksara sesuai pilihan aktif
    if (active_write_index >= 0 && active_write_index < 18) {
      char b[32];
      snprintf(b, sizeof(b), "Aksara %s", BALI_NAMES[active_write_index]);
      lv_label_set_text(write_target_title, b);
      lv_obj_set_style_text_font(write_preview_lbl, &font_bali_38, 0);
      lv_label_set_text(write_preview_lbl, BALI_AKSARA[active_write_index]);
      update_bali_write_guide();
    }
  } else {
    // Tab Setelan
    lv_obj_clear_flag(view_settings, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(tab_btn_settings, C_CARD_PR, 0);
    lv_obj_set_style_text_color(tab_lbl_settings, C_BALI_1, 0);
  }
}

// Inisialisasi Layar Aksara Bali
void ui_bali_init(const AppCallbacks* callbacks) {
  bali_cbs = callbacks;

  scr_bali = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_bali, C_BG, 0);
  lv_obj_set_style_bg_opa(scr_bali, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr_bali, LV_OBJ_FLAG_SCROLLABLE);

  // ===== TOP HEADER BAR =====
  lv_obj_t* header = ui_make_card(scr_bali, 0, 0, 480, 44, C_BG, C_BG, 0);

  // Tombol Kembali (< Back)
  lv_obj_t* btnBack = ui_make_card(header, 10, 5, 40, 34, C_CARD, C_CARD, 10);
  ui_set_pressed(btnBack, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btnBack, on_back_home_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* backIc = ui_make_label(btnBack, LV_SYMBOL_LEFT, &lv_font_montserrat_20, C_TEXT);
  lv_obj_center(backIc);

  // Judul & Subjudul
  lv_obj_t* title = ui_make_label(header, "Aksara Bali", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 58, 3);

  header_sub = ui_make_label(header, "Jalur Belajar • Babak 1 / 3", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(header_sub, 58, 23);

  // Badge Status XP Kanan
  lv_obj_t* badgeXp = ui_make_card(header, 366, 7, 104, 30, lv_color_hex(0x451A03), lv_color_hex(0x270E02), 15);
  lv_obj_t* badgeLbl = ui_make_label(badgeXp, "140 XP", &lv_font_montserrat_14, lv_color_hex(0xFBBF24));
  lv_obj_center(badgeLbl);

  // ===== KONTEN UTAMA (Tengah) =====
  lv_obj_t* content_area = lv_obj_create(scr_bali);
  lv_obj_remove_style_all(content_area);
  lv_obj_set_pos(content_area, 0, 44);
  lv_obj_set_size(content_area, 480, 226);
  lv_obj_clear_flag(content_area, LV_OBJ_FLAG_SCROLLABLE);

  build_path_view(content_area);
  build_write_view(content_area);
  build_settings_view(content_area);

  // ===== BOTTOM NAVIGATION BAR (3 OPSI) =====
  lv_obj_t* botNav = ui_make_card(scr_bali, 0, 270, 480, 50, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(botNav, lv_color_hex(0x232938), 0);
  lv_obj_set_style_border_width(botNav, 1, 0);

  // Opsi 1: Belajar
  tab_btn_learn = ui_make_card(botNav, 4, 3, 154, 44, C_CARD_PR, C_CARD_PR, 12);
  lv_obj_add_event_cb(tab_btn_learn, on_tab_click, LV_EVENT_CLICKED, (void*)(intptr_t)0);
  tab_lbl_learn = ui_make_label(tab_btn_learn, LV_SYMBOL_HOME "  Belajar", &lv_font_montserrat_14, C_BALI_1);
  lv_obj_center(tab_lbl_learn);

  // Opsi 2: Nulis Aksara
  tab_btn_write = ui_make_card(botNav, 162, 3, 156, 44, C_CARD, C_CARD, 12);
  lv_obj_add_event_cb(tab_btn_write, on_tab_click, LV_EVENT_CLICKED, (void*)(intptr_t)1);
  tab_lbl_write = ui_make_label(tab_btn_write, LV_SYMBOL_EDIT "  Nulis Aksara", &lv_font_montserrat_14, C_SUB);
  lv_obj_center(tab_lbl_write);

  // Opsi 3: Setelan
  tab_btn_settings = ui_make_card(botNav, 322, 3, 154, 44, C_CARD, C_CARD, 12);
  lv_obj_add_event_cb(tab_btn_settings, on_tab_click, LV_EVENT_CLICKED, (void*)(intptr_t)2);
  tab_lbl_settings = ui_make_label(tab_btn_settings, LV_SYMBOL_SETTINGS "  Setelan", &lv_font_montserrat_14, C_SUB);
  lv_obj_center(tab_lbl_settings);
}

void ui_bali_show() {
  if (scr_bali) {
    switch_tab(0); // Buka tab Belajar secara default
    lv_scr_load_anim(scr_bali, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
  }
}
