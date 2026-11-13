/**
 * OpenAksara EduCloud - Dashboard Application Logic
 * Integrates LocalStorage, REST API, Server-Sent Events (SSE), Chart.js, and Telegram Bot Notification System
 */

// Global State
let studentsData = [];
let filteredStudents = [];
let currentFilterClass = 'ALL';
let currentFilterStatus = 'ALL';
let currentSearchQuery = '';
let currentSortKey = 'final_score';
let currentSortAsc = false;
let isConnectedToServer = false;
let sseSource = null;

// Chart Instances
let comparisonChart = null;
let gradeDistributionChart = null;
let studentRadarChart = null;

// Initial Fallback Data (if opening directly without server)
const INITIAL_STUDENTS = [
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
    last_active: new Date(Date.now() - 3 * 60000).toISOString(),
    status: "Tuntas",
    best_bali: "Ca (ᬘ)",
    best_jawa: "Ha (ꦲ)",
    notes: "Sangat mahir goresan lengkung Aksara Bali. Perlu penguatan sedikit di Sandhangan Wulu.",
    history: [
      { timestamp: new Date(Date.now() - 3 * 60000).toISOString(), script: "bali", letter: "Ca", score: 96, accuracy: 95.8, duration: 12 },
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
    last_active: new Date(Date.now() - 4 * 60000).toISOString(),
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
    last_active: new Date(Date.now() - 5 * 60000).toISOString(),
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

// Aksara letter sample options for simulator
const BALI_SIM_LETTERS = [
  { name: "Ca (ᬘ)", value: "Ca" },
  { name: "Ha (ᬳ)", value: "Ha" },
  { name: "Na (ᬦ)", value: "Na" },
  { name: "Ra (ᬭ)", value: "Ra" },
  { name: "Ka (ᬓ)", value: "Ka" },
  { name: "Tedung (◌ᬵ)", value: "Tedung" },
  { name: "Ulu (◌ᬶ)", value: "Ulu" },
  { name: "Suku (◌ᬸ)", value: "Suku" }
];

const JAWA_SIM_LETTERS = [
  { name: "Ha (ꦲ)", value: "Ha" },
  { name: "Na (ꦤ)", value: "Na" },
  { name: "Ca (ꦕ)", value: "Ca" },
  { name: "Ra (ꦫ)", value: "Ra" },
  { name: "Ka (ꦏ)", value: "Ka" },
  { name: "Wulu (◌ꦶ)", value: "Wulu" },
  { name: "Suku (◌ꦸ)", value: "Suku" },
  { name: "Taling Tarung (ꦺ◌ꦴ)", value: "Taling Tarung" }
];

// Difficulty Heatmap Data
const DIFFICULTY_LETTERS = [
  { script: "Bali", name: "Ca", glyph: "ᬘ", errRate: "34%", note: "Lekukan loop ganda sering terputus" },
  { script: "Jawa", name: "Taling Tarung", glyph: "ꦺ◌ꦴ", errRate: "31%", note: "Urutan penempatan mengapit konsonan" },
  { script: "Bali", name: "Tedung", glyph: "◌ᬵ", errRate: "28%", note: "Jarak spasi di belakang huruf" },
  { script: "Jawa", name: "Ca", glyph: "ꦕ", errRate: "26%", note: "Kemiripan lengkung dengan aksara Wa" },
  { script: "Bali", name: "Ulu Sari", glyph: "◌ᬷ", errRate: "22%", note: "Lingkaran mahkota atas terpotong" },
  { script: "Jawa", name: "Suku Mendut", glyph: "◌ꦹ", errRate: "19%", note: "Ekor ganda di bawah garis alas" }
];

// Telegram State
let telegramLogs = [
  {
    id: "TG-101",
    timestamp: new Date(Date.now() - 5 * 60000).toISOString(),
    type: "student_score",
    title: "Laporan Belajar Siswa (Aksara Bali)",
    recipient: "Grup Wali Murid Kelas 4A",
    status: "Delivered",
    is_dummy: true,
    content: "📱 <b>OpenAksara EduCloud • Laporan Belajar Siswa</b>\n━━━━━━━━━━━━━━━━━━━━━━━\n👤 <b>Nama:</b> I Putu Arya Wiguna\n🆔 <b>NISN:</b> 20260401 (Kelas 4A)\n🏷️ <b>Naskah:</b> Aksara Bali\n✍️ <b>Materi:</b> Aksara Ca (ᬘ)\n\n🏆 <b>Hasil Evaluasi:</b>\n• Skor Menulis: <b>96 / 100</b> (Grade A)\n• Akurasi Goresan: <b>95.8%</b> (Toleransi 18px)\n• Waktu Selesai: <b>12 detik</b>\n• Status: <b>TUNTAS BELAJAR ✅</b>\n\n💡 <i>Catatan: Lekukan loop aksara sangat halus dan konsisten.</i>\n━━━━━━━━━━━━━━━━━━━━━━━\n🤖 <i>Dikirim otomatis via OpenAksara ESP32 IoT</i>"
  }
];

let telegramConfig = {
  is_dummy_mode: true,
  bot_token: "",
  chat_id: "",
  bot_name: "@OpenAksaraBot"
};

// Initialize Application
document.addEventListener("DOMContentLoaded", () => {
  lucide.createIcons();
  initTheme();
  initClock();
  initDataAndConnection();
  renderDifficultyRanking();
  updateSimLetters();
});

// Realtime Clock in Header
function initClock() {
  const updateClock = () => {
    const now = new Date();
    const clockEl = document.getElementById("lastUpdatedClock");
    if (clockEl) {
      clockEl.textContent = now.toLocaleTimeString('id-ID', { hour: '2-digit', minute: '2-digit', second: '2-digit' }) + ' WIB';
    }
  };
  updateClock();
  setInterval(updateClock, 1000);
}

// Fetch data from server or localStorage
async function initDataAndConnection() {
  const badge = document.getElementById("connectionBadge");
  const text = document.getElementById("connectionText");

  try {
    const res = await fetch('/api/students');
    if (res.ok) {
      const json = await res.json();
      if (json.success && Array.isArray(json.data) && json.data.length > 0) {
        studentsData = json.data;
        isConnectedToServer = true;
        badge.className = "inline-block w-2 h-2 rounded-full bg-emerald-400 live-pulse";
        text.textContent = "Terhubung ke IoT Server (Port 3000)";
        initSSE();
      }
    } else {
      throw new Error("Server not responding to API");
    }
  } catch (err) {
    console.warn("[EduCloud] Tidak terhubung ke server backend, menggunakan penyimpanan lokal (LocalStorage).");
    isConnectedToServer = false;
    badge.className = "inline-block w-2 h-2 rounded-full bg-amber-400";
    text.textContent = "Mode Offline Standalone (LocalStorage)";

    const localSaved = localStorage.getItem("openaksara_students");
    if (localSaved) {
      try {
        studentsData = JSON.parse(localSaved);
      } catch (e) {
        studentsData = INITIAL_STUDENTS;
      }
    } else {
      studentsData = INITIAL_STUDENTS;
      localStorage.setItem("openaksara_students", JSON.stringify(studentsData));
    }
  }

  applyFiltersAndRender();
  initCharts();
  renderLiveActivity();
}

// Setup Server-Sent Events (SSE) for Realtime Updates from ESP32 & Telegram
function initSSE() {
  if (sseSource) sseSource.close();
  try {
    sseSource = new EventSource('/api/events');
    
    sseSource.addEventListener('telemetry_received', (e) => {
      const data = JSON.parse(e.data);
      handleIncomingTelemetry(data.telemetry, data.student);
    });

    sseSource.addEventListener('student_updated', (e) => {
      const data = JSON.parse(e.data);
      const idx = studentsData.findIndex(s => s.id === data.student.id);
      if (idx >= 0) studentsData[idx] = data.student;
      else studentsData.unshift(data.student);
      applyFiltersAndRender();
      updateCharts();
    });

    sseSource.addEventListener('student_deleted', (e) => {
      const data = JSON.parse(e.data);
      studentsData = studentsData.filter(s => s.id !== data.id);
      applyFiltersAndRender();
      updateCharts();
    });

    sseSource.addEventListener('telegram_message_sent', (e) => {
      const data = JSON.parse(e.data);
      handleTelegramMessageReceived(data);
    });

    sseSource.onerror = () => {
      console.warn("[SSE] Koneksi SSE terputus, mencoba lagi...");
    };
  } catch (err) {
    console.error("[SSE] Gagal menginisialisasi EventSource:", err);
  }
}

// Handle incoming live telemetry (from real ESP32 or Simulator)
function handleIncomingTelemetry(telemetry, student) {
  playSuccessChime();

  const idx = studentsData.findIndex(s => s.id === student.id || s.nisn === student.nisn);
  if (idx >= 0) {
    studentsData[idx] = student;
  } else {
    studentsData.unshift(student);
  }

  if (!isConnectedToServer) {
    localStorage.setItem("openaksara_students", JSON.stringify(studentsData));
  }

  applyFiltersAndRender();
  updateCharts();
  renderLiveActivity();

  showToast(`⚡ Skor Baru dari ${student.name}: Menulis Aksara ${telemetry.letter} (${telemetry.score} Poin, Akurasi ${telemetry.accuracy}%)`, 'success');

  if (telemetry.score >= 95) {
    triggerConfetti();
  }
}

// Calculation & Filtering
function calculateFinalScore(s) {
  return Math.round(((s.score_bali || 0) + (s.score_jawa || 0) + (s.score_quiz || 0)) / 3);
}

function getGradeLetter(score) {
  if (score >= 90) return { letter: 'A', text: 'Sangat Baik', color: 'text-emerald-400', bg: 'bg-emerald-500/20' };
  if (score >= 80) return { letter: 'B', text: 'Baik', color: 'text-sky-400', bg: 'bg-sky-500/20' };
  if (score >= 75) return { letter: 'C', text: 'Cukup', color: 'text-amber-400', bg: 'bg-amber-500/20' };
  return { letter: 'D', text: 'Remedial', color: 'text-rose-400', bg: 'bg-rose-500/20' };
}

function applyFiltersAndRender() {
  filteredStudents = studentsData.filter(s => {
    if (currentFilterClass !== 'ALL' && s.class !== currentFilterClass) return false;

    if (currentFilterStatus !== 'ALL') {
      const final = calculateFinalScore(s);
      const isPass = final >= 75;
      if (currentFilterStatus === 'Tuntas' && !isPass) return false;
      if (currentFilterStatus === 'Remedial' && isPass) return false;
    }

    if (currentSearchQuery) {
      const q = currentSearchQuery.toLowerCase();
      const matchName = (s.name || '').toLowerCase().includes(q);
      const matchNisn = (s.nisn || '').toLowerCase().includes(q);
      const matchDevice = (s.device_id || '').toLowerCase().includes(q);
      if (!matchName && !matchNisn && !matchDevice) return false;
    }

    return true;
  });

  filteredStudents.sort((a, b) => {
    let valA, valB;
    if (currentSortKey === 'final_score') {
      valA = calculateFinalScore(a);
      valB = calculateFinalScore(b);
    } else {
      valA = a[currentSortKey];
      valB = b[currentSortKey];
    }

    if (typeof valA === 'string') {
      return currentSortAsc ? valA.localeCompare(valB) : valB.localeCompare(valA);
    }
    return currentSortAsc ? (valA - valB) : (valB - valA);
  });

  renderTable();
  updateTopKPIs();
  updateStudentSimulatorDropdown();
  updateTelegramStudentDropdown();
}

// Render Main Table
function renderTable() {
  const tbody = document.getElementById("studentTableBody");
  if (!tbody) return;

  if (filteredStudents.length === 0) {
    tbody.innerHTML = `
      <tr>
        <td colspan="10" class="text-center py-12 text-gray-500">
          <i data-lucide="inbox" class="w-8 h-8 mx-auto mb-2 opacity-50"></i>
          Tidak ada data siswa yang cocok dengan filter atau pencarian Anda.
        </td>
      </tr>
    `;
    lucide.createIcons();
    return;
  }

  let html = '';
  filteredStudents.forEach((s, idx) => {
    const finalScore = calculateFinalScore(s);
    const grade = getGradeLetter(finalScore);
    const isPass = finalScore >= 75;
    const initial = s.name.split(' ').map(w => w[0]).slice(0, 2).join('');

    let accColor = 'text-emerald-400 bg-emerald-500/10 border-emerald-500/30';
    if (s.accuracy < 75) accColor = 'text-rose-400 bg-rose-500/10 border-rose-500/30';
    else if (s.accuracy < 85) accColor = 'text-amber-400 bg-amber-500/10 border-amber-500/30';

    html += `
      <tr class="hover:bg-gray-800/40 transition-colors group">
        <td class="py-3.5 px-4 font-mono text-gray-400">
          <div class="flex items-center gap-2">
            <span class="w-5 h-5 rounded-md ${idx < 3 ? 'bg-amber-500/20 text-amber-400 font-bold border border-amber-500/30' : 'bg-gray-800 text-gray-400'} flex items-center justify-center text-[10px]">
              ${idx + 1}
            </span>
            <span>${s.nisn}</span>
          </div>
        </td>

        <td class="py-3.5 px-4">
          <div class="flex items-center gap-3">
            <div class="w-8 h-8 rounded-lg bg-gradient-to-tr from-sky-600 to-indigo-600 flex items-center justify-center font-bold text-white text-[11px] shadow-sm">
              ${initial}
            </div>
            <div>
              <div class="font-bold text-gray-100 hover:text-sky-400 cursor-pointer flex items-center gap-1.5" onclick="openDetailModal('${s.id}')">
                <span>${s.name}</span>
                <span class="text-[10px] px-1.5 py-0.2 rounded bg-gray-800 text-gray-400">${s.gender}</span>
              </div>
              <div class="text-[11px] text-gray-400 flex items-center gap-2 mt-0.5">
                <span class="px-1.5 py-0.2 rounded bg-sky-500/10 text-sky-400 font-semibold">Kelas ${s.class}</span>
                <span class="text-gray-500 font-mono">${s.device_id || 'ESP32'}</span>
              </div>
            </div>
          </div>
        </td>

        <td class="py-3.5 px-4 text-center">
          <div class="inline-flex flex-col items-center">
            <span class="font-bold text-sky-400 text-sm">${s.score_bali}</span>
            <span class="text-[10px] text-gray-400 truncate max-w-[80px]">${s.best_bali || '-'}</span>
          </div>
        </td>

        <td class="py-3.5 px-4 text-center">
          <div class="inline-flex flex-col items-center">
            <span class="font-bold text-indigo-400 text-sm">${s.score_jawa}</span>
            <span class="text-[10px] text-gray-400 truncate max-w-[80px]">${s.best_jawa || '-'}</span>
          </div>
        </td>

        <td class="py-3.5 px-4 text-center font-bold text-amber-400">
          ${s.score_quiz}
        </td>

        <td class="py-3.5 px-4 text-center">
          <span class="px-2 py-0.5 rounded-full text-[11px] font-semibold border ${accColor}">
            ${s.accuracy}%
          </span>
        </td>

        <td class="py-3.5 px-4 text-center text-gray-300 font-mono">
          ${s.speed_sec}s
        </td>

        <td class="py-3.5 px-4 text-center">
          <div class="inline-flex items-center gap-1.5">
            <span class="font-black text-white text-sm">${finalScore}</span>
            <span class="w-5 h-5 rounded flex items-center justify-center font-bold text-[10px] ${grade.bg} ${grade.color}">
              ${grade.letter}
            </span>
          </div>
        </td>

        <td class="py-3.5 px-4 text-center">
          <span class="px-2.5 py-1 rounded-full text-[10px] font-bold ${isPass ? 'bg-emerald-500/10 text-emerald-400 border border-emerald-500/20' : 'bg-rose-500/10 text-rose-400 border border-rose-500/20'}">
            ${isPass ? 'TUNTAS' : 'REMEDIAL'}
          </span>
        </td>

        <td class="py-3.5 px-4 text-right">
          <div class="flex items-center justify-end gap-1.5">
            <button onclick="sendStudentTelegramAlert('${s.id}')" class="p-1.5 rounded-lg bg-sky-500/10 hover:bg-sky-500/20 text-sky-400 transition-all" title="Kirim Rapor ke Telegram Orang Tua">
              <i data-lucide="send" class="w-4 h-4"></i>
            </button>
            <button onclick="openDetailModal('${s.id}')" class="p-1.5 rounded-lg bg-sky-500/10 hover:bg-sky-500/20 text-sky-400 transition-all" title="Lihat Rapor Detail">
              <i data-lucide="file-text" class="w-4 h-4"></i>
            </button>
            <button onclick="openEditStudentModal('${s.id}')" class="p-1.5 rounded-lg bg-gray-800 hover:bg-gray-700 text-gray-300 transition-all" title="Edit Data Siswa">
              <i data-lucide="edit-3" class="w-4 h-4"></i>
            </button>
            <button onclick="deleteStudent('${s.id}')" class="p-1.5 rounded-lg bg-rose-500/10 hover:bg-rose-500/20 text-rose-400 transition-all" title="Hapus Siswa">
              <i data-lucide="trash-2" class="w-4 h-4"></i>
            </button>
          </div>
        </td>
      </tr>
    `;
  });

  tbody.innerHTML = html;
  lucide.createIcons();

  const showingEl = document.getElementById("showingCount");
  const totalEl = document.getElementById("totalCount");
  const filterCountEl = document.getElementById("filteredCountText");
  if (showingEl) showingEl.textContent = filteredStudents.length;
  if (totalEl) totalEl.textContent = studentsData.length;
  if (filterCountEl) filterCountEl.textContent = `${filteredStudents.length} Siswa`;
}

// Update Top KPI Cards
function updateTopKPIs() {
  if (studentsData.length === 0) return;

  const total = studentsData.length;
  let sumScore = 0;
  let sumAcc = 0;
  let passCount = 0;

  studentsData.forEach(s => {
    const final = calculateFinalScore(s);
    sumScore += final;
    sumAcc += (s.accuracy || 0);
    if (final >= 75) passCount++;
  });

  const avgScore = (sumScore / total).toFixed(1);
  const avgAcc = (sumAcc / total).toFixed(1);
  const passRate = ((passCount / total) * 100).toFixed(1);
  const remedialCount = total - passCount;

  document.getElementById("statAvgScore").textContent = avgScore;
  document.getElementById("statAvgScoreBar").style.width = Math.min(100, avgScore) + "%";
  const diffFromKkm = (avgScore - 75).toFixed(1);
  document.getElementById("statScoreBadge").textContent = diffFromKkm >= 0 ? `+${diffFromKkm}% di atas KKM` : `${diffFromKkm}% di bawah KKM`;

  document.getElementById("statAvgAccuracy").textContent = avgAcc + "%";
  document.getElementById("statAvgAccBar").style.width = Math.min(100, avgAcc) + "%";

  document.getElementById("statPassRate").textContent = passRate + "%";
  document.getElementById("statPassRatio").textContent = `${passCount} / ${total} Siswa`;
  document.getElementById("statPassBar").style.width = passRate + "%";
  document.getElementById("statRemedialBadge").textContent = `${remedialCount} Remedial`;

  document.getElementById("statActiveDevices").textContent = total;
}

// Render Real-time Activity Feed
function renderLiveActivity() {
  const container = document.getElementById("liveActivityFeed");
  if (!container) return;

  const activeList = [...studentsData]
    .sort((a, b) => new Date(b.last_active || 0) - new Date(a.last_active || 0))
    .slice(0, 3);

  let html = '';
  activeList.forEach(s => {
    const lastSession = (s.history && s.history.length > 0) ? s.history[0] : null;
    const letter = lastSession ? lastSession.letter : (s.best_bali || 'Ha');
    const script = lastSession ? (lastSession.script === 'bali' ? 'Bali' : 'Jawa') : 'Bali';
    const score = lastSession ? lastSession.score : s.score_bali;
    const acc = lastSession ? lastSession.accuracy : s.accuracy;

    html += `
      <div class="bg-gray-800/50 border border-gray-700/60 rounded-xl p-3 flex items-center justify-between">
        <div class="flex items-center gap-2.5">
          <div class="w-8 h-8 rounded-lg bg-sky-500/10 text-sky-400 flex items-center justify-center font-bold text-xs border border-sky-500/20">
            <i data-lucide="tablet" class="w-4 h-4"></i>
          </div>
          <div>
            <div class="font-bold text-xs text-white truncate max-w-[140px]">${s.name}</div>
            <div class="text-[10px] text-gray-400">
              Gores Aksara <strong class="text-sky-300">${letter} (${script})</strong>
            </div>
          </div>
        </div>
        <div class="text-right">
          <div class="text-xs font-bold text-emerald-400">${score} Poin</div>
          <div class="text-[10px] text-gray-400">${acc}% Akurasi</div>
        </div>
      </div>
    `;
  });

  container.innerHTML = html;
  lucide.createIcons();
}

// Render Script Difficulty Ranking
function renderDifficultyRanking() {
  const container = document.getElementById("difficultyRankingContainer");
  if (!container) return;

  let html = '';
  DIFFICULTY_LETTERS.forEach((item, idx) => {
    const isHigh = idx < 2;
    html += `
      <div class="bg-gray-800/40 border ${isHigh ? 'border-amber-500/40 bg-amber-500/5' : 'border-gray-700/60'} rounded-xl p-3 text-center transition-all hover:scale-[1.02]">
        <div class="text-[10px] uppercase font-bold ${item.script === 'Bali' ? 'text-sky-400' : 'text-indigo-400'}">
          Aksara ${item.script}
        </div>
        <div class="text-2xl my-1 font-bold ${item.script === 'Bali' ? 'aksara-bali-glyph' : 'aksara-jawa-glyph'} text-white">
          ${item.glyph}
        </div>
        <div class="text-xs font-bold text-gray-200">${item.name}</div>
        <div class="text-[11px] font-bold text-rose-400 mt-1">Deviasi: ${item.errRate}</div>
        <div class="text-[9px] text-gray-400 mt-0.5 line-clamp-1" title="${item.note}">${item.note}</div>
      </div>
    `;
  });

  container.innerHTML = html;
}

// Charts Initialization
function initCharts() {
  const ctxComp = document.getElementById("comparisonChart");
  if (ctxComp) {
    comparisonChart = new Chart(ctxComp, {
      type: 'bar',
      data: {
        labels: ['Modul 1 (Wreastra/Carakan)', 'Modul 2 (Sandhangan Swara)', 'Modul 3 (Pasangan/Gantungan)', 'Modul 4 (Kuis Cepat)'],
        datasets: [
          {
            label: 'Aksara Bali',
            data: [92, 86, 78, 89],
            backgroundColor: 'rgba(56, 189, 248, 0.85)',
            borderColor: '#38BDF8',
            borderRadius: 6,
            borderWidth: 1
          },
          {
            label: 'Aksara Jawa',
            data: [88, 84, 75, 87],
            backgroundColor: 'rgba(99, 102, 241, 0.85)',
            borderColor: '#6366F1',
            borderRadius: 6,
            borderWidth: 1
          }
        ]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: {
            labels: { color: '#94A3B8', font: { family: 'Plus Jakarta Sans', size: 12 } }
          }
        },
        scales: {
          y: {
            min: 50,
            max: 100,
            grid: { color: 'rgba(55, 65, 81, 0.3)' },
            ticks: { color: '#94A3B8' }
          },
          x: {
            grid: { display: false },
            ticks: { color: '#94A3B8' }
          }
        }
      }
    });
  }

  const ctxDist = document.getElementById("gradeDistributionChart");
  if (ctxDist) {
    gradeDistributionChart = new Chart(ctxDist, {
      type: 'doughnut',
      data: {
        labels: ['Sangat Baik (A)', 'Baik (B)', 'Cukup (C)', 'Remedial (D)'],
        datasets: [{
          data: [6, 4, 1, 1],
          backgroundColor: ['#10B981', '#38BDF8', '#F59E0B', '#F43F5E'],
          borderWidth: 2,
          borderColor: '#111827'
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        cutout: '70%',
        plugins: {
          legend: { display: false }
        }
      }
    });
  }

  updateCharts();
}

// Update Charts based on current data
function updateCharts() {
  if (!gradeDistributionChart) return;

  let countA = 0, countB = 0, countC = 0, countD = 0;
  studentsData.forEach(s => {
    const final = calculateFinalScore(s);
    if (final >= 90) countA++;
    else if (final >= 80) countB++;
    else if (final >= 75) countC++;
    else countD++;
  });

  gradeDistributionChart.data.datasets[0].data = [countA, countB, countC, countD];
  gradeDistributionChart.update();

  const elA = document.getElementById("countA");
  const elB = document.getElementById("countB");
  const elC = document.getElementById("countC");
  const elD = document.getElementById("countD");
  if (elA) elA.textContent = countA;
  if (elB) elB.textContent = countB;
  if (elC) elC.textContent = countC;
  if (elD) elD.textContent = countD;

  if (comparisonChart && studentsData.length > 0) {
    const avgBali = Math.round(studentsData.reduce((acc, cur) => acc + (cur.score_bali || 0), 0) / studentsData.length);
    const avgJawa = Math.round(studentsData.reduce((acc, cur) => acc + (cur.score_jawa || 0), 0) / studentsData.length);
    const avgQuiz = Math.round(studentsData.reduce((acc, cur) => acc + (cur.score_quiz || 0), 0) / studentsData.length);

    comparisonChart.data.datasets[0].data = [avgBali, Math.max(50, avgBali - 6), Math.max(50, avgBali - 12), avgQuiz];
    comparisonChart.data.datasets[1].data = [avgJawa, Math.max(50, avgJawa - 5), Math.max(50, avgJawa - 14), avgQuiz - 2];
    comparisonChart.update();
  }
}

// Filter and Sort Handlers
function filterByClass() {
  currentFilterClass = document.getElementById("classFilter").value;
  applyFiltersAndRender();
}

function setFilterStatus(status) {
  currentFilterStatus = status;

  const btnAll = document.getElementById("tabFilterAll");
  const btnTuntas = document.getElementById("tabFilterTuntas");
  const btnRemedial = document.getElementById("tabFilterRemedial");

  [btnAll, btnTuntas, btnRemedial].forEach(b => {
    b.className = "px-3 py-1.5 rounded-lg text-gray-400 hover:text-white transition-all";
  });

  if (status === 'ALL') btnAll.className = "px-3 py-1.5 rounded-lg font-semibold bg-sky-600 text-white transition-all";
  else if (status === 'Tuntas') btnTuntas.className = "px-3 py-1.5 rounded-lg font-semibold bg-emerald-600 text-white transition-all";
  else if (status === 'Remedial') btnRemedial.className = "px-3 py-1.5 rounded-lg font-semibold bg-rose-600 text-white transition-all";

  applyFiltersAndRender();
}

function handleSearch() {
  currentSearchQuery = document.getElementById("searchInput").value.trim();
  applyFiltersAndRender();
}

function sortTable(key) {
  if (currentSortKey === key) {
    currentSortAsc = !currentSortAsc;
  } else {
    currentSortKey = key;
    currentSortAsc = false;
  }
  applyFiltersAndRender();
}

// Student Detail Modal (Rapor & Radar Chart)
let currentModalStudent = null;

function openDetailModal(studentId) {
  const s = studentsData.find(item => item.id === studentId);
  if (!s) return;
  currentModalStudent = s;

  const modal = document.getElementById("studentDetailModal");
  modal.classList.remove("hidden");

  const initial = s.name.split(' ').map(w => w[0]).slice(0, 2).join('');
  document.getElementById("modalAvatar").textContent = initial;
  document.getElementById("modalStudentName").textContent = s.name;
  document.getElementById("modalStudentGender").textContent = s.gender === 'L' ? 'Laki-Laki' : 'Perempuan';
  document.getElementById("modalStudentNisn").textContent = s.nisn;
  document.getElementById("modalStudentClass").textContent = `Kelas ${s.class}`;
  document.getElementById("modalStudentDevice").textContent = s.device_id || 'ESP32';

  const finalScore = calculateFinalScore(s);
  const grade = getGradeLetter(finalScore);

  document.getElementById("modalScoreBali").textContent = s.score_bali;
  document.getElementById("modalBestBali").textContent = `Terbaik: ${s.best_bali || '-'}`;
  document.getElementById("modalScoreJawa").textContent = s.score_jawa;
  document.getElementById("modalBestJawa").textContent = `Terbaik: ${s.best_jawa || '-'}`;
  document.getElementById("modalScoreQuiz").textContent = s.score_quiz;
  document.getElementById("modalFinalScore").textContent = finalScore;
  
  const badge = document.getElementById("modalFinalBadge");
  badge.textContent = `${grade.letter} • ${grade.text.toUpperCase()}`;
  badge.className = `text-[10px] px-2 py-0.5 rounded-full font-bold ${grade.bg} ${grade.color}`;

  document.getElementById("modalRadarAcc").textContent = `${s.accuracy}%`;
  document.getElementById("modalBarAcc").style.width = `${s.accuracy}%`;
  document.getElementById("modalRadarSpeed").textContent = `${s.speed_sec} detik`;
  document.getElementById("modalBarSpeed").style.width = `${Math.min(100, Math.max(20, (30 - s.speed_sec) * 5))}%`;

  renderStudentRadarChart(s);

  const historyTbody = document.getElementById("modalHistoryTable");
  if (s.history && s.history.length > 0) {
    historyTbody.innerHTML = s.history.slice(0, 5).map(h => `
      <tr class="hover:bg-gray-800/60">
        <td class="p-2.5 text-gray-400 font-mono text-[10px]">${new Date(h.timestamp).toLocaleTimeString('id-ID', { hour: '2-digit', minute: '2-digit' })} WIB</td>
        <td class="p-2.5 font-semibold ${h.script === 'bali' ? 'text-sky-400' : 'text-indigo-400'}">Aksara ${h.script === 'bali' ? 'Bali' : 'Jawa'}</td>
        <td class="p-2.5 font-bold text-white">${h.letter}</td>
        <td class="p-2.5 text-center font-bold text-emerald-400">${h.score}</td>
        <td class="p-2.5 text-center font-semibold text-gray-200">${h.accuracy}%</td>
        <td class="p-2.5 text-center text-gray-400">${h.duration}s</td>
      </tr>
    `).join('');
  } else {
    historyTbody.innerHTML = `
      <tr>
        <td colspan="6" class="p-4 text-center text-gray-500">Belum ada riwayat sesi pengerjaan terekam.</td>
      </tr>
    `;
  }

  document.getElementById("modalStudentNotes").value = s.notes || '';
  lucide.createIcons();
}

function closeDetailModal() {
  document.getElementById("studentDetailModal").classList.add("hidden");
}

function renderStudentRadarChart(s) {
  const ctx = document.getElementById("studentRadarChart");
  if (!ctx) return;

  if (studentRadarChart) studentRadarChart.destroy();

  const speedScore = Math.min(100, Math.max(40, Math.round((35 - s.speed_sec) * 3.5)));

  studentRadarChart = new Chart(ctx, {
    type: 'radar',
    data: {
      labels: ['Akurasi Goresan', 'Kecepatan', 'Aksara Bali', 'Aksara Jawa', 'Kuis Teori'],
      datasets: [{
        label: s.name,
        data: [s.accuracy || 80, speedScore, s.score_bali || 80, s.score_jawa || 80, s.score_quiz || 80],
        backgroundColor: 'rgba(56, 189, 248, 0.25)',
        borderColor: '#38BDF8',
        borderWidth: 2,
        pointBackgroundColor: '#38BDF8',
        pointRadius: 3
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        r: {
          min: 40,
          max: 100,
          ticks: { display: false, stepSize: 20 },
          grid: { color: 'rgba(75, 85, 99, 0.3)' },
          angleLines: { color: 'rgba(75, 85, 99, 0.3)' },
          pointLabels: { color: '#94A3B8', font: { size: 10, family: 'Plus Jakarta Sans' } }
        }
      },
      plugins: {
        legend: { display: false }
      }
    }
  });
}

async function saveStudentNotes() {
  if (!currentModalStudent) return;
  const newNotes = document.getElementById("modalStudentNotes").value;
  currentModalStudent.notes = newNotes;

  if (isConnectedToServer) {
    try {
      await fetch('/api/students', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(currentModalStudent)
      });
    } catch (e) {
      console.warn("Gagal simpan notes ke server:", e);
    }
  } else {
    localStorage.setItem("openaksara_students", JSON.stringify(studentsData));
  }
  showToast("Catatan guru berhasil disimpan!", "success");
}

function printStudentReport() {
  window.print();
}

// Add & Edit Student Modal Handlers
function openAddStudentModal() {
  document.getElementById("formModalTitle").innerHTML = `<i data-lucide="user-plus" class="w-4 h-4 text-sky-400"></i> Tambah Siswa Baru`;
  document.getElementById("studentForm").reset();
  document.getElementById("formStudentId").value = "";
  document.getElementById("formNisn").value = "202604" + String(studentsData.length + 1).padStart(2, '0');
  document.getElementById("addStudentModal").classList.remove("hidden");
  lucide.createIcons();
}

function openEditStudentModal(studentId) {
  const s = studentsData.find(item => item.id === studentId);
  if (!s) return;

  document.getElementById("formModalTitle").innerHTML = `<i data-lucide="edit-3" class="w-4 h-4 text-sky-400"></i> Edit Data Siswa`;
  document.getElementById("formStudentId").value = s.id;
  document.getElementById("formName").value = s.name;
  document.getElementById("formNisn").value = s.nisn;
  document.getElementById("formClass").value = s.class;
  document.getElementById("formGender").value = s.gender;
  document.getElementById("formDeviceId").value = s.device_id || '';
  document.getElementById("formScoreBali").value = s.score_bali;
  document.getElementById("formScoreJawa").value = s.score_jawa;
  document.getElementById("formScoreQuiz").value = s.score_quiz;

  document.getElementById("addStudentModal").classList.remove("hidden");
  lucide.createIcons();
}

function closeAddStudentModal() {
  document.getElementById("addStudentModal").classList.add("hidden");
}

async function handleSaveStudent(e) {
  e.preventDefault();
  const id = document.getElementById("formStudentId").value || ("STU-" + String(Date.now()).slice(-6));
  const name = document.getElementById("formName").value.trim();
  const nisn = document.getElementById("formNisn").value.trim();
  const studentClass = document.getElementById("formClass").value;
  const gender = document.getElementById("formGender").value;
  const device_id = document.getElementById("formDeviceId").value.trim() || `ESP32-${studentClass}-${nisn.slice(-2)}`;
  const score_bali = parseInt(document.getElementById("formScoreBali").value) || 80;
  const score_jawa = parseInt(document.getElementById("formScoreJawa").value) || 80;
  const score_quiz = parseInt(document.getElementById("formScoreQuiz").value) || 80;

  const finalScore = Math.round((score_bali + score_jawa + score_quiz) / 3);
  const status = finalScore >= 75 ? "Tuntas" : "Remedial";

  const studentObj = {
    id,
    nisn,
    name,
    class: studentClass,
    gender,
    device_id,
    score_bali,
    score_jawa,
    score_quiz,
    accuracy: 92.0,
    speed_sec: 14,
    last_active: new Date().toISOString(),
    status,
    best_bali: "Ha (ᬳ)",
    best_jawa: "Ha (ꦲ)",
    notes: "Data siswa baru ditambahkan oleh guru.",
    history: []
  };

  const existingIdx = studentsData.findIndex(s => s.id === id);
  if (existingIdx >= 0) {
    studentsData[existingIdx] = { ...studentsData[existingIdx], ...studentObj };
  } else {
    studentsData.unshift(studentObj);
  }

  if (isConnectedToServer) {
    try {
      await fetch('/api/students', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(studentObj)
      });
    } catch (err) {
      console.warn("Gagal simpan ke server, fallback lokal:", err);
    }
  } else {
    localStorage.setItem("openaksara_students", JSON.stringify(studentsData));
  }

  closeAddStudentModal();
  applyFiltersAndRender();
  updateCharts();
  showToast(`Data siswa "${name}" berhasil disimpan!`, 'success');
}

async function deleteStudent(studentId) {
  const s = studentsData.find(item => item.id === studentId);
  if (!s) return;

  if (confirm(`Apakah Anda yakin ingin menghapus data siswa "${s.name}" (NISN: ${s.nisn})?`)) {
    studentsData = studentsData.filter(item => item.id !== studentId);

    if (isConnectedToServer) {
      try {
        await fetch(`/api/students?id=${studentId}`, { method: 'DELETE' });
      } catch (err) {
        console.warn("Gagal hapus di server:", err);
      }
    } else {
      localStorage.setItem("openaksara_students", JSON.stringify(studentsData));
    }

    applyFiltersAndRender();
    updateCharts();
    showToast(`Data siswa "${s.name}" berhasil dihapus.`, 'info');
  }
}

// ESP32 Simulator Modal
function openSimulatorModal() {
  updateStudentSimulatorDropdown();
  document.getElementById("simulatorModal").classList.remove("hidden");
  lucide.createIcons();
}

function closeSimulatorModal() {
  document.getElementById("simulatorModal").classList.add("hidden");
}

function updateStudentSimulatorDropdown() {
  const select = document.getElementById("simStudentSelect");
  if (!select) return;

  select.innerHTML = studentsData.map(s => `
    <option value="${s.id}">
      ${s.name} (${s.device_id || 'ESP32'} - Kelas ${s.class})
    </option>
  `).join('');
}

function updateSimLetters() {
  const script = document.getElementById("simScript").value;
  const select = document.getElementById("simLetter");
  const list = script === 'bali' ? BALI_SIM_LETTERS : JAWA_SIM_LETTERS;

  select.innerHTML = list.map(item => `
    <option value="${item.value}">${item.name}</option>
  `).join('');
}

async function executeSimulation() {
  const studentId = document.getElementById("simStudentSelect").value;
  const targetStudent = studentsData.find(s => s.id === studentId);
  if (!targetStudent) return;

  const script = document.getElementById("simScript").value;
  const letter = document.getElementById("simLetter").value;
  const score = parseInt(document.getElementById("simScore").value);
  const accuracy = parseFloat(document.getElementById("simAccuracy").value);
  const duration = parseInt(document.getElementById("simDuration").value);
  const battery = parseInt(document.getElementById("simBattery").value);

  const payload = {
    device_id: targetStudent.device_id || "ESP32-DEV-01",
    student_id: targetStudent.id,
    nisn: targetStudent.nisn,
    script,
    letter,
    score,
    accuracy,
    duration_sec: duration,
    battery,
    rssi: -58
  };

  closeSimulatorModal();

  if (isConnectedToServer) {
    try {
      const res = await fetch('/api/telemetry', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      });
      const json = await res.json();
      console.log("[Simulator] Response server:", json);
    } catch (e) {
      console.warn("[Simulator] Server error, fallback local processing:", e);
      simulateLocalTelemetry(payload, targetStudent);
    }
  } else {
    simulateLocalTelemetry(payload, targetStudent);
  }
}

function simulateLocalTelemetry(telemetry, student) {
  student.last_active = new Date().toISOString();
  if (telemetry.script === 'bali') {
    student.score_bali = Math.round((student.score_bali * 0.7) + (telemetry.score * 0.3));
    if (telemetry.score >= 90) student.best_bali = `${telemetry.letter}`;
  } else {
    student.score_jawa = Math.round((student.score_jawa * 0.7) + (telemetry.score * 0.3));
    if (telemetry.score >= 90) student.best_jawa = `${telemetry.letter}`;
  }

  student.accuracy = parseFloat(((student.accuracy * 0.8) + (telemetry.accuracy * 0.2)).toFixed(1));
  student.speed_sec = Math.round((student.speed_sec + telemetry.duration_sec) / 2);

  const finalScore = calculateFinalScore(student);
  student.status = finalScore >= 75 ? "Tuntas" : "Remedial";

  if (!student.history) student.history = [];
  student.history.unshift({
    timestamp: new Date().toISOString(),
    script: telemetry.script,
    letter: telemetry.letter,
    score: telemetry.score,
    accuracy: telemetry.accuracy,
    duration: telemetry.duration_sec
  });
  if (student.history.length > 20) student.history.pop();

  localStorage.setItem("openaksara_students", JSON.stringify(studentsData));
  handleIncomingTelemetry(telemetry, student);
}

// Audio Chime (Web Audio API Synthesizer)
function playSuccessChime() {
  try {
    const AudioContext = window.AudioContext || window.webkitAudioContext;
    if (!AudioContext) return;
    const ctx = new AudioContext();

    const osc1 = ctx.createOscillator();
    const osc2 = ctx.createOscillator();
    const gain = ctx.createGain();

    osc1.type = "sine";
    osc2.type = "triangle";

    osc1.frequency.setValueAtTime(659.25, ctx.currentTime);
    osc1.frequency.setValueAtTime(830.61, ctx.currentTime + 0.1);
    osc1.frequency.setValueAtTime(987.77, ctx.currentTime + 0.2);

    osc2.frequency.setValueAtTime(329.63, ctx.currentTime);

    gain.gain.setValueAtTime(0.15, ctx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.6);

    osc1.connect(gain);
    osc2.connect(gain);
    gain.connect(ctx.destination);

    osc1.start();
    osc2.start();
    osc1.stop(ctx.currentTime + 0.6);
    osc2.stop(ctx.currentTime + 0.6);
  } catch (e) {
    // Audio autoplay restrictions
  }
}

// Confetti Celebration
function triggerConfetti() {
  if (typeof confetti === 'function') {
    confetti({
      particleCount: 80,
      spread: 70,
      origin: { y: 0.6 },
      colors: ['#38BDF8', '#10B981', '#F59E0B', '#818CF8']
    });
  }
}

// Toast Notifications
function showToast(message, type = 'info') {
  const container = document.getElementById("toastContainer");
  if (!container) return;

  const toast = document.createElement("div");
  const bg = type === 'success' ? 'bg-emerald-900/90 border-emerald-500/50 text-emerald-100' : 'bg-gray-800/90 border-gray-600 text-gray-100';
  
  toast.className = `p-3.5 rounded-xl border shadow-xl text-xs flex items-center gap-2.5 backdrop-blur-md pointer-events-auto transition-all transform translate-y-2 opacity-0 max-w-sm ${bg}`;
  toast.innerHTML = `
    <i data-lucide="${type === 'success' ? 'check-circle' : 'info'}" class="w-4 h-4 shrink-0"></i>
    <span>${message}</span>
  `;

  container.appendChild(toast);
  lucide.createIcons();

  requestAnimationFrame(() => {
    toast.classList.remove("translate-y-2", "opacity-0");
  });

  setTimeout(() => {
    toast.classList.add("opacity-0", "translate-y-2");
    setTimeout(() => toast.remove(), 300);
  }, 4000);
}

// Export Table to CSV
function exportToCSV() {
  if (studentsData.length === 0) {
    alert("Tidak ada data untuk diekspor!");
    return;
  }

  let csv = "ID,NISN,Nama Siswa,Kelas,Jenis Kelamin,Device ID,Nilai Bali,Nilai Jawa,Nilai Kuis,Akurasi (%),Kecepatan (detik),Nilai Akhir,Predikat,Status,Huruf Terbaik Bali,Huruf Terbaik Jawa,Catatan Guru\n";

  studentsData.forEach(s => {
    const finalScore = calculateFinalScore(s);
    const grade = getGradeLetter(finalScore);
    const cleanNotes = (s.notes || '').replace(/"/g, '""');

    csv += `"${s.id}","${s.nisn}","${s.name}","${s.class}","${s.gender}","${s.device_id || ''}",${s.score_bali},${s.score_jawa},${s.score_quiz},${s.accuracy},${s.speed_sec},${finalScore},"${grade.letter}","${s.status}","${s.best_bali || ''}","${s.best_jawa || ''}","${cleanNotes}"\n`;
  });

  const blob = new Blob(["\uFEFF" + csv], { type: 'text/csv;charset=utf-8;' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = `Rekap_Nilai_OpenAksara_${new Date().toISOString().slice(0, 10)}.csv`;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
  showToast("File CSV rekap nilai berhasil diunduh!", "success");
}

// Dark / Light Theme Toggle
function initTheme() {
  const isDark = localStorage.getItem("openaksara_theme") !== 'light';
  applyTheme(isDark);
}

function toggleTheme() {
  const isDark = document.documentElement.classList.contains("dark");
  applyTheme(!isDark);
}

function applyTheme(isDark) {
  const icon = document.getElementById("themeIcon");
  if (isDark) {
    document.documentElement.classList.add("dark");
    document.documentElement.classList.remove("light");
    localStorage.setItem("openaksara_theme", "dark");
    if (icon) icon.setAttribute("data-lucide", "sun");
  } else {
    document.documentElement.classList.remove("dark");
    document.documentElement.classList.add("light");
    localStorage.setItem("openaksara_theme", "light");
    if (icon) icon.setAttribute("data-lucide", "moon");
  }
  lucide.createIcons();
}

// ==================== TELEGRAM NOTIFICATION SYSTEM ====================

function openTelegramModal() {
  updateTelegramStudentDropdown();
  renderTelegramChatPreview(telegramLogs[0]);
  renderTelegramLogsTable();
  loadTelegramConfigFromStorageOrServer();
  document.getElementById("telegramModal").classList.remove("hidden");
  lucide.createIcons();
}

function closeTelegramModal() {
  document.getElementById("telegramModal").classList.add("hidden");
}

function switchTelegramTab(tab) {
  const btnPrev = document.getElementById("tgTabBtnPreview");
  const btnLogs = document.getElementById("tgTabBtnLogs");
  const btnSet = document.getElementById("tgTabBtnSettings");

  const tabPrev = document.getElementById("tgTabPreview");
  const tabLogs = document.getElementById("tgTabLogs");
  const tabSet = document.getElementById("tgTabSettings");

  [btnPrev, btnLogs, btnSet].forEach(b => {
    b.className = "py-3 px-4 border-b-2 border-transparent text-gray-400 hover:text-gray-200 flex items-center gap-2 transition-all";
  });
  [tabPrev, tabLogs, tabSet].forEach(t => t.classList.add("hidden"));

  if (tab === 'preview') {
    btnPrev.className = "py-3 px-4 border-b-2 border-sky-400 text-sky-400 flex items-center gap-2 transition-all";
    tabPrev.classList.remove("hidden");
  } else if (tab === 'logs') {
    btnLogs.className = "py-3 px-4 border-b-2 border-sky-400 text-sky-400 flex items-center gap-2 transition-all";
    tabLogs.classList.remove("hidden");
    renderTelegramLogsTable();
  } else if (tab === 'settings') {
    btnSet.className = "py-3 px-4 border-b-2 border-sky-400 text-sky-400 flex items-center gap-2 transition-all";
    tabSet.classList.remove("hidden");
  }
  lucide.createIcons();
}

function updateTelegramStudentDropdown() {
  const select = document.getElementById("tgTargetStudent");
  if (!select) return;

  select.innerHTML = studentsData.map(s => `
    <option value="${s.id}">
      ${s.name} (NISN: ${s.nisn} - Kelas ${s.class})
    </option>
  `).join('');
}

async function triggerTelegramAction(type) {
  const studentId = document.getElementById("tgTargetStudent") ? document.getElementById("tgTargetStudent").value : (studentsData[0] ? studentsData[0].id : null);
  await sendTelegramNotification(type, studentId);
}

function sendStudentTelegramAlert(studentId) {
  openTelegramModal();
  const select = document.getElementById("tgTargetStudent");
  if (select) select.value = studentId;
  triggerTelegramAction('student_score');
}

function sendCurrentStudentTelegram() {
  if (currentModalStudent) {
    sendStudentTelegramAlert(currentModalStudent.id);
  }
}

async function sendTelegramNotification(type, studentId) {
  const payload = {
    type,
    student_id: studentId,
    timestamp: new Date().toISOString()
  };

  if (isConnectedToServer) {
    try {
      const res = await fetch('/api/telegram/send', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      });
      const data = await res.json();
      if (data.success && data.log) {
        handleTelegramMessageReceived(data.log);
      }
    } catch (e) {
      console.warn("[Telegram] Server request failed, using local dummy processing:", e);
      simulateLocalTelegramSend(payload);
    }
  } else {
    simulateLocalTelegramSend(payload);
  }
}

function simulateLocalTelegramSend(payload) {
  const student = studentsData.find(s => s.id === payload.student_id) || studentsData[0];
  let title = "Notifikasi Nilai OpenAksara";
  let content = "";

  if (payload.type === 'student_score') {
    const finalScore = calculateFinalScore(student);
    const isPass = finalScore >= 75;
    title = `Laporan Belajar - ${student.name}`;
    content = `📱 <b>OpenAksara EduCloud • Laporan Belajar Siswa</b>\n━━━━━━━━━━━━━━━━━━━━━━━\n👤 <b>Nama:</b> ${student.name}\n🆔 <b>NISN:</b> ${student.nisn} (Kelas ${student.class})\n🏷️ <b>Aksara Bali:</b> ${student.score_bali} Poin\n🏷️ <b>Aksara Jawa:</b> ${student.score_jawa} Poin\n📝 <b>Kuis Teori:</b> ${student.score_quiz} Poin\n\n🏆 <b>Nilai Akhir: ${finalScore} / 100</b>\n🎯 <b>Akurasi Goresan:</b> ${student.accuracy}%\n⏱️ <b>Rata-rata Waktu:</b> ${student.speed_sec} detik\nStatus: <b>${isPass ? 'TUNTAS BELAJAR ✅' : 'PERLU REMEDIAL ⚠️'}</b>\n\n💡 <i>Catatan Guru: ${student.notes || 'Siswa menunjukkan ketelitian baik.'}</i>\n━━━━━━━━━━━━━━━━━━━━━━━\n🤖 <i>Dikirim otomatis via Tablet OpenAksara ESP32 IoT</i>`;
  } else if (payload.type === 'class_summary') {
    title = "Laporan Rekapitulasi Kelas Harian";
    const total = studentsData.length;
    const avgScore = (studentsData.reduce((a, b) => a + calculateFinalScore(b), 0) / total).toFixed(1);
    const avgAcc = (studentsData.reduce((a, b) => a + (b.accuracy || 0), 0) / total).toFixed(1);
    const passCount = studentsData.filter(s => calculateFinalScore(s) >= 75).length;
    content = `📊 <b>OpenAksara EduCloud • Ringkasan Belajar Harian</b>\n━━━━━━━━━━━━━━━━━━━━━━━\n🏫 <b>Sekolah:</b> SDN 1 Tuban • Semester Genap 2026/2027\n👥 <b>Total Siswa:</b> ${total} Siswa Terdata\n🏆 <b>Rata-Rata Nilai:</b> ${avgScore} / 100\n🎯 <b>Rata-Rata Akurasi:</b> ${avgAcc}%\n✅ <b>Siswa Tuntas:</b> ${passCount} Siswa (${((passCount/total)*100).toFixed(1)}%)\n⚠️ <b>Siswa Remedial:</b> ${total - passCount} Siswa\n\n🔍 <b>Aksara Perlu Bimbingan Ekstra:</b>\n1. Aksara Bali: <b>Ca (ᬘ)</b> (Deviasi 34%)\n2. Aksara Jawa: <b>Taling Tarung (ꦺ◌ꦴ)</b> (Deviasi 31%)\n━━━━━━━━━━━━━━━━━━━━━━━\n🤖 <i>Laporan otomatis harian untuk Guru & Wali Murid</i>`;
  } else if (payload.type === 'remedial_alert') {
    title = "Peringatan Siswa Perlu Remedial";
    const remedialStudent = studentsData.find(s => calculateFinalScore(s) < 75) || student;
    content = `⚠️ <b>PERINGATAN SISWA PERLU REMEDIAL</b>\n━━━━━━━━━━━━━━━━━━━━━━━\n👤 <b>Nama:</b> ${remedialStudent.name}\n🆔 <b>NISN:</b> ${remedialStudent.nisn} • Kelas ${remedialStudent.class}\n📱 <b>Perangkat:</b> ${remedialStudent.device_id || 'ESP32'}\n\n📉 <b>Hasil Evaluasi:</b>\n• Skor Rata-rata: <b>${calculateFinalScore(remedialStudent)} / 100</b> (KKM: 75)\n• Akurasi Goresan: <b>${remedialStudent.accuracy}%</b>\n• Status: <b>REMEDIAL DIPERLUKAN ⚠️</b>\n\n📌 <i>Rekomendasi: Latihan ulang menggunakan mode bantuan garis pemandu di tablet OpenAksara.</i>\n━━━━━━━━━━━━━━━━━━━━━━━\n🤖 <i>Notifikasi Otomatis Terusan ke Guru & Orang Tua Siswa</i>`;
  }

  const logEntry = {
    id: "TG-" + Date.now().toString().slice(-6),
    timestamp: new Date().toISOString(),
    type: payload.type,
    title,
    recipient: telegramConfig.chat_id || ("Grup Wali Murid Kelas " + (student ? student.class : '4A')),
    status: "Delivered",
    is_dummy: telegramConfig.is_dummy_mode !== false || !telegramConfig.bot_token,
    content
  };

  // Jika user mengaktifkan Mode Riil dan mengisi Bot Token & Chat ID di browser
  if (!telegramConfig.is_dummy_mode && telegramConfig.bot_token && telegramConfig.chat_id) {
    fetch(`https://api.telegram.org/bot${telegramConfig.bot_token}/sendMessage`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        chat_id: telegramConfig.chat_id,
        text: content,
        parse_mode: 'HTML'
      })
    })
    .then(res => res.json())
    .then(tgJson => {
      if (tgJson.ok) {
        console.log("[Telegram API Direct] Berhasil terkirim!", tgJson);
        logEntry.status = "Terkirim ke Telegram";
        renderTelegramLogsTable();
        showToast(`✅ Berhasil terkirim ke Telegram asli!`, 'success');
      } else {
        console.error("[Telegram API Direct Error]", tgJson);
        logEntry.status = "Gagal: " + (tgJson.description || 'Error');
        renderTelegramLogsTable();
        showToast(`❌ Telegram Gagal: ${tgJson.description}`, 'error');
      }
    })
    .catch(err => {
      console.error("[Telegram API Network Error]", err);
      logEntry.status = "Network Error";
      renderTelegramLogsTable();
      showToast(`❌ Gagal terhubung ke api.telegram.org: ${err.message}`, 'error');
    });
  }

  handleTelegramMessageReceived(logEntry);
}

