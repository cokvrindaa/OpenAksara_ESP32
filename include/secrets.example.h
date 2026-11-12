#pragma once

/**
 * OpenAksara ESP32 - Hardware Secrets Template
 * 
 * CARA PAKAI:
 * 1. Salin file ini dan simpan dengan nama "secrets.h" di folder include/
 * 2. Masukkan SSID Wi-Fi, Password, dan Kredensial Firebase.
 * 3. File "secrets.h" sudah otomatis diabaikan oleh .gitignore!
 */

// Konfigurasi Wi-Fi Sekolah / Hotspot
#define SECRET_WIFI_SSID "WIFI_SEKOLAH_ATAU_HOTSPOT"
#define SECRET_WIFI_PASS "PASSWORD_WIFI"

// Konfigurasi Firebase Project
#define SECRET_FIREBASE_API_KEY "AIzaSy..."
#define SECRET_FIREBASE_PROJECT_ID "aksarabuddy"
#define SECRET_FIREBASE_USER_EMAIL "guru@aksarabuddy.id"
#define SECRET_FIREBASE_USER_PASSWORD "password123"
