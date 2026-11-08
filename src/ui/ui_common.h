#pragma once
#include <lvgl.h>
#include "audio_player.h"

// ---------- Palet Warna (Modern Dark Theme) ----------
#define C_BG lv_color_hex(0x0B0D12)
#define C_CARD lv_color_hex(0x161A22)
#define C_CARD_PR lv_color_hex(0x232938)
#define C_TEXT lv_color_hex(0xF2F4F8)
#define C_SUB lv_color_hex(0x8A93A6)
#define C_MUTED lv_color_hex(0x565E6D)

// Aksara Jawa Theme (Indigo / Purple)
#define C_JAWA_1 lv_color_hex(0x7C5CFF)
#define C_JAWA_2 lv_color_hex(0x5B3FE0)

// Aksara Bali Theme (Warm Amber / Gold)
#define C_BALI_1 lv_color_hex(0xF59E0B)
#define C_BALI_2 lv_color_hex(0xD97706)

// Accent tambahan
#define C_PINK lv_color_hex(0xFF5C8A)
#define C_PINK_D lv_color_hex(0xD93F6C)

// Jenis aksara
enum AksaraType
{
  AKSARA_JAWA = 0,
  AKSARA_BALI = 1
};

// Callback aplikasi
struct AppCallbacks
{
  void (*onSelectScript)(AksaraType type);            // Saat memilih Jawa / Bali
  void (*onStartPractice)(AksaraType type);           // Kartu besar "Mulai Latihan"
  void (*onSelectAksara)(AksaraType type, int index); // Memilih salah satu aksara (0-4)
  void (*onAllAksara)(AksaraType type);               // Tombol "Semua Aksara"
  void (*onSettings)();                               // Tombol "Setelan"
};

// Helper UI umum (dioptimasi: gradien hanya jika warna beda)
inline lv_obj_t *ui_make_card(lv_obj_t *parent, int x, int y, int w, int h,
                              lv_color_t c1, lv_color_t c2, int radius)
{
  lv_obj_t *o = lv_obj_create(parent);
  if (!o)
    return nullptr;
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_radius(o, radius, 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(o, c1, 0);
  if (c1.blue != c2.blue || c1.green != c2.green || c1.red != c2.red)
  {
    lv_obj_set_style_bg_grad_color(o, c2, 0);
    lv_obj_set_style_bg_grad_dir(o, LV_GRAD_DIR_VER, 0);
  }
  else
  {
    lv_obj_set_style_bg_grad_dir(o, LV_GRAD_DIR_NONE, 0);
  }
  lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
  return o;
}

inline lv_obj_t *ui_make_label(lv_obj_t *parent, const char *text,
                               const lv_font_t *font, lv_color_t color)
{
  lv_obj_t *l = lv_label_create(parent);
  if (!l)
    return nullptr;
  if (font)
    lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, color, 0);
  if (text)
    lv_label_set_text(l, text);
  return l;
}

inline void ui_set_pressed(lv_obj_t *o, lv_color_t c1, lv_color_t c2)
{
  if (!o)
    return;
  lv_obj_set_style_bg_color(o, c1, LV_PART_MAIN | LV_STATE_PRESSED);
  if (c1.blue != c2.blue || c1.green != c2.green || c1.red != c2.red)
  {
    lv_obj_set_style_bg_grad_color(o, c2, LV_PART_MAIN | LV_STATE_PRESSED);
  }
  lv_obj_set_style_translate_y(o, 2, LV_PART_MAIN | LV_STATE_PRESSED);
}

// Toast global
void ui_toast_init();
void ui_show_toast(const char *text);

// Maskot Buddy (Varian Babak 1, 2, 3 & Home Avatar)
#ifdef __cplusplus
extern "C"
{
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
  LV_FONT_DECLARE(font_jawa_160);
  LV_FONT_DECLARE(font_bali_160);
#ifdef __cplusplus
}
#endif

// ---------- Kanvas Goresan Aksara Interaktif (Sequential Grid Checkpoints & Corridor Guard) ----------
#define MAX_TRACE_STROKES 6
#define MAX_TRACE_POINTS_PER_STROKE 100
#define MAX_CHECKPOINTS 100

struct StrokeWaypoint
{
  int16_t x;
  int16_t y;
};

struct StrokeEvalResult
{
  bool success;
  const char *message;
};

struct TraceCheckpoint
{
  int16_t x;
  int16_t y;
  lv_obj_t *marker_card; // Lingkaran penanda titik
  lv_obj_t *label_obj;   // Label teks "P1", "P2", dst.
  bool reached;
};

