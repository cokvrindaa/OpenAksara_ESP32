# OpenAksara - Standalone Telegram Bridge Worker

Program background worker independen yang bertugas mengambil data baru langsung dari **Cloud Firestore** dan meneruskannya secara otomatis ke **Telegram Bot** Guru / Grup Wali Murid.

## Keunggulan
- **100% Bebas dari Web**: Tidak memerlukan web browser atau dashboard dibuka.
- **Zero Dependency**: Murni menggunakan modul bawaan Node.js (`https`, `fs`, `path`).
- **Otomatis Real-time**: Memantau dokumen baru pada koleksi `submissions`, `assessments`, dan `feedbacks`.
- **Bisa Dijalankan di Mana Saja**: Di laptop (background service), PC lab sekolah, Raspberry Pi, VPS, atau cloud service gratis (Render / Railway / Koyeb).

## Cara Menjalankan
1. Pastikan `TELEGRAM_BOT_TOKEN` dan `TELEGRAM_CHAT_ID` sudah terisi di file `.env`.
2. Klik ganda **`start_telegram_bot.bat`** (atau jalankan `node telegram_bot.js`).
3. Bot akan langsung memantau dan mengirim notifikasi setiap ada data baru dari ESP32!
