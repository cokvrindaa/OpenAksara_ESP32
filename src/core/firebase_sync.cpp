#include "firebase_sync.h"
#include "secrets.h"
#include "audio_player.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

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

// Helper format timestamp ISO-8601 standar Cloud Firestore
static String get_iso_timestamp() {
  time_t now = time(nullptr);
  if (now > 1600000000) {
    char buf[32];
    struct tm* t = gmtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", t);
    return String(buf);
  }
  char buf[32];
  snprintf(buf, sizeof(buf), "2026-11-12T%02lu:%02lu:%02luZ",
           (millis() / 3600000) % 24,
           (millis() / 60000) % 60,
           (millis() / 1000) % 60);
  return String(buf);
}

// Helper eksekusi HTTP POST Submission (Hanya dieksekusi di Core 0 background worker)
static void send_submission_http(const SyncQueueItem& item) {
  // Tunggu sejenak jika Core 1 sedang merender frame LVGL agar heap tidak tabrakan
  vTaskDelay(pdMS_TO_TICKS(60));

  // ===== DIAGNOSTIK =====
  Serial.printf("[Diag] RSSI: %d dBm | FreeHeap: %u B | MaxBlock: %u B\n",
                WiFi.RSSI(), (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMaxAllocHeap());

  if (ESP.getMaxAllocHeap() < 28000) {
    Serial.println(F("[Firebase Cloud] ⚠️ Heap terlalu rendah untuk TLS, menunda 200ms..."));
    vTaskDelay(pdMS_TO_TICKS(200));
  }

  // 1. DNS
  IPAddress targetIP;
  if (!WiFi.hostByName(FIRESTORE_HOST, targetIP)) {
    Serial.printf("[Diag] ❌ DNS GAGAL resolve '%s' (Gateway: %s, DNS: %s)\n",
                  FIRESTORE_HOST, WiFi.gatewayIP().toString().c_str(), WiFi.dnsIP().toString().c_str());
    return;
  }
  Serial.printf("[Diag] ✅ DNS OK -> %s\n", targetIP.toString().c_str());

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(12000); // 12 detik timeout

  HTTPClient https;
  https.setTimeout(12000);
  https.setConnectTimeout(12000);
  https.setReuse(false);

  String url;
  url.reserve(180);
  url = String("https://") + FIRESTORE_HOST + "/v1/projects/" + FIREBASE_PROJECT_ID +
        "/databases/(default)/documents/submissions?key=" + FIREBASE_API_KEY;

  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");
    https.addHeader("User-Agent", "OpenAksara-ESP32");
    https.addHeader("Connection", "close");

    String isoTime = get_iso_timestamp();
    String body;
    body.reserve(420);
    body = "{";
    body += "\"fields\":{";
    body += "\"name\":{\"stringValue\":\"" + String(item.str1) + "\"},";
    body += "\"class\":{\"stringValue\":\"" + String(item.str2) + "\"},";
    body += "\"script\":{\"stringValue\":\"" + String(item.str3) + "\"},";
    body += "\"aksara\":{\"stringValue\":\"" + String(item.str4) + "\"},";
    body += "\"precision\":{\"integerValue\":" + String(item.num1) + "},";
    body += "\"score\":{\"integerValue\":" + String(item.num2) + "},";
    body += "\"status\":{\"stringValue\":\"" + String(item.str5) + "\"},";
    body += "\"time\":{\"stringValue\":\"Baru saja\"},";
    body += "\"createdAt\":{\"stringValue\":\"" + isoTime + "\"}";
    body += "}}";

    int httpCode = https.POST(body);
    if (httpCode == 200 || httpCode == 201) {
      Serial.printf("[Firebase Cloud] 🟢 Submission %s (%s) BERHASIL tersimpan di Firestore!\n", item.str1, item.str4);
    } else {
      Serial.printf("[Firebase Cloud] ⚠️ Gagal POST submission, HTTP Code: %d (%s)\n", httpCode, https.errorToString(httpCode).c_str());
      char tls_err[128] = {0};
      int tls_code = client.lastError(tls_err, sizeof(tls_err));
      Serial.printf("[Diag] TLS lastError: %d (%s) | FreeHeap: %u | MaxBlock: %u\n",
                    tls_code, tls_err, (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMaxAllocHeap());
    }
    https.end();
  } else {
    Serial.println(F("[Firebase Cloud] ⚠️ Gagal https.begin() untuk submission"));
  }
}

// Helper eksekusi HTTP POST Assessment (Hanya dieksekusi di Core 0 background worker)
static void send_assessment_http(const SyncQueueItem& item) {
  vTaskDelay(pdMS_TO_TICKS(60));
  if (ESP.getMaxAllocHeap() < 28000) {
    vTaskDelay(pdMS_TO_TICKS(200));
  }

  IPAddress targetIP;
  if (!WiFi.hostByName(FIRESTORE_HOST, targetIP)) {
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(12000);

  HTTPClient https;
  https.setTimeout(12000);
  https.setConnectTimeout(12000);
  https.setReuse(false);

  String url;
  url.reserve(180);
  url = String("https://") + FIRESTORE_HOST + "/v1/projects/" + FIREBASE_PROJECT_ID +
        "/databases/(default)/documents/assessments?key=" + FIREBASE_API_KEY;

  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");
    https.addHeader("User-Agent", "OpenAksara-ESP32");
    https.addHeader("Connection", "close");

    String isoTime = get_iso_timestamp();
    String body;
    body.reserve(520);
    body = "{";
    body += "\"fields\":{";
    body += "\"studentName\":{\"stringValue\":\"" + String(item.str1) + "\"},";
    body += "\"class\":{\"stringValue\":\"" + String(item.str2) + "\"},";
    body += "\"materiTitle\":{\"stringValue\":\"" + String(item.str4) + "\"},";
    body += "\"script\":{\"stringValue\":\"" + String(item.str3) + "\"},";
    body += "\"finalScore\":{\"integerValue\":" + String(item.num1) + "},";
    body += "\"grade\":{\"stringValue\":\"" + String(item.str5) + "\"},";
    body += "\"totalXp\":{\"integerValue\":" + String(item.num2) + "},";
    body += "\"status\":{\"stringValue\":\"TUNTAS\"},";
    body += "\"feedback\":{\"stringValue\":\"" + String(item.str6) + "\"},";
    body += "\"completedDate\":{\"stringValue\":\"Hari ini\"},";
    body += "\"createdAt\":{\"stringValue\":\"" + isoTime + "\"}";
    body += "}}";

    int httpCode = https.POST(body);
    if (httpCode == 200 || httpCode == 201) {
      Serial.printf("[Firebase Cloud] 🟢 Rapor Materi %s (+%d XP) tersimpan di Firestore!\n", item.str4, item.num2);
    } else {
      Serial.printf("[Firebase Cloud] ⚠️ Gagal POST assessment, HTTP Code: %d (%s)\n", httpCode, https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.println(F("[Firebase Cloud] ⚠️ Gagal https.begin() untuk assessment"));
  }
}

// Helper eksekusi HTTP PATCH Telemetry (Hanya dieksekusi di Core 0 background worker)
static void send_telemetry_http(float voltage, int tolerance_px, int volume) {
  vTaskDelay(pdMS_TO_TICKS(60));
  if (ESP.getMaxAllocHeap() < 28000) {
    return;
  }

  IPAddress targetIP;
  if (!WiFi.hostByName(FIRESTORE_HOST, targetIP)) {
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(12000);

  HTTPClient https;
  https.setTimeout(12000);
  https.setConnectTimeout(12000);
  https.setReuse(false);

  String url;
  url.reserve(190);
  url = String("https://") + FIRESTORE_HOST + "/v1/projects/" + FIREBASE_PROJECT_ID +
        "/databases/(default)/documents/telemetry/esp32_status?key=" + FIREBASE_API_KEY;

  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");
    https.addHeader("User-Agent", "OpenAksara-ESP32");
    https.addHeader("Connection", "close");

    String body;
    body.reserve(320);
    body = "{";
    body += "\"fields\":{";
    body += "\"device\":{\"stringValue\":\"" + String(DEVICE_ID) + "\"},";
    body += "\"voltage\":{\"doubleValue\":" + String(voltage, 2) + "},";
    body += "\"tolerancePx\":{\"integerValue\":" + String(tolerance_px) + "},";
    body += "\"volume\":{\"integerValue\":" + String(volume) + "},";
    body += "\"status\":{\"stringValue\":\"online\"},";
    body += "\"freeHeap\":{\"integerValue\":" + String(ESP.getFreeHeap()) + "}";
    body += "}}";

    int httpCode = https.sendRequest("PATCH", body);
    if (httpCode == 200 || httpCode == 201) {
      Serial.printf("[Firebase Cloud] 📡 Telemetri hardware tersinkronisasi (Heap: %u B)\n", (unsigned int)ESP.getFreeHeap());
    }
    https.end();
  }
}

// Background Worker Task di Core 0 (Terisolasi dari UI & sentuhan layar)
static void firebase_sync_worker_task(void* pvParameters) {
  (void)pvParameters;
  SyncQueueItem item;
  uint32_t last_reconnect = 0;

  for (;;) {
    // 1. Proses item antrian (timeout 250ms)
    if (sync_queue && xQueueReceive(sync_queue, &item, pdMS_TO_TICKS(250)) == pdTRUE) {
      if (WiFi.status() == WL_CONNECTED) {
        if (item.type == SYNC_ITEM_SUBMISSION) {
          send_submission_http(item);
        } else if (item.type == SYNC_ITEM_ASSESSMENT) {
          send_assessment_http(item);
        } else if (item.type == SYNC_ITEM_TELEMETRY) {
          send_telemetry_http(item.f1, item.num1, item.num2);
        }
      } else {
        Serial.println(F("[Firebase Worker] ⚠️ Wi-Fi BELUM terhubung! Data baru keluar via USB Serial saja."));
        Serial.println(F("[Firebase Worker] 💡 TIPS: Ganti 'NAMA_WIFI' & password di file include/secrets.h agar data otomatis masuk ke Firestore Cloud!"));
      }
    }

    uint32_t now = millis();

    // CATATAN PENTING: Heartbeat HTTPS berkala 30 detik DIHAPUS agar mbedTLS tidak tiba-tiba
    // menyedot ~40 KB heap di tengah permainan pembelajaran (seperti Memori Kartu) yang
    // menyebabkan lv_malloc_zeroed gagal (StoreProhibited 0x00000008).

    // 2. Rekoneksi background jika Wi-Fi terputus
    if (has_valid_wifi_config && WiFi.status() != WL_CONNECTED && (now - last_reconnect > 20000)) {
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
      configTime(7 * 3600, 0, "pool.ntp.org", "time.google.com");
    } else {
      Serial.println(F("[Wi-Fi] 🔴 Terputus dari jaringan Wi-Fi."));
    }
  }
}

bool firebase_sync_is_connected() {
  return (WiFi.status() == WL_CONNECTED);
}

/**
 * Kirim hasil latihan goresan siswa (Koleksi 'submissions')
 * NON-BLOCKING (< 0.1ms): Output Serial instan + Push ke Antrian Core 0
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

  // 2. Masukkan ke antrian background worker (Core 0) tanpa memblokir thread UI sama sekali
  if (sync_queue) {
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

    xQueueSend(sync_queue, &item, 0); // Non-blocking (0 tick timeout)
  }

  return true;
}

/**
 * Kirim capaian materi & perolehan XP (Koleksi 'assessments')
 * NON-BLOCKING (< 0.1ms)
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

  // 2. Masukkan ke antrian background worker (Core 0)
  if (sync_queue) {
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

    xQueueSend(sync_queue, &item, 0);
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
  if (sync_queue) {
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
