#include "battery_monitor.h"

namespace {
struct VoltPctPoint {
  float v;
  int pct;
};

// Kurva pengosongan baterai 1S Li-Ion / LiPo
constexpr VoltPctPoint kLiIonCurve[] = {
  {4.20f, 100},
  {4.10f, 92},
  {4.00f, 82},
  {3.90f, 70},
  {3.80f, 55},
  {3.70f, 38},
  {3.60f, 22},
  {3.50f, 12},
  {3.40f, 5},
  {3.20f, 0}
};

int voltage_to_percent(float v) {
  if (v >= kLiIonCurve[0].v) return 100;
  constexpr size_t n = sizeof(kLiIonCurve) / sizeof(kLiIonCurve[0]);
  if (v <= kLiIonCurve[n - 1].v) return 0;

  for (size_t i = 0; i + 1 < n; ++i) {
    const float hi_v = kLiIonCurve[i].v;
    const float lo_v = kLiIonCurve[i + 1].v;
    if (v >= lo_v && v <= hi_v) {
      const float ratio = (v - lo_v) / (hi_v - lo_v);
      const float pct = kLiIonCurve[i + 1].pct + ratio * (kLiIonCurve[i].pct - kLiIonCurve[i + 1].pct);
      int rounded = (int)(pct + 0.5f);
      if (rounded < 0) rounded = 0;
      if (rounded > 100) rounded = 100;
      return rounded;
    }
  }
  return 0;
}
} // namespace

void battery_monitor_init() {
  pinMode(BATTERY_ADC_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);
}

BatteryStatus battery_monitor_read() {
  constexpr int kSamples = 16;
  uint32_t sum_mv = 0;
  for (int i = 0; i < kSamples; ++i) {
    sum_mv += analogReadMilliVolts(BATTERY_ADC_PIN);
  }

  BatteryStatus st{};
  st.pin_voltage = (sum_mv / (float)kSamples) / 1000.0f;
  st.voltage = st.pin_voltage * BATTERY_DIVIDER_RATIO;
  st.valid_battery = (st.voltage >= 2.50f && st.voltage <= 5.40f);

  if (!st.valid_battery) {
    st.percent = 0;
    st.state_label = (st.voltage < 0.30f) ? "Sensor Tidak Terpasang" : "Pin GPIO34 Mengambang";
    return st;
  }

  st.percent = voltage_to_percent(st.voltage);
  if (st.voltage >= 4.28f) {
    st.state_label = "Penuh / Mengisi Daya";
  } else if (st.percent >= 75) {
    st.state_label = "Daya Penuh";
  } else if (st.percent >= 40) {
    st.state_label = "Normal";
  } else if (st.percent >= 20) {
    st.state_label = "Mulai Lemah";
  } else {
    st.state_label = "Baterai Kritis!";
  }
  return st;
}
