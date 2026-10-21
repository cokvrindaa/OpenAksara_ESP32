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

// ---------- Kanvas Goresan Aksara Interaktif (Sequential Grid Checkpoints & Corridor Guard) ----------
#define MAX_TRACE_STROKES 6
#define MAX_TRACE_POINTS_PER_STROKE 60
#define MAX_CHECKPOINTS 16

struct StrokeWaypoint {
  int16_t x;
  int16_t y;
};

struct StrokeEvalResult {
  bool success;
  const char* message;
};

struct TraceCheckpoint {
  int16_t x;
  int16_t y;
  lv_obj_t* marker_card; // Lingkaran penanda titik
  lv_obj_t* label_obj;   // Label teks "P1", "P2", dst.
  bool reached;
};

struct TraceState {
  lv_obj_t* canvas_card;
  lv_obj_t* draw_layer;
  lv_obj_t* feedback_lbl;
  lv_point_precise_t (*points)[MAX_TRACE_POINTS_PER_STROKE];
  uint16_t pt_counts[MAX_TRACE_STROKES];
  uint8_t stroke_count;
  bool is_drawing;

  // Sequential Checkpoints & Corridor Guard Engine
  TraceCheckpoint checkpoints[MAX_CHECKPOINTS];
  uint8_t checkpoint_count;
  int8_t active_checkpoint_idx; // Indeks titik Pi yang wajib dicapai berikutnya
  uint8_t tolerance_r;          // Radius batas sentuhan checkpoint R (default 18px)
  uint8_t corridor_r;           // Batas radius toleransi keluar garis (default 20px)
  uint8_t fatal_corridor_r;     // Batas radius kegagalan fatal (default 32px)
  bool is_completed;            // Apakah seluruh titik telah tuntas berurutan
  bool is_failed;               // Status gagal total jika keluar garis parah
  bool segment_out_of_bounds;   // Status melenceng ke luar garis pada segmen aktif
  uint16_t stray_points;        // Jumlah titik coretan yang berada di luar batas garis
  uint16_t offtrack_samples;    // Penghitung deviasi ke luar jalur
};

// Menghitung kuadrat jarak minimum dari titik P ke segmen garis AB (Point-to-Segment Math)
static inline int32_t dist_sq_point_to_segment(int32_t px, int32_t py, int32_t ax, int32_t ay, int32_t bx, int32_t by) {
  int32_t l2 = (bx - ax) * (bx - ax) + (by - ay) * (by - ay);
  if (l2 == 0) {
    int32_t dx = px - ax;
    int32_t dy = py - ay;
    return dx * dx + dy * dy;
  }
  int32_t dot = (px - ax) * (bx - ax) + (py - ay) * (by - ay);
  if (dot <= 0) {
    int32_t dx = px - ax;
    int32_t dy = py - ay;
    return dx * dx + dy * dy;
  }
  if (dot >= l2) {
    int32_t dx = px - bx;
    int32_t dy = py - by;
    return dx * dx + dy * dy;
  }
  int64_t cross = (int64_t)(bx - ax) * (ay - py) - (int64_t)(by - ay) * (ax - px);
  return (int32_t)((cross * cross) / l2);
}