function handleTelegramMessageReceived(logEntry) {
  telegramLogs.unshift(logEntry);
  if (telegramLogs.length > 30) telegramLogs.pop();

  renderTelegramChatPreview(logEntry);
  renderTelegramLogsTable();

  playTelegramSound();
  showToast(`📱 Notifikasi Telegram: ${logEntry.title}`, 'success');

  if (logEntry.content && (logEntry.content.includes("PRESTASI") || logEntry.content.includes("Sempurna") || logEntry.content.includes("Grade A") || logEntry.content.includes("TUNTAS"))) {
    triggerConfetti();
  }
}

function renderTelegramChatPreview(logEntry) {
  if (!logEntry) return;

  const bodyEl = document.getElementById("tgMessageBody");
  const timeEl = document.getElementById("tgChatTime");
  if (!bodyEl) return;

  const dateObj = new Date(logEntry.timestamp);
  const timeStr = dateObj.toLocaleTimeString('id-ID', { hour: '2-digit', minute: '2-digit' }) + ' WIB';

  if (timeEl) timeEl.textContent = timeStr;

  const formattedHtml = logEntry.content.replace(/\n/g, '<br>');
  bodyEl.innerHTML = formattedHtml;

  const container = document.getElementById("tgChatBubbleContainer");
  if (container) {
    container.scrollTop = container.scrollHeight;
  }
}

