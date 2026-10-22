# 🌐 OpenAksara EduCloud - Dashboard Pemantauan Nilai Siswa IoT

Sistem web modern untuk guru, pengajar, dan instruktur guna memantau nilai, akurasi goresan stylus, kecepatan menulis, dan perkembangan belajar siswa pada pembelajaran **Aksara Bali** dan **Aksara Jawa** menggunakan perangkat tablet cerdas **OpenAksara ESP32**.

---

## ✨ Fitur Unggulan Dashboard

1. **Dashboard KPI & Statistik Real-Time:**
   - Rata-rata nilai kelas dengan komparasi terhadap KKM (75.0).
   - Rata-rata akurasi goresan terhadap checkpoints radius toleransi 18px.
   - Rasio ketuntasan belajar (Siswa Tuntas vs Remedial).
   - Status perangkat ESP32 online di laboratorium/kelas.

2. **Visual Analytics & Chart Interaktif:**
   - **Perbandingan Aksara Bali vs Jawa:** Grafik batang capaian nilai per modul (Konsonan Dasar, Sandhangan/Pengangge Suara, Pasangan, Kuis Teori).
   - **Distribusi Predikat Nilai:** Donut chart proporsi nilai A (90-100), B (80-89), C (75-79), dan Remedial (<75).
   - **Heatmap Kesulitan Aksara:** Peringkat huruf yang paling sering salah atau memiliki deviasi lintasan terbesar (misal: Aksara Bali *Ca*, Aksara Jawa *Taling Tarung*).

3. **Buku Nilai Interaktif (Gradebook):**
   - Pencarian instan (Nama, NISN, atau ID Perangkat).
   - Filter cepat berdasarkan Kelas (4A, 4B, 5A) dan Status (Semua, Tuntas, Remedial).
   - Pengurutan dinamis (sorting) per kolom nilai, akurasi, atau waktu pengerjaan.
   - Ekspor rekapitulasi nilai ke format **CSV / Excel**.

4. **Rapor Detail & Radar Kompetensi Siswa:**
   - Profil lengkap siswa dan perangkat ESP32 yang digunakan.
   - **Radar Chart 5 Dimensi:** Akurasi goresan, kecepatan menulis, capaian Aksara Bali, capaian Aksara Jawa, dan kuis teori.
   - Riwayat log 5 sesi pengerjaan terakhir dari hardware ESP32.
   - Kolom catatan evaluasi guru yang tersimpan secara persisten.
   - Tombol **Cetak Rapor** (Print-ready / save as PDF).

5. **Live ESP32 Activity Stream & Simulator:**
   - Bilah aktivitas live yang menampilkan siswa yang sedang aktif menulis di kelas.
   - **Fitur Simulator ESP32:** Memungkinkan guru menguji kirim data telemetri langsung dari browser tanpa harus menyalakan ESP32 terlebih dahulu (dilengkapi chime audio sintetis dan konfeti perayaan jika nilai ≥ 95).

6. **Layanan Notifikasi Otomatis Telegram Bot (Dummy & Live):**
   - **Mode Dummy / Proposal:** Mensimulasikan notifikasi Telegram lengkap dengan bubble chat realistis, suara double-chime Telegram khas, dan tombol inline tanpa memerlukan token Telegram nyata.
   - **Mode Real API:** Mendukung integrasi riil ke Telegram Bot API (`https://api.telegram.org`) untuk meneruskan laporan belajar langsung ke grup Telegram wali murid atau orang tua.
   - **3 Jenis Template:**
     1. *Laporan Hasil Belajar Siswa* (Skor nilai, akurasi goresan, waktu pengerjaan, status tuntas/remedial).
     2. *Rekapitulasi Harian Kelas* (Rata-rata nilai kelas, rasio kelulusan KKM, aksara yang paling banyak salah).
     3. *Peringatan Remedial* (Alert otomatis jika siswa membutuhkan bimbingan khusus).

---

## 🚀 Cara Menjalankan

