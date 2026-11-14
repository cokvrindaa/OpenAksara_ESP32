# Sound Effects Pack untuk DFPlayer Mini - OpenAksara ESP32

Koleksi efek suara antarmuka (UI Audio Pack) berlisensi **CC0 1.0 Universal (Public Domain)** yang dirancang khusus untuk layar sentuh edukasi interaktif anak-anak (tema ramah seperti Duolingo / Nintendo Switch).

## Daftar Trek Audio (1 - 7)

| Nomor Trek | Nama File | Karakter Suara | Penggunaan di OpenAksara |
| :---: | :---: | :--- | :--- |
| **`0001`** | `0001.mp3` | *Crisp UI Tap* (Ketukan tombol jernih & lembut) | **Klik tombol umum** (navigasi, bersihkan kanvas, dll.) |
| **`0002`** | `0002.mp3` | *Bubble Pluck* (Nada marimba/petikan ceria) | **Pilih node level aksara** di jalur belajar & dengarkan audio |
| **`0003`** | `0003.mp3` | *Fanfare Jingle* (Melodi kemenangan bonus) | **Buka peti bonus**, perolehan XP, klaim piala |
| **`0004`** | `0004.mp3` | *Mascot Chirp* (Suara ceria maskot Buddy) | **Sapaan maskot AksaraBuddy** saat disentuh |
| **`0005`** | `0005.mp3` | *Breezy Smooth Whoosh* (Desiran udara halus & cepat, 0.15s) | **Ganti babak / halaman** naik-turun pada peta belajar |
| **`0006`** | `0006.mp3` | *Sparkling Ding* (Nada konfirmasi tuntas) | **Goresan berhasil** / jawaban kuis benar |
| **`0007`** | `0007.mp3` | *Soft Bonk* (Buzzer kartun lembut ramah anak) | **Goresan gagal** / salah checkpoint / remedial |

---

## Cara Memasang ke MicroSD Card DFPlayer Mini

1. Siapkan kartu MicroSD (kapasitas 512 MB s/d 32 GB, format **FAT32**).
2. Salin folder **`mp3`** yang ada di sini ke dalam MicroSD card Anda:
   ```
   MicroSD Card/
   └── mp3/
       ├── 0001.mp3
       ├── 0002.mp3
       ├── 0003.mp3
       ├── 0004.mp3
       ├── 0005.mp3
       ├── 0006.mp3
       └── 0007.mp3
   ```
   *(Tips: Salin juga file `0001.mp3` s/d `0007.mp3` langsung ke root MicroSD untuk kompatibilitas maksimal semua tipe chip DFPlayer clone/original).*
3. Masukkan MicroSD card ke modul **DFPlayer Mini** di ESP32 Anda.
4. Nyalakan ESP32, dan seluruh tombol serta animasi goresan akan bersuara otomatis!