// Update tampilan visual sebuah checkpoint Pi sesuai statusnya
inline void ui_trace_style_checkpoint(TraceState* st, int idx) {
  if (!st || idx < 0 || idx >= st->checkpoint_count) return;
  TraceCheckpoint& cp = st->checkpoints[idx];
  if (!cp.marker_card) return;

  if (cp.reached) {
    // Berhasil dicapai: Hijau sukses
    lv_obj_set_style_bg_color(cp.marker_card, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_border_color(cp.marker_card, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(cp.marker_card, 2, 0);
    if (cp.label_obj) {
      lv_obj_set_style_text_color(cp.label_obj, lv_color_hex(0xFFFFFF), 0);
    }
  } else if (idx == st->active_checkpoint_idx) {
    // Target aktif saat ini: Kuning/Emas menyala
    lv_obj_set_style_bg_color(cp.marker_card, lv_color_hex(0xEAB308), 0);
    lv_obj_set_style_border_color(cp.marker_card, lv_color_hex(0xFEF08A), 0);
    lv_obj_set_style_border_width(cp.marker_card, 2, 0);
    if (cp.label_obj) {
      lv_obj_set_style_text_color(cp.label_obj, lv_color_hex(0x000000), 0);
    }
  } else if (idx == st->checkpoint_count - 1) {
    // Titik akhir target (belum tercapai): Oranye / Merah
    lv_obj_set_style_bg_color(cp.marker_card, lv_color_hex(0xEA580C), 0);
    lv_obj_set_style_border_color(cp.marker_card, lv_color_hex(0xFDBA74), 0);
    lv_obj_set_style_border_width(cp.marker_card, 2, 0);
    if (cp.label_obj) {
      lv_obj_set_style_text_color(cp.label_obj, lv_color_hex(0xFFFFFF), 0);
    }
  } else {
    // Titik acuan menunggu: Biru gelap dengan border cyan tipis
    lv_obj_set_style_bg_color(cp.marker_card, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_color(cp.marker_card, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_style_border_width(cp.marker_card, 1, 0);
    if (cp.label_obj) {
      lv_obj_set_style_text_color(cp.label_obj, lv_color_hex(0xBAE6FD), 0);
    }
  }
}

inline void ui_trace_reset_checkpoints(TraceState* st) {
  if (!st) return;
  st->active_checkpoint_idx = 0;
  st->is_completed = false;
  st->is_failed = false;
  st->segment_out_of_bounds = false;
  st->stray_points = 0;
  st->offtrack_samples = 0;
  for (int i = 0; i < st->checkpoint_count; i++) {
    st->checkpoints[i].reached = false;
    ui_trace_style_checkpoint(st, i);
  }
  if (st->feedback_lbl) {
    char buf[48];
    snprintf(buf, sizeof(buf), "Mulai dari P1 (Tetap di garis)");
    lv_label_set_text(st->feedback_lbl, buf);
    lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0x38BDF8), 0);
  }
}

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
  ui_trace_reset_checkpoints(st);
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
  for (int i = 0; i < MAX_CHECKPOINTS; i++) {
    if (st->checkpoints[i].marker_card) {
      lv_obj_delete(st->checkpoints[i].marker_card);
      st->checkpoints[i].marker_card = nullptr;
      st->checkpoints[i].label_obj = nullptr;
    }
  }
  if (st->draw_layer) {
    lv_obj_delete(st->draw_layer);
    st->draw_layer = nullptr;
  }
  if (st->points) {
    free(st->points);
    st->points = nullptr;
  }
}

inline void ui_trace_set_checkpoints(TraceState* st, const StrokeWaypoint* pts, uint8_t count,
                                    uint8_t tolerance_r = 18, uint8_t corridor_r = 20, uint8_t fatal_corridor_r = 32) {
  if (!st || !st->canvas_card) return;

  for (int i = 0; i < MAX_CHECKPOINTS; i++) {
    if (st->checkpoints[i].marker_card) {
      lv_obj_delete(st->checkpoints[i].marker_card);
      st->checkpoints[i].marker_card = nullptr;
      st->checkpoints[i].label_obj = nullptr;
    }
  }

  if (count > MAX_CHECKPOINTS) count = MAX_CHECKPOINTS;
  st->checkpoint_count = count;
  st->tolerance_r = tolerance_r > 0 ? tolerance_r : 18;
  st->corridor_r = corridor_r > 0 ? corridor_r : 20;
  st->fatal_corridor_r = fatal_corridor_r > 0 ? fatal_corridor_r : 32;

  const int BADGE_SZ = 20;
  const int BADGE_HALF = BADGE_SZ / 2;

  for (uint8_t i = 0; i < count; i++) {
    st->checkpoints[i].x = pts[i].x;
    st->checkpoints[i].y = pts[i].y;
    st->checkpoints[i].reached = false;

    lv_obj_t* card = ui_make_card(st->canvas_card,
                                  pts[i].x - BADGE_HALF,
                                  pts[i].y - BADGE_HALF,
                                  BADGE_SZ, BADGE_SZ,
                                  lv_color_hex(0x1E293B), lv_color_hex(0x1E293B),
                                  BADGE_HALF);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    char ptxt[8];
    snprintf(ptxt, sizeof(ptxt), "P%d", i + 1);
    lv_obj_t* lbl = ui_make_label(card, ptxt, &lv_font_montserrat_10, lv_color_hex(0xBAE6FD));
    lv_obj_clear_flag(lbl, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(lbl);

    st->checkpoints[i].marker_card = card;
    st->checkpoints[i].label_obj = lbl;
  }

  ui_trace_reset_checkpoints(st);
}

// Pemrosesan sentuhan stylus dengan penjagaan koridor batas garis & deteksi gagal fatal
inline void ui_trace_process_point(TraceState* st, int32_t rx, int32_t ry, int32_t prev_x, int32_t prev_y, bool has_prev) {
  if (!st || st->is_completed || st->is_failed || st->checkpoint_count == 0) return;
  if (st->active_checkpoint_idx >= st->checkpoint_count) return;

  int curr = st->active_checkpoint_idx;
  int prev = curr > 0 ? curr - 1 : 0;

  // 1. Cek Batas Koridor Garis (Corridor Boundary Enforcement)
  if (curr == 0) {
    // Titik awal P1
    int32_t d2_start = (rx - st->checkpoints[0].x) * (rx - st->checkpoints[0].x) +
                       (ry - st->checkpoints[0].y) * (ry - st->checkpoints[0].y);
    int32_t r2 = (int32_t)st->tolerance_r * (int32_t)st->tolerance_r;
    int32_t fatal_r2 = (int32_t)st->fatal_corridor_r * (int32_t)st->fatal_corridor_r;

    // Jika mulai terlalu jauh dari P1 dan sudah menyeret stylus: GAGAL!
    if (has_prev && d2_start > fatal_r2) {
      st->is_failed = true;
      audio_play(SOUND_ERROR);
      if (st->feedback_lbl) {
        lv_label_set_text(st->feedback_lbl, "GAGAL! Mulai dari titik P1");
        lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xEF4444), 0);
      }
      return;
    }

    if (d2_start > r2) {
      st->segment_out_of_bounds = true;
      st->stray_points++;
      if (st->feedback_lbl) {
        lv_label_set_text(st->feedback_lbl, "Mulai goresan dari P1!");
        lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xF59E0B), 0);
      }
      return;
    } else {
      st->segment_out_of_bounds = false;
    }
  } else {
    // Segmen aktif antara P(prev) -> P(curr)
    int32_t d2_corridor = dist_sq_point_to_segment(rx, ry,
                                                   st->checkpoints[prev].x, st->checkpoints[prev].y,
                                                   st->checkpoints[curr].x, st->checkpoints[curr].y);
    int32_t max_corridor_d2 = (int32_t)st->corridor_r * (int32_t)st->corridor_r;
    int32_t fatal_corridor_d2 = (int32_t)st->fatal_corridor_r * (int32_t)st->fatal_corridor_r;

    // A. KELUAR GARIS PARAH: GAGAL TOTAL! HENTIKAN PENULISAN!
    if (d2_corridor > fatal_corridor_d2) {
      st->is_failed = true;
      st->segment_out_of_bounds = true;
      audio_play(SOUND_ERROR);
      if (st->feedback_lbl) {
        lv_label_set_text(st->feedback_lbl, "GAGAL! Keluar garis terlalu jauh");
        lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xEF4444), 0);
      }
      return;
    }

    // B. Keluar batas koridor toleransi sedang
    if (d2_corridor > max_corridor_d2) {
      st->segment_out_of_bounds = true;
      st->stray_points++;
      if (st->stray_points > 5) {
        // Terlalu banyak titik di luar garis secara kontinu -> GAGAL!
        st->is_failed = true;
        audio_play(SOUND_ERROR);
        if (st->feedback_lbl) {
          lv_label_set_text(st->feedback_lbl, "GAGAL! Coretan di luar jalur");
          lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xEF4444), 0);
        }
        return;
      }
      if (st->feedback_lbl) {
        char buf[54];
        snprintf(buf, sizeof(buf), "Keluar garis! Kembali ke P%d", curr);
        lv_label_set_text(st->feedback_lbl, buf);
        lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xEF4444), 0);
      }
    } else {
      // Pemulihan jika stylus kembali mendekat ke titik awal segmen
      int32_t d2_origin = (rx - st->checkpoints[prev].x) * (rx - st->checkpoints[prev].x) +
                          (ry - st->checkpoints[prev].y) * (ry - st->checkpoints[prev].y);
      if (d2_origin <= ((int32_t)st->tolerance_r * (int32_t)st->tolerance_r)) {
        st->segment_out_of_bounds = false;
      } else if (d2_corridor <= ((int32_t)(st->corridor_r - 4) * (int32_t)(st->corridor_r - 4))) {
        st->segment_out_of_bounds = false;
      }
    }
  }

  // Jika sedang keluar garis atau gagal: Pi DIKUNCI! Tidak bisa menempel/loncat dari luar batas
  if (st->segment_out_of_bounds || st->is_failed) {
    return;
  }

  // 2. Evaluasi apakah target checkpoint Pi tercapai secara sah
  while (st->active_checkpoint_idx < st->checkpoint_count) {
    int target = st->active_checkpoint_idx;
    int32_t tx = st->checkpoints[target].x;
    int32_t ty = st->checkpoints[target].y;
    int32_t d2 = (rx - tx) * (rx - tx) + (ry - ty) * (ry - ty);
    int32_t r2 = (int32_t)st->tolerance_r * (int32_t)st->tolerance_r;
    bool reached = (d2 <= r2);

    if (!reached && has_prev) {
      if (dist_sq_point_to_segment(tx, ty, prev_x, prev_y, rx, ry) <= r2) {
        reached = true;
      }
    }

    if (reached) {
      st->checkpoints[target].reached = true;
      ui_trace_style_checkpoint(st, target);
      audio_play(SOUND_CLICK);

      st->active_checkpoint_idx++;
      st->segment_out_of_bounds = false;
      st->offtrack_samples = 0;

      if (st->active_checkpoint_idx >= st->checkpoint_count) {
        // Verifikasi apakah siswa membuat coretan liar keluar garis
        if (st->stray_points > 5) {
          st->is_failed = true;
          audio_play(SOUND_ERROR);
          if (st->feedback_lbl) {
            lv_label_set_text(st->feedback_lbl, "GAGAL! Garis tidak rapi");
            lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xEF4444), 0);
          }
        } else {
          st->is_completed = true;
          audio_play(SOUND_SUCCESS);
          audio_play(SOUND_REWARD);
          if (st->feedback_lbl) {
            lv_label_set_text(st->feedback_lbl, "BENAR! Sempurna (+15 XP)");
            lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0x22C55E), 0);
          }
        }
        return;
      } else {
        ui_trace_style_checkpoint(st, st->active_checkpoint_idx);
        if (st->feedback_lbl) {
          char buf[48];
          snprintf(buf, sizeof(buf), "Bagus! Lanjut ke P%d", st->active_checkpoint_idx + 1);
          lv_label_set_text(st->feedback_lbl, buf);
          lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0x38BDF8), 0);
        }
      }
    } else {
      break;
    }
  }

  // 3. Peringatan Urutan Salah / Mundur
  if (!st->is_completed && !st->is_failed && st->active_checkpoint_idx < st->checkpoint_count) {
    int curr_target = st->active_checkpoint_idx;
    int32_t r2 = (int32_t)st->tolerance_r * (int32_t)st->tolerance_r;

    // A. Melompati urutan
    for (int k = curr_target + 1; k < st->checkpoint_count; k++) {
      int32_t kx = st->checkpoints[k].x;
      int32_t ky = st->checkpoints[k].y;
      int32_t d2_k = (rx - kx) * (rx - kx) + (ry - ky) * (ry - ky);
      if (d2_k <= r2) {
        if (st->feedback_lbl) {
          char buf[54];
          snprintf(buf, sizeof(buf), "Urutan terlewati! Cari P%d dahulu", curr_target + 1);
          lv_label_set_text(st->feedback_lbl, buf);
          lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xF59E0B), 0);
        }
        break;
      }
    }

    // B. Mundur ke titik sebelumnya
    if (curr_target > 0) {
      int prev_cp = curr_target - 1;
      int32_t px = st->checkpoints[prev_cp].x;
      int32_t py = st->checkpoints[prev_cp].y;
      int32_t d2_prev = (rx - px) * (rx - px) + (ry - py) * (ry - py);
      if (d2_prev <= r2) {
        if (st->feedback_lbl) {
          char buf[48];
          snprintf(buf, sizeof(buf), "Arah terbalik! Tuju P%d", curr_target + 1);
          lv_label_set_text(st->feedback_lbl, buf);
          lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xEF4444), 0);
        }
      }
    }
  }
}