struct TraceState
{
  lv_obj_t *canvas_card;
  lv_obj_t *draw_layer;
  lv_obj_t *feedback_lbl;
  lv_obj_t *watermark_lbl;      // Watermark glif aksara di kanvas (160px)
  lv_obj_t *guide_track;        // Jalur garis panduan (biru)
  lv_obj_t *celebration_layer;  // Layer efek perayaan kejutan (surprise)
  lv_color_t default_border_color;
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
static inline int32_t dist_sq_point_to_segment(int32_t px, int32_t py, int32_t ax, int32_t ay, int32_t bx, int32_t by)
{
  int32_t l2 = (bx - ax) * (bx - ax) + (by - ay) * (by - ay);
  if (l2 == 0)
  {
    int32_t dx = px - ax;
    int32_t dy = py - ay;
    return dx * dx + dy * dy;
  }
  int32_t dot = (px - ax) * (bx - ax) + (py - ay) * (by - ay);
  if (dot <= 0)
  {
    int32_t dx = px - ax;
    int32_t dy = py - ay;
    return dx * dx + dy * dy;
  }
  if (dot >= l2)
  {
    int32_t dx = px - bx;
    int32_t dy = py - by;
    return dx * dx + dy * dy;
  }
  int64_t cross = (int64_t)(bx - ax) * (ay - py) - (int64_t)(by - ay) * (ax - px);
  return (int32_t)((cross * cross) / l2);
}

// Update tampilan visual sebuah checkpoint Pi sesuai statusnya
inline void ui_trace_style_checkpoint(TraceState *st, int idx)
{
  if (!st || idx < 0 || idx >= st->checkpoint_count)
    return;
  TraceCheckpoint &cp = st->checkpoints[idx];
  if (!cp.marker_card)
    return;

  if (cp.reached)
  {
    // Berhasil dicapai: Hijau sukses
    lv_obj_set_style_bg_color(cp.marker_card, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_border_color(cp.marker_card, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(cp.marker_card, 2, 0);
    if (cp.label_obj)
    {
      lv_obj_set_style_text_color(cp.label_obj, lv_color_hex(0xFFFFFF), 0);
    }
  }
  else if (idx == st->active_checkpoint_idx)
  {
    // Target aktif saat ini: Kuning/Emas menyala
    lv_obj_set_style_bg_color(cp.marker_card, lv_color_hex(0xEAB308), 0);
    lv_obj_set_style_border_color(cp.marker_card, lv_color_hex(0xFEF08A), 0);
    lv_obj_set_style_border_width(cp.marker_card, 2, 0);
    if (cp.label_obj)
    {
      lv_obj_set_style_text_color(cp.label_obj, lv_color_hex(0x000000), 0);
    }
  }
  else if (idx == st->checkpoint_count - 1)
  {
    // Titik akhir target (belum tercapai): Oranye / Merah
    lv_obj_set_style_bg_color(cp.marker_card, lv_color_hex(0xEA580C), 0);
    lv_obj_set_style_border_color(cp.marker_card, lv_color_hex(0xFDBA74), 0);
    lv_obj_set_style_border_width(cp.marker_card, 2, 0);
    if (cp.label_obj)
    {
      lv_obj_set_style_text_color(cp.label_obj, lv_color_hex(0xFFFFFF), 0);
    }
  }
  else
  {
    // Titik acuan menunggu: Biru gelap dengan border cyan tipis
    lv_obj_set_style_bg_color(cp.marker_card, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_color(cp.marker_card, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_style_border_width(cp.marker_card, 1, 0);
    if (cp.label_obj)
    {
      lv_obj_set_style_text_color(cp.label_obj, lv_color_hex(0xBAE6FD), 0);
    }
  }
}

inline void ui_trace_reset_checkpoints(TraceState *st)
{
  if (!st)
    return;
  st->active_checkpoint_idx = 0;
  st->is_completed = false;
  st->is_failed = false;
  st->segment_out_of_bounds = false;
  st->stray_points = 0;
  st->offtrack_samples = 0;
  for (int i = 0; i < st->checkpoint_count; i++)
  {
    st->checkpoints[i].reached = false;
    ui_trace_style_checkpoint(st, i);
  }
  if (st->feedback_lbl)
  {
    lv_label_set_text(st->feedback_lbl, "Mulai dari titik hijau (ikuti garis)");
    lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0x38BDF8), 0);
  }
}

// Callback animasi untuk efek perayaan kejutan (celebration surprise)
static void celebration_anim_x_cb(void* var, int32_t val) {
  lv_obj_set_x((lv_obj_t*)var, val);
}

static void celebration_anim_y_cb(void* var, int32_t val) {
  lv_obj_set_y((lv_obj_t*)var, val);
}

static void celebration_anim_opa_cb(void* var, int32_t val) {
  lv_obj_set_style_opa((lv_obj_t*)var, (lv_opa_t)val, 0);
}

static void celebration_watermark_bounce_cb(void* var, int32_t val) {
  lv_obj_align((lv_obj_t*)var, LV_ALIGN_CENTER, 0, val);
}

inline void ui_trace_set_completed_visuals(TraceState *st)
{
  if (!st)
    return;

  // 1. Sembunyikan coretan stylus user (coretannya di-hide)
  if (st->draw_layer)
  {
    lv_obj_add_flag(st->draw_layer, LV_OBJ_FLAG_HIDDEN);
  }

  // 2. Sembunyikan garis panduan alur dan titik-titik acuan
  if (st->guide_track)
  {
    lv_obj_add_flag(st->guide_track, LV_OBJ_FLAG_HIDDEN);
  }
  for (int i = 0; i < st->checkpoint_count; i++)
  {
    if (st->checkpoints[i].marker_card)
    {
      lv_obj_add_flag(st->checkpoints[i].marker_card, LV_OBJ_FLAG_HIDDEN);
    }
  }

  // 3. Tebalkan aksara (Bold aksara: 100% Opacity Penuh & Solid) + Animasi Melompat Girang
  if (st->watermark_lbl)
  {
    lv_obj_set_style_text_opa(st->watermark_lbl, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(st->watermark_lbl, lv_color_hex(0x38BDF8), 0);

    lv_anim_del(st->watermark_lbl, celebration_watermark_bounce_cb);
    lv_anim_t aw;
    lv_anim_init(&aw);
    lv_anim_set_var(&aw, st->watermark_lbl);
    lv_anim_set_exec_cb(&aw, celebration_watermark_bounce_cb);
    lv_anim_set_values(&aw, 29, 14);
    lv_anim_set_duration(&aw, 160);
    lv_anim_set_reverse_duration(&aw, 180);
    lv_anim_set_path_cb(&aw, lv_anim_path_ease_out);
    lv_anim_start(&aw);
  }

  // 4. Efek Surprise: Pendaran Border Emas & Kembang Api Simbol Ikon Font (Bebas Kotak-Kotak Error)
  if (st->canvas_card)
  {
    lv_obj_set_style_border_color(st->canvas_card, lv_color_hex(0xFACC15), 0);
    lv_obj_set_style_border_width(st->canvas_card, 3, 0);

    if (st->celebration_layer)
    {
      lv_obj_delete(st->celebration_layer);
      st->celebration_layer = nullptr;
    }

    st->celebration_layer = lv_obj_create(st->canvas_card);
    lv_obj_remove_style_all(st->celebration_layer);
    lv_obj_set_size(st->celebration_layer, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(st->celebration_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(st->celebration_layer, LV_OBJ_FLAG_SCROLLABLE);

    // Kembang api partikel simbolik (menggunakan simbol resmi font, BUKAN card kotak)
    static const char* SPARKLE_ICONS[8] = {
      LV_SYMBOL_CHARGE, LV_SYMBOL_OK, LV_SYMBOL_CHARGE, LV_SYMBOL_OK,
      LV_SYMBOL_CHARGE, LV_SYMBOL_OK, LV_SYMBOL_CHARGE, LV_SYMBOL_OK
    };
    static const lv_color_t SPARKLE_COLORS[8] = {
      lv_color_hex(0xFACC15), lv_color_hex(0x22C55E),
      lv_color_hex(0x38BDF8), lv_color_hex(0xF59E0B),
      lv_color_hex(0x10B981), lv_color_hex(0xFACC15),
      lv_color_hex(0x38BDF8), lv_color_hex(0x22C55E)
    };
    static const int8_t SPARKLE_OFFSETS[8][2] = {
      {  65,   0 }, {  46,  46 }, {   0,  62 }, { -46,  46 },
      { -65,   0 }, { -46, -46 }, {   0, -60 }, {  46, -46 }
    };

    const int cx = 154;
    const int cy = 76;

    for (int i = 0; i < 8; i++)
    {
      lv_obj_t* sp = ui_make_label(st->celebration_layer, SPARKLE_ICONS[i],
                                   &lv_font_montserrat_14, SPARKLE_COLORS[i]);
      lv_obj_set_pos(sp, cx, cy);
      lv_obj_clear_flag(sp, LV_OBJ_FLAG_CLICKABLE);

      int tx = cx + SPARKLE_OFFSETS[i][0];
      int ty = cy + SPARKLE_OFFSETS[i][1];

      // Animasi gerak X
      lv_anim_t ax;
      lv_anim_init(&ax);
      lv_anim_set_var(&ax, sp);
      lv_anim_set_exec_cb(&ax, celebration_anim_x_cb);
      lv_anim_set_values(&ax, cx, tx);
      lv_anim_set_duration(&ax, 600 + (i % 2) * 50);
      lv_anim_set_path_cb(&ax, lv_anim_path_ease_out);
      lv_anim_start(&ax);

      // Animasi gerak Y
      lv_anim_t ay;
      lv_anim_init(&ay);
      lv_anim_set_var(&ay, sp);
      lv_anim_set_exec_cb(&ay, celebration_anim_y_cb);
      lv_anim_set_values(&ay, cy, ty);
      lv_anim_set_duration(&ay, 600 + (i % 2) * 50);
      lv_anim_set_path_cb(&ay, lv_anim_path_ease_out);
      lv_anim_start(&ay);

      // Animasi fade out
      lv_anim_t ao;
      lv_anim_init(&ao);
      lv_anim_set_var(&ao, sp);
      lv_anim_set_exec_cb(&ao, celebration_anim_opa_cb);
      lv_anim_set_values(&ao, LV_OPA_COVER, LV_OPA_0);
      lv_anim_set_delay(&ao, 300);
      lv_anim_set_duration(&ao, 350);
      lv_anim_start(&ao);
    }

    // 5. Floating Badge Perayaan Kejutan: Lencana Emas Berpendar (Menggunakan Simbol Resmi Font)
    lv_obj_t* pill = ui_make_card(st->celebration_layer, 44, 115, 220, 32,
                                  lv_color_hex(0xF59E0B), lv_color_hex(0xD97706), 16);
    lv_obj_set_style_border_color(pill, lv_color_hex(0xFEF08A), 0);
    lv_obj_set_style_border_width(pill, 2, 0);
    lv_obj_clear_flag(pill, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(pill, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* pill_lbl = ui_make_label(pill, LV_SYMBOL_OK " SEMPURNA! +15 XP " LV_SYMBOL_CHARGE,
                                       &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
    lv_obj_center(pill_lbl);

    // Animasi melonjak ke atas (overshoot bounce)
    lv_anim_t ap;
    lv_anim_init(&ap);
    lv_anim_set_var(&ap, pill);
    lv_anim_set_exec_cb(&ap, celebration_anim_y_cb);
    lv_anim_set_values(&ap, 115, 90);
    lv_anim_set_duration(&ap, 350);
    lv_anim_set_path_cb(&ap, lv_anim_path_overshoot);
    lv_anim_start(&ap);

    // Fade out pill secara halus setelah 2.2 detik
    lv_anim_t ap_fade;
    lv_anim_init(&ap_fade);
    lv_anim_set_var(&ap_fade, pill);
    lv_anim_set_exec_cb(&ap_fade, celebration_anim_opa_cb);
    lv_anim_set_values(&ap_fade, LV_OPA_COVER, LV_OPA_0);
    lv_anim_set_delay(&ap_fade, 2200);
    lv_anim_set_duration(&ap_fade, 400);
    lv_anim_start(&ap_fade);
  }

  // 6. Notifikasi Toast & Feedback Label (Simbol Resmi Font, Bukan Emoji)
  ui_show_toast(LV_SYMBOL_OK " Luar Biasa! Sempurna (+15 XP) " LV_SYMBOL_CHARGE);
  if (st->feedback_lbl)
  {
    lv_label_set_text(st->feedback_lbl, LV_SYMBOL_OK " BENAR! (+15 XP)");
    lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0x22C55E), 0);
  }
}

inline void ui_trace_clear(TraceState *st)
{
  if (!st)
    return;
  if (st->celebration_layer)
  {
    lv_obj_delete(st->celebration_layer);
    st->celebration_layer = nullptr;
  }
  if (st->canvas_card)
  {
    lv_obj_set_style_border_color(st->canvas_card, st->default_border_color, 0);
    lv_obj_set_style_border_width(st->canvas_card, 2, 0);
  }
  if (st->draw_layer)
  {
    lv_obj_clean(st->draw_layer);
    lv_obj_remove_flag(st->draw_layer, LV_OBJ_FLAG_HIDDEN);
  }
  if (st->guide_track)
  {
    lv_obj_remove_flag(st->guide_track, LV_OBJ_FLAG_HIDDEN);
  }
  for (int i = 0; i < st->checkpoint_count; i++)
  {
    if (st->checkpoints[i].marker_card)
    {
      lv_obj_remove_flag(st->checkpoints[i].marker_card, LV_OBJ_FLAG_HIDDEN);
    }
  }
  if (st->watermark_lbl)
  {
    lv_anim_del(st->watermark_lbl, celebration_watermark_bounce_cb);
    lv_obj_set_style_text_opa(st->watermark_lbl, LV_OPA_30, 0);
    lv_obj_set_style_text_color(st->watermark_lbl, lv_color_hex(0x38BDF8), 0);
    lv_obj_align(st->watermark_lbl, LV_ALIGN_CENTER, 0, 29);
  }
  for (int i = 0; i < MAX_TRACE_STROKES; i++)
  {
    st->pt_counts[i] = 0;
  }
  st->stroke_count = 0;
  st->is_drawing = false;
  ui_trace_reset_checkpoints(st);
}

inline int ui_trace_total_points(const TraceState *st)
{
  if (!st)
    return 0;
  int total = 0;
  for (int i = 0; i < st->stroke_count; i++)
  {
    total += st->pt_counts[i];
  }
  return total;
}

inline void ui_trace_detach(TraceState *st)
{
  if (!st)
    return;
  ui_trace_clear(st);
  for (int i = 0; i < MAX_CHECKPOINTS; i++)
  {
    if (st->checkpoints[i].marker_card)
    {
      lv_obj_delete(st->checkpoints[i].marker_card);
      st->checkpoints[i].marker_card = nullptr;
      st->checkpoints[i].label_obj = nullptr;
    }
  }
  if (st->draw_layer)
  {
    lv_obj_delete(st->draw_layer);
    st->draw_layer = nullptr;
  }
  if (st->points)
  {
    free(st->points);
    st->points = nullptr;
  }
}

inline void ui_trace_set_checkpoints(TraceState *st, const StrokeWaypoint *pts, uint8_t count,
                                     uint8_t tolerance_r = 28, uint8_t corridor_r = 35, uint8_t fatal_corridor_r = 60)
{
  if (!st || !st->canvas_card)
    return;

  for (int i = 0; i < MAX_CHECKPOINTS; i++)
  {
    if (st->checkpoints[i].marker_card)
    {
      lv_obj_delete(st->checkpoints[i].marker_card);
      st->checkpoints[i].marker_card = nullptr;
      st->checkpoints[i].label_obj = nullptr;
    }
  }

  if (count > MAX_CHECKPOINTS)
    count = MAX_CHECKPOINTS;
  st->checkpoint_count = count;
  st->tolerance_r = tolerance_r > 0 ? tolerance_r : 28;
  st->corridor_r = corridor_r > 0 ? corridor_r : 35;
  st->fatal_corridor_r = fatal_corridor_r > 0 ? fatal_corridor_r : 60;

  for (uint8_t i = 0; i < count; i++)
  {
    st->checkpoints[i].x = pts[i].x;
    st->checkpoints[i].y = pts[i].y;
    st->checkpoints[i].reached = false;
    st->checkpoints[i].marker_card = nullptr;
    st->checkpoints[i].label_obj = nullptr;
  }

  // Tampilkan titik Awal (Hijau) & titik Akhir (Oranye) agar kanvas tetap bersih & kontur terbaca jelas
  if (count > 0)
  {
    const int BADGE_SZ = 14;
    const int BADGE_HALF = BADGE_SZ / 2;
    lv_obj_t *start_card = ui_make_card(st->canvas_card,
                                        pts[0].x - BADGE_HALF,
                                        pts[0].y - BADGE_HALF,
                                        BADGE_SZ, BADGE_SZ,
                                        lv_color_hex(0x22C55E), lv_color_hex(0x22C55E),
                                        BADGE_HALF);
    lv_obj_clear_flag(start_card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(start_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_color(start_card, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(start_card, 2, 0);
    st->checkpoints[0].marker_card = start_card;
  }

  if (count > 1)
  {
    const int BADGE_SZ = 14;
    const int BADGE_HALF = BADGE_SZ / 2;
    int last = count - 1;
    lv_obj_t *end_card = ui_make_card(st->canvas_card,
                                      pts[last].x - BADGE_HALF,
                                      pts[last].y - BADGE_HALF,
                                      BADGE_SZ, BADGE_SZ,
                                      lv_color_hex(0xEA580C), lv_color_hex(0xEA580C),
                                      BADGE_HALF);
    lv_obj_clear_flag(end_card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(end_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_color(end_card, lv_color_hex(0xFDBA74), 0);
    lv_obj_set_style_border_width(end_card, 2, 0);
    st->checkpoints[last].marker_card = end_card;
  }

  ui_trace_reset_checkpoints(st);
}

// Pemrosesan sentuhan stylus dengan penjagaan koridor batas garis & deteksi alur aksara cerdas
inline void ui_trace_process_point(TraceState *st, int32_t rx, int32_t ry, int32_t prev_x, int32_t prev_y, bool has_prev)
{
  if (!st || st->is_completed || st->is_failed || st->checkpoint_count == 0)
    return;
  if (st->active_checkpoint_idx >= st->checkpoint_count)
    return;

  int curr = st->active_checkpoint_idx;
  int32_t r2 = (int32_t)st->tolerance_r * (int32_t)st->tolerance_r;
  int32_t max_corridor_d2 = (int32_t)st->corridor_r * (int32_t)st->corridor_r;
  int32_t fatal_corridor_d2 = (int32_t)st->fatal_corridor_r * (int32_t)st->fatal_corridor_r;

  // 1. Cek Checkpoint Target dengan Lookahead Adaptif (Mencegah macet jika ada 1 titik terlewat di tikungan rapat)
  int best_target = -1;
  int max_lookahead = 4; // Boleh menjangkau hingga 4 titik ke depan
  int end_look = curr + max_lookahead;
  if (end_look >= st->checkpoint_count)
    end_look = st->checkpoint_count - 1;

  for (int target = curr; target <= end_look; target++)
  {
    int32_t tx = st->checkpoints[target].x;
    int32_t ty = st->checkpoints[target].y;
    int32_t d2 = (rx - tx) * (rx - tx) + (ry - ty) * (ry - ty);

    bool reached = (d2 <= r2);
    if (!reached && has_prev)
    {
      if (dist_sq_point_to_segment(tx, ty, prev_x, prev_y, rx, ry) <= r2)
      {
        reached = true;
      }
    }

    if (reached)
    {
      best_target = target;
    }
  }

  // Jika checkpoint terdeteksi tercapai
  if (best_target >= curr)
  {
    for (int k = curr; k <= best_target; k++)
    {
      st->checkpoints[k].reached = true;
      ui_trace_style_checkpoint(st, k);
    }
    st->active_checkpoint_idx = best_target + 1;
    st->segment_out_of_bounds = false;
    st->offtrack_samples = 0;
    if (st->stray_points > 0)
      st->stray_points--;
    audio_play(SOUND_CLICK);

    // Cek apakah seluruh alur aksara selesai
    if (st->active_checkpoint_idx >= st->checkpoint_count)
    {
      st->is_completed = true;
      ui_trace_set_completed_visuals(st);
      audio_play(SOUND_SUCCESS);
      audio_play(SOUND_REWARD);
      if (st->feedback_lbl)
      {
        lv_label_set_text(st->feedback_lbl, "BENAR! Sempurna (+15 XP)");
        lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0x22C55E), 0);
      }
      return;
    }
    else
    {
      ui_trace_style_checkpoint(st, st->active_checkpoint_idx);
      if (st->feedback_lbl)
      {
        lv_label_set_text(st->feedback_lbl, "Bagus! Teruskan goresan...");
        lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0x38BDF8), 0);
      }
    }
  }

  // 2. Cek Batas Koridor Garis (Corridor Boundary Check)
  curr = st->active_checkpoint_idx;
  if (curr == 0)
  {
    // Jika belum mulai di titik hijau P1
    int32_t d2_start = (rx - st->checkpoints[0].x) * (rx - st->checkpoints[0].x) +
                       (ry - st->checkpoints[0].y) * (ry - st->checkpoints[0].y);

    if (d2_start > r2)
    {
      st->segment_out_of_bounds = true;
      st->stray_points++;
      if (st->stray_points > 40)
      {
        st->is_failed = true;
        audio_play(SOUND_ERROR);
        if (st->feedback_lbl)
        {
          lv_label_set_text(st->feedback_lbl, "GAGAL! Mulai dari titik awal (hijau)");
          lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xEF4444), 0);
        }
        return;
      }
      if (st->feedback_lbl)
      {
        lv_label_set_text(st->feedback_lbl, "Mulai dari titik hijau!");
        lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xF59E0B), 0);
      }
      return;
    }
    else
    {
      st->segment_out_of_bounds = false;
    }
  }
  else
  {
    // Segmen aktif: periksa jarak ke segmen terdekat di sekitar titik aktif (prev -> curr atau curr -> next)
    int prev = curr > 0 ? curr - 1 : 0;
    int32_t min_d2 = dist_sq_point_to_segment(rx, ry,
                                              st->checkpoints[prev].x, st->checkpoints[prev].y,
                                              st->checkpoints[curr].x, st->checkpoints[curr].y);
    if (curr + 1 < st->checkpoint_count)
    {
      int32_t d2_next = dist_sq_point_to_segment(rx, ry,
                                                 st->checkpoints[curr].x, st->checkpoints[curr].y,
                                                 st->checkpoints[curr + 1].x, st->checkpoints[curr + 1].y);
      if (d2_next < min_d2)
        min_d2 = d2_next;
    }
    if (curr + 2 < st->checkpoint_count)
    {
      int32_t d2_next2 = dist_sq_point_to_segment(rx, ry,
                                                  st->checkpoints[curr + 1].x, st->checkpoints[curr + 1].y,
                                                  st->checkpoints[curr + 2].x, st->checkpoints[curr + 2].y);
      if (d2_next2 < min_d2)
        min_d2 = d2_next2;
    }

    // Keluar koridor batas
    if (min_d2 > fatal_corridor_d2)
    {
      st->segment_out_of_bounds = true;
      st->stray_points += 2;
      if (st->stray_points > 35)
      {
        st->is_failed = true;
        audio_play(SOUND_ERROR);
        if (st->feedback_lbl)
        {
          lv_label_set_text(st->feedback_lbl, "GAGAL! Keluar garis terlalu jauh");
          lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xEF4444), 0);
        }
        return;
      }
      if (st->feedback_lbl)
      {
        lv_label_set_text(st->feedback_lbl, "Keluar garis! Ikuti alur aksara");
        lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xEF4444), 0);
      }
    }
    else if (min_d2 > max_corridor_d2)
    {
      st->segment_out_of_bounds = true;
      st->stray_points++;
      if (st->feedback_lbl)
      {
        lv_label_set_text(st->feedback_lbl, "Mendekat ke alur biru!");
        lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0xF59E0B), 0);
      }
    }
    else
    {
      st->segment_out_of_bounds = false;
      if (st->stray_points > 0)
        st->stray_points--;
      if (st->feedback_lbl)
      {
        lv_label_set_text(st->feedback_lbl, "Bagus! Teruskan goresan...");
        lv_obj_set_style_text_color(st->feedback_lbl, lv_color_hex(0x38BDF8), 0);
      }
    }
  }
}

