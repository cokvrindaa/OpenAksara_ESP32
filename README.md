# OpenAksara ESP32 - Sabak Digital Interaktif Aksara Nusantara

OpenAksara adalah perangkat sabak digital edukasi berbasis **ESP32**, layar sentuh **3.5" TFT LCD ILI9488**, audio **DFPlayer Mini**, dan sensor goresan untuk pembelajaran interaktif **Aksara Jawa** dan **Aksara Bali**, terhubung secara real-time ke **Cloud Firestore & Dashboard Web EduCloud**.

---

## 📁 Struktur Direktori Repositori (Terorganisir)

```
OpenAksara_ESP32/
├── 📁 3d_viewer/          # Desain Casing 3D Enclosure & Interactive Three.js 3D Viewer
│   ├── 3d_viewer.html    # Viewer interaktif di browser (Bongkar/Pasang, rotasi 360°)
│   ├── index.html        # Alias pembuka viewer 3D
│   ├── models/           # Berkas STL siap 3D print (casing_atas, tutup_bawah, assembly)
│   ├── docs/             # Dimensi modul layar LCD 3.5" MSP3520 / ILI9488
│   ├── casing_preview.png# Gambar preview teknis isometrik & tampak samping
│   └── *.py              # Script generator STL parametrik (Manifold3D) & render
│
├── 📁 waypoint_editor/   # Editor Visual Koordinat Lintasan Aksara (Stroke Waypoints)
│   ├── waypoint_editor.html # Kanvas interaktif plotter titik hijau (mulai) & merah (akhir)
│   ├── index.html        # Alias visual plotter
│   ├── update_waypoint.bat  # 1-klik buka GUI injector C++
│   ├── update_waypoint.ps1  # Script PowerShell auto-update ke src/ui/ui_*.cpp
│   └── backup/           # Arsip cadangan editor
│
├── 📁 web/               # Platform EduCloud & Portal Guru / Orang Tua Siswa
│   ├── index.html        # Dashboard modern pemantauan kelas, nilai, XP, & buku penghubung
│   ├── server.js         # Backend lokal Node.js (REST API, SSE, proxy, & router /3d & /waypoint)
│   ├── start_server.bat  # 1-klik jalankan server port 3000
│   ├── firebase-config.js# Driver sinkronisasi Firestore & format waktu real-time
│   ├── assets/           # Maskot AksaraBuddy & ilustrasi
│   ├── fonts/            # NotoSansBalinese & NotoSansJavanese
│   ├── screenshots/      # Cuplikan layar antarmuka dashboard
│   └── legacy/           # Arsip kode versi v1
│
├── 📁 src/               # Kode Sumber Firmware ESP32 (C++ / Arduino / FreeRTOS)
│   ├── core/             # Background task FreeRTOS, Firebase HTTP TLS, WiFi, telemetri
│   ├── ui/               # Tampilan LVGL 9, logika panduan menulis Aksara Jawa & Bali
│   └── main.cpp          # Setup hardware & loop utama
│
├── 📁 include/           # Header C++ & secrets.h (kredensial WiFi & Firebase)
├── 📁 asset/             # Aset C-array grafis bitmap avatar & font embedded
├── 📁 lib/               # Pustaka komponen pihak ketiga
├── platformio.ini        # Konfigurasi build PlatformIO (Tuned 240MHz, 80MHz SPI Flash)
└── update_waypoint.bat   # Shortcut cepat auto-updater waypoint dari root
```

---

## 🚀 Panduan Ringkas Penggunaan

### 1. Menjalankan Dashboard Web & Monitoring Siswa
- Buka folder `web/` lalu klik ganda **`start_server.bat`** (atau jalankan `node web/server.js`).
- Buka peramban di `http://localhost:3000`.
- Rute cepat ekosistem:
  - Dashboard Guru & Orang Tua: `http://localhost:3000/`
  - 3D Casing Viewer: `http://localhost:3000/3d`
  - Waypoint Plotter Editor: `http://localhost:3000/waypoint`

### 2. Memetakan Titik Goresan Aksara Baru (Waypoint Editor)
- Buka `waypoint_editor/index.html` (atau klik menu **Editor Waypoint** di dashboard).
- Sesuaikan titik goresan huruf aksara, lalu salin kode C++.
- Buka `waypoint_editor/update_waypoint.bat`, tempel kodenya, dan klik **🚀 OKE / UPDATE KODE C++**. File `src/ui/ui_bali.cpp` atau `src/ui/ui_jawa.cpp` akan terisi otomatis.

### 3. Memeriksa / Mencetak Casing 3D (3D Enclosure)
- Buka `3d_viewer/index.html` di browser untuk melihat preview 3D interaktif.
- File STL siap cetak tersedia di `3d_viewer/models/`:
  - `casing_atas.stl`
  - `tutup_bawah.stl` (desain 100% flat mulus dengan countersunk baut M3)
- Dimensi cetak: **180 mm × 100 mm × 50 mm**.

### 4. Mengompilasi & Flash Firmware ke ESP32
Gunakan PlatformIO di VS Code atau terminal:
```bash
pio run -e esp32dev --target upload
```
