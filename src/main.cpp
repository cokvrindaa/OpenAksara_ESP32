#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "audio_player.h"
#include "ui_home.h"

TFT_eSPI tft = TFT_eSPI();
uint16_t calData[5] = { 262, 3665, 291, 3496, 7 };   // kalibrasi, rotasi 1

// Buffer display LVGL v9 (partial rendering 480x40 pixels)
static uint8_t buf1[480 * 40 * 2];
static uint8_t buf2[480 * 40 * 2];

static lv_display_t* disp = nullptr;
static lv_indev_t* indev = nullptr;

// ---- Driver layar LVGL v9 ----
void flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  // swap=false karena LV_COLOR_FORMAT_RGB565_SWAPPED sudah menghasilkan format hardware native
  tft.pushColors((uint16_t*)px_map, w * h, false);
  tft.endWrite();
  lv_display_flush_ready(disp);
}

// ---- Driver touch super cepat & non-blocking untuk LVGL v9 ----
static bool is_touching = false;

static inline bool touch_read_fast(uint16_t* out_x, uint16_t* out_y) {
  // 1. Cek pressure (Z) XPT2046 secara instan (< 25us) tanpa delay
  uint16_t z = tft.getTouchRawZ();
  if (z < 300) {
    is_touching = false;
    return false;
  }

  // 2. Baca 2 sampel ADC mentah untuk stabilitas (tanpa delay blocking)
  uint16_t x1 = 0, y1 = 0;
  uint16_t x2 = 0, y2 = 0;
  tft.getTouchRaw(&x1, &y1);
  tft.getTouchRaw(&x2, &y2);

  // Filter anomali saat jari/stylus baru lepas
  if (abs((int)x1 - (int)x2) > 60 || abs((int)y1 - (int)y2) > 60) {
    if (tft.getTouchRawZ() < 300) {
      is_touching = false;
      return false;
    }
  }

  uint16_t rx = (x1 + x2) >> 1;
  uint16_t ry = (y1 + y2) >> 1;

  // 3. Konversi kalibrasi koordinat sesuai orientasi landscape (480x320)
  tft.convertRawXY(&rx, &ry);

  if (rx >= 480 || ry >= 320) {
    is_touching = false;
    return false;
  }

  *out_x = rx;
  *out_y = ry;
  is_touching = true;
  return true;
}

void touch_cb(lv_indev_t* indev, lv_indev_data_t* data) {
  uint16_t x = 0, y = 0;
  if (touch_read_fast(&x, &y)) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// ---- Tick provider untuk LVGL v9 ----
static uint32_t my_tick(void) {
  return millis();
}

// ---- Callbacks Aplikasi ----
void onSelectScript(AksaraType type) {
  Serial.printf("Memilih aksara: %s\n", type == AKSARA_JAWA ? "Aksara Jawa" : "Aksara Bali");
}

void onStartPractice(AksaraType type) {
  Serial.printf("Mulai latihan %s\n", type == AKSARA_JAWA ? "Aksara Jawa" : "Aksara Bali");
  // TODO: buka layar canvas menggambar/latihan aksara
}

void onSelectAksara(AksaraType type, int index) {
  static const char* nama[5] = { "Ha", "Na", "Ca", "Ra", "Ka" };
  if (index >= 0 && index < 5) {
    Serial.printf("Pilih aksara %s (%s)\n", nama[index], type == AKSARA_JAWA ? "Jawa" : "Bali");
  }
  // TODO: buka latihan khusus untuk aksara ini
}

void onAllAksara(AksaraType type) {
  Serial.printf("Buka daftar semua aksara (%s)\n", type == AKSARA_JAWA ? "Jawa" : "Bali");
  // TODO: buka daftar semua aksara
}

void onSettings() {
  Serial.println("Buka setelan");
  // TODO: buka layar setelan
}

AppCallbacks appCbs = {
  onSelectScript,
  onStartPractice,
  onSelectAksara,
  onAllAksara,
  onSettings
};

void setup() {
  Serial.begin(115200);
  audio_init();

  lv_init();
  lv_tick_set_cb(my_tick);

  tft.init();
  tft.setRotation(1);
  tft.setTouch(calData);

  disp = lv_display_create(480, 320);
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565_SWAPPED);
  lv_display_set_buffers(disp, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(disp, flush_cb);

  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touch_cb);
  lv_timer_set_period(lv_indev_get_read_timer(indev), 10); // Polling touch 100 Hz (10ms)

  ui_init(&appCbs);
}

void loop() {
  audio_update();
  uint32_t ms = lv_timer_handler();
  if (is_touching) {
    // Mode responsif maksimal: saat menggores, NOL delay untuk 60+ FPS zero-lag!
    yield();
  } else {
    if (ms > 5) ms = 5;
    if (ms == 0) ms = 1;
    delay(ms);
  }
}