// Algoritma Evaluasi Akhir untuk tombol "Cek Goresan"
inline StrokeEvalResult ui_eval_stroke_trace(TraceState *st)
{
  StrokeEvalResult res = {false, "Goresan kosong!"};
  if (!st || st->checkpoint_count == 0)
    return res;

  if (st->is_completed)
  {
    if (!st->celebration_layer)
    {
      ui_trace_set_completed_visuals(st);
    }
    res.success = true;
    res.message = "BENAR! Sempurna (+15 XP)";
    return res;
  }

  if (st->is_failed)
  {
    res.success = false;
    res.message = "GAGAL! Keluar garis terlalu parah. Tekan Bersihkan untuk ulang.";
    return res;
  }

  int required = (st->checkpoint_count * 70) / 100;
  if (required < 2) required = st->checkpoint_count;

  if (st->active_checkpoint_idx >= required && st->stray_points <= 25)
  {
    st->is_completed = true;
    for (int i = 0; i < st->checkpoint_count; i++)
    {
      st->checkpoints[i].reached = true;
      ui_trace_style_checkpoint(st, i);
    }
    ui_trace_set_completed_visuals(st);
    res.success = true;
    res.message = "BENAR! Aksara berhasil ditulis dengan rapi (+15 XP)";
    return res;
  }

  if (st->stray_points > 25)
  {
    res.success = false;
    res.message = "Goresan keluar garis! Bersihkan dan ikuti alur biru.";
    return res;
  }

  if (st->active_checkpoint_idx == 0)
  {
    res.success = false;
    res.message = "Goresan belum dimulai! Ikuti dari titik hijau.";
    return res;
  }

  static char msg_buf[64];
  snprintf(msg_buf, sizeof(msg_buf), "Progres %d/%d titik! Lanjutkan goresan.",
           st->active_checkpoint_idx, st->checkpoint_count);
  res.success = false;
  res.message = msg_buf;
  return res;
}

