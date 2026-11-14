#pragma once

#include <Arduino.h>

/**
 * OpenAksara ESP32 - Monitor Tegangan & Persentase Baterai (ADC1)
 *
 * Menggunakan GPIO 34 (ADC1_CH6) karena aman dibaca bersamaan saat Wi-Fi aktif
 * (ADC2 tidak bisa dipakai bersamaan dengan Wi-Fi di ESP32).
 *
 * Rangkaian standar pembagi tegangan 1:1 untuk baterai 1S Li-Ion/LiPo (3.0V - 4.2V):
 *   VBAT+ ---> [100k Ohm] ---> GPIO 34 ---> [100k Ohm] ---> GND
 */
#ifndef BATTERY_ADC_PIN
#define BATTERY_ADC_PIN 34
#endif

#ifndef BATTERY_DIVIDER_RATIO
#define BATTERY_DIVIDER_RATIO 2.0f
#endif

struct BatteryStatus {
  float voltage;           // Voltase baterai terhitung (V)
  float pin_voltage;       // Tegangan mentah di pin ADC GPIO34 (V)
  int percent;             // Persentase 0 - 100 (%)
  bool valid_battery;      // true jika pembacaan berada pada rentang baterai/daya wajar (>= 2.50V)
  const char* state_label; // Status ringkas kondisi daya/baterai
};

void battery_monitor_init();
BatteryStatus battery_monitor_read();
