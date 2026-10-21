#include "ui_jawa.h"
#include "ui_home.h"
#include "ui_learn.h"
#include <stdio.h>

static const AppCallbacks* jawa_cbs = nullptr;
static lv_obj_t* scr_jawa = nullptr;

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

// Aksara Jawa Data (Hanacaraka)
struct PathNodeData {
  const char* name;
  const char* symbol;
  int state; // 0 = Selesai, 1 = Aktif, 2 = Peti/Bonus, 3 = Terkunci
};

// 3 Halaman, masing-masing 3 node (total 9 node)
static const PathNodeData JAWA_PAGES[3][3] = {
  {
    { "Ha", "\xEA\xA6\xB2", 0 },
    { "Na", "\xEA\xA6\xA4", 0 },
    { "Ca", "\xEA\xA6\x95", 1 }
  },
  {
    { "Peti", LV_SYMBOL_IMAGE, 2 },
    { "Ra", "\xEA\xA6\xAA", 3 },
    { "Ka", "\xEA\xA6\x8F", 3 }
  },
  {
    { "Da", "\xEA\xA6\xA2", 3 },
    { "Ta", "\xEA\xA6\xA0", 3 },
    { "Piala", LV_SYMBOL_OK, 2 }
  }
};

// Canvas Nulis Aksara
static lv_obj_t* write_target_title = nullptr;
static lv_obj_t* write_target_sub = nullptr;
static lv_obj_t* write_guide_track = nullptr;
static lv_obj_t* write_start_marker = nullptr;
static lv_obj_t* write_end_marker = nullptr;
static lv_obj_t* write_feedback_lbl = nullptr;
static lv_obj_t* write_preview_lbl = nullptr;
static int active_write_index = 2; // Default ke aksara "Ca"
static TraceState jawa_tab_trace;
static lv_point_precise_t jawa_guide_pts[24];

// Waypoint Jalur Aksara Jawa (Pola Bentuk Native)
static const StrokeWaypoint JAWA_GUIDE_HA[] = {
  { 72, 110 }, { 80, 75 }, { 95, 55 }, { 110, 75 }, { 122, 110 },
  { 140, 112 }, { 155, 70 }, { 170, 55 }, { 188, 75 }, { 202, 110 }
};
static const StrokeWaypoint JAWA_GUIDE_NA[] = {
  { 70, 110 }, { 85, 60 }, { 105, 55 }, { 120, 108 },
  { 135, 60 }, { 155, 55 }, { 170, 108 }, { 185, 110 }, { 198, 75 }
};
static const StrokeWaypoint JAWA_GUIDE_CA[] = {
  { 70, 80 }, { 85, 110 }, { 110, 55 }, { 135, 75 }, { 122, 100 },
  { 148, 112 }, { 172, 60 }, { 188, 75 }, { 200, 110 }
};
static const StrokeWaypoint JAWA_GUIDE_RA[] = {
  { 80, 110 }, { 98, 65 }, { 115, 55 }, { 132, 85 }, { 145, 110 },
  { 165, 60 }, { 182, 55 }, { 195, 110 }
};
static const StrokeWaypoint JAWA_GUIDE_KA[] = {
  { 75, 55 }, { 88, 85 }, { 98, 110 }, { 118, 85 }, { 130, 110 },
  { 152, 52 }, { 172, 70 }, { 188, 110 }
};
static const StrokeWaypoint JAWA_GUIDE_DA[] = {
  { 80, 110 }, { 95, 60 }, { 115, 55 }, { 130, 85 }, { 152, 55 },
  { 172, 75 }, { 188, 110 }
};
static const StrokeWaypoint JAWA_GUIDE_TA[] = {
  { 75, 110 }, { 92, 60 }, { 112, 55 }, { 128, 105 }, { 145, 60 },
  { 165, 55 }, { 182, 110 }
};