inline void ui_trace_event_cb(lv_event_t *e)
{
  TraceState *st = (TraceState *)lv_event_get_user_data(e);
  if (!st || !st->canvas_card || !st->points)
    return;

  lv_event_code_t code = lv_event_get_code(e);
  lv_indev_t *indev = lv_indev_active();
  if (!indev)
    return;

  lv_point_t p;
  lv_indev_get_point(indev, &p);

  lv_area_t area;
  lv_obj_get_coords(st->canvas_card, &area);

  int32_t rx = p.x - area.x1;
  int32_t ry = p.y - area.y1;
  int32_t w = lv_obj_get_width(st->canvas_card);
  int32_t h = lv_obj_get_height(st->canvas_card);

  if (rx < 0)
    rx = 0;
  if (rx >= w)
    rx = w - 1;
  if (ry < 0)
    ry = 0;
  if (ry >= h)
    ry = h - 1;

  if (code == LV_EVENT_PRESSED)
  {
    if (st->is_completed || st->is_failed)
      return;
    if (st->stroke_count < MAX_TRACE_STROKES)
    {
      uint8_t s = st->stroke_count;
      st->pt_counts[s] = 1;
      st->points[s][0].x = rx;
      st->points[s][0].y = ry;
      st->is_drawing = true;
      ui_trace_process_point(st, rx, ry, rx, ry, false);
    }
  }
  else if (code == LV_EVENT_PRESSING)
  {
    if (st->is_completed || st->is_failed)
      return;
    if (st->is_drawing && st->stroke_count < MAX_TRACE_STROKES)
    {
      uint8_t s = st->stroke_count;
      uint16_t count = st->pt_counts[s];

      if (count < MAX_TRACE_POINTS_PER_STROKE)
      {
        int32_t prev_x = st->points[s][count - 1].x;
        int32_t prev_y = st->points[s][count - 1].y;
        int32_t dx = rx - prev_x;
        int32_t dy = ry - prev_y;
        if (dx * dx + dy * dy >= 25)
        {
          st->points[s][count].x = rx;
          st->points[s][count].y = ry;

          // Proses posisi stylus & periksa koridor batas garis
          ui_trace_process_point(st, rx, ry, prev_x, prev_y, true);

          // Render segmen: warna PUTIH jika di dalam garis, warna MERAH jika keluar garis atau gagal!
          if (st->draw_layer)
          {
            lv_obj_t *seg = lv_line_create(st->draw_layer);
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
  }
  else if (code == LV_EVENT_RELEASED)
  {
    if (st->is_drawing && st->stroke_count < MAX_TRACE_STROKES)
    {
      uint8_t s = st->stroke_count;
      if (st->pt_counts[s] == 1)
      {
        st->points[s][1].x = st->points[s][0].x + 1;
        st->points[s][1].y = st->points[s][0].y;
        st->pt_counts[s] = 2;
        if (st->draw_layer)
        {
          lv_obj_t *seg = lv_line_create(st->draw_layer);
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

      // Evaluasi otomatis saat jari/stylus diangkat:
      // Jika goresan mencapai minimal 75% titik tanpa coretan liar berlebihan, otomatis sukses!
      if (!st->is_completed && !st->is_failed && st->checkpoint_count > 0)
      {
        int required = (st->checkpoint_count * 75) / 100;
        if (required < 2) required = st->checkpoint_count;
        if (st->active_checkpoint_idx >= required && st->stray_points <= 20)
        {
          st->is_completed = true;
          for (int i = 0; i < st->checkpoint_count; i++)
          {
            st->checkpoints[i].reached = true;
            ui_trace_style_checkpoint(st, i);
          }
          ui_trace_set_completed_visuals(st);
          audio_play(SOUND_SUCCESS);
          audio_play(SOUND_REWARD);
        }
      }
    }
  }
}

inline void ui_trace_attach(TraceState *st, lv_obj_t *canvas_card)
{
  if (!st || !canvas_card)
    return;
  st->canvas_card = canvas_card;
  st->feedback_lbl = nullptr;
  st->watermark_lbl = nullptr;
  st->guide_track = nullptr;
  st->celebration_layer = nullptr;
  st->default_border_color = lv_obj_get_style_border_color(canvas_card, LV_PART_MAIN);
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

  for (int i = 0; i < MAX_CHECKPOINTS; i++)
  {
    st->checkpoints[i].marker_card = nullptr;
    st->checkpoints[i].label_obj = nullptr;
    st->checkpoints[i].reached = false;
  }

  for (int i = 0; i < MAX_TRACE_STROKES; i++)
  {
    st->pt_counts[i] = 0;
  }
  if (!st->points)
  {
    st->points = (lv_point_precise_t(*)[MAX_TRACE_POINTS_PER_STROKE])malloc(sizeof(lv_point_precise_t) * MAX_TRACE_STROKES * MAX_TRACE_POINTS_PER_STROKE);
  }
  if (!st->draw_layer)
  {
    st->draw_layer = lv_obj_create(canvas_card);
    lv_obj_remove_style_all(st->draw_layer);
    lv_obj_set_pos(st->draw_layer, 0, 0);
    lv_obj_set_size(st->draw_layer, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(st->draw_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(st->draw_layer, LV_OBJ_FLAG_SCROLLABLE);
  }
  else
  {
    lv_obj_clean(st->draw_layer);
  }

  lv_obj_add_flag(canvas_card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(canvas_card, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(canvas_card, ui_trace_event_cb, LV_EVENT_ALL, st);
}
