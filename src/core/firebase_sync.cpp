#include "firebase_sync.h"
#include "secrets.h"
#include "audio_player.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// Endpoint Base Google Cloud Firestore REST API
static const char* FIRESTORE_HOST = "firestore.googleapis.com";

// Status apakah user sudah mengatur SSID khusus (bukan placeholder template)
static bool has_valid_wifi_config = false;

// Tipe item antrian sinkronisasi
enum SyncItemType {
  SYNC_ITEM_SUBMISSION,
  SYNC_ITEM_ASSESSMENT,
  SYNC_ITEM_TELEMETRY
};

struct SyncQueueItem {
  SyncItemType type;
  char str1[48];  // student_name / device_id
  char str2[16];  // student_class
  char str3[24];  // script_type
  char str4[64];  // aksara_name / materi_title
  char str5[24];  // status_label / grade
  char str6[96];  // feedback_note
  int num1;       // precision / final_score / tolerance_px
  int num2;       // score / total_xp / volume
  float f1;       // voltage
};

// Queue & Background Worker Task Handle (FreeRTOS)
static QueueHandle_t sync_queue = NULL;
static TaskHandle_t sync_task_handle = NULL;

// Penampung Sesi Sementara saat Offline (Ring FIFO statis agar hemat heap & aman TLS)
static constexpr int OFFLINE_SESSION_MAX = 10;
static SyncQueueItem offline_sessions[OFFLINE_SESSION_MAX];
static int offline_head = 0;
static int offline_tail = 0;
static int offline_count = 0;
static uint32_t offline_synced_total = 0;
static SemaphoreHandle_t offline_mutex = NULL;
static volatile bool reconnect_requested = false;
static volatile bool flush_requested = false;

static void offline_store_item(const SyncQueueItem& item) {
  if (item.type == SYNC_ITEM_TELEMETRY) return; // Sesi sementara khusus data belajar siswa

  if (offline_mutex) xSemaphoreTake(offline_mutex, portMAX_DELAY);
  if (offline_count == OFFLINE_SESSION_MAX) {
    // Buffer penuh: geser head agar item paling lama terganti item terbaru
    offline_head = (offline_head + 1) % OFFLINE_SESSION_MAX;
    offline_count--;
  }
  offline_sessions[offline_tail] = item;
  offline_tail = (offline_tail + 1) % OFFLINE_SESSION_MAX;
  offline_count++;
  int saved_count = offline_count;
  if (offline_mutex) xSemaphoreGive(offline_mutex);

  Serial.printf("[Sesi Sementara] 📦 Wi-Fi mati — data '%s' disimpan ke sesi sementara (%d/%d)\n",
                item.str4, saved_count, OFFLINE_SESSION_MAX);
}

static bool offline_peek_item(SyncQueueItem& out) {
  bool ok = false;
  if (offline_mutex) xSemaphoreTake(offline_mutex, portMAX_DELAY);
  if (offline_count > 0) {
    out = offline_sessions[offline_head];
    ok = true;
  }
  if (offline_mutex) xSemaphoreGive(offline_mutex);
  return ok;
}

static void offline_pop_head() {
  if (offline_mutex) xSemaphoreTake(offline_mutex, portMAX_DELAY);
  if (offline_count > 0) {
    offline_head = (offline_head + 1) % OFFLINE_SESSION_MAX;
    offline_count--;
    offline_synced_total++;
  }
  if (offline_mutex) xSemaphoreGive(offline_mutex);
}