static void get_jawa_guide(int index, const StrokeWaypoint*& pts, uint8_t& count) {
  switch (index) {
    case 0: pts = JAWA_GUIDE_HA; count = sizeof(JAWA_GUIDE_HA)/sizeof(JAWA_GUIDE_HA[0]); break;
    case 1: pts = JAWA_GUIDE_NA; count = sizeof(JAWA_GUIDE_NA)/sizeof(JAWA_GUIDE_NA[0]); break;
    case 2: pts = JAWA_GUIDE_CA; count = sizeof(JAWA_GUIDE_CA)/sizeof(JAWA_GUIDE_CA[0]); break;
    case 4: pts = JAWA_GUIDE_RA; count = sizeof(JAWA_GUIDE_RA)/sizeof(JAWA_GUIDE_RA[0]); break;
    case 5: pts = JAWA_GUIDE_KA; count = sizeof(JAWA_GUIDE_KA)/sizeof(JAWA_GUIDE_KA[0]); break;
    case 6: pts = JAWA_GUIDE_DA; count = sizeof(JAWA_GUIDE_DA)/sizeof(JAWA_GUIDE_DA[0]); break;
    case 7: pts = JAWA_GUIDE_TA; count = sizeof(JAWA_GUIDE_TA)/sizeof(JAWA_GUIDE_TA[0]); break;
    default: pts = JAWA_GUIDE_HA; count = sizeof(JAWA_GUIDE_HA)/sizeof(JAWA_GUIDE_HA[0]); break;
  }
}

static void update_jawa_write_guide() {
  if (!write_guide_track) return;
  const StrokeWaypoint* guide_pts = nullptr;
  uint8_t count = 0;
  get_jawa_guide(active_write_index, guide_pts, count);

  for (int i = 0; i < count && i < 24; i++) {
    jawa_guide_pts[i].x = guide_pts[i].x;
    jawa_guide_pts[i].y = guide_pts[i].y;
  }
  lv_line_set_points(write_guide_track, jawa_guide_pts, count);

  if (write_start_marker) {
    lv_obj_set_pos(write_start_marker, guide_pts[0].x - 8, guide_pts[0].y - 8);
  }
  if (write_end_marker) {
    lv_obj_set_pos(write_end_marker, guide_pts[count - 1].x - 8, guide_pts[count - 1].y - 8);
  }

  if (write_feedback_lbl) {
    lv_label_set_text(write_feedback_lbl, "Mulai di HIJAU -> MERAH");
    lv_obj_set_style_text_color(write_feedback_lbl, lv_color_hex(0x38BDF8), 0);
  }
  ui_trace_clear(&jawa_tab_trace);
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
  if (current_page < 2) {
    audio_play(SOUND_PAGE);
    current_page++;
    update_path_page();
  }
}

static void on_node_click(lv_event_t* e) {
  int node_idx = (int)(intptr_t)lv_event_get_user_data(e);
  const PathNodeData& node = JAWA_PAGES[current_page][node_idx];

  char buf[48];
  if (node.state == 0) {
    audio_play(SOUND_NODE);
    snprintf(buf, sizeof(buf), "Aksara %s: Sudah Selesai!", node.name);
    ui_show_toast(buf);
    active_write_index = current_page * 3 + node_idx;
    ui_learn_start(AKSARA_JAWA, current_page * 3 + node_idx); // Buka Kuis Pembelajaran
  } else if (node.state == 1) {
    audio_play(SOUND_NODE);
    snprintf(buf, sizeof(buf), "Level Aktif: Aksara %s!", node.name);
    ui_show_toast(buf);
    active_write_index = current_page * 3 + node_idx;
    ui_learn_start(AKSARA_JAWA, current_page * 3 + node_idx); // Buka Kuis Pembelajaran
  } else if (node.state == 2) {
    audio_play(SOUND_REWARD);
    ui_show_toast("Peti Hadiah: +50 XP Belajar!");
  } else {
    audio_play(SOUND_CLICK);
    snprintf(buf, sizeof(buf), "Aksara %s masih terkunci", node.name);
    ui_show_toast(buf);
  }

  if (jawa_cbs && jawa_cbs->onSelectAksara) {
    jawa_cbs->onSelectAksara(AKSARA_JAWA, current_page * 3 + node_idx);
  }
}

static void on_clear_canvas_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  ui_trace_clear(&jawa_tab_trace);
  if (write_feedback_lbl) {
    lv_label_set_text(write_feedback_lbl, "Mulai di HIJAU -> MERAH");
    lv_obj_set_style_text_color(write_feedback_lbl, lv_color_hex(0x38BDF8), 0);
  }
  ui_show_toast("Kanvas dibersihkan");
}