// Algoritma Evaluasi Akhir untuk tombol "Cek Goresan"
inline StrokeEvalResult ui_eval_stroke_trace(const TraceState* st) {
  StrokeEvalResult res = { false, "Goresan kosong!" };
  if (!st || st->checkpoint_count == 0) return res;

  if (st->is_failed) {
    res.success = false;
    res.message = "GAGAL! Keluar garis terlalu parah. Tekan Bersihkan untuk ulang.";
    return res;
  }

  if (st->is_completed) {
    res.success = true;
    res.message = "BENAR! Seluruh titik dilewati berurutan & rapi (+15 XP)";
    return res;
  }

  if (st->stray_points > 5) {
    res.success = false;
    res.message = "Goresan keluar garis! Bersihkan dan ikuti alur biru.";
    return res;
  }

  if (st->active_checkpoint_idx == 0) {
    res.success = false;
    res.message = "Goresan belum dimulai! Ikuti dari P1.";
    return res;
  }

  static char msg_buf[64];
  snprintf(msg_buf, sizeof(msg_buf), "Progres P%d dari P%d titik! Lanjutkan alur.",
           st->active_checkpoint_idx, st->checkpoint_count);
  res.success = false;
  res.message = msg_buf;
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
      ui_trace_process_point(st, rx, ry, rx, ry, false);
    }
  } else if (code == LV_EVENT_PRESSING) {
    if (st->is_drawing && st->stroke_count < MAX_TRACE_STROKES) {
      uint8_t s = st->stroke_count;
      uint16_t count = st->pt_counts[s];

      if (count < MAX_TRACE_POINTS_PER_STROKE) {
        int32_t prev_x = st->points[s][count - 1].x;
        int32_t prev_y = st->points[s][count - 1].y;
        int32_t dx = rx - prev_x;
        int32_t dy = ry - prev_y;
        if (dx * dx + dy * dy >= 25) {
          st->points[s][count].x = rx;
          st->points[s][count].y = ry;

          // Proses posisi stylus & periksa koridor batas garis
          ui_trace_process_point(st, rx, ry, prev_x, prev_y, true);

          // Render segmen: warna PUTIH jika di dalam garis, warna MERAH jika keluar garis atau gagal!
          if (st->draw_layer) {
            lv_obj_t* seg = lv_line_create(st->draw_layer);
            lv_obj_clear_flag(seg, LV_OBJ_FLAG_CLICKABLE);
            lv_line_set_points(seg, &st->points[s][count - 1], 2);
            lv_color_t line_col = (st->segment_out_of_bounds || st->is_failed) ? lv_color_hex(0xEF4444) : lv_color_hex(0xFFFFFF);
            lv_obj_set_style_line_color(seg, line_col, 0);
            lv_obj_set_style_line_width(seg, 5, 0);
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
        st->points[s][1].x = st->points[s][0].x + 1;
        st->points[s][1].y = st->points[s][0].y;
        st->pt_counts[s] = 2;
        if (st->draw_layer) {
          lv_obj_t* seg = lv_line_create(st->draw_layer);
          lv_obj_clear_flag(seg, LV_OBJ_FLAG_CLICKABLE);
          lv_line_set_points(seg, &st->points[s][0], 2);
          lv_color_t line_col = (st->segment_out_of_bounds || st->is_failed) ? lv_color_hex(0xEF4444) : lv_color_hex(0xFFFFFF);
          lv_obj_set_style_line_color(seg, line_col, 0);
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
  st->checkpoint_count = 0;
  st->active_checkpoint_idx = 0;
  st->tolerance_r = 18;
  st->corridor_r = 20;
  st->fatal_corridor_r = 32;
  st->is_completed = false;
  st->is_failed = false;
  st->segment_out_of_bounds = false;
  st->stray_points = 0;
  st->offtrack_samples = 0;

  for (int i = 0; i < MAX_CHECKPOINTS; i++) {
    st->checkpoints[i].marker_card = nullptr;
    st->checkpoints[i].label_obj = nullptr;
    st->checkpoints[i].reached = false;
  }

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
