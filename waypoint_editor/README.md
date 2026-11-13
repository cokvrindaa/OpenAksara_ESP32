# OpenAksara - Waypoint Coordinate Editor

Alat visual berbasis browser dan script GUI untuk memplot koordinat goresan (*Stroke Waypoints*) aksara Jawa dan Bali untuk layar sentuh 3.5" (320 × 480 / orientasi lanskap 480 × 320) pada perangkat OpenAksara ESP32.

## Daftar Berkas
- **`index.html`** / **`waypoint_editor.html`**: Visual editor interaktif berbasis web (HTML5 Canvas + font aksara) untuk menggambar titik awal (hijau), lekukan lintasan (biru), dan titik akhir (merah).
- **`update_waypoint.bat`**: Shortcut Windows untuk membuka updater otomatis.
- **`update_waypoint.ps1`**: Script PowerShell dengan dialog GUI untuk menempelkan kode C++ `StrokeWaypoint NAMA[] = {...}` langsung dari clipboard dan menyuntikkannya ke `src/ui/ui_bali.cpp` atau `src/ui/ui_jawa.cpp`.
- **`backup/`**: Arsip versi cadangan editor sebelumnya.

## Cara Menggunakan
1. Buka berkas `index.html` di browser (atau klik ganda `update_waypoint.bat` lalu pilih tombol **Buka HTML Plotter**).
2. Pilih Aksara yang ingin dipetakan (Aksara Jawa / Bali).
3. Gambar dan sesuaikan titik-titik goresan di kanvas.
4. Klik **Salin Kode C++**.
5. Buka `update_waypoint.bat`, tempel kode C++ di kotak teks, lalu klik **🚀 OKE / UPDATE KODE C++**. File C++ firmware akan langsung diperbarui secara otomatis!