static void on_check_write_click(lv_event_t* e) {
  (void)e;
  const StrokeWaypoint* guide_pts = nullptr;
  uint8_t count = 0;
  get_jawa_guide(active_write_index, guide_pts, count);

  StrokeEvalResult eval = ui_eval_stroke_trace(&jawa_tab_trace, guide_pts, count);
  if (eval.success) {
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
      lv_label_set_text(write_feedback_lbl, "COBA LAGI");
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
    "Buddy: Semangat belajar aksaranya!",
    "Buddy: Kamu hebat, yuk selesaikan babak ini!",
    "Buddy: Setiap goresan bikin kamu makin mahir!",
    "Buddy: Aksara Jawa itu indah banget!"
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
                                 lv_color_hex(0x22C55E), lv_color_hex(0x16A34A), 11);
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
  snprintf(ind_buf, sizeof(ind_buf), "%d / 3", current_page + 1);
  lv_label_set_text(lbl_page_ind, ind_buf);

  char sub_buf[32];
  snprintf(sub_buf, sizeof(sub_buf), "Jalur Belajar • Babak %d / 3", current_page + 1);
  if (header_sub) lv_label_set_text(header_sub, sub_buf);

  // Update tombol panah atas/bawah (disable jika di ujung)
  if (current_page == 0) {
    lv_obj_set_style_opa(btn_page_up, LV_OPA_30, 0);
  } else {
    lv_obj_set_style_opa(btn_page_up, LV_OPA_COVER, 0);
  }

  if (current_page == 2) {
    lv_obj_set_style_opa(btn_page_down, LV_OPA_30, 0);
  } else {
    lv_obj_set_style_opa(btn_page_down, LV_OPA_COVER, 0);
  }

  // Koordinat 3 Node berbeda untuk setiap babak (variasi jalur berliku dinamis)
  static const int POS[3][3][2] = {
    // Babak 1: Melengkung ke Kiri (Node di X=70..165, Info Card di KANAN X=246)
    { { 165, 16 }, { 70,  82 }, { 155, 148 } },
    // Babak 2: Melengkung ke Kanan (Node di X=185..280, Info Card di KIRI X=16)
    { { 185, 16 }, { 280, 82 }, { 195, 148 } },
    // Babak 3: Jalur Gelombang Kiri (Node di X=85..165, Info Card di KANAN X=246)
    { { 85,  18 }, { 165, 82 }, { 85,  146 } }
  };

  // Titik jalur penghubung (S-curve sesuai babak)
  create_trail_dots(path_nodes_cont, POS[current_page][0][0], POS[current_page][0][1], POS[current_page][1][0], POS[current_page][1][1]);
  create_trail_dots(path_nodes_cont, POS[current_page][1][0], POS[current_page][1][1], POS[current_page][2][0], POS[current_page][2][1]);

  // Gambar 3 Node halaman ini dengan posisi khusus babak tersebut
  for (int i = 0; i < 3; i++) {
    const PathNodeData& node = JAWA_PAGES[current_page][i];
    lv_color_t c_top, c_dark;
    const char* txt = node.symbol;
    const lv_font_t* f = &lv_font_montserrat_20;

    if (node.state == 0) {
      // Selesai (Centang biru)
      c_top = lv_color_hex(0x0284C7);
      c_dark = lv_color_hex(0x0369A1);
      txt = node.symbol;
      f = &font_jawa_22;
    } else if (node.state == 1) {
      // Sedang Aktif (Ungu Jawa)
      c_top = C_JAWA_1;
      c_dark = C_JAWA_2;
      txt = node.symbol;
      f = &font_jawa_22;
    } else if (node.state == 2) {
      // Peti Hadiah / Piala
      c_top = lv_color_hex(0xF59E0B);
      c_dark = lv_color_hex(0xB45309);
      f = &lv_font_montserrat_14;
      txt = (current_page == 1) ? "Peti" : "Win";
      // font aksara
    } else {
      // Terkunci
      c_top = lv_color_hex(0x334155);
      c_dark = lv_color_hex(0x1E293B);
      txt = node.symbol;
      f = &font_jawa_22;
      // font aksara
    }

    create_duo_node(path_nodes_cont, POS[current_page][i][0], POS[current_page][i][1], 62,
                    c_top, c_dark, txt, f, (node.state == 1), i);
  }

  // Info Card & Maskot menyesuaikan ruang kosong agar tidak tabrakan dengan node:
  // Di Babak 2 (node ke kanan) -> Card & Maskot di KIRI
  // Di Babak 1 & 3 (node ke kiri) -> Card & Maskot di KANAN
  int card_x = (current_page == 1) ? 14 : 244;
  int card_y = 10;
  lv_obj_t* infoCard = ui_make_card(path_nodes_cont, card_x, card_y, 146, 70, C_CARD, C_CARD, 12);
  lv_obj_clear_flag(infoCard, LV_OBJ_FLAG_CLICKABLE);

  if (current_page == 0) {
    lv_obj_t* t1 = ui_make_label(infoCard, "Babak 1: Dasar", &lv_font_montserrat_14, C_TEXT);
    lv_obj_set_pos(t1, 10, 6);
    lv_obj_t* t2 = ui_make_label(infoCard, "Ha • Na • Ca", &lv_font_montserrat_14, C_JAWA_1);
    lv_obj_set_pos(t2, 10, 26);
    lv_obj_t* t3 = ui_make_label(infoCard, "Level 1 - 3", &lv_font_montserrat_14, C_SUB);
    lv_obj_set_pos(t3, 10, 46);
  } else if (current_page == 1) {
    lv_obj_t* t1 = ui_make_label(infoCard, "Babak 2: Lanjutan", &lv_font_montserrat_14, C_TEXT);
    lv_obj_set_pos(t1, 10, 6);
    lv_obj_t* t2 = ui_make_label(infoCard, "Peti • Ra • Ka", &lv_font_montserrat_14, lv_color_hex(0xF59E0B));
    lv_obj_set_pos(t2, 10, 26);
    lv_obj_t* t3 = ui_make_label(infoCard, "Bonus + Belajar", &lv_font_montserrat_14, C_SUB);
    lv_obj_set_pos(t3, 10, 46);
  } else {
    lv_obj_t* t1 = ui_make_label(infoCard, "Babak 3: Mahir", &lv_font_montserrat_14, C_TEXT);
    lv_obj_set_pos(t1, 10, 6);
    lv_obj_t* t2 = ui_make_label(infoCard, "Da • Ta • Piala", &lv_font_montserrat_14, lv_color_hex(0x22C55E));
    lv_obj_set_pos(t2, 10, 26);
    lv_obj_t* t3 = ui_make_label(infoCard, "Selesai Babak 1!", &lv_font_montserrat_14, C_SUB);
    lv_obj_set_pos(t3, 10, 46);
  }

  // Variasi Maskot Buddy untuk setiap babak (Pose berbeda di Babak 1, 2, dan 3)
  const lv_image_dsc_t* buddy_src = &BuddyMaskot;
  int mascot_x = 268;
  int mascot_y = 88;

  if (current_page == 0) {
    buddy_src = &BuddyMaskot;              // Pose Babak 1: Buddy Maskot (100x124)
    mascot_x = 268;
    mascot_y = 88;
  } else if (current_page == 1) {
    buddy_src = &Buddy3;                   // Pose Babak 2: Chibi ceria (100x124)
    mascot_x = 36;
    mascot_y = 86;
  } else {
    buddy_src = &Buddy4;                   // Pose Babak 3: Cheerful semangat juara (100x134)
    mascot_x = 268;
    mascot_y = 82;
  }

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
  lv_obj_clear_flag(view_path, LV_OBJ_FLAG_SCROLLABLE); // Tanpa scroll = Anti nge-lag
  lv_obj_set_style_bg_color(view_path, C_BG, 0);
  lv_obj_set_style_bg_opa(view_path, LV_OPA_COVER, 0);

  // Area Node Aksara (Kiri & Tengah)
  path_nodes_cont = lv_obj_create(view_path);
  lv_obj_remove_style_all(path_nodes_cont);
  lv_obj_set_pos(path_nodes_cont, 0, 0);
  lv_obj_set_size(path_nodes_cont, 396, 224);
  lv_obj_clear_flag(path_nodes_cont, LV_OBJ_FLAG_SCROLLABLE);

  // Panel Tombol Navigasi Panah Atas & Bawah (Sisi Kanan Layar)
  lv_obj_t* nav_side = ui_make_card(view_path, 400, 8, 70, 208, C_CARD, C_CARD, 16);
  lv_obj_clear_flag(nav_side, LV_OBJ_FLAG_SCROLLABLE);

  // Tombol Panah Atas (▲)
  btn_page_up = ui_make_card(nav_side, 8, 10, 54, 52, C_JAWA_1, C_JAWA_2, 14);
  ui_set_pressed(btn_page_up, C_JAWA_2, C_JAWA_2);
  lv_obj_add_event_cb(btn_page_up, on_page_up_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* upIc = ui_make_label(btn_page_up, LV_SYMBOL_UP, &lv_font_montserrat_20, lv_color_hex(0xFFFFFF));
  lv_obj_center(upIc);

  // Indikator Halaman (Contoh: "1 / 3")
  lv_obj_t* ind_box = ui_make_card(nav_side, 6, 80, 58, 36, C_CARD_PR, C_CARD_PR, 10);
  lv_obj_clear_flag(ind_box, LV_OBJ_FLAG_CLICKABLE);
  lbl_page_ind = ui_make_label(ind_box, "1 / 3", &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(lbl_page_ind);

  // Tombol Panah Bawah (▼)
  btn_page_down = ui_make_card(nav_side, 8, 134, 54, 52, C_JAWA_1, C_JAWA_2, 14);
  ui_set_pressed(btn_page_down, C_JAWA_2, C_JAWA_2);
  lv_obj_add_event_cb(btn_page_down, on_page_down_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* downIc = ui_make_label(btn_page_down, LV_SYMBOL_DOWN, &lv_font_montserrat_20, lv_color_hex(0xFFFFFF));
  lv_obj_center(downIc);

  // Gambar konten halaman pertama
  update_path_page();
}

// ---------- 2. Tampilan Nulis Aksara (Writing Canvas) ----------
static void build_write_view(lv_obj_t* parent) {
  view_write = lv_obj_create(parent);
  lv_obj_remove_style_all(view_write);
  lv_obj_set_pos(view_write, 0, 0);
  lv_obj_set_size(view_write, 480, 224);
  lv_obj_clear_flag(view_write, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(view_write, LV_OBJ_FLAG_HIDDEN);

  // Kartu Target Aksara (Kiri)
  lv_obj_t* cardTarget = ui_make_card(view_write, 12, 8, 140, 208, C_CARD, C_CARD, 16);
  write_target_title = ui_make_label(cardTarget, "Aksara Ca", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(write_target_title, 14, 14);

  write_target_sub = ui_make_label(cardTarget, "Goresan 1 dari 3\nBunyi: /ca/", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(write_target_sub, 14, 40);

  // Preview Kotak Huruf
  lv_obj_t* boxHuruf = ui_make_card(cardTarget, 14, 84, 112, 110, C_JAWA_1, C_JAWA_2, 14);
  write_preview_lbl = ui_make_label(boxHuruf, "Ca", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF));
  lv_obj_center(write_preview_lbl);

  // Area Kanvas Tracing ala Contoh Gambar User (308x160 px)
  lv_obj_t* canvasCard = ui_make_card(view_write, 160, 8, 308, 160, lv_color_hex(0x0A0F1D), lv_color_hex(0x0A0F1D), 16);
  lv_obj_set_style_border_color(canvasCard, C_JAWA_1, 0);
  lv_obj_set_style_border_width(canvasCard, 2, 0);

  // Label Feedback di pojok kanan atas (seperti "BENAR!" pada gambar contoh user)
  write_feedback_lbl = ui_make_label(canvasCard, "Mulai di HIJAU -> MERAH", &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_align(write_feedback_lbl, LV_ALIGN_TOP_RIGHT, -12, 8);

  // Jalur Biru Tebal (Track ala Contoh Gambar User)
  write_guide_track = lv_line_create(canvasCard);
  lv_obj_clear_flag(write_guide_track, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_line_color(write_guide_track, lv_color_hex(0x2563EB), 0); // Biru pekat
  lv_obj_set_style_line_width(write_guide_track, 16, 0); // Tebal 16px
  lv_obj_set_style_line_rounded(write_guide_track, true, 0);

  // Titik Mulai (Lingkaran Hijau)
  write_start_marker = ui_make_card(canvasCard, 0, 0, 16, 16, lv_color_hex(0x22C55E), lv_color_hex(0x22C55E), 8);
  lv_obj_set_style_border_color(write_start_marker, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_border_width(write_start_marker, 2, 0);
  lv_obj_clear_flag(write_start_marker, LV_OBJ_FLAG_CLICKABLE);

  // Titik Akhir (Lingkaran Oranye/Merah)
  write_end_marker = ui_make_card(canvasCard, 0, 0, 16, 16, lv_color_hex(0xF97316), lv_color_hex(0xF97316), 8);
  lv_obj_set_style_border_color(write_end_marker, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_border_width(write_end_marker, 2, 0);
  lv_obj_clear_flag(write_end_marker, LV_OBJ_FLAG_CLICKABLE);

  ui_trace_attach(&jawa_tab_trace, canvasCard);

  // Tombol Bersihkan & Periksa
  lv_obj_t* btnClear = ui_make_card(view_write, 160, 174, 146, 42, C_CARD, C_CARD, 12);
  ui_set_pressed(btnClear, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btnClear, on_clear_canvas_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* btnClearLbl = ui_make_label(btnClear, LV_SYMBOL_TRASH "  Bersihkan", &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(btnClearLbl);

  lv_obj_t* btnCheck = ui_make_card(view_write, 316, 174, 152, 42, C_JAWA_1, C_JAWA_2, 12);
  ui_set_pressed(btnCheck, C_JAWA_2, C_JAWA_2);
  lv_obj_add_event_cb(btnCheck, on_check_write_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* btnCheckLbl = ui_make_label(btnCheck, LV_SYMBOL_OK "  Cek Goresan", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(btnCheckLbl);
}

// ---------- 3. Tampilan Setelan Aksara Jawa ----------
static void build_settings_view(lv_obj_t* parent) {
  view_settings = lv_obj_create(parent);
  lv_obj_remove_style_all(view_settings);
  lv_obj_set_pos(view_settings, 0, 0);
  lv_obj_set_size(view_settings, 480, 224);
  lv_obj_clear_flag(view_settings, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(view_settings, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t* cardSet = ui_make_card(view_settings, 16, 8, 448, 208, C_CARD, C_CARD, 16);

  lv_obj_t* sTitle = ui_make_label(cardSet, "Setelan Pembelajaran Jawa", &lv_font_montserrat_20, C_TEXT);
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
  lv_obj_t* r2Lbl = ui_make_label(row2, "Audio Pelafalan Aksara Otomatis", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(r2Lbl, 14, 12);
  lv_obj_t* sw2 = lv_switch_create(row2);
  lv_obj_set_pos(sw2, 350, 8);
  lv_obj_add_state(sw2, LV_STATE_CHECKED);

  // Tombol Reset Kemajuan
  lv_obj_t* btnReset = ui_make_card(cardSet, 16, 150, 416, 42, lv_color_hex(0x3B151E), lv_color_hex(0x270B13), 10);
  ui_set_pressed(btnReset, lv_color_hex(0x4C1D27), lv_color_hex(0x4C1D27));
  lv_obj_t* rstLbl = ui_make_label(btnReset, LV_SYMBOL_REFRESH "  Reset Ulang Kemajuan Belajar Jawa",
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
    lv_obj_set_style_text_color(tab_lbl_learn, C_JAWA_1, 0);
  } else if (tab_index == 1) {
    // Tab Nulis Aksara
    lv_obj_clear_flag(view_write, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(tab_btn_write, C_CARD_PR, 0);
    lv_obj_set_style_text_color(tab_lbl_write, C_JAWA_1, 0);

    // Update target aksara sesuai pilihan aktif
    static const char* NAMA[9] = { "Ha", "Na", "Ca", "Peti", "Ra", "Ka", "Da", "Ta", "Piala" };
    char b[24];
    snprintf(b, sizeof(b), "Aksara %s", NAMA[active_write_index]);
    lv_label_set_text(write_target_title, b);
    static const char* AKSARA[9] = { "\xEA\xA6\xB2", "\xEA\xA6\xA4", "\xEA\xA6\x95", "Peti", "\xEA\xA6\xAA", "\xEA\xA6\x8F", "\xEA\xA6\xA2", "\xEA\xA6\xA0", "Win" };
    lv_obj_set_style_text_font(write_preview_lbl, (active_write_index == 3 || active_write_index == 8) ? &lv_font_montserrat_20 : &font_jawa_38, 0);
    lv_label_set_text(write_preview_lbl, AKSARA[active_write_index]);
    update_jawa_write_guide();
  } else {
    // Tab Setelan
    lv_obj_clear_flag(view_settings, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(tab_btn_settings, C_CARD_PR, 0);
    lv_obj_set_style_text_color(tab_lbl_settings, C_JAWA_1, 0);
  }
}

// Inisialisasi Layar Aksara Jawa
void ui_jawa_init(const AppCallbacks* callbacks) {
  jawa_cbs = callbacks;

  scr_jawa = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_jawa, C_BG, 0);
  lv_obj_set_style_bg_opa(scr_jawa, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr_jawa, LV_OBJ_FLAG_SCROLLABLE);

  // ===== TOP HEADER BAR =====
  lv_obj_t* header = ui_make_card(scr_jawa, 0, 0, 480, 44, C_BG, C_BG, 0);

  // Tombol Kembali (< Back)
  lv_obj_t* btnBack = ui_make_card(header, 10, 5, 40, 34, C_CARD, C_CARD, 10);
  ui_set_pressed(btnBack, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btnBack, on_back_home_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* backIc = ui_make_label(btnBack, LV_SYMBOL_LEFT, &lv_font_montserrat_20, C_TEXT);
  lv_obj_center(backIc);

  // Judul & Subjudul
  lv_obj_t* title = ui_make_label(header, "Aksara Jawa", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 58, 3);

  header_sub = ui_make_label(header, "Jalur Belajar • Babak 1 / 3", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(header_sub, 58, 23);

  // Badge Status XP Kanan
  lv_obj_t* badgeXp = ui_make_card(header, 366, 7, 104, 30, lv_color_hex(0x2E1065), lv_color_hex(0x1E1B4B), 15);
  lv_obj_t* badgeLbl = ui_make_label(badgeXp, "120 XP", &lv_font_montserrat_14, lv_color_hex(0xC084FC));
  lv_obj_center(badgeLbl);

  // ===== KONTEN UTAMA (Tengah) =====
  lv_obj_t* content_area = lv_obj_create(scr_jawa);
  lv_obj_remove_style_all(content_area);
  lv_obj_set_pos(content_area, 0, 44);
  lv_obj_set_size(content_area, 480, 226);
  lv_obj_clear_flag(content_area, LV_OBJ_FLAG_SCROLLABLE);

  build_path_view(content_area);
  build_write_view(content_area);
  build_settings_view(content_area);

  // ===== BOTTOM NAVIGATION BAR (3 OPSI) =====
  lv_obj_t* botNav = ui_make_card(scr_jawa, 0, 270, 480, 50, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(botNav, lv_color_hex(0x232938), 0);
  lv_obj_set_style_border_width(botNav, 1, 0);

  // Opsi 1: Belajar
  tab_btn_learn = ui_make_card(botNav, 4, 3, 154, 44, C_CARD_PR, C_CARD_PR, 12);
  lv_obj_add_event_cb(tab_btn_learn, on_tab_click, LV_EVENT_CLICKED, (void*)(intptr_t)0);
  tab_lbl_learn = ui_make_label(tab_btn_learn, LV_SYMBOL_HOME "  Belajar", &lv_font_montserrat_14, C_JAWA_1);
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

void ui_jawa_show() {
  if (scr_jawa) {
    switch_tab(0); // Buka tab Belajar secara default
    lv_scr_load_anim(scr_jawa, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
  }
}