function renderTelegramLogsTable() {
  const tbody = document.getElementById("tgLogsTableBody");
  const countEl = document.getElementById("tgLogsCount");
  if (countEl) countEl.textContent = telegramLogs.length;
  if (!tbody) return;

  if (telegramLogs.length === 0) {
    tbody.innerHTML = `<tr><td colspan="5" class="p-4 text-center text-gray-500">Belum ada notifikasi Telegram yang dikirimkan.</td></tr>`;
    return;
  }

  tbody.innerHTML = telegramLogs.map(log => {
    const timeStr = new Date(log.timestamp).toLocaleTimeString('id-ID', { hour: '2-digit', minute: '2-digit' });
    return `
      <tr class="hover:bg-gray-800/40">
        <td class="p-2.5 font-mono text-gray-400 text-[10px]">${timeStr}</td>
        <td class="p-2.5 font-bold text-white">${log.title}</td>
        <td class="p-2.5 text-gray-300">${log.recipient}</td>
        <td class="p-2.5 text-center">
          <span class="text-[9px] px-1.5 py-0.5 rounded font-bold ${log.is_dummy ? 'bg-amber-500/20 text-amber-300' : 'bg-sky-500/20 text-sky-300'}">
            ${log.is_dummy ? 'DUMMY' : 'LIVE'}
          </span>
        </td>
        <td class="p-2.5 text-center">
          <span class="text-[9px] px-2 py-0.5 rounded-full font-bold bg-emerald-500/20 text-emerald-400 flex items-center justify-center gap-1">
            <span class="w-1.5 h-1.5 rounded-full bg-emerald-400"></span> ${log.status}
          </span>
        </td>
      </tr>
    `;
  }).join('');
}