// Pastikan SNTP sudah tersinkronisasi (Core 0 worker only, tunggu singkat maks ~1.2 detik jika baru connect)
static bool ensure_ntp_synced() {
  time_t now = time(nullptr);
  if (now > 1700000000) return true;
  if (WiFi.status() == WL_CONNECTED) {
    configTime(8 * 3600, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
    for (int i = 0; i < 10; i++) {
      vTaskDelay(pdMS_TO_TICKS(120));
      now = time(nullptr);
      if (now > 1700000000) return true;
    }
  }
  return false;
}

// Buffer statis terisolasi untuk Core 0 worker (mencegah fragmentasi heap sebelum TLS)
static char worker_url[256];
static char worker_body[768];

// Helper format timestamp ISO-8601 UTC standar Cloud Firestore (hanya valid jika NTP synced)
static bool get_iso_timestamp(bool& has_ntp, char* buf, size_t buf_sz) {
  has_ntp = ensure_ntp_synced();
  if (has_ntp && buf && buf_sz > 0) {
    time_t now = time(nullptr);
    struct tm* tm_info = gmtime(&now);
    if (tm_info) {
      strftime(buf, buf_sz, "%Y-%m-%dT%H:%M:%SZ", tm_info);
      return true;
    }
  }
  if (buf && buf_sz > 0) buf[0] = '\0';
  return false;
}

// Helper format label waktu lokal terbaca ("13 Nov, 09:15") bila NTP tersedia
static void get_display_time_label(bool has_ntp, const char* fallback_label, char* buf, size_t buf_sz) {
  if (!buf || buf_sz == 0) return;
  if (!has_ntp) {
    snprintf(buf, buf_sz, "%s", fallback_label);
    return;
  }
  static const char* BULAN[] = {"Jan","Feb","Mar","Apr","Mei","Jun","Jul","Agu","Sep","Okt","Nov","Des"};
  time_t now = time(nullptr);
  struct tm* tm_info = localtime(&now);
  if (!tm_info || tm_info->tm_mon < 0 || tm_info->tm_mon > 11) {
    snprintf(buf, buf_sz, "%s", fallback_label);
    return;
  }
  snprintf(buf, buf_sz, "%d %s, %02d:%02d", tm_info->tm_mday, BULAN[tm_info->tm_mon], tm_info->tm_hour, tm_info->tm_min);
}

// Shared HTTPS sender untuk Cloud Firestore REST API (Core 0 worker only, zero heap churn)
static int firestore_send_json(const char* doc_subpath, const char* method, const char* json_body, size_t body_len, bool log_diag) {
  // Tunggu sejenak jika Core 1 sedang merender frame LVGL agar heap tidak tabrakan
  vTaskDelay(pdMS_TO_TICKS(60));

  if (log_diag) {
    Serial.printf("[Diag] RSSI: %d dBm | FreeHeap: %u B | MaxBlock: %u B\n",
                  WiFi.RSSI(), (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMaxAllocHeap());
  }

  if (ESP.getMaxAllocHeap() < 28000) {
    if (log_diag) {
      Serial.println(F("[Firebase Cloud] ⚠️ Heap terlalu rendah untuk TLS, menunda 200ms..."));
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    if (ESP.getMaxAllocHeap() < 24000) {
      return -1;
    }
  }

  IPAddress targetIP;
  if (!WiFi.hostByName(FIRESTORE_HOST, targetIP)) {
    if (log_diag) {
      Serial.printf("[Diag] ❌ DNS GAGAL resolve '%s' (Gateway: %s, DNS: %s)\n",
                    FIRESTORE_HOST, WiFi.gatewayIP().toString().c_str(), WiFi.dnsIP().toString().c_str());
    }
    return -2;
  }
  if (log_diag) {
    Serial.printf("[Diag] ✅ DNS OK -> %s\n", targetIP.toString().c_str());
  }

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(12000);

  HTTPClient https;
  https.setTimeout(12000);
  https.setConnectTimeout(12000);
  https.setReuse(false);

  snprintf(worker_url, sizeof(worker_url),
           "https://%s/v1/projects/%s/databases/(default)/documents/%s?key=%s",
           FIRESTORE_HOST, FIREBASE_PROJECT_ID, doc_subpath, FIREBASE_API_KEY);

  int httpCode = -3;
  if (https.begin(client, worker_url)) {
    https.addHeader("Content-Type", "application/json");
    https.addHeader("User-Agent", "OpenAksara-ESP32");
    https.addHeader("Connection", "close");

    httpCode = https.sendRequest(method, (uint8_t*)json_body, body_len);
    if (log_diag && httpCode != 200 && httpCode != 201) {
      char tls_err[128] = {0};
      int tls_code = client.lastError(tls_err, sizeof(tls_err));
      Serial.printf("[Diag] TLS lastError: %d (%s) | FreeHeap: %u | MaxBlock: %u\n",
                    tls_code, tls_err, (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMaxAllocHeap());
    }
    https.end();
  } else {
    Serial.printf("[Firebase Cloud] ⚠️ Gagal https.begin() untuk %s\n", doc_subpath);
  }
  return httpCode;
}

// Helper eksekusi HTTP POST Submission (Hanya dieksekusi di Core 0 background worker)
static bool send_submission_http(const SyncQueueItem& item) {
  bool has_ntp = false;
  char iso_time[32];
  char disp_time[24];
  get_iso_timestamp(has_ntp, iso_time, sizeof(iso_time));
  get_display_time_label(has_ntp, "Baru saja", disp_time, sizeof(disp_time));

  int n = 0;
  if (has_ntp && iso_time[0] != '\0') {
    n = snprintf(worker_body, sizeof(worker_body),
                 "{\"fields\":{\"name\":{\"stringValue\":\"%s\"},\"class\":{\"stringValue\":\"%s\"},"
                 "\"script\":{\"stringValue\":\"%s\"},\"aksara\":{\"stringValue\":\"%s\"},"
                 "\"precision\":{\"integerValue\":%d},\"score\":{\"integerValue\":%d},"
                 "\"status\":{\"stringValue\":\"%s\"},\"time\":{\"stringValue\":\"%s\"},"
                 "\"createdAt\":{\"timestampValue\":\"%s\"}}}",
                 item.str1, item.str2, item.str3, item.str4, item.num1, item.num2, item.str5, disp_time, iso_time);
  } else {
    n = snprintf(worker_body, sizeof(worker_body),
                 "{\"fields\":{\"name\":{\"stringValue\":\"%s\"},\"class\":{\"stringValue\":\"%s\"},"
                 "\"script\":{\"stringValue\":\"%s\"},\"aksara\":{\"stringValue\":\"%s\"},"
                 "\"precision\":{\"integerValue\":%d},\"score\":{\"integerValue\":%d},"
                 "\"status\":{\"stringValue\":\"%s\"},\"time\":{\"stringValue\":\"%s\"}}}",
                 item.str1, item.str2, item.str3, item.str4, item.num1, item.num2, item.str5, disp_time);
  }

  size_t len = (n > 0 && (size_t)n < sizeof(worker_body)) ? (size_t)n : strlen(worker_body);
  int httpCode = firestore_send_json("submissions", "POST", worker_body, len, true);
  if (httpCode == 200 || httpCode == 201) {
    Serial.printf("[Firebase Cloud] 🟢 Submission %s (%s) BERHASIL tersimpan di Firestore!\n", item.str1, item.str4);
    return true;
  } else if (httpCode > -2) {
    Serial.printf("[Firebase Cloud] ⚠️ Gagal POST submission, HTTP Code: %d\n", httpCode);
  }
  return false;
}

// Helper eksekusi HTTP POST Assessment (Hanya dieksekusi di Core 0 background worker)
static bool send_assessment_http(const SyncQueueItem& item) {
  bool has_ntp = false;
  char iso_time[32];
  char disp_date[24];
  get_iso_timestamp(has_ntp, iso_time, sizeof(iso_time));
  get_display_time_label(has_ntp, "Hari ini", disp_date, sizeof(disp_date));

  int n = 0;
  if (has_ntp && iso_time[0] != '\0') {
    n = snprintf(worker_body, sizeof(worker_body),
                 "{\"fields\":{\"studentName\":{\"stringValue\":\"%s\"},\"class\":{\"stringValue\":\"%s\"},"
                 "\"materiTitle\":{\"stringValue\":\"%s\"},\"script\":{\"stringValue\":\"%s\"},"
                 "\"finalScore\":{\"integerValue\":%d},\"grade\":{\"stringValue\":\"%s\"},"
                 "\"totalXp\":{\"integerValue\":%d},\"status\":{\"stringValue\":\"TUNTAS\"},"
                 "\"feedback\":{\"stringValue\":\"%s\"},\"completedDate\":{\"stringValue\":\"%s\"},"
                 "\"createdAt\":{\"timestampValue\":\"%s\"}}}",
                 item.str1, item.str2, item.str4, item.str3, item.num1, item.str5, item.num2, item.str6, disp_date, iso_time);
  } else {
    n = snprintf(worker_body, sizeof(worker_body),
                 "{\"fields\":{\"studentName\":{\"stringValue\":\"%s\"},\"class\":{\"stringValue\":\"%s\"},"
                 "\"materiTitle\":{\"stringValue\":\"%s\"},\"script\":{\"stringValue\":\"%s\"},"
                 "\"finalScore\":{\"integerValue\":%d},\"grade\":{\"stringValue\":\"%s\"},"
                 "\"totalXp\":{\"integerValue\":%d},\"status\":{\"stringValue\":\"TUNTAS\"},"
                 "\"feedback\":{\"stringValue\":\"%s\"},\"completedDate\":{\"stringValue\":\"%s\"}}}",
                 item.str1, item.str2, item.str4, item.str3, item.num1, item.str5, item.num2, item.str6, disp_date);
  }

  size_t len = (n > 0 && (size_t)n < sizeof(worker_body)) ? (size_t)n : strlen(worker_body);
  int httpCode = firestore_send_json("assessments", "POST", worker_body, len, false);
  if (httpCode == 200 || httpCode == 201) {
    Serial.printf("[Firebase Cloud] 🟢 Rapor Materi %s (+%d XP) tersimpan di Firestore!\n", item.str4, item.num2);
    return true;
  } else if (httpCode > -2) {
    Serial.printf("[Firebase Cloud] ⚠️ Gagal POST assessment, HTTP Code: %d\n", httpCode);
  }
  return false;
}

// Helper eksekusi HTTP PATCH Telemetry (Hanya dieksekusi di Core 0 background worker)
static void send_telemetry_http(float voltage, int tolerance_px, int volume) {
  int n = snprintf(worker_body, sizeof(worker_body),
                   "{\"fields\":{\"device\":{\"stringValue\":\"%s\"},\"voltage\":{\"doubleValue\":%.2f},"
                   "\"tolerancePx\":{\"integerValue\":%d},\"volume\":{\"integerValue\":%d},"
                   "\"status\":{\"stringValue\":\"online\"},\"freeHeap\":{\"integerValue\":%u}}}",
                   DEVICE_ID, voltage, tolerance_px, volume, (unsigned)ESP.getFreeHeap());

  size_t len = (n > 0 && (size_t)n < sizeof(worker_body)) ? (size_t)n : strlen(worker_body);
  int httpCode = firestore_send_json("telemetry/esp32_status", "PATCH", worker_body, len, false);
  if (httpCode == 200 || httpCode == 201) {
    Serial.printf("[Firebase Cloud] 📡 Telemetri hardware tersinkronisasi (Heap: %u B)\n", (unsigned int)ESP.getFreeHeap());
  }
}

// Background Worker Task di Core 0 (Terisolasi dari UI & sentuhan layar)
static void firebase_sync_worker_task(void* pvParameters) {
  (void)pvParameters;
  SyncQueueItem item;
  uint32_t last_reconnect = 0;
  uint32_t last_flush_attempt = 0;
  bool was_connected = false;

  for (;;) {
    uint32_t now = millis();

    // 0. Permintaan rekoneksi manual dari halaman Setelan
    if (reconnect_requested) {
      reconnect_requested = false;
      last_reconnect = now;
      if (has_valid_wifi_config) {
        Serial.printf("[Wi-Fi] 🔄 Menghubungkan ulang ke '%s'...\n", WIFI_SSID);
        WiFi.disconnect(false, false);
        vTaskDelay(pdMS_TO_TICKS(100));
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      }
    }

    // 1. Proses item antrian live (timeout 250ms)
    if (sync_queue && xQueueReceive(sync_queue, &item, pdMS_TO_TICKS(250)) == pdTRUE) {
      if (WiFi.status() == WL_CONNECTED) {
        if (item.type == SYNC_ITEM_SUBMISSION) {
          if (!send_submission_http(item)) offline_store_item(item);
        } else if (item.type == SYNC_ITEM_ASSESSMENT) {
          if (!send_assessment_http(item)) offline_store_item(item);
        } else if (item.type == SYNC_ITEM_TELEMETRY) {
          send_telemetry_http(item.f1, item.num1, item.num2);
        }
      } else {
        if (item.type == SYNC_ITEM_SUBMISSION || item.type == SYNC_ITEM_ASSESSMENT) {
          offline_store_item(item);
        } else {
          Serial.println(F("[Firebase Worker] ⚠️ Wi-Fi BELUM terhubung! Telemetri hanya via USB Serial."));
        }
      }
    }

    now = millis();
    bool is_connected = (WiFi.status() == WL_CONNECTED);

    // Saat Wi-Fi baru saja terhubung, langsung izinkan flush sesi sementara
    if (is_connected && !was_connected) {
      last_flush_attempt = 0;
    }
    was_connected = is_connected;

    // 2. Sinkronisasi otomatis Sesi Sementara begitu Wi-Fi kembali online
    if (is_connected && firebase_sync_get_offline_count() > 0) {
      if (flush_requested || last_flush_attempt == 0 || (now - last_flush_attempt >= 12000)) {
        flush_requested = false;
        SyncQueueItem pending;
        if (offline_peek_item(pending)) {
          bool ok = false;
          if (pending.type == SYNC_ITEM_SUBMISSION) {
            ok = send_submission_http(pending);
          } else if (pending.type == SYNC_ITEM_ASSESSMENT) {
            ok = send_assessment_http(pending);
          }
          if (ok) {
            offline_pop_head();
            Serial.printf("[Sesi Sementara] ✅ Data terkirim ke Cloud! Sisa antrian: %d\n",
                          firebase_sync_get_offline_count());
            last_flush_attempt = 0;
            vTaskDelay(pdMS_TO_TICKS(250));
          } else {
            last_flush_attempt = now;
          }
        }
      }
    } else if (!is_connected) {
      flush_requested = false;
    }

    // CATATAN PENTING: Heartbeat HTTPS berkala 30 detik DIHAPUS agar mbedTLS tidak tiba-tiba
    // menyedot ~40 KB heap di tengah permainan pembelajaran (seperti Memori Kartu) yang
    // menyebabkan lv_malloc_zeroed gagal (StoreProhibited 0x00000008).

    // 3. Rekoneksi background jika Wi-Fi terputus
    if (has_valid_wifi_config && !is_connected && (now - last_reconnect > 20000)) {
      last_reconnect = now;
      WiFi.reconnect();
    }

    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

void firebase_sync_init() {
  Serial.println(F("\n========================================================"));
  Serial.println(F("🚀 OpenAksara ESP32 - Firebase Cloud Sync Engine Active"));
  Serial.printf("📡 Project ID   : %s\n", FIREBASE_PROJECT_ID);
  Serial.printf("📱 Device ID    : %s\n", DEVICE_ID);
  Serial.printf("👤 Active Siswa : %s (Kelas %s)\n", DEFAULT_STUDENT_NAME, DEFAULT_STUDENT_CLASS);
  Serial.println(F("========================================================"));

  // Output telemetri awal via USB Serial (Format JSON instan untuk Web Dashboard)
  Serial.printf("{\"type\":\"telemetry\",\"device\":\"%s\",\"status\":\"online\",\"voltage\":4.98,\"port\":\"COM3\"}\n", DEVICE_ID);

  if (!offline_mutex) {
    offline_mutex = xSemaphoreCreateMutex();
  }

  // Buat FreeRTOS Queue (4 slot cukup & hemat ~1.8 KB heap)
  if (!sync_queue) {
    sync_queue = xQueueCreate(4, sizeof(SyncQueueItem));
  }

  // Buat Dedicated Background Task di Core 0 dengan Stack 8 KB (hemat 4 KB heap untuk LVGL)
  if (!sync_task_handle) {
    xTaskCreatePinnedToCore(
      firebase_sync_worker_task,
      "fb_sync_task",
      8192,
      NULL,
      1, // Priority 1 (Background)
      &sync_task_handle,
      0  // Pinned ke Core 0 (PRO_CPU), Core 1 100% bebas untuk UI & Touchscreen
    );
  }

  // Cek apakah SSID sudah diisi dengan SSID asli pengguna
  has_valid_wifi_config = (strlen(WIFI_SSID) > 0 &&
                           strcmp(WIFI_SSID, "NAMA_WIFI") != 0 &&
                           strcmp(WIFI_SSID, "Hotspot_HP") != 0);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // Mode performa penuh tanpa sleep

  if (has_valid_wifi_config) {
    Serial.printf("[Wi-Fi] 📡 Menghubungkan ke Wi-Fi: %s ...\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  } else {
    Serial.println(F("[Wi-Fi] ℹ️ Menggunakan mode USB Serial lokal. (Isi SSID di include/secrets.h untuk mode Wi-Fi langsung)."));
  }
}

void firebase_sync_update() {
  // Hanya mendeteksi transisi status koneksi tanpa membebani loop() utama (0 delay)
  static bool prev_wifi_status = false;
  bool curr = (WiFi.status() == WL_CONNECTED);

  if (curr != prev_wifi_status) {
    prev_wifi_status = curr;
    if (curr) {
      Serial.printf("[Wi-Fi] 🟢 Terhubung ke jaringan! IP ESP32: %s\n", WiFi.localIP().toString().c_str());
      configTime(8 * 3600, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
    } else {
      Serial.println(F("[Wi-Fi] 🔴 Terputus dari jaringan Wi-Fi."));
    }
  }
}

bool firebase_sync_is_connected() {
  return (WiFi.status() == WL_CONNECTED);
}

int firebase_sync_get_wifi_status_code() {
  return (int)WiFi.status();
}

const char* firebase_sync_get_ssid() {
  return WIFI_SSID;
}

void firebase_sync_get_ip(char* buf, size_t len) {
  if (!buf || len == 0) return;
  if (WiFi.status() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    snprintf(buf, len, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  } else {
    snprintf(buf, len, "Belum ada IP");
  }
}

int firebase_sync_get_rssi() {
  if (WiFi.status() == WL_CONNECTED) {
    return (int)WiFi.RSSI();
  }
  return 0;
}

int firebase_sync_get_offline_count() {
  int c = 0;
  if (offline_mutex) xSemaphoreTake(offline_mutex, portMAX_DELAY);
  c = offline_count;
  if (offline_mutex) xSemaphoreGive(offline_mutex);
  return c;
}

int firebase_sync_get_offline_capacity() {
  return OFFLINE_SESSION_MAX;
}

uint32_t firebase_sync_get_offline_synced_total() {
  uint32_t t = 0;
  if (offline_mutex) xSemaphoreTake(offline_mutex, portMAX_DELAY);
  t = offline_synced_total;
  if (offline_mutex) xSemaphoreGive(offline_mutex);
  return t;
}

void firebase_sync_trigger_reconnect() {
  reconnect_requested = true;
}

void firebase_sync_trigger_flush() {
  flush_requested = true;
}

/**
 * Kirim hasil latihan goresan siswa (Koleksi 'submissions')
 * NON-BLOCKING (< 0.1ms): Output Serial instan + Push ke Antrian Core 0 / Sesi Sementara jika offline
 */
bool firebase_sync_send_submission(
    const char* student_name,
    const char* student_class,
    const char* script_type,
    const char* aksara_name,
    int precision,
    int score,
    const char* status_label
) {
  const char* final_name = (student_name && strlen(student_name) > 0) ? student_name : DEFAULT_STUDENT_NAME;
  const char* final_class = (student_class && strlen(student_class) > 0) ? student_class : DEFAULT_STUDENT_CLASS;

  // 1. Output INSTAN ke Serial USB (Terbaca langsung oleh web/server dalam 0.1ms)
  Serial.printf(
    "{\"type\":\"submission\",\"name\":\"%s\",\"class\":\"%s\",\"script\":\"%s\",\"aksara\":\"%s\",\"precision\":%d,\"score\":%d,\"status\":\"%s\",\"time\":\"Baru saja\"}\n",
    final_name, final_class, script_type, aksara_name, precision, score, status_label
  );

  // 2. Siapkan payload item
  SyncQueueItem item;
  memset(&item, 0, sizeof(SyncQueueItem));
  item.type = SYNC_ITEM_SUBMISSION;
  strncpy(item.str1, final_name, sizeof(item.str1) - 1);
  strncpy(item.str2, final_class, sizeof(item.str2) - 1);
  strncpy(item.str3, script_type ? script_type : "Jawa", sizeof(item.str3) - 1);
  strncpy(item.str4, aksara_name ? aksara_name : "Aksara", sizeof(item.str4) - 1);
  strncpy(item.str5, status_label ? status_label : "Selesai", sizeof(item.str5) - 1);
  item.num1 = precision;
  item.num2 = score;

  // Jika TIDAK ADA Wi-Fi, langsung taruh ke Sesi Sementara!
  if (WiFi.status() != WL_CONNECTED) {
    offline_store_item(item);
    return true;
  }

  // Wi-Fi aktif: kirim ke antrian Core 0 (jika antrian penuh, simpan ke Sesi Sementara)
  if (!sync_queue || xQueueSend(sync_queue, &item, 0) != pdTRUE) {
    offline_store_item(item);
  }

  return true;
}

/**
 * Kirim capaian materi & perolehan XP (Koleksi 'assessments')
 * NON-BLOCKING (< 0.1ms): Push ke Core 0 / Sesi Sementara jika offline
 */
bool firebase_sync_send_assessment(
    const char* student_name,
    const char* student_class,
    const char* materi_title,
    const char* script_type,
    int final_score,
    const char* grade,
    int xp_earned,
    const char* feedback_note
) {
  const char* final_name = (student_name && strlen(student_name) > 0) ? student_name : DEFAULT_STUDENT_NAME;
  const char* final_class = (student_class && strlen(student_class) > 0) ? student_class : DEFAULT_STUDENT_CLASS;

  // 1. Output INSTAN ke Serial USB
  Serial.printf(
    "{\"type\":\"assessment\",\"studentName\":\"%s\",\"class\":\"%s\",\"materiTitle\":\"%s\",\"script\":\"%s\",\"finalScore\":%d,\"grade\":\"%s\",\"totalXp\":%d,\"feedback\":\"%s\"}\n",
    final_name, final_class, materi_title, script_type, final_score, grade, xp_earned, feedback_note
  );

  // 2. Siapkan payload item
  SyncQueueItem item;
  memset(&item, 0, sizeof(SyncQueueItem));
  item.type = SYNC_ITEM_ASSESSMENT;
  strncpy(item.str1, final_name, sizeof(item.str1) - 1);
  strncpy(item.str2, final_class, sizeof(item.str2) - 1);
  strncpy(item.str3, script_type ? script_type : "Jawa", sizeof(item.str3) - 1);
  strncpy(item.str4, materi_title ? materi_title : "Materi", sizeof(item.str4) - 1);
  strncpy(item.str5, grade ? grade : "A", sizeof(item.str5) - 1);
  strncpy(item.str6, feedback_note ? feedback_note : "", sizeof(item.str6) - 1);
  item.num1 = final_score;
  item.num2 = xp_earned;

  // Jika TIDAK ADA Wi-Fi, langsung taruh ke Sesi Sementara!
  if (WiFi.status() != WL_CONNECTED) {
    offline_store_item(item);
    return true;
  }

  if (!sync_queue || xQueueSend(sync_queue, &item, 0) != pdTRUE) {
    offline_store_item(item);
  }

  return true;
}

/**
 * Kirim telemetri status hardware ESP32 (Dokumen 'telemetry/esp32_status')
 * NON-BLOCKING (< 0.1ms)
 */
bool firebase_sync_send_telemetry(float voltage, int tolerance_px, int volume) {
  // 1. Output INSTAN ke Serial USB
  Serial.printf(
    "{\"type\":\"telemetry\",\"device\":\"%s\",\"voltage\":%.2f,\"tolerance\":%d,\"volume\":%d,\"status\":\"online\"}\n",
    DEVICE_ID, voltage, tolerance_px, volume
  );

  // 2. Masukkan ke antrian background worker (Core 0)
  if (sync_queue && WiFi.status() == WL_CONNECTED) {
    SyncQueueItem item;
    memset(&item, 0, sizeof(SyncQueueItem));
    item.type = SYNC_ITEM_TELEMETRY;
    strncpy(item.str1, DEVICE_ID, sizeof(item.str1) - 1);
    item.f1 = voltage;
    item.num1 = tolerance_px;
    item.num2 = volume;

    xQueueSend(sync_queue, &item, 0);
  }

  return true;
}
