#include "audio_player.h"

// Fungsi kirim paket raw 10-byte DFPlayer Mini dengan Feedback (0x01)
static void send_df_cmd(uint8_t cmd, uint16_t param) {
  uint8_t buf[10];
  buf[0] = 0x7E; // Start byte
  buf[1] = 0xFF; // Version
  buf[2] = 0x06; // Length
  buf[3] = cmd;  // Command ID
  buf[4] = 0x01; // Feedback (1 = DFPlayer wajib kirim balasan/ACK)
  buf[5] = (uint8_t)(param >> 8);
  buf[6] = (uint8_t)(param & 0xFF);
  
  // Hitung Checksum
  uint16_t sum = 0;
  for (int i = 1; i <= 6; i++) {
    sum += buf[i];
  }
  uint16_t checksum = -sum;
  buf[7] = (uint8_t)(checksum >> 8);
  buf[8] = (uint8_t)(checksum & 0xFF);
  buf[9] = 0xEF; // End byte

  Serial2.write(buf, 10);
  Serial2.flush();

  // Tampilkan paket HEX yang benar-benar dikirim keluar dari ESP32 GPIO 17
  Serial.print(F("[Audio TX >> GPIO17] Paket Hex: "));
  for (int i = 0; i < 10; i++) {
    if (buf[i] < 0x10) Serial.print("0");
    Serial.print(buf[i], HEX);
    Serial.print(" ");
  }
  Serial.printf("| Cmd: 0x%02X, Param: %d\n", cmd, param);
}

static unsigned long last_init_retry = 0;
static bool vol_initialized = false;
static unsigned long last_ping = 0;

void audio_init() {
  // ESP32 RX2: GPIO 16 (hubungkan ke pin TX DFPlayer)
  // ESP32 TX2: GPIO 17 (hubungkan ke pin RX DFPlayer)
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println(F("[Audio] Inisialisasi DFPlayer Mini di RX2:GPIO16, TX2:GPIO17 (9600 baud)..."));
  
  delay(100);
  // Pilih sumber media MicroSD (0x02)
  send_df_cmd(0x09, 0x0002);
  delay(50);
  // Set volume awal 28
  send_df_cmd(0x06, 28);

  last_init_retry = millis();
}

void audio_play(int track) {
  if (track <= 0) return;
  Serial.printf("[Audio] Memutar trek #%d (/mp3/%04d.mp3)\n", track, track);

  // Putar dari folder /mp3/ (0x12 -> /mp3/000X.mp3)
  send_df_cmd(0x12, (uint16_t)track);
}

void audio_set_volume(int vol) {
  if (vol < 0) vol = 0;
  if (vol > 30) vol = 30;
  send_df_cmd(0x06, (uint16_t)vol);
  Serial.printf("[Audio] Set Volume: %d\n", vol);
}

// Pantau pesan balik dari DFPlayer
void audio_update() {
  // Kirim ulang volume sekali setelah 1.5 detik sejak boot (DFPlayer butuh waktu booting)
  if (!vol_initialized && millis() - last_init_retry > 1500) {
    vol_initialized = true;
    send_df_cmd(0x06, 28);
  }

  static uint8_t rx_buf[10];
  static uint8_t rx_idx = 0;

  while (Serial2.available()) {
    uint8_t c = Serial2.read();

    if (rx_idx == 0) {
      if (c == 0x7E) {
        rx_buf[rx_idx++] = c;
      }
    } else {
      rx_buf[rx_idx++] = c;
      if (rx_idx == 10) {
        rx_idx = 0;
        if (rx_buf[9] == 0xEF) {
          uint8_t cmd = rx_buf[3];
          uint16_t param = (rx_buf[5] << 8) | rx_buf[6];
          
          if (cmd == 0x3F) {
            Serial.println(F("[Audio-DFPlayer] Status: MicroSD Card Online (0x3F)"));
          } else if (cmd == 0x3A) {
            Serial.println(F("[Audio-DFPlayer] Status: MicroSD Card Dimasukkan"));
          } else if (cmd == 0x3B) {
            Serial.println(F("[Audio-DFPlayer] Status: MicroSD Card Dicabut!"));
          } else if (cmd == 0x3D) {
            Serial.printf("[Audio-DFPlayer] Selesai memutar trek #%d\n", param);
          } else if (cmd == 0x40) {
            if (param == 0x01) {
              Serial.println(F("[Audio-DFPlayer] ERROR 0x01: MicroSD Card tidak terdeteksi!"));
            } else if (param == 0x06) {
              Serial.println(F("[Audio-DFPlayer] ERROR 0x06: File MP3 tidak ditemukan di MicroSD!"));
            }
          }
        }
      }
    }
  }
}