function clearTelegramLogs() {
  telegramLogs = [];
  renderTelegramLogsTable();
  showToast("Log Telegram dibersihkan.", "info");
}

function toggleTgModeRadio(isDummy) {
  telegramConfig.is_dummy_mode = isDummy;
  const section = document.getElementById("tgRealConfigSection");
  const badge = document.getElementById("tgModeBadge");

  if (isDummy) {
    if (section) {
      section.classList.add("opacity-60", "pointer-events-none");
    }
    if (badge) {
      badge.textContent = "Mode Dummy / Proposal";
      badge.className = "text-[10px] px-2 py-0.5 rounded-full font-bold bg-amber-500/20 text-amber-300 border border-amber-500/30";
    }
  } else {
    if (section) {
      section.classList.remove("opacity-60", "pointer-events-none");
    }
    if (badge) {
      badge.textContent = "Mode Telegram Live API";
      badge.className = "text-[10px] px-2 py-0.5 rounded-full font-bold bg-sky-500/20 text-sky-300 border border-sky-500/30";
    }
  }
}

async function loadTelegramConfigFromStorageOrServer() {
  if (isConnectedToServer) {
    try {
      const res = await fetch('/api/telegram/config');
      const json = await res.json();
      if (json.success && json.config) {
        telegramConfig = { ...telegramConfig, ...json.config };
      }
    } catch (e) {
      // fallback
    }
  } else {
    const saved = localStorage.getItem("openaksara_tg_config");
    if (saved) {
      try {
        telegramConfig = JSON.parse(saved);
      } catch (e) {}
    }
  }

  const isDummy = telegramConfig.is_dummy_mode !== false;
  const dummyRadio = document.querySelector('input[name="tgModeRadio"][value="dummy"]');
  const liveRadio = document.querySelector('input[name="tgModeRadio"][value="live"]');
  if (isDummy && dummyRadio) dummyRadio.checked = true;
  else if (!isDummy && liveRadio) liveRadio.checked = true;

  if (document.getElementById("tgInputToken") && telegramConfig.bot_token) {
    document.getElementById("tgInputToken").value = telegramConfig.bot_token;
  }
  if (document.getElementById("tgInputChatId") && telegramConfig.chat_id) {
    document.getElementById("tgInputChatId").value = telegramConfig.chat_id;
  }
  toggleTgModeRadio(isDummy);
}

