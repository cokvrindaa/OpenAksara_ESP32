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

// Font Aksara Daerah Native (22px standar & 38px JUMBO)
LV_FONT_DECLARE(font_jawa_22);
LV_FONT_DECLARE(font_bali_22);
LV_FONT_DECLARE(font_jawa_38);
LV_FONT_DECLARE(font_bali_38);
#ifdef __cplusplus
}
#endif

// ---------- Kanvas Goresan Aksara Interaktif (Tracing) ----------
#define MAX_TRACE_STROKES 6
#define MAX_TRACE_POINTS_PER_STROKE 60

struct StrokeWaypoint {
  int16_t x;
  int16_t y;
};

struct StrokeEvalResult {
  bool success;
  const char* message;
};

struct TraceState {
  lv_obj_t* canvas_card;
  lv_obj_t* draw_layer;
  lv_point_precise_t (*points)[MAX_TRACE_POINTS_PER_STROKE];
  uint16_t pt_counts[MAX_TRACE_STROKES];
  uint8_t stroke_count;
  bool is_drawing;
};

inline void ui_trace_clear(TraceState* st) {
  if (!st) return;
  if (st->draw_layer) {
    lv_obj_clean(st->draw_layer);
  }
  for (int i = 0; i < MAX_TRACE_STROKES; i++) {
    st->pt_counts[i] = 0;
  }
  st->stroke_count = 0;
  st->is_drawing = false;
}

inline int ui_trace_total_points(const TraceState* st) {
  if (!st) return 0;
  int total = 0;
  for (int i = 0; i < st->stroke_count; i++) {
    total += st->pt_counts[i];
  }
  return total;
}

inline void ui_trace_detach(TraceState* st) {
  if (!st) return;
  ui_trace_clear(st);
  if (st->draw_layer) {
    lv_obj_delete(st->draw_layer);
    st->draw_layer = nullptr;
  }
  if (st->points) {
    free(st->points);
    st->points = nullptr;
  }
}

// Algoritma Validasi Goresan Aksara (Waypoint Traversal & Deviation Check)
inline StrokeEvalResult ui_eval_stroke_trace(const TraceState* st, const StrokeWaypoint* guide, uint8_t guide_count) {
  StrokeEvalResult res = { false, "Goresan kosong!" };
  if (!st || !guide || guide_count < 3) return res;

  int total_user_pts = 0;
  for (int s = 0; s < st->stroke_count; s++) {
    total_user_pts += st->pt_counts[s];
  }

  // 1. Syarat minimal titik (hindari ketukan acak sesaat)
  if (total_user_pts < 6) {
    res.message = "Goresan terlalu pendek! Ikuti seluruh alur aksara.";
    return res;
  }

  int32_t start_x = st->points[0][0].x;
  int32_t start_y = st->points[0][0].y;

  int last_stroke = st->stroke_count - 1;
  int last_pt_idx = st->pt_counts[last_stroke] - 1;
  int32_t end_x = st->points[last_stroke][last_pt_idx].x;
  int32_t end_y = st->points[last_stroke][last_pt_idx].y;

  // 2. Cek Kedekatan Titik Mulai (Start Check) - Maksimal radius 38px dari titik hijau
  int32_t dx_start = start_x - guide[0].x;
  int32_t dy_start = start_y - guide[0].y;
  int32_t dist_sq_start = dx_start * dx_start + dy_start * dy_start;
  if (dist_sq_start > (38 * 38)) {
    res.message = "Mulai goresan dari titik HIJAU!";
    return res;
  }

  // 3. Cek Kedekatan Titik Akhir (End Check) - Maksimal radius 38px dari titik merah
  int32_t dx_end = end_x - guide[guide_count - 1].x;
  int32_t dy_end = end_y - guide[guide_count - 1].y;
  int32_t dist_sq_end = dx_end * dx_end + dy_end * dy_end;
  if (dist_sq_end > (38 * 38)) {
    res.message = "Selesaikan goresan di titik MERAH!";
    return res;
  }

  // 4. Verifikasi Penelusuran Waypoint Berurutan & Koridor
  int current_guide_idx = 0;
  int in_bounds_count = 0;

  for (int s = 0; s < st->stroke_count; s++) {
    for (int p = 0; p < st->pt_counts[s]; p++) {
      int32_t ux = st->points[s][p].x;
      int32_t uy = st->points[s][p].y;

      if (current_guide_idx < guide_count) {
        int32_t gx = guide[current_guide_idx].x;
        int32_t gy = guide[current_guide_idx].y;
        int32_t d2 = (ux - gx) * (ux - gx) + (uy - gy) * (uy - gy);
        if (d2 <= (28 * 28)) {
          current_guide_idx++;
        }
      }

      bool near_any = false;
      for (int g = 0; g < guide_count; g++) {
        int32_t d2 = (ux - guide[g].x) * (ux - guide[g].x) + (uy - guide[g].y) * (uy - guide[g].y);
        if (d2 <= (30 * 30)) {
          near_any = true;
          break;
        }
      }
      if (near_any) in_bounds_count++;
    }
  }

  // 5. Cek Deviasi / Coretan di Luar Jalur
  float in_bounds_ratio = (float)in_bounds_count / (float)total_user_pts;
  if (in_bounds_ratio < 0.65f) {
    res.message = "Goresan keluar jalur! Tetap ikuti jalur biru.";
    return res;
  }

  // 6. Cek Cakupan Waypoint (Minimal 60% urutan jalur terlalui)
  float coverage_ratio = (float)current_guide_idx / (float)guide_count;
  if (coverage_ratio < 0.60f) {
    res.message = "Bentuk aksara belum lengkap! Ikuti alur dari awal sampai akhir.";
    return res;
  }

  res.success = true;
  res.message = "BENAR! Goresan rapi dan sesuai jalur (+15 XP)";
  return res;
}

