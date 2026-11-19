#include "ui_settings.h"
#include "ui_home.h"
#include "firebase_sync.h"
#include "battery_monitor.h"
#include "secrets.h"

#include <Arduino.h>
#include <WiFi.h>
#include <stdio.h>

static lv_obj_t* scr_settings = nullptr;
static lv_obj_t* prev_screen = nullptr;
static lv_timer_t* settings_timer = nullptr;

// Widget Wi-Fi & Sesi Sementara
static lv_obj_t* pill_wifi_status = nullptr;
static lv_obj_t* lbl_wifi_status = nullptr;
static lv_obj_t* lbl_wifi_ssid = nullptr;
static lv_obj_t* lbl_wifi_ip = nullptr;
static lv_obj_t* lbl_wifi_rssi = nullptr;
static lv_obj_t* lbl_offline_queue = nullptr;
static lv_obj_t* lbl_offline_synced = nullptr;

// Widget Baterai & Sistem
static lv_obj_t* lbl_batt_pct = nullptr;
static lv_obj_t* lbl_batt_volt = nullptr;
static lv_obj_t* lbl_batt_pin = nullptr;
static lv_obj_t* bar_batt = nullptr;
static lv_obj_t* lbl_batt_state = nullptr;
static lv_obj_t* lbl_heap = nullptr;

static const char* rssi_quality_label(int rssi) {
  if (rssi >= -60) return "Sangat Kuat";
  if (rssi >= -72) return "Baik";
  if (rssi >= -82) return "Cukup";
  return "Lemah";
}

void ui_settings_refresh() {
  if (!scr_settings) return;

  // 1. Status Wi-Fi
  bool connected = firebase_sync_is_connected();
  if (connected) {
    lv_obj_set_style_bg_color(pill_wifi_status, lv_color_hex(0x0F3325), 0);
    lv_label_set_text(lbl_wifi_status, LV_SYMBOL_OK "  WIFI TERHUBUNG");
    lv_obj_set_style_text_color(lbl_wifi_status, lv_color_hex(0x34D399), 0);

    char ip_buf[24];
    firebase_sync_get_ip(ip_buf, sizeof(ip_buf));
    char line_ip[40];
    snprintf(line_ip, sizeof(line_ip), "IP     : %s", ip_buf);
    lv_label_set_text(lbl_wifi_ip, line_ip);

    int rssi = firebase_sync_get_rssi();
    char line_rssi[44];
    snprintf(line_rssi, sizeof(line_rssi), "Sinyal : %d dBm (%s)", rssi, rssi_quality_label(rssi));
    lv_label_set_text(lbl_wifi_rssi, line_rssi);
  } else {
    int st = firebase_sync_get_wifi_status_code();
    lv_obj_set_style_bg_color(pill_wifi_status, lv_color_hex(0x3B151E), 0);
    if (st == WL_NO_SSID_AVAIL) {
      lv_label_set_text(lbl_wifi_status, LV_SYMBOL_WARNING "  SSID TIDAK ADA");
    } else if (st == WL_CONNECT_FAILED) {
      lv_label_set_text(lbl_wifi_status, LV_SYMBOL_WARNING "  SANDI / KONEKSI GAGAL");
    } else {
      lv_label_set_text(lbl_wifi_status, LV_SYMBOL_CLOSE "  WIFI TERPUTUS / OFFLINE");
    }
    lv_obj_set_style_text_color(lbl_wifi_status, lv_color_hex(0xFB7185), 0);
    lv_label_set_text(lbl_wifi_ip, "IP     : Belum terhubung");
    lv_label_set_text(lbl_wifi_rssi, "Sinyal : Mode Offline");
  }

  char line_ssid[48];
  snprintf(line_ssid, sizeof(line_ssid), "SSID   : %s", firebase_sync_get_ssid());
  lv_label_set_text(lbl_wifi_ssid, line_ssid);

  // 2. Sesi Sementara (Offline Buffer)
  int pending = firebase_sync_get_offline_count();
  int cap = firebase_sync_get_offline_capacity();
  uint32_t synced = firebase_sync_get_offline_synced_total();

  char line_q[44];
  snprintf(line_q, sizeof(line_q), "%d / %d data menunggu", pending, cap);
  lv_label_set_text(lbl_offline_queue, line_q);
  lv_obj_set_style_text_color(
      lbl_offline_queue,
      pending > 0 ? lv_color_hex(0xFBBF24) : lv_color_hex(0x34D399),
      0
  );

  char line_s[44];
  snprintf(line_s, sizeof(line_s), "Terkirim dari sesi sementara: %u", (unsigned)synced);
  lv_label_set_text(lbl_offline_synced, line_s);

  // 3. Baterai (Voltase & Persentase via ADC1 GPIO34)
  BatteryStatus batt = battery_monitor_read();
  char v_buf[20];
  snprintf(v_buf, sizeof(v_buf), "%.2f V", batt.voltage);
  lv_label_set_text(lbl_batt_volt, v_buf);

  char pin_buf[24];
  snprintf(pin_buf, sizeof(pin_buf), "Pin ADC: %.2fV", batt.pin_voltage);
  lv_label_set_text(lbl_batt_pin, pin_buf);

  char pct_buf[12];
  if (batt.valid_battery) {
    snprintf(pct_buf, sizeof(pct_buf), "%d%%", batt.percent);
    lv_bar_set_value(bar_batt, batt.percent, LV_ANIM_OFF);

    lv_color_t c = lv_color_hex(0x34D399);
    if (batt.percent < 20) {
      c = lv_color_hex(0xFB7185);
    } else if (batt.percent < 50) {
      c = lv_color_hex(0xFBBF24);
    }
    lv_obj_set_style_text_color(lbl_batt_pct, c, 0);
    lv_obj_set_style_bg_color(bar_batt, c, LV_PART_INDICATOR);
  } else {
    snprintf(pct_buf, sizeof(pct_buf), "--%%");
    lv_bar_set_value(bar_batt, 0, LV_ANIM_OFF);
    lv_obj_set_style_text_color(lbl_batt_pct, C_SUB, 0);
    lv_obj_set_style_bg_color(bar_batt, C_MUTED, LV_PART_INDICATOR);
  }
  lv_label_set_text(lbl_batt_pct, pct_buf);

  char st_buf[44];
  snprintf(st_buf, sizeof(st_buf), "Status: %s", batt.state_label);
  lv_label_set_text(lbl_batt_state, st_buf);

  // 4. Info Memori ESP32
  char heap_buf[36];
  snprintf(heap_buf, sizeof(heap_buf), "Heap Bebas: %u KB", (unsigned)(ESP.getFreeHeap() / 1024));
  lv_label_set_text(lbl_heap, heap_buf);
}

