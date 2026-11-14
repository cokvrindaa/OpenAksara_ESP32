#pragma once

#include <Arduino.h>

/**
 * OpenAksara ESP32 - Firebase Cloud Firestore Sync Engine
 * 
 * Menghubungkan Hardware Sabak Cerdas ESP32 ke Google Cloud Firestore:
 * 1. Kirim hasil goresan stylus (precision, score, status) secara realtime ke Firestore
 * 2. Kirim capaian materi belajar & XP ke koleksi 'assessments'
 * 3. Kirim status telemetri hardware (voltase 4.98V, port, status online)
 * 4. Terima pesan/feedback dari guru (Pak Guru Yoga / Bu Guru Desak)
 * 5. Fallback otomatis ke USB Serial (115200) jika offline/tanpa Wi-Fi
 */

// Inisialisasi Wi-Fi & Koneksi Firebase
void firebase_sync_init();

// Update loop berkala (heartbeat telemetri & feedback polling non-blocking)
void firebase_sync_update();

// Cek apakah Wi-Fi & Firebase siap
bool firebase_sync_is_connected();

// Informasi status Wi-Fi & Antrian Sesi Sementara (Offline Buffer)
int firebase_sync_get_wifi_status_code();
const char* firebase_sync_get_ssid();
void firebase_sync_get_ip(char* buf, size_t len);
int firebase_sync_get_rssi();
int firebase_sync_get_offline_count();
int firebase_sync_get_offline_capacity();
uint32_t firebase_sync_get_offline_synced_total();
void firebase_sync_trigger_reconnect();
void firebase_sync_trigger_flush();

// Kirim hasil latihan goresan siswa (Koleksi 'submissions')
bool firebase_sync_send_submission(
    const char* student_name,
    const char* student_class,
    const char* script_type,
    const char* aksara_name,
    int precision,
    int score,
    const char* status_label
);

// Kirim capaian materi & XP (Koleksi 'assessments')
bool firebase_sync_send_assessment(
    const char* student_name,
    const char* student_class,
    const char* materi_title,
    const char* script_type,
    int final_score,
    const char* grade,
    int xp_earned,
    const char* feedback_note
);

// Kirim telemetri status hardware ESP32 (Dokumen 'telemetry/esp32_status')
bool firebase_sync_send_telemetry(float voltage, int tolerance_px, int volume);