async function saveTelegramSettings() {
  const isDummy = document.querySelector('input[name="tgModeRadio"]:checked').value === 'dummy';
  const token = document.getElementById("tgInputToken").value.trim();
  const chatId = document.getElementById("tgInputChatId").value.trim();

  telegramConfig.is_dummy_mode = isDummy;
  telegramConfig.bot_token = token;
  telegramConfig.chat_id = chatId;

  if (isConnectedToServer) {
    try {
      await fetch('/api/telegram/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(telegramConfig)
      });
    } catch (e) {}
  } else {
    localStorage.setItem("openaksara_tg_config", JSON.stringify(telegramConfig));
  }

  toggleTgModeRadio(isDummy);
  showToast("Pengaturan Telegram berhasil disimpan!", "success");
  switchTelegramTab('preview');
}

// Telegram Sound Synthesizer (Realistic Telegram Double Chime: D5 -> A5)
function playTelegramSound() {
  try {
    const AudioContext = window.AudioContext || window.webkitAudioContext;
    if (!AudioContext) return;
    const ctx = new AudioContext();

    const osc = ctx.createOscillator();
    const gain = ctx.createGain();

    osc.type = "sine";

    // Telegram's distinctive double chime (587.33 Hz [D5] -> 880.00 Hz [A5])
    osc.frequency.setValueAtTime(587.33, ctx.currentTime);
    osc.frequency.setValueAtTime(880.00, ctx.currentTime + 0.08);

    gain.gain.setValueAtTime(0.2, ctx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.45);

    osc.connect(gain);
    gain.connect(ctx.destination);

    osc.start();
    osc.stop(ctx.currentTime + 0.45);
  } catch (e) {
    // Audio autoplay restrictions
  }
}
