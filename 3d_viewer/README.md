# OpenAksara - 3D Enclosure & Interactive Viewer

Folder ini berisi model 3D CAD, berkas STL siap cetak (3D print), preview visual, dan viewer 3D interaktif untuk casing perangkat sabak digital **OpenAksara ESP32**.

## Dimensi Fisik Casing
- **Panjang (X)**: 180 mm (18.0 cm)
- **Lebar (Y)**: 100 mm (10.0 cm)
- **Tinggi / Kedalaman (Z)**: 50 mm (5.0 cm)
- **Desain Pelat Bawah**: Tutup Bawah 100% Rata Mulus (*Flat*), tanpa bibir timbul luar, dilengkapi lubang baut countersunk M3 dan grill speaker radial.
- **Port & Fitur**:
  - Cutout Layar TFT LCD 3.5" ILI9488 + ceruk kaca bezel + notch kabel FPC sentuh
  - Lubang USB Type-C horizontal pas di tengah dinding samping kiri (Z = 25.0 mm)
  - Saklar geser mini di dinding samping kanan (Z = 25.0 mm)
  - Lubang grill mikrofon INMP441 di atas layar

## Struktur Berkas
- **`index.html`** / **`3d_viewer.html`**: Viewer 3D interaktif berbasis Three.js di browser. Mendukung rotasi 360°, zoom, pan, serta fitur **Bongkar / Pasang (Exploded View)** antara casing atas dan tutup bawah.
- **`models/`**:
  - `casing_atas.stl`: Casing bagian atas siap cetak 3D.
  - `tutup_bawah.stl`: Tutup pelat bawah siap cetak 3D.
  - `casing_lengkap_assembly.stl`: Rakitan utuh (assembly) untuk simulasi CAD.
- **`docs/`**: Datasheet dan ukuran dimensi modul LCD MSP3520 / ILI9488.
- **`casing_preview.png`**: Gambar render tampak 3D isometrik dan proyeksi tampak samping/bawah berdimensi teknis.
- **`generate_enclosure.py`**: Script Python (Manifold3D + Trimesh) untuk meng-generate ulang berkas STL parametrik.
- **`generate_viewer.py`**: Script Python untuk mengekstrak geometri STL menjadi `index.html` interaktif Three.js.
- **`render_preview.py`**: Script Python (Matplotlib + Trimesh) untuk merender gambar diagram teknis `casing_preview.png`.

## Cara Melihat Model 3D
Cukup buka berkas **`index.html`** langsung di peramban (browser) web apa pun (Chrome, Edge, Firefox), atau akses melalui rute `/3d` saat server lokal `web/server.js` sedang berjalan.