### Pilihan A: Mode Server IoT Lengkap (Rekomendasi untuk Uji Coba & Integrasi ESP32)
1. Pastikan komputer Anda terpasang [Node.js](https://nodejs.org/).
2. Cukup klik ganda (double-click) file **`start_server.bat`**, atau buka terminal di folder `web` dan jalankan:
   ```bash
   node server.js
   ```
3. Browser otomatis terbuka di: **`http://localhost:3000`**
4. Server lokal langsung siap menerima data telemetri dari ESP32 melalui endpoint `POST /api/telemetry`.

### Pilihan B: Mode Standalone Offline (Tanpa Server / Langsung di Browser)
1. Cukup klik ganda file **`index.html`** di browser Chrome, Edge, atau Firefox.
2. Dashboard langsung berfungsi penuh menggunakan penyimpanan data browser (**LocalStorage**) dengan data awal 12 siswa SDN 1 Tuban.

---

## 📡 Panduan Integrasi ESP32 ke Dashboard

Untuk mengirimkan hasil evaluasi goresan atau kuis dari ESP32 ke dashboard secara real-time via WiFi:

### Endpoint:
- **URL:** `http://<IP_KOMPUTER_GURU>:3000/api/telemetry`
- **Method:** `POST`
- **Header:** `Content-Type: application/json`

### Contoh Payload JSON:
```json
{
  "device_id": "ESP32-BALI-01",
  "nisn": "20260401",
  "script": "bali",
  "letter": "Ca",
  "score": 96,
  "accuracy": 95.8,
  "duration_sec": 12,
  "battery": 87,
  "rssi": -58
}
```

### Contoh Potongan Kode C++ (Arduino/PlatformIO):
```cpp
#include <WiFi.h>
#include <HTTPClient.h>

void send_score_telemetry(const char* script, const char* letter, int score, float accuracy, int duration_sec) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin("http://192.168.1.100:3000/api/telemetry"); // Ganti dengan IP komputer guru
  http.addHeader("Content-Type", "application/json");

  char jsonPayload[256];
  snprintf(jsonPayload, sizeof(jsonPayload),
    "{\"device_id\":\"ESP32-BALI-01\",\"nisn\":\"20260401\",\"script\":\"%s\",\"letter\":\"%s\",\"score\":%d,\"accuracy\":%.1f,\"duration_sec\":%d}",
    script, letter, score, accuracy, duration_sec
  );

  int httpCode = http.POST(jsonPayload);
  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("Telemetri terkirim! Status: %d, Response: %s\n", httpCode, response.c_str());
  } else {
    Serial.printf("Gagal mengirim telemetri, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}
```

---

## 📱 Panduan API Notifikasi Telegram

### Endpoint Kirim Pesan Telegram:
- **URL:** `http://<IP_KOMPUTER_GURU>:3000/api/telegram/send`
- **Method:** `POST`
- **Header:** `Content-Type: application/json`

### Contoh Payload Notifikasi Nilai Siswa:
```json
{
  "type": "student_score",
  "student_id": "STU-001"
}
```

### Contoh Payload Rekapitulasi Kelas Harian:
```json
{
  "type": "class_summary"
}
```

### Contoh Payload Alert Remedial:
```json
{
  "type": "remedial_alert"
}
```

> **File Driver Firmware ESP32:** Telah disediakan di [`src/telegram_notify.h`](file:///c:/Users/cokvr/Documents/OpenAksara_ESP32/src/telegram_notify.h) siap digunakan pada program mikrokontroler.

---

## 📂 Struktur Berkas Folder `web/`

```
web/
├── fonts/
│   ├── NotoSansBalinese-Regular.ttf   # Font resmi Aksara Bali
│   └── NotoSansJavanese-Regular.ttf   # Font resmi Aksara Jawa
├── index.html                        # Tampilan dashboard modern (Tailwind, Lucide, Chart.js)
├── app.js                            # Logika interaktif, SSE, filter, chart, simulator & Telegram
├── server.js                         # Backend server ringan Node.js (REST API, SSE & Telegram Relay)
├── start_server.bat                  # Script 1-klik untuk menjalankan server di Windows
└── README.md                         # Dokumentasi lengkap
```

