/**
 * OpenAksara EduCloud - IoT Monitoring Local Server
 * Zero-dependency Node.js HTTP & SSE Server
 * 
 * Endpoints:
 * - GET  /                 -> Serves dashboard UI (index.html)
 * - GET  /api/students     -> Get all student records
 * - POST /api/students     -> Add or update student record
 * - DELETE /api/students   -> Delete student
 * - POST /api/telemetry    -> ESP32 telemetry endpoint (live scores from hardware)
 * - GET  /api/events       -> Server-Sent Events (SSE) for instant realtime push
 */

const http = require('http');
const https = require('https');
const fs = require('fs');
const path = require('path');

// Load Environment Variables (.env) secara zero-dependency
function loadEnvFile() {
  const envCandidates = [
    path.join(__dirname, '.env'),
    path.join(__dirname, '..', '.env')
  ];
  const env = {};
  for (const envPath of envCandidates) {
    if (fs.existsSync(envPath)) {
      try {
        const content = fs.readFileSync(envPath, 'utf8');
        content.split('\n').forEach(line => {
          line = line.trim();
          if (!line || line.startsWith('#')) return;
          const idx = line.indexOf('=');
          if (idx !== -1) {
            const key = line.substring(0, idx).trim();
            const val = line.substring(idx + 1).trim().replace(/(^["']|["']$)/g, '');
            env[key] = val;
            if (!process.env[key]) process.env[key] = val;
          }
        });
        console.log(`[Env] Berhasil memuat konfigurasi dari: ${path.basename(envPath)}`);
        break;
      } catch (e) {
        console.warn(`[Env] Gagal membaca ${envPath}:`, e.message);
      }
    }
  }
  return env;
}
const localEnv = loadEnvFile();

const PORT = process.env.PORT || 3000;
const DATA_FILE = path.join(__dirname, 'students_data.json');
const TELEGRAM_CONFIG_FILE = path.join(__dirname, 'telegram_config.json');

// Telegram Configuration & Logs
let telegramConfig = loadTelegramConfig();
let telegramLogs = [
  {
    id: "TG-101",
    timestamp: new Date(Date.now() - 5 * 60000).toISOString(),
    type: "student_score",
    title: "Laporan Belajar Siswa (Aksara Bali)",
    recipient: "Grup Wali Murid Kelas 4A",
    status: "Delivered",
    content: "📱 <b>OpenAksara EduCloud • Laporan Belajar Siswa</b>\n━━━━━━━━━━━━━━━━━━━━━━━\n👤 <b>Nama:</b> I Putu Arya Wiguna\n🆔 <b>NISN:</b> 20260401 (Kelas 4A)\n🏷️ <b>Naskah:</b> Aksara Bali\n✍️ <b>Materi:</b> Aksara Ca (ᬘ)\n\n🏆 <b>Hasil Evaluasi:</b>\n• Skor Menulis: <b>96 / 100</b> (Grade A)\n• Akurasi Goresan: <b>95.8%</b> (Toleransi 18px)\n• Waktu Selesai: <b>12 detik</b>\n• Status: <b>TUNTAS BELAJAR ✅</b>\n\n💡 <i>Catatan: Lekukan loop aksara sangat halus dan konsisten.</i>\n━━━━━━━━━━━━━━━━━━━━━━━\n🤖 <i>Dikirim otomatis via OpenAksara ESP32 IoT</i>"
  }
];

function loadTelegramConfig() {
  try {
    if (fs.existsSync(TELEGRAM_CONFIG_FILE)) {
      return JSON.parse(fs.readFileSync(TELEGRAM_CONFIG_FILE, 'utf8'));
    }
  } catch (e) {
    console.warn('[Telegram] Gagal membaca telegram_config.json, menggunakan konfigurasi default');
  }
  return {
    is_dummy_mode: true,
    bot_token: "",
    chat_id: "",
    bot_name: "@OpenAksaraBot"
  };
}

function saveTelegramConfig(cfg) {
  try {
    fs.writeFileSync(TELEGRAM_CONFIG_FILE, JSON.stringify(cfg, null, 2), 'utf8');
  } catch (e) {
    console.error('[Telegram] Gagal menyimpan telegram_config.json:', e);
  }
}

// Default initial student dataset
const DEFAULT_STUDENTS = [
  {
    id: "STU-001",
    nisn: "20260401",
    name: "I Putu Arya Wiguna",
    class: "4A",
    gender: "L",
    device_id: "ESP32-BALI-01",
    score_bali: 94,
    score_jawa: 88,
    score_quiz: 92,
    accuracy: 94.5,
    speed_sec: 14,
    last_active: new Date(Date.now() - 5 * 60000).toISOString(),
    status: "Tuntas",
    best_bali: "Ca (ᬘ)",
    best_jawa: "Ha (ꦲ)",
    notes: "Sangat mahir goresan lengkung Aksara Bali. Perlu penguatan sedikit di Sandhangan Wulu.",
    history: [
      { timestamp: new Date(Date.now() - 10 * 60000).toISOString(), script: "bali", letter: "Ca", score: 96, accuracy: 95.8, duration: 12 },
      { timestamp: new Date(Date.now() - 25 * 60000).toISOString(), script: "bali", letter: "Na", score: 93, accuracy: 93.2, duration: 15 },
      { timestamp: new Date(Date.now() - 40 * 60000).toISOString(), script: "jawa", letter: "Ha", score: 88, accuracy: 89.0, duration: 18 }
    ]
  },
  {
    id: "STU-002",
    nisn: "20260402",
    name: "Ni Kadek Ayu Saraswati",
    class: "4A",
    gender: "P",
    device_id: "ESP32-BALI-02",
    score_bali: 98,
    score_jawa: 91,
    score_quiz: 96,
    accuracy: 97.2,
    speed_sec: 11,
    last_active: new Date(Date.now() - 2 * 60000).toISOString(),
    status: "Tuntas",
    best_bali: "Ulu Sari (◌ᬷ)",
    best_jawa: "Ra (ꦫ)",
    notes: "Akurasi goresan sangat konsisten, hampir mendekati master kaligrafi.",
    history: [
      { timestamp: new Date(Date.now() - 2 * 60000).toISOString(), script: "bali", letter: "Ulu Sari", score: 99, accuracy: 98.4, duration: 10 },
      { timestamp: new Date(Date.now() - 15 * 60000).toISOString(), script: "bali", letter: "Tedung", score: 97, accuracy: 96.0, duration: 11 }
    ]
  },
  {
    id: "STU-003",
    nisn: "20260403",
    name: "Raden Mas Banyu Wicaksono",
    class: "4A",
    gender: "L",
    device_id: "ESP32-JAWA-01",
    score_bali: 86,
    score_jawa: 96,
    score_quiz: 94,
    accuracy: 95.0,
    speed_sec: 13,
    last_active: new Date(Date.now() - 8 * 60000).toISOString(),
    status: "Tuntas",
    best_bali: "Ha (ᬳ)",
    best_jawa: "Taling Tarung (ꦺ◌ꦴ)",
    notes: "Spesialis aksara Jawa carakan dan sandhangan swara. Sangat teliti di bagian pasak.",
    history: [
      { timestamp: new Date(Date.now() - 8 * 60000).toISOString(), script: "jawa", letter: "Taling Tarung", score: 97, accuracy: 96.5, duration: 13 }
    ]
  },
  {
    id: "STU-004",
    nisn: "20260404",
    name: "Dewi Sekar Kinanthi",
    class: "4A",
    gender: "P",
    device_id: "ESP32-JAWA-02",
    score_bali: 84,
    score_jawa: 92,
    score_quiz: 90,
    accuracy: 91.5,
    speed_sec: 16,
    last_active: new Date(Date.now() - 12 * 60000).toISOString(),
    status: "Tuntas",
    best_bali: "Ra (ᬭ)",
    best_jawa: "Wulu (◌ꦶ)",
    notes: "Konsisten dan cepat memahami urutan checkpoints P1 ke Pn.",
    history: []
  },
  {
    id: "STU-005",
    nisn: "20260405",
    name: "I Komang Gede Danendra",
    class: "4A",
    gender: "L",
    device_id: "ESP32-BALI-03",
    score_bali: 78,
    score_jawa: 68,
    score_quiz: 72,
    accuracy: 74.0,
    speed_sec: 24,
    last_active: new Date(Date.now() - 18 * 60000).toISOString(),
    status: "Remedial",
    best_bali: "Ka (ᬓ)",
    best_jawa: "Ka (ꦏ)",
    notes: "Perlu bimbingan ekstra untuk lekukan Aksara Jawa dan kecepatan goresan di stylus.",
    history: [
      { timestamp: new Date(Date.now() - 18 * 60000).toISOString(), script: "jawa", letter: "Ca", score: 65, accuracy: 68.2, duration: 26 }
    ]
  },
  {
    id: "STU-006",
    nisn: "20260406",
    name: "Anindya Citra Lestari",
    class: "4B",
    gender: "P",
    device_id: "ESP32-BALI-04",
    score_bali: 90,
    score_jawa: 89,
    score_quiz: 91,
    accuracy: 92.8,
    speed_sec: 13,
    last_active: new Date(Date.now() - 3 * 60000).toISOString(),
    status: "Tuntas",
    best_bali: "Na (ᬦ)",
    best_jawa: "Da (ꦢ)",
    notes: "Nilai sangat seimbang di kedua rumpun aksara daerah.",
    history: []
  },
  {
    id: "STU-007",
    nisn: "20260407",
    name: "Bagus Satria Pratama",
    class: "4B",
    gender: "L",
    device_id: "ESP32-BALI-05",
    score_bali: 82,
    score_jawa: 74,
    score_quiz: 79,
    accuracy: 80.2,
    speed_sec: 19,
    last_active: new Date(Date.now() - 30 * 60000).toISOString(),
    status: "Tuntas",
    best_bali: "Da (ᬤ)",
    best_jawa: "Ta (ꦠ)",
    notes: "Menunjukkan progres peningkatan akurasi setelah latihan ulang 3 sesi.",
    history: []
  },
  {
    id: "STU-008",
    nisn: "20260408",
    name: "Ni Made Tari Melati",
    class: "4B",
    gender: "P",
    device_id: "ESP32-BALI-06",
    score_bali: 95,
    score_jawa: 94,
    score_quiz: 95,
    accuracy: 96.0,
    speed_sec: 12,
    last_active: new Date(Date.now() - 1 * 60000).toISOString(),
    status: "Tuntas",
    best_bali: "Taleng (ᬾ◌)",
    best_jawa: "Pepet (◌ꦼ)",
    notes: "Salah satu siswa berprestasi tertinggi di kelas 4B.",
    history: []
  },
  {
    id: "STU-009",
    nisn: "20260409",
    name: "Muhammad Rizky Fauzi",
    class: "5A",
    gender: "L",
    device_id: "ESP32-JAWA-03",
    score_bali: 71,
    score_jawa: 85,
    score_quiz: 80,
    accuracy: 82.5,
    speed_sec: 20,
    last_active: new Date(Date.now() - 45 * 60000).toISOString(),
    status: "Tuntas",
    best_bali: "Ha (ᬳ)",
    best_jawa: "Na (ꦤ)",
    notes: "Fokus belajar di Aksara Jawa sangat baik, aksara Bali perlu latihan tambahan.",
    history: []
  },
  {
    id: "STU-010",
    nisn: "20260410",
    name: "I Wayan Aditya Wardana",
    class: "5A",
    gender: "L",
    device_id: "ESP32-BALI-07",
    score_bali: 97,
    score_jawa: 86,
    score_quiz: 93,
    accuracy: 94.0,
    speed_sec: 12,
    last_active: new Date(Date.now() - 4 * 60000).toISOString(),
    status: "Tuntas",
    best_bali: "Suku Ilut (◌ᬹ)",
    best_jawa: "Ha (ꦲ)",
    notes: "Cepat dalam menyelesaikan goresan berantai dan toleransi radius.",
    history: []
  },
  {
    id: "STU-011",
    nisn: "20260411",
    name: "Siti Rahma Azzahra",
    class: "5A",
    gender: "P",
    device_id: "ESP32-JAWA-04",
    score_bali: 88,
    score_jawa: 95,
    score_quiz: 94,
    accuracy: 94.8,
    speed_sec: 14,
    last_active: new Date(Date.now() - 15 * 60000).toISOString(),
    status: "Tuntas",
    best_bali: "Ta (ᬢ)",
    best_jawa: "Suku Mendut (◌ꦹ)",
    notes: "Memiliki ketelitian tinggi di tanda vokal panjang (dirga / mendut).",
    history: []
  },
  {
    id: "STU-012",
    nisn: "20260412",
    name: "Dimas Panji Nugroho",
    class: "5A",
    gender: "L",
    device_id: "ESP32-JAWA-05",
    score_bali: 65,
    score_jawa: 70,
    score_quiz: 68,
    accuracy: 71.2,
    speed_sec: 27,
    last_active: new Date(Date.now() - 50 * 60000).toISOString(),
    status: "Remedial",
    best_bali: "Ha (ᬳ)",
    best_jawa: "Ra (ꦫ)",
    notes: "Sering keluar jalur koridor toleransi (24px). Disarankan latihan bertahap dengan mode panduan titik tebal.",
    history: []
  }
];

// In-memory data store
let students = loadData();

// Connected SSE clients
let sseClients = [];

function loadData() {
  try {
    if (fs.existsSync(DATA_FILE)) {
      const content = fs.readFileSync(DATA_FILE, 'utf8');
      return JSON.parse(content);
    }
  } catch (err) {
    console.error('[Storage] Gagal membaca data file, menggunakan data default:', err.message);
  }
  saveData(DEFAULT_STUDENTS);
  return DEFAULT_STUDENTS;
}

function saveData(data) {
  try {
    fs.writeFileSync(DATA_FILE, JSON.stringify(data, null, 2), 'utf8');
  } catch (err) {
    console.error('[Storage] Gagal menyimpan data:', err.message);
  }
}

function broadcastEvent(type, payload) {
  const message = `event: ${type}\ndata: ${JSON.stringify(payload)}\n\n`;
  sseClients.forEach(res => {
    try {
      res.write(message);
    } catch (e) {
      // client disconnected
    }
  });
}

// MIME types lookup
const MIME_TYPES = {
  '.html': 'text/html; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.js': 'application/javascript; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.png': 'image/png',
  '.jpg': 'image/jpeg',
  '.jpeg': 'image/jpeg',
  '.gif': 'image/gif',
  '.svg': 'image/svg+xml',
  '.ico': 'image/x-icon',
  '.ttf': 'font/ttf',
  '.woff': 'font/woff',
  '.woff2': 'font/woff2',
  '.stl': 'model/stl',
  '.pdf': 'application/pdf'
};

const server = http.createServer((req, res) => {
  // CORS Headers
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type, Authorization');

  if (req.method === 'OPTIONS') {
    res.writeHead(204);
    res.end();
    return;
  }

  const parsedUrl = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
  const pathname = parsedUrl.pathname;

  // 1. SSE Stream endpoint: /api/events
  if (pathname === '/api/events' && req.method === 'GET') {
    res.writeHead(200, {
      'Content-Type': 'text/event-stream',
      'Cache-Control': 'no-cache',
      'Connection': 'keep-alive'
    });
    res.write('event: connected\ndata: {"status":"connected"}\n\n');
    sseClients.push(res);
    console.log(`[SSE] Dashboard client terhubung. Total client aktif: ${sseClients.length}`);

    req.on('close', () => {
      sseClients = sseClients.filter(c => c !== res);
      console.log(`[SSE] Dashboard client terputus. Sisa client: ${sseClients.length}`);
    });
    return;
  }

  // 1b. API: GET /api/config (Safe Firebase Web Config from .env)
  if (pathname === '/api/config' && req.method === 'GET') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({
      success: true,
      firebase: {
        apiKey: process.env.FIREBASE_API_KEY || localEnv.FIREBASE_API_KEY || '',
        authDomain: process.env.FIREBASE_AUTH_DOMAIN || localEnv.FIREBASE_AUTH_DOMAIN || '',
        projectId: process.env.FIREBASE_PROJECT_ID || localEnv.FIREBASE_PROJECT_ID || '',
        storageBucket: process.env.FIREBASE_STORAGE_BUCKET || localEnv.FIREBASE_STORAGE_BUCKET || '',
        messagingSenderId: process.env.FIREBASE_MESSAGING_SENDER_ID || localEnv.FIREBASE_MESSAGING_SENDER_ID || '',
        appId: process.env.FIREBASE_APP_ID || localEnv.FIREBASE_APP_ID || ''
      }
    }));
    return;
  }

  // 2. API: GET /api/students (Urutkan dari aktivitas terbaru ke terlama)
  if (pathname === '/api/students' && req.method === 'GET') {
    const sortedStudents = students.slice().sort((a, b) => {
      const ta = a.last_active ? Date.parse(a.last_active) : 0;
      const tb = b.last_active ? Date.parse(b.last_active) : 0;
      return tb - ta;
    });
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ success: true, data: sortedStudents }));
    return;
  }

  // 3. API: POST /api/students (Add or update student)
  if (pathname === '/api/students' && req.method === 'POST') {
    let body = '';
    req.on('data', chunk => body += chunk);
    req.on('end', () => {
      try {
        const studentData = JSON.parse(body);
        if (!studentData.id) {
          studentData.id = "STU-" + String(Date.now()).slice(-6);
        }
        studentData.last_active = new Date().toISOString();

        const existingIndex = students.findIndex(s => s.id === studentData.id || s.nisn === studentData.nisn);
        if (existingIndex >= 0) {
          students[existingIndex] = { ...students[existingIndex], ...studentData };
        } else {
          students.unshift(studentData);
        }
        saveData(students);
        broadcastEvent('student_updated', { student: studentData });

        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ success: true, student: studentData }));
      } catch (err) {
        res.writeHead(400, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ success: false, error: err.message }));
      }
    });
    return;
  }

  // 4. API: DELETE /api/students?id=STU-XXX
  if (pathname === '/api/students' && req.method === 'DELETE') {
    const id = parsedUrl.searchParams.get('id');
    if (id) {
      students = students.filter(s => s.id !== id);
      saveData(students);
      broadcastEvent('student_deleted', { id });
      res.writeHead(200, { 'Content-Type': 'application/json' });
      res.end(JSON.stringify({ success: true, id }));
    } else {
      res.writeHead(400, { 'Content-Type': 'application/json' });
      res.end(JSON.stringify({ success: false, error: 'Parameter id diperlukan' }));
    }
    return;
  }

  // 5. API: POST /api/telemetry (Incoming live score from ESP32)
  if (pathname === '/api/telemetry' && req.method === 'POST') {
    let body = '';
    req.on('data', chunk => body += chunk);
    req.on('end', () => {
      try {
        const telemetry = JSON.parse(body);
        // Format expected:
        // {
        //   "device_id": "ESP32-BALI-01",
        //   "nisn": "20260401",          // or student_id
        //   "script": "bali" | "jawa",
        //   "letter": "Ca",
        //   "score": 95,
        //   "accuracy": 94.2,
        //   "duration_sec": 14,
        //   "battery": 87,
        //   "rssi": -62
        // }

        console.log(`[Telemetry] Diterima dari ${telemetry.device_id || 'ESP32'}:`, telemetry);

        let targetStudent = null;
        if (telemetry.nisn) {
          targetStudent = students.find(s => s.nisn === telemetry.nisn);
        } else if (telemetry.student_id) {
          targetStudent = students.find(s => s.id === telemetry.student_id);
        } else if (telemetry.device_id) {
          targetStudent = students.find(s => s.device_id === telemetry.device_id);
        }

        if (targetStudent) {
          targetStudent.last_active = new Date().toISOString();
          if (telemetry.script === 'bali') {
            targetStudent.score_bali = Math.round((targetStudent.score_bali * 0.7) + (telemetry.score * 0.3));
            if (telemetry.score >= 90) targetStudent.best_bali = `${telemetry.letter}`;
          } else if (telemetry.script === 'jawa') {
            targetStudent.score_jawa = Math.round((targetStudent.score_jawa * 0.7) + (telemetry.score * 0.3));
            if (telemetry.score >= 90) targetStudent.best_jawa = `${telemetry.letter}`;
          }

          if (telemetry.accuracy) {
            targetStudent.accuracy = parseFloat(((targetStudent.accuracy * 0.8) + (telemetry.accuracy * 0.2)).toFixed(1));
          }
          if (telemetry.duration_sec) {
            targetStudent.speed_sec = Math.round((targetStudent.speed_sec + telemetry.duration_sec) / 2);
          }

          // Update status
          const finalScore = (targetStudent.score_bali + targetStudent.score_jawa + targetStudent.score_quiz) / 3;
          targetStudent.status = finalScore >= 75 ? "Tuntas" : "Remedial";

          if (!targetStudent.history) targetStudent.history = [];
          targetStudent.history.unshift({
            timestamp: new Date().toISOString(),
            script: telemetry.script || 'bali',
            letter: telemetry.letter || 'Aksara',
            score: telemetry.score || 0,
            accuracy: telemetry.accuracy || 0,
            duration: telemetry.duration_sec || 0
          });
          if (targetStudent.history.length > 20) targetStudent.history.pop();

          saveData(students);
          broadcastEvent('telemetry_received', {
            telemetry,
            student: targetStudent
          });

          // Auto-trigger telegram notification if student gets high score or needs remedial
          if (telemetry.score >= 95 || telemetry.score < 75) {
            const isHigh = telemetry.score >= 95;
            const autoTgMsg = {
              id: "TG-" + Date.now().toString().slice(-6),
              timestamp: new Date().toISOString(),
              type: isHigh ? 'high_score' : 'remedial_alert',
              title: isHigh ? `Prestasi Baru - ${targetStudent.name}` : `Peringatan Remedial - ${targetStudent.name}`,
              recipient: `Grup EduCloud Kelas ${targetStudent.class}`,
              status: "Delivered",
              is_dummy: telegramConfig.is_dummy_mode !== false || !telegramConfig.bot_token,
              content: isHigh
                ? `🌟 <b>PRESTASI MENULIS AKSARA DAERAH!</b>\n━━━━━━━━━━━━━━━━━━━━━━━\n👤 <b>Nama:</b> ${targetStudent.name}\n🆔 <b>NISN:</b> ${targetStudent.nisn} (Kelas ${targetStudent.class})\n✍️ <b>Aksara:</b> ${telemetry.letter} (${telemetry.script === 'bali' ? 'Bali' : 'Jawa'})\n🏆 <b>Skor Sempurna:</b> ${telemetry.score} / 100\n🎯 <b>Akurasi Goresan:</b> ${telemetry.accuracy}%\n⏱️ <b>Waktu:</b> ${telemetry.duration_sec} detik\n━━━━━━━━━━━━━━━━━━━━━━━\n🤖 <i>Notifikasi otomatis OpenAksara ESP32</i>`
                : `⚠️ <b>PERINGATAN REMEDIAL GOJESAN</b>\n━━━━━━━━━━━━━━━━━━━━━━━\n👤 <b>Nama:</b> ${targetStudent.name}\n🆔 <b>NISN:</b> ${targetStudent.nisn} (Kelas ${targetStudent.class})\n✍️ <b>Aksara:</b> ${telemetry.letter} (${telemetry.script === 'bali' ? 'Bali' : 'Jawa'})\n📉 <b>Skor:</b> ${telemetry.score} / 100 (Di bawah KKM 75)\n🎯 <b>Akurasi:</b> ${telemetry.accuracy}%\n━━━━━━━━━━━━━━━━━━━━━━━\n📌 <i>Saran: Latihan ulang menggunakan panduan garis tebal.</i>`
            };
            telegramLogs.unshift(autoTgMsg);
            if (telegramLogs.length > 30) telegramLogs.pop();
            broadcastEvent('telegram_message_sent', autoTgMsg);
          }

          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end(JSON.stringify({ success: true, message: "Telemetri berhasil dicatat", student_name: targetStudent.name }));
        } else {
          // Device belum diasosiasikan ke siswa tertentu
          broadcastEvent('unassigned_telemetry', telemetry);
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end(JSON.stringify({ success: true, message: "Telemetri diterima (perangkat belum terdaftar pada siswa)" }));
        }
      } catch (err) {
        res.writeHead(400, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ success: false, error: err.message }));
      }
    });
    return;
  }

  // 6. API: GET /api/telegram/config
  if (pathname === '/api/telegram/config' && req.method === 'GET') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({
      success: true,
      config: {
        is_dummy_mode: telegramConfig.is_dummy_mode !== false,
        has_token: !!telegramConfig.bot_token,
        chat_id: telegramConfig.chat_id || '',
        bot_name: telegramConfig.bot_name || '@OpenAksaraBot'
      }
    }));
    return;
  }

  // 7. API: POST /api/telegram/config
  if (pathname === '/api/telegram/config' && req.method === 'POST') {
    let body = '';
    req.on('data', chunk => body += chunk);
    req.on('end', () => {
      try {
        const payload = JSON.parse(body);
        if (payload.is_dummy_mode !== undefined) telegramConfig.is_dummy_mode = !!payload.is_dummy_mode;
        if (payload.bot_token !== undefined) telegramConfig.bot_token = payload.bot_token.trim();
        if (payload.chat_id !== undefined) telegramConfig.chat_id = payload.chat_id.trim();
        if (payload.bot_name !== undefined) telegramConfig.bot_name = payload.bot_name.trim();
        saveTelegramConfig(telegramConfig);
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ success: true, message: "Konfigurasi Telegram disimpan" }));
      } catch (e) {
        res.writeHead(400, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ success: false, error: e.message }));
      }
    });
    return;
  }

  // 8. API: GET /api/telegram/logs
  if (pathname === '/api/telegram/logs' && req.method === 'GET') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ success: true, logs: telegramLogs }));
    return;
  }

  // 9. API: POST /api/telegram/send
  if (pathname === '/api/telegram/send' && req.method === 'POST') {
    let body = '';
    req.on('data', chunk => body += chunk);
    req.on('end', () => {
      try {
        const payload = JSON.parse(body);
        let messageText = payload.message || '';
        let title = payload.title || 'Notifikasi OpenAksara';
        let recipient = payload.recipient || (telegramConfig.chat_id || 'Grup EduCloud Siswa');

        // Generate template otomatis jika belum disediakan
        if (!messageText && payload.type === 'student_score' && payload.student_id) {
          const student = students.find(s => s.id === payload.student_id);
          if (student) {
            const finalScore = Math.round((student.score_bali + student.score_jawa + student.score_quiz) / 3);
            const isPass = finalScore >= 75;
            title = `Laporan Belajar - ${student.name}`;
            messageText = `📱 <b>OpenAksara EduCloud • Laporan Belajar Siswa</b>\n━━━━━━━━━━━━━━━━━━━━━━━\n👤 <b>Nama:</b> ${student.name}\n🆔 <b>NISN:</b> ${student.nisn} (Kelas ${student.class})\n🏷️ <b>Aksara Bali:</b> ${student.score_bali} Poin\n🏷️ <b>Aksara Jawa:</b> ${student.score_jawa} Poin\n📝 <b>Kuis Teori:</b> ${student.score_quiz} Poin\n\n🏆 <b>Nilai Akhir: ${finalScore} / 100</b>\n🎯 <b>Akurasi Goresan:</b> ${student.accuracy}%\n⏱️ <b>Rata-rata Waktu:</b> ${student.speed_sec} detik\nStatus: <b>${isPass ? 'TUNTAS BELAJAR ✅' : 'PERLU REMEDIAL ⚠️'}</b>\n\n💡 <i>Catatan Guru: ${student.notes || 'Siswa menunjukkan progres baik.'}</i>\n━━━━━━━━━━━━━━━━━━━━━━━\n🤖 <i>Dikirim otomatis via Tablet OpenAksara ESP32 IoT</i>`;
          }
        } else if (!messageText && payload.type === 'class_summary') {
          title = "Laporan Rekapitulasi Kelas Harian";
          const total = students.length;
          const avgScore = (students.reduce((a, b) => a + ((b.score_bali + b.score_jawa + b.score_quiz) / 3), 0) / total).toFixed(1);
          const avgAcc = (students.reduce((a, b) => a + b.accuracy, 0) / total).toFixed(1);
          const passCount = students.filter(s => ((s.score_bali + s.score_jawa + s.score_quiz) / 3) >= 75).length;
          messageText = `📊 <b>OpenAksara EduCloud • Ringkasan Belajar Harian</b>\n━━━━━━━━━━━━━━━━━━━━━━━\n🏫 <b>Sekolah:</b> SDN 1 Tuban • Semester Genap 2026/2027\n👥 <b>Total Siswa:</b> ${total} Siswa Terdata\n🏆 <b>Rata-Rata Nilai:</b> ${avgScore} / 100\n🎯 <b>Rata-Rata Akurasi:</b> ${avgAcc}%\n✅ <b>Siswa Tuntas:</b> ${passCount} Siswa (${((passCount/total)*100).toFixed(1)}%)\n⚠️ <b>Siswa Remedial:</b> ${total - passCount} Siswa\n\n🔍 <b>Aksara Perlu Bimbingan Ekstra:</b>\n1. Aksara Bali: <b>Ca (ᬘ)</b> (Deviasi 34%)\n2. Aksara Jawa: <b>Taling Tarung (ꦺ◌ꦴ)</b> (Deviasi 31%)\n━━━━━━━━━━━━━━━━━━━━━━━\n🤖 <i>Laporan otomatis harian untuk Guru & Wali Murid</i>`;
        } else if (!messageText && payload.type === 'remedial_alert') {
          title = "Peringatan Siswa Perlu Remedial";
          const remedialList = students.filter(s => ((s.score_bali + s.score_jawa + s.score_quiz) / 3) < 75);
          const student = remedialList[0] || students[0];
          messageText = `⚠️ <b>PERINGATAN SISWA PERLU REMEDIAL</b>\n━━━━━━━━━━━━━━━━━━━━━━━\n👤 <b>Nama:</b> ${student.name}\n🆔 <b>NISN:</b> ${student.nisn} • Kelas ${student.class}\n📱 <b>Perangkat:</b> ${student.device_id || 'ESP32'}\n\n📉 <b>Hasil Evaluasi:</b>\n• Skor Rata-rata: <b>${Math.round((student.score_bali + student.score_jawa + student.score_quiz)/3)} / 100</b> (KKM: 75)\n• Akurasi Goresan: <b>${student.accuracy}%</b>\n• Status: <b>REMEDIAL DIPERLUKAN ⚠️</b>\n\n📌 <i>Rekomendasi: Latihan ulang menggunakan mode bantuan garis pemandu di tablet OpenAksara.</i>\n━━━━━━━━━━━━━━━━━━━━━━━\n🤖 <i>Notifikasi Otomatis Terusan ke Guru & Orang Tua Siswa</i>`;
        }

        const logEntry = {
          id: "TG-" + Date.now().toString().slice(-6),
          timestamp: new Date().toISOString(),
          type: payload.type || 'custom',
          title,
          recipient,
          status: "Delivered",
          is_dummy: telegramConfig.is_dummy_mode !== false || !telegramConfig.bot_token,
          content: messageText
        };

        telegramLogs.unshift(logEntry);
        if (telegramLogs.length > 30) telegramLogs.pop();

        // Broadcast ke semua dashboard yang sedang terhubung melalui SSE
        broadcastEvent('telegram_message_sent', logEntry);

        // Jika Bot Token riil disediakan, kirim via HTTPS
        if (!telegramConfig.is_dummy_mode && telegramConfig.bot_token && telegramConfig.chat_id) {
          sendActualTelegramMessage(telegramConfig.bot_token, telegramConfig.chat_id, messageText, (err, resData) => {
            if (err) {
              console.error("[Telegram API Error]", err.message);
              logEntry.status = "Failed";
            } else {
              console.log("[Telegram API Success] Terkirim ke chat ID:", telegramConfig.chat_id);
            }
          });
        }

        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({
          success: true,
          mode: logEntry.is_dummy ? 'dummy' : 'live',
          log: logEntry
        }));
      } catch (err) {
        res.writeHead(400, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ success: false, error: err.message }));
      }
    });
    return;
  }

  // 10. Static File Serving (Terintegrasi: web, 3D viewer, & waypoint editor)
  const repoRoot = path.resolve(__dirname, '..');
  const viewerDir = path.join(repoRoot, '3d_viewer');
  const editorDir = path.join(repoRoot, 'waypoint_editor');

  let filePath = '';
  if (pathname === '/3d' || pathname === '/3d/' || pathname === '/3d-viewer' || pathname === '/3d_viewer') {
    filePath = path.join(viewerDir, 'index.html');
  } else if (pathname.startsWith('/3d_viewer/')) {
    filePath = path.join(repoRoot, pathname.replace(/^\/+/, ''));
  } else if (pathname === '/waypoint' || pathname === '/waypoint/' || pathname === '/waypoint-editor' || pathname === '/waypoint_editor' || pathname === '/editor') {
    filePath = path.join(editorDir, 'index.html');
  } else if (pathname.startsWith('/waypoint_editor/')) {
    filePath = path.join(repoRoot, pathname.replace(/^\/+/, ''));
  } else {
    filePath = path.join(__dirname, pathname === '/' ? 'index.html' : pathname);
  }

  // Security: Cegah directory traversal di luar folder yang diizinkan
  const normalizedFilePath = path.normalize(filePath);
  const isAllowedPath = normalizedFilePath.startsWith(__dirname) ||
                        normalizedFilePath.startsWith(viewerDir) ||
                        normalizedFilePath.startsWith(editorDir);

  if (!isAllowedPath) {
    res.writeHead(403);
    res.end('Access Denied');
    return;
  }

  fs.stat(filePath, (err, stats) => {
    if (err || !stats.isFile()) {
      res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
      res.end('404 Not Found - Halaman atau berkas tidak ditemukan.');
      return;
    }

    const ext = path.extname(filePath).toLowerCase();
    const contentType = MIME_TYPES[ext] || 'application/octet-stream';

    res.writeHead(200, { 'Content-Type': contentType });
    const readStream = fs.createReadStream(filePath);
    readStream.pipe(res);
  });
});

function sendActualTelegramMessage(botToken, chatId, text, callback) {
  const postData = JSON.stringify({
    chat_id: chatId,
    text: text,
    parse_mode: 'HTML'
  });

  const options = {
    hostname: 'api.telegram.org',
    port: 443,
    path: `/bot${botToken}/sendMessage`,
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Content-Length': Buffer.byteLength(postData)
    }
  };

  const req = https.request(options, (res) => {
    let body = '';
    res.on('data', chunk => body += chunk);
    res.on('end', () => {
      try {
        const json = JSON.parse(body);
        if (json.ok) callback(null, json);
        else callback(new Error(json.description || 'Telegram API Error'));
      } catch (e) {
        callback(e);
      }
    });
  });

  req.on('error', (e) => {
    callback(e);
  });

  req.write(postData);
  req.end();
}

server.listen(PORT, () => {
  console.log('================================================================');
  console.log('🚀 OpenAksara EduCloud - IoT Monitoring Server AKTIF!');
  console.log(`🌐 Dashboard Guru   : http://localhost:${PORT}`);
  console.log(`📡 ESP32 Telemetry  : http://localhost:${PORT}/api/telemetry`);
  console.log('================================================================');
});