static void on_settings_timer(lv_timer_t* t) {
  (void)t;
  if (scr_settings && lv_screen_active() == scr_settings) {
    ui_settings_refresh();
  }
}

static void on_back_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  if (settings_timer) lv_timer_pause(settings_timer);
  if (prev_screen && prev_screen != scr_settings) {
    lv_scr_load_anim(prev_screen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, false);
  } else {
    ui_home_show();
  }
}

static void on_refresh_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  ui_settings_refresh();
  ui_show_toast("Status Wi-Fi & baterai diperbarui");
}

static void on_reconnect_wifi_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  firebase_sync_trigger_reconnect();
  ui_show_toast("Menghubungkan ulang ke Wi-Fi...");
}

static void on_sync_offline_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  int pending = firebase_sync_get_offline_count();
  if (pending <= 0) {
    ui_show_toast("Sesi sementara sudah kosong / tersinkron");
    return;
  }
  if (!firebase_sync_is_connected()) {
    ui_show_toast("Wi-Fi belum terhubung! Data tetap aman di sesi sementara");
    return;
  }
  firebase_sync_trigger_flush();
  ui_show_toast("Mengirim data sesi sementara ke Cloud...");
}

static void build_settings_screen() {
  scr_settings = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_settings, C_BG, 0);
  lv_obj_set_style_bg_opa(scr_settings, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr_settings, LV_OBJ_FLAG_SCROLLABLE);

  // ===== HEADER =====
  lv_obj_t* btnBack = ui_make_card(scr_settings, 14, 10, 98, 34, C_CARD, C_CARD, 12);
  ui_set_pressed(btnBack, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btnBack, on_back_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* lblBack = ui_make_label(btnBack, LV_SYMBOL_LEFT " Kembali", &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(lblBack);

  lv_obj_t* title = ui_make_label(scr_settings, "Setelan Perangkat", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 122, 6);

  lv_obj_t* sub = ui_make_label(scr_settings, "Wi-Fi, Sesi & Baterai", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(sub, 122, 29);

  lv_obj_t* btnRef = ui_make_card(scr_settings, 386, 10, 80, 34, C_CARD, C_CARD, 12);
  ui_set_pressed(btnRef, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btnRef, on_refresh_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* lblRef = ui_make_label(btnRef, LV_SYMBOL_REFRESH " Cek", &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(lblRef);

  // ===== KARTU KIRI: STATUS WIFI & SESI SEMENTARA =====
  lv_obj_t* cardWifi = ui_make_card(scr_settings, 14, 54, 224, 252, C_CARD, C_CARD, 18);

  lv_obj_t* wTitle = ui_make_label(cardWifi, LV_SYMBOL_WIFI "  Koneksi Wi-Fi", &lv_font_montserrat_16, C_TEXT);
  lv_obj_set_pos(wTitle, 14, 10);

  pill_wifi_status = ui_make_card(cardWifi, 12, 34, 200, 26, lv_color_hex(0x3B151E), lv_color_hex(0x3B151E), 8);
  lv_obj_clear_flag(pill_wifi_status, LV_OBJ_FLAG_CLICKABLE);
  lbl_wifi_status = ui_make_label(pill_wifi_status, "MEMERIKSA...", &lv_font_montserrat_14, lv_color_hex(0xFB7185));
  lv_obj_center(lbl_wifi_status);

  lbl_wifi_ssid = ui_make_label(cardWifi, "SSID   : -", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(lbl_wifi_ssid, 14, 66);
  lv_obj_set_width(lbl_wifi_ssid, 196);
  lv_label_set_long_mode(lbl_wifi_ssid, LV_LABEL_LONG_CLIP);

  lbl_wifi_ip = ui_make_label(cardWifi, "IP     : -", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(lbl_wifi_ip, 14, 86);
  lv_obj_set_width(lbl_wifi_ip, 196);
  lv_label_set_long_mode(lbl_wifi_ip, LV_LABEL_LONG_CLIP);

  lbl_wifi_rssi = ui_make_label(cardWifi, "Sinyal : -", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(lbl_wifi_rssi, 14, 106);
  lv_obj_set_width(lbl_wifi_rssi, 196);
  lv_label_set_long_mode(lbl_wifi_rssi, LV_LABEL_LONG_CLIP);

  // Kotak Sesi Sementara (Offline Buffer)
  lv_obj_t* boxQueue = ui_make_card(cardWifi, 12, 130, 200, 68, C_CARD_PR, C_CARD_PR, 12);
  lv_obj_clear_flag(boxQueue, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t* qHdr = ui_make_label(boxQueue, "Sesi Sementara:", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(qHdr, 10, 6);

  lbl_offline_queue = ui_make_label(boxQueue, "0 / 10 menunggu", &lv_font_montserrat_14, lv_color_hex(0x34D399));
  lv_obj_set_pos(lbl_offline_queue, 10, 26);

  lbl_offline_synced = ui_make_label(boxQueue, "Terkirim: 0", &lv_font_montserrat_14, C_MUTED);
  lv_obj_set_pos(lbl_offline_synced, 10, 46);

  // Tombol Aksi Wi-Fi & Sync
  lv_obj_t* btnReconn = ui_make_card(cardWifi, 12, 206, 96, 36, C_JAWA_1, C_JAWA_2, 10);
  ui_set_pressed(btnReconn, C_JAWA_2, C_JAWA_2);
  lv_obj_add_event_cb(btnReconn, on_reconnect_wifi_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* lblReconn = ui_make_label(btnReconn, LV_SYMBOL_REFRESH " Wi-Fi", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(lblReconn);

  lv_obj_t* btnSync = ui_make_card(cardWifi, 116, 206, 96, 36, C_BALI_1, C_BALI_2, 10);
  ui_set_pressed(btnSync, C_BALI_2, C_BALI_2);
  lv_obj_add_event_cb(btnSync, on_sync_offline_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* lblSync = ui_make_label(btnSync, LV_SYMBOL_UPLOAD " Kirim", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(lblSync);

  // ===== KARTU KANAN: BATERAI & SISTEM =====
  lv_obj_t* cardBatt = ui_make_card(scr_settings, 246, 54, 220, 252, C_CARD, C_CARD, 18);

  lv_obj_t* bTitle = ui_make_label(cardBatt, LV_SYMBOL_BATTERY_FULL "  Baterai & Daya", &lv_font_montserrat_16, C_TEXT);
  lv_obj_set_pos(bTitle, 14, 10);

  // Kotak Pembacaan Persentase + Voltase
  lv_obj_t* boxBatt = ui_make_card(cardBatt, 12, 36, 196, 102, C_CARD_PR, C_CARD_PR, 14);
  lv_obj_clear_flag(boxBatt, LV_OBJ_FLAG_CLICKABLE);

  lbl_batt_pct = ui_make_label(boxBatt, "--%", &lv_font_montserrat_28, lv_color_hex(0x34D399));
  lv_obj_set_pos(lbl_batt_pct, 12, 10);

  lbl_batt_volt = ui_make_label(boxBatt, "0.00 V", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(lbl_batt_volt, 104, 12);

  lbl_batt_pin = ui_make_label(boxBatt, "ADC: 0.00V", &lv_font_montserrat_14, C_MUTED);
  lv_obj_set_pos(lbl_batt_pin, 104, 36);

  bar_batt = lv_bar_create(boxBatt);
  lv_obj_set_pos(bar_batt, 12, 60);
  lv_obj_set_size(bar_batt, 172, 12);
  lv_bar_set_range(bar_batt, 0, 100);
  lv_bar_set_value(bar_batt, 0, LV_ANIM_OFF);
  lv_obj_set_style_radius(bar_batt, 6, 0);
  lv_obj_set_style_radius(bar_batt, 6, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(bar_batt, lv_color_hex(0x0E121B), 0);
  lv_obj_set_style_bg_opa(bar_batt, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(bar_batt, lv_color_hex(0x34D399), LV_PART_INDICATOR);

  lbl_batt_state = ui_make_label(boxBatt, "Status: Cek...", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(lbl_batt_state, 12, 78);
  lv_obj_set_width(lbl_batt_state, 174);
  lv_label_set_long_mode(lbl_batt_state, LV_LABEL_LONG_CLIP);

  // Kotak Info Pin Sensor Baterai
  lv_obj_t* boxPin = ui_make_card(cardBatt, 12, 144, 196, 54, lv_color_hex(0x11151E), lv_color_hex(0x11151E), 10);
  lv_obj_clear_flag(boxPin, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* lblHint = ui_make_label(
      boxPin,
      "Sensor GPIO34 (ADC1)\nPembagi 100k + 100k",
      &lv_font_montserrat_14,
      C_SUB
  );
  lv_obj_set_pos(lblHint, 10, 8);
  lv_obj_set_width(lblHint, 176);

  // Footer Info Device
  char dev_buf[40];
  snprintf(dev_buf, sizeof(dev_buf), "ID: %s", DEVICE_ID);
  lv_obj_t* lblDev = ui_make_label(cardBatt, dev_buf, &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(lblDev, 14, 206);

  lbl_heap = ui_make_label(cardBatt, "Heap: -- KB", &lv_font_montserrat_14, C_MUTED);
  lv_obj_set_pos(lbl_heap, 14, 226);
}

void ui_settings_init() {
  build_settings_screen();
  settings_timer = lv_timer_create(on_settings_timer, 1500, NULL);
  if (settings_timer) {
    lv_timer_pause(settings_timer);
  }
}

void ui_settings_show() {
  if (!scr_settings) return;
  lv_obj_t* active = lv_screen_active();
  if (active && active != scr_settings) {
    prev_screen = active;
  }
  ui_settings_refresh();
  if (settings_timer) {
    lv_timer_resume(settings_timer);
  }
  if (active != scr_settings) {
    lv_scr_load_anim(scr_settings, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
  }
}
