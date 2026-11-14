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
static uint8_t current_volume = 22; // Volume 22 (sweet spot: kencang, adem, dan MicroSD card stabil)
static uint8_t boot_0x3f_count = 0;

static void apply_df_volume_only() {
  // Cukup kirim 0x06 (Set Volume). JANGAN kirim 0x09 (Select TF) di dalam handler 0x3F
  // karena 0x09 memicu DFPlayer mount ulang MicroSD dan mengirim 0x3F lagi (infinite loop)!
  send_df_cmd(0x06, (uint16_t)current_volume);
  vol_initialized = true;
}

void audio_init() {
  // ESP32 RX2: GPIO 16 (hubungkan ke pin TX DFPlayer)
  // ESP32 TX2: GPIO 17 (hubungkan ke pin RX DFPlayer)
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println(F("[Audio] Inisialisasi DFPlayer Mini di RX2:GPIO16, TX2:GPIO17 (9600 baud)..."));

  delay(250);
  send_df_cmd(0x09, 0x0002);
  delay(50);
  send_df_cmd(0x06, (uint16_t)current_volume);

  vol_initialized = false;
  boot_0x3f_count = 0;
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
  current_volume = (uint8_t)vol;
  send_df_cmd(0x06, (uint16_t)vol);
  Serial.printf("[Audio] Set Volume: %d\n", vol);
}

// Pantau pesan balik dari DFPlayer & pastikan volume aktif setelah DFPlayer selesai boot
void audio_update() {
  if (!vol_initialized && (millis() - last_init_retry >= 1200)) {
    Serial.println(F("[Audio] Set volume DFPlayer pasca-boot..."));
    apply_df_volume_only();
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
            boot_0x3f_count++;
            Serial.printf("[Audio-DFPlayer] Status: MicroSD Card Online (0x3F, #%u)\n", boot_0x3f_count);
            if (boot_0x3f_count > 2) {
              Serial.println(F("[Audio-DFPlayer] ⚠️ DFPlayer ter-reset berulang! Cek VCC 5V/GND/tegangan drop."));
            }
            apply_df_volume_only();
          } else if (cmd == 0x3A) {
            Serial.println(F("[Audio-DFPlayer] Status: MicroSD Card Dimasukkan"));
            apply_df_volume_only();
          } else if (cmd == 0x3B) {
            Serial.println(F("[Audio-DFPlayer] Status: MicroSD Card Dicabut!"));
          } else if (cmd == 0x3D) {
            Serial.printf("[Audio-DFPlayer] Selesai memutar trek #%d\n", param);
          } else if (cmd == 0x40) {
            if (param == 0x01) {
              Serial.println(F("[Audio-DFPlayer] ERROR 0x01: MicroSD Card tidak terdeteksi / module busy!"));
            } else if (param == 0x03) {
              Serial.println(F("[Audio-DFPlayer] ERROR 0x03: Serial RX DFPlayer putus / timeout!"));
            } else if (param == 0x04) {
              Serial.println(F("[Audio-DFPlayer] ERROR 0x04: Checksum UART salah!"));
            } else if (param == 0x05) {
              Serial.println(F("[Audio-DFPlayer] ERROR 0x05: File di luar rentang indeks MicroSD!"));
            } else if (param == 0x06) {
              Serial.println(F("[Audio-DFPlayer] ERROR 0x06: File MP3 tidak ditemukan di MicroSD!"));
            } else {
              Serial.printf("[Audio-DFPlayer] ERROR 0x%02X dari DFPlayer!\n", param);
            }
          }
        }
      }
    }
  }
}
