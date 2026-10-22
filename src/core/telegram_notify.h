/**
 * OpenAksara ESP32 - Telegram Notification System
 * Modul pengiriman notifikasi otomatis hasil belajar menulis aksara ke Telegram Guru & Orang Tua
 * Mendukung Direct HTTPS langsung ke Telegram Bot API tanpa perlu server PC!
 */

#ifndef TELEGRAM_NOTIFY_H
#define TELEGRAM_NOTIFY_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ============================================================================
// ⚠️ MASUKKAN TOKEN BOT & CHAT ID ANDA DI SINI
// 1. Bot Token didapat dari chat @BotFather di Telegram (contoh: 7123456789:AAHq...)
// 2. Chat ID didapat dari chat @userinfobot atau ID Grup (contoh: 123456789 atau -100xxxxxxxxxx)
// ============================================================================
#define TELEGRAM_BOT_TOKEN   "MASUKKAN_BOT_TOKEN_DISINI"
#define TELEGRAM_CHAT_ID     "MASUKKAN_CHAT_ID_DISINI"

// (Opsional) URL server EduCloud lokal jika menggunakan server PC
#define EDUCLOUD_SERVER_URL  "http://192.168.1.100:3000/api/telemetry"

/**
 * @brief Kirim pesan teks / HTML langsung ke Telegram Bot via HTTPS
 * @param html_message Pesan dengan format HTML yang didukung Telegram
 * @return true jika berhasil terkirim ke Telegram
 */
inline bool telegram_send_raw(const char* html_message) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("[Telegram Direct] Error: WiFi belum terhubung!"));
    return false;
  }

  if (strcmp(TELEGRAM_BOT_TOKEN, "MASUKKAN_BOT_TOKEN_DISINI") == 0 ||
      strcmp(TELEGRAM_CHAT_ID, "MASUKKAN_CHAT_ID_DISINI") == 0) {
    Serial.println(F("[Telegram Direct] ⚠️ Error: TELEGRAM_BOT_TOKEN atau TELEGRAM_CHAT_ID belum diisi di telegram_notify.h!"));
    return false;
  }

  Serial.println(F("[Telegram Direct] Menghubungkan ke https://api.telegram.org ..."));

  WiFiClientSecure client;
  client.setInsecure(); // Bypass verifikasi sertifikat SSL agar hemat RAM dan cepat di ESP32

  HTTPClient https;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) + "/sendMessage";

  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");

    // Escape karakter newline dan kutip dua untuk JSON
    String escapedMsg = "";
    for (int i = 0; html_message[i] != '\0'; i++) {
      if (html_message[i] == '"') escapedMsg += "\\\"";
      else if (html_message[i] == '\n') escapedMsg += "\\n";
      else if (html_message[i] == '\r') continue;
      else escapedMsg += html_message[i];
    }

    String jsonBody = "{\"chat_id\":\"" + String(TELEGRAM_CHAT_ID) + "\",\"text\":\"" + escapedMsg + "\",\"parse_mode\":\"HTML\"}";

    int httpCode = https.POST(jsonBody);

    if (httpCode == HTTP_CODE_OK || httpCode == 200) {
      String response = https.getString();
      Serial.printf("[Telegram Direct] ✅ SUKSES TERKIRIM KE TELEGRAM! Response: %s\n", response.c_str());
      https.end();
      return true;
    } else {
      Serial.printf("[Telegram Direct] ❌ Gagal terkirim! HTTP Code: %d, Response: %s\n",
                    httpCode, https.getString().c_str());
      https.end();
      return false;
    }
  } else {
    Serial.println(F("[Telegram Direct] ❌ Gagal memulai koneksi HTTPS!"));
    return false;
  }
}

/**
 * @brief Kirim laporan nilai siswa otomatis ke Telegram Bot
 */
inline bool telegram_send_score_report(
    const char* student_name,
    const char* nisn,
    const char* script_type,
    const char* letter_name,
    int score,
    float accuracy,
    int duration_sec
) {
  char msg[512];
  const char* scriptLabel = (strcmp(script_type, "bali") == 0) ? "Aksara Bali" : "Aksara Jawa";
  const char* statusLabel = (score >= 75) ? "TUNTAS BELAJAR ✅" : "PERLU REMEDIAL ⚠️";

  snprintf(msg, sizeof(msg),
    "📱 <b>OpenAksara EduCloud • Laporan Belajar</b>\n"
    "━━━━━━━━━━━━━━━━━━━━━━━\n"
    "👤 <b>Nama:</b> %s\n"
    "🆔 <b>NISN:</b> %s\n"
    "🏷️ <b>Naskah:</b> %s\n"
    "✍️ <b>Aksara:</b> %s\n\n"
    "🏆 <b>Hasil Evaluasi:</b>\n"
    "• Skor Menulis: <b>%d / 100</b>\n"
    "• Akurasi Goresan: <b>%.1f%%</b>\n"
    "• Waktu Pengerjaan: <b>%d detik</b>\n"
    "• Status: <b>%s</b>\n"
    "━━━━━━━━━━━━━━━━━━━━━━━\n"
    "🤖 <i>Dikirim otomatis via Tablet OpenAksara ESP32</i>",
    student_name, nisn, scriptLabel, letter_name, score, accuracy, duration_sec, statusLabel
  );

  return telegram_send_raw(msg);
}

/**
 * @brief Kirim pesan darurat remedial jika siswa kesulitan menulis aksara tertentu
 */
inline bool telegram_send_remedial_alert(const char* student_name, const char* letter_name, int score) {
  char msg[384];
  snprintf(msg, sizeof(msg),
    "⚠️ <b>PERINGATAN REMEDIAL GOJESAN</b>\n"
    "━━━━━━━━━━━━━━━━━━━━━━━\n"
    "👤 <b>Siswa:</b> %s\n"
    "✍️ <b>Aksara:</b> %s\n"
    "📉 <b>Skor:</b> %d / 100 (Di bawah KKM 75)\n"
    "━━━━━━━━━━━━━━━━━━━━━━━\n"
    "📌 <i>Saran: Latihan ulang menggunakan panduan garis tebal pada tablet.</i>\n"
    "🤖 <i>OpenAksara ESP32 Alert System</i>",
    student_name, letter_name, score
  );

  return telegram_send_raw(msg);
}

#endif // TELEGRAM_NOTIFY_H