inline void ui_trace_event_cb(lv_event_t* e) {
  TraceState* st = (TraceState*)lv_event_get_user_data(e);
  if (!st || !st->canvas_card || !st->points) return;

  lv_event_code_t code = lv_event_get_code(e);
  lv_indev_t* indev = lv_indev_active();
  if (!indev) return;

  lv_point_t p;
  lv_indev_get_point(indev, &p);

  lv_area_t area;
  lv_obj_get_coords(st->canvas_card, &area);

  int32_t rx = p.x - area.x1;
  int32_t ry = p.y - area.y1;
  int32_t w = lv_obj_get_width(st->canvas_card);
  int32_t h = lv_obj_get_height(st->canvas_card);

  if (rx < 0) rx = 0;
  if (rx >= w) rx = w - 1;
  if (ry < 0) ry = 0;
  if (ry >= h) ry = h - 1;

  if (code == LV_EVENT_PRESSED) {
    if (st->stroke_count < MAX_TRACE_STROKES) {
      uint8_t s = st->stroke_count;
      st->pt_counts[s] = 1;
      st->points[s][0].x = rx;
      st->points[s][0].y = ry;
      st->is_drawing = true;
    }
  } else if (code == LV_EVENT_PRESSING) {
    if (st->is_drawing && st->stroke_count < MAX_TRACE_STROKES) {
      uint8_t s = st->stroke_count;
      uint16_t count = st->pt_counts[s];

      if (count < MAX_TRACE_POINTS_PER_STROKE) {
        int32_t dx = rx - st->points[s][count - 1].x;
        int32_t dy = ry - st->points[s][count - 1].y;
        // Jarak minimal 5px: ultra responsif, presisi dan super mulus!
        if (dx * dx + dy * dy >= 25) {
          st->points[s][count].x = rx;
          st->points[s][count].y = ry;

          // Render segmen instan: hanya kotak ~12x12 px yang di-refresh (< 0.1ms render!)
          if (st->draw_layer) {
            lv_obj_t* seg = lv_line_create(st->draw_layer);
            lv_obj_clear_flag(seg, LV_OBJ_FLAG_CLICKABLE);
            lv_line_set_points(seg, &st->points[s][count - 1], 2);
            lv_obj_set_style_line_color(seg, lv_color_hex(0xFFFFFF), 0); // Putih terang
            lv_obj_set_style_line_width(seg, 5, 0);                       // Tebal 5px
            lv_obj_set_style_line_rounded(seg, true, 0);
          }

          st->pt_counts[s]++;
        }
      }
    }
  } else if (code == LV_EVENT_RELEASED) {
    if (st->is_drawing && st->stroke_count < MAX_TRACE_STROKES) {
      uint8_t s = st->stroke_count;
      if (st->pt_counts[s] == 1) {
        // Jika pengguna hanya tap 1 titik tanpa menggeser, render dot 5px
        st->points[s][1].x = st->points[s][0].x + 1;
        st->points[s][1].y = st->points[s][0].y;
        st->pt_counts[s] = 2;
        if (st->draw_layer) {
          lv_obj_t* seg = lv_line_create(st->draw_layer);
          lv_obj_clear_flag(seg, LV_OBJ_FLAG_CLICKABLE);
          lv_line_set_points(seg, &st->points[s][0], 2);
          lv_obj_set_style_line_color(seg, lv_color_hex(0xFFFFFF), 0);
          lv_obj_set_style_line_width(seg, 5, 0);
          lv_obj_set_style_line_rounded(seg, true, 0);
        }
      }
      st->stroke_count++;
      st->is_drawing = false;
    }
  }
}

inline void ui_trace_attach(TraceState* st, lv_obj_t* canvas_card) {
  if (!st || !canvas_card) return;
  st->canvas_card = canvas_card;
  st->stroke_count = 0;
  st->is_drawing = false;
  for (int i = 0; i < MAX_TRACE_STROKES; i++) {
    st->pt_counts[i] = 0;
  }
  if (!st->points) {
    st->points = (lv_point_precise_t(*)[MAX_TRACE_POINTS_PER_STROKE])malloc(sizeof(lv_point_precise_t) * MAX_TRACE_STROKES * MAX_TRACE_POINTS_PER_STROKE);
  }
  if (!st->draw_layer) {
    st->draw_layer = lv_obj_create(canvas_card);
    lv_obj_remove_style_all(st->draw_layer);
    lv_obj_set_pos(st->draw_layer, 0, 0);
    lv_obj_set_size(st->draw_layer, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(st->draw_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(st->draw_layer, LV_OBJ_FLAG_SCROLLABLE);
  } else {
    lv_obj_clean(st->draw_layer);
  }

  lv_obj_add_flag(canvas_card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(canvas_card, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(canvas_card, ui_trace_event_cb, LV_EVENT_ALL, st);
}
