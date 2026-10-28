#include "audio_player.h"

// Fungsi kirim paket raw 10-byte DFPlayer Mini (No Feedback 0x00 untuk stabilitas chip clone)
static void send_df_cmd(uint8_t cmd, uint16_t param) {
  uint8_t buf[10];
  buf[0] = 0x7E; // Start byte
  buf[1] = 0xFF; // Version
  buf[2] = 0x06; // Length
  buf[3] = cmd;  // Command ID
  buf[4] = 0x00; // Feedback (0 = No feedback, standar DFRobot anti-stutter)
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

static const uint8_t DEFAULT_VOLUME = 22; // Volume 22 (sweet spot: kencang, adem, dan MicroSD card stabil)

void audio_init() {
  // ESP32 RX2: GPIO 16 (hubungkan ke pin TX DFPlayer)
  // ESP32 TX2: GPIO 17 (hubungkan ke pin RX DFPlayer)
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println(F("[Audio] Inisialisasi DFPlayer Mini di RX2:GPIO16, TX2:GPIO17 (9600 baud)..."));
  
  delay(100);
  // Pilih sumber media MicroSD (0x02)
  send_df_cmd(0x09, 0x0002);
  delay(50);
  // Set volume maksimal (dengan resistor 47 ohm terpasang, arus sudah terproteksi aman)
  send_df_cmd(0x06, DEFAULT_VOLUME);

  last_init_retry = millis();
}

static unsigned long last_play_time = 0;
static int last_play_track = 0;

void audio_play(int track) {
  if (track <= 0) return;
  unsigned long now = millis();

  // Cegah spam & suara terpotong:
  // Suara efek panjang (seperti maskot Buddy / reward) butuh jeda proteksi lebih lama
  // agar tidak terpotong oleh jitter sentuhan touchscreen yang memicu klik ganda.
  unsigned long min_interval = 200;
  if (track == SOUND_BUDDY) {
    min_interval = 1500; // 1.5 detik proteksi penuh untuk sapaan maskot Buddy
  } else if (track == SOUND_REWARD || track == SOUND_SUCCESS) {
    min_interval = 800;  // 800 ms untuk efek kemenangan / reward
  }

  if (now - last_play_time < min_interval && track == last_play_track) return;
  last_play_time = now;
  last_play_track = track;

  Serial.printf("[Audio] Memutar trek #%d (/mp3/%04d.mp3)\n", track, track);

  // Gunakan perintah standar internasional DFPlayer: 0x03 (Specify tracking / Play Track Number)
  send_df_cmd(0x03, (uint16_t)track);
}

void audio_set_volume(int vol) {
  if (vol < 0) vol = 0;
  if (vol > 30) vol = 30;
  send_df_cmd(0x06, (uint16_t)vol);
  Serial.printf("[Audio] Set Volume: %d\n", vol);
}

// Pantau pesan balik dari DFPlayer
void audio_update() {
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
