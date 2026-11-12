/**
 * AksaraBuddy - Full Firebase Integration Engine
 * Mengintegrasikan SEMUA Fitur Dashboard ke Firebase Firestore & Auth:
 * 1. Autentikasi Guru (Firebase Auth Login & Profile Sync)
 * 2. Submission Siswa Real-time (Collection 'submissions')
 * 3. Penilaian Materi & XP (Collection 'assessments')
 * 4. Leaderboard & Data Siswa (Collection 'students')
 * 5. Feedback Guru ke Sabak ESP32 (Collection 'feedbacks')
 * 6. Setelan Hardware & Kalibrasi ESP32 (Document 'settings/esp32_hardware')
 * 7. One-Click Data Seeder (Migrasikan Data Default ke Firestore)
 */

// Konfigurasi aktif Firebase
const FIREBASE_CONFIG = {
  apiKey: "",
  authDomain: "",
  projectId: "",
  storageBucket: "",
  messagingSenderId: "",
  appId: ""
};

// State Status & Client Firebase
const FirebaseState = {
  isConfigured: false,
  isConnected: false,
  app: null,
  db: null,
  auth: null,
  currentUser: null,
  unsubscribers: []
};

// Cek apakah konfigurasi sudah valid diisi (bukan placeholder template)
function checkFirebaseConfig(cfg) {
  const target = cfg || FIREBASE_CONFIG;
  return Boolean(
    target.apiKey && 
    target.apiKey.trim() !== "" && 
    !target.apiKey.includes("MASUKKAN_") &&
    !target.apiKey.includes("your_firebase_api_key") &&
    target.projectId && 
    target.projectId.trim() !== "" &&
    !target.projectId.includes("proyek-anda")
  );
}

/**
 * Inisialisasi Firebase App, Auth, dan Firestore
 */
async function initFirebase() {
  // 1. Coba ambil dari window.FIREBASE_ENV, window.firebaseConfig, atau global firebaseConfig
  let foundConfig = null;
  if (typeof window !== 'undefined') {
    if (window.FIREBASE_ENV && checkFirebaseConfig(window.FIREBASE_ENV)) {
      foundConfig = window.FIREBASE_ENV;
    } else if (window.firebaseConfig && checkFirebaseConfig(window.firebaseConfig)) {
      foundConfig = window.firebaseConfig;
    }
  }
  if (!foundConfig) {
    try {
      if (typeof firebaseConfig !== 'undefined' && checkFirebaseConfig(firebaseConfig)) {
        foundConfig = firebaseConfig;
      }
    } catch (e) {}
  }
  if (foundConfig) {
    Object.assign(FIREBASE_CONFIG, foundConfig);
  }

  // 2. Jika belum ada, coba fetch dari server Node.js /api/config (yang membaca .env)
  if (!checkFirebaseConfig(FIREBASE_CONFIG)) {
    try {
      const res = await fetch('/api/config');
      if (res.ok) {
        const json = await res.json();
        if (json.firebase && checkFirebaseConfig(json.firebase)) {
          Object.assign(FIREBASE_CONFIG, json.firebase);
        }
      }
    } catch (e) {}
  }

  FirebaseState.isConfigured = checkFirebaseConfig(FIREBASE_CONFIG);
  
  if (!FirebaseState.isConfigured) {
    console.log("%c[AksaraBuddy Database] Status: LOCAL / DEMO MODE", "color: #0d9488; font-weight: bold;");
    updateDatabaseStatusBadge(false, "Demo Mode");
    return false;
  }

  try {
    if (typeof firebase !== 'undefined') {
      if (!firebase.apps.length) {
        FirebaseState.app = firebase.initializeApp(FIREBASE_CONFIG);
      } else {
        FirebaseState.app = firebase.app();
      }
      
      FirebaseState.db = firebase.firestore();
      FirebaseState.auth = firebase.auth();
      FirebaseState.isConnected = true;

      // Monitor status auth Firebase
      FirebaseState.auth.onAuthStateChanged((user) => {
        FirebaseState.currentUser = user;
        if (user) {
          console.log(`[AksaraBuddy Auth] Guru login sebagai: ${user.email}`);
        }
      });

      console.log("%c[AksaraBuddy Database] 🟢 Berhasil tersambung ke Firebase Firestore & Auth!", "color: #10b981; font-weight: bold;");
      updateDatabaseStatusBadge(true, "Firebase Cloud");

      // Aktifkan semua Realtime Listeners
      setupAllRealtimeListeners();
      return true;
    } else {
      console.warn("[AksaraBuddy Database] SDK Firebase belum termuat.");
      updateDatabaseStatusBadge(false, "SDK Not Loaded");
      return false;
    }
  } catch (err) {
    console.error("[AksaraBuddy Database] Gagal inisialisasi Firebase:", err);
    updateDatabaseStatusBadge(false, "Config Error");
    return false;
  }
}

/**
 * Update badge status koneksi database pada header
 */
function updateDatabaseStatusBadge(connected, label) {
  const badge = document.getElementById('db-status-badge');
  const text = document.getElementById('db-status-text');
  const dot = document.getElementById('db-status-dot');

  if (badge && text && dot) {
    if (connected) {
      badge.className = "flex items-center gap-1.5 px-2.5 py-1.5 rounded-xl text-[11px] font-bold bg-emerald-50 text-emerald-700 border border-emerald-200 shadow-xs shrink-0 cursor-pointer hover:bg-emerald-100 transition";
      dot.className = "w-2 h-2 rounded-full bg-emerald-500 animate-pulse";
      text.innerText = label || "Firebase Cloud";
    } else {
      badge.className = "flex items-center gap-1.5 px-2.5 py-1.5 rounded-xl text-[11px] font-bold bg-amber-50 text-amber-800 border border-amber-200 shadow-xs shrink-0 cursor-pointer hover:bg-amber-100 transition";
      dot.className = "w-2 h-2 rounded-full bg-amber-500";
      text.innerText = label || "Demo Mode";
    }
  }
}

// =========================================================================
// 2. REALTIME LISTENERS UNTUK SEMUA FITUR
// =========================================================================

/**
 * Parsing waktu fleksibel untuk data Firebase (Timestamp Firestore, string ISO, atau teks)
 * Mengembalikan epoch milidetik agar pengurutan 'terbaru ke terlama' presisi & tidak crash.
 */
function parseFirebaseItemTimestamp(item) {
  if (!item) return 0;
  const raw = item.createdAt || item.completedDate || item.time || item.timestamp;
  if (raw) {
    if (typeof raw.toMillis === 'function') return raw.toMillis();
    if (typeof raw.toDate === 'function') return raw.toDate().getTime();
    if (typeof raw === 'object' && raw.seconds !== undefined) {
      return raw.seconds * 1000 + (raw.nanoseconds ? raw.nanoseconds / 1000000 : 0);
    }
    if (typeof raw === 'number') return raw;
    const parsed = Date.parse(raw);
    if (!isNaN(parsed)) return parsed;
  }
  const str = String(item.time || item.completedDate || '').toLowerCase();
  if (str.includes('baru') || str.includes('detik')) return Date.now();
  if (str.includes('hari ini')) {
    const match = str.match(/(\d{1,2}):(\d{2})/);
    const d = new Date();
    if (match) d.setHours(parseInt(match[1], 10), parseInt(match[2], 10), 0, 0);
    return d.getTime();
  }
  if (str.includes('kemarin')) {
    const match = str.match(/(\d{1,2}):(\d{2})/);
    const d = new Date(Date.now() - 86400000);
    if (match) d.setHours(parseInt(match[1], 10), parseInt(match[2], 10), 0, 0);
    return d.getTime();
  }
  const parsed = Date.parse(str);
  if (!isNaN(parsed)) return parsed;
  return 0;
}

function setupAllRealtimeListeners() {
  if (!FirebaseState.db) return;

  // Bersihkan listener lama jika ada
  FirebaseState.unsubscribers.forEach(unsub => unsub());
  FirebaseState.unsubscribers = [];

  try {
    // -------------------------------------------------------------
    // FITUR 1: SUBMISSIONS SISWA (REALTIME DARI HARDWARE / WEB)
    // -------------------------------------------------------------
    const unsubSub = FirebaseState.db.collection('submissions')
      .limit(50)
      .onSnapshot((snapshot) => {
        if (!snapshot.empty) {
          const liveSubmissions = [];
          snapshot.forEach(doc => {
            const d = doc.data();
            liveSubmissions.push({
              id: doc.id,
              name: d.name || "Siswa",
              class: d.class || "5A",
              script: d.script || "Jawa",
              aksara: d.aksara || "-",
              precision: d.precision !== undefined ? d.precision : 85,
              score: d.score !== undefined ? d.score : 80,
              status: d.status || (d.score >= 90 ? "Sempurna" : (d.score >= 75 ? "Lulus" : "Remedial")),
              time: d.time || "Baru saja",
              createdAt: d.createdAt || ""
            });
          });

          // Urutkan submission terbaru di atas (Terbaru ke Terlama / Descending)
          liveSubmissions.sort((a, b) => parseFirebaseItemTimestamp(b) - parseFirebaseItemTimestamp(a));

          if (typeof window.onFirebaseSubmissionsSync === 'function') {
            window.onFirebaseSubmissionsSync(liveSubmissions);
          }
        }
      }, err => console.warn("[Firestore] Submissions listener error:", err.message));
    FirebaseState.unsubscribers.push(unsubSub);

    // -------------------------------------------------------------
    // FITUR 2: PENILAIAN MATERI & RAPOR XP SISWA
    // -------------------------------------------------------------
    const unsubAssess = FirebaseState.db.collection('assessments')
      .limit(40)
      .onSnapshot((snapshot) => {
        if (!snapshot.empty) {
          const liveAssessments = [];
          snapshot.forEach(doc => {
            const d = doc.data();
            liveAssessments.push({
              id: doc.id,
              studentName: d.studentName || "Siswa",
              class: d.class || "5A",
              avatar: d.avatar || "SW",
              materiTitle: d.materiTitle || "Aksara Nusantara",
              script: d.script || "Jawa",
              completedDate: d.completedDate || "Hari ini",
              createdAt: d.createdAt || "",
              accuracyScore: d.accuracyScore || 85,
              speedScore: d.speedScore || 80,
              strokeStability: d.strokeStability || 82,
              finalScore: d.finalScore || 85,
              grade: d.grade || "Grade A",
              xpEarned: d.xpEarned || 200,
              bonusStreakXp: d.bonusStreakXp || 0,
              totalXp: d.totalXp || (d.xpEarned || 200),
              status: d.status || "TUNTAS",
              feedback: d.feedback || "Evaluasi materi tuntas."
            });
          });

          // Urutkan assessment terbaru di atas (Terbaru ke Terlama / Descending)
          liveAssessments.sort((a, b) => parseFirebaseItemTimestamp(b) - parseFirebaseItemTimestamp(a));

          if (typeof window.onFirebaseAssessmentsSync === 'function') {
            window.onFirebaseAssessmentsSync(liveAssessments);
          }
        }
      }, err => console.warn("[Firestore] Assessments listener error:", err.message));
    FirebaseState.unsubscribers.push(unsubAssess);

    // -------------------------------------------------------------
    // FITUR 3: LEADERBOARD KELAS & SISWA
    // -------------------------------------------------------------
    const unsubStudents = FirebaseState.db.collection('students')
      .orderBy('xp', 'desc')
      .limit(30)
      .onSnapshot((snapshot) => {
        if (!snapshot.empty) {
          const liveStudents = [];
          snapshot.forEach(doc => {
            const d = doc.data();
            liveStudents.push({
              id: doc.id,
              name: d.name || "Siswa",
              class: d.class || "5A",
              xp: d.xp || 0,
              acc: d.acc || "85%",
              badge: d.badge || "Pelajar Teladan"
            });
          });

          if (typeof window.onFirebaseStudentsSync === 'function') {
            window.onFirebaseStudentsSync(liveStudents);
          }
        }
      }, err => console.warn("[Firestore] Students listener error:", err.message));
    FirebaseState.unsubscribers.push(unsubStudents);

    // -------------------------------------------------------------
    // FITUR 4: FEEDBACK GURU KE SABAK SISWA (AUDIO & TEKS)
    // -------------------------------------------------------------
    const unsubFeedbacks = FirebaseState.db.collection('feedbacks')
      .limit(30)
      .onSnapshot((snapshot) => {
        if (!snapshot.empty) {
          const liveFeedbacks = [];
          snapshot.forEach(doc => {
            const d = doc.data();
            liveFeedbacks.push({
              id: doc.id,
              time: d.time || "Baru saja",
              createdAt: d.createdAt || "",
              target: d.target || "Sabak Siswa",
              text: d.text || "-",
              status: d.status || "Terkirim (Cloud)"
            });
          });

          // Urutkan feedback terbaru di atas (Terbaru ke Terlama / Descending)
          liveFeedbacks.sort((a, b) => parseFirebaseItemTimestamp(b) - parseFirebaseItemTimestamp(a));

          if (typeof window.onFirebaseFeedbacksSync === 'function') {
            window.onFirebaseFeedbacksSync(liveFeedbacks);
          }
        }
      }, err => console.warn("[Firestore] Feedbacks listener error:", err.message));
    FirebaseState.unsubscribers.push(unsubFeedbacks);

    // -------------------------------------------------------------
    // FITUR 5: SETELAN ESP32 & KALIBRASI HARDWARE
    // -------------------------------------------------------------
    const unsubSettings = FirebaseState.db.collection('settings').doc('esp32_hardware')
      .onSnapshot((doc) => {
        if (doc.exists) {
          const cfg = doc.data();
          if (typeof window.onFirebaseSettingsSync === 'function') {
            window.onFirebaseSettingsSync(cfg);
          }
        }
      }, err => console.warn("[Firestore] Settings listener error:", err.message));
    FirebaseState.unsubscribers.push(unsubSettings);

  } catch (err) {
    console.error("[AksaraBuddy Firebase] Error setting up listeners:", err);
  }
}

// =========================================================================
// 3. API METHOD ACTIONS (CRUD KE FIREBASE)
// =========================================================================

/**
 * Login Guru via Firebase Authentication
 */
async function firebaseLoginTeacher(email, password) {
  if (!FirebaseState.auth) {
    return { success: false, demoOnly: true, error: "Firebase Auth belum terhubung." };
  }

  try {
    const cred = await FirebaseState.auth.signInWithEmailAndPassword(email, password);
    return { success: true, user: cred.user };
  } catch (err) {
    // Jika user belum ada, daftarkan otomatis agar guru langsung bisa login
    if (err.code === 'auth/user-not-found' || err.code === 'auth/invalid-credential') {
      try {
        const newCred = await FirebaseState.auth.createUserWithEmailAndPassword(email, password);
        return { success: true, user: newCred.user, isNew: true };
      } catch (createErr) {
        return { success: false, error: createErr.message };
      }
    }
    return { success: false, error: err.message };
  }
}

/**
 * Logout Guru dari Firebase
 */
async function firebaseLogoutTeacher() {
  if (FirebaseState.auth) {
    await FirebaseState.auth.signOut();
  }
}

/**
 * Simpan Submission Baru ke Firestore
 */
async function firebaseSaveSubmission(data) {
  if (!FirebaseState.db) return { success: true, localOnly: true };

  try {
    const docRef = await FirebaseState.db.collection('submissions').add({
      ...data,
      createdAt: new Date().toISOString()
    });
    return { success: true, id: docRef.id };
  } catch (err) {
    console.error("[Firestore] Gagal menyimpan submission:", err);
    return { success: false, error: err.message };
  }
}

/**
 * Kirim Pesan Feedback Guru ke Firestore
 */
async function firebaseSendFeedback(data) {
  if (!FirebaseState.db) return { success: true, localOnly: true };

  try {
    const docRef = await FirebaseState.db.collection('feedbacks').add({
      ...data,
      createdAt: new Date().toISOString()
    });
    return { success: true, id: docRef.id };
  } catch (err) {
    console.error("[Firestore] Gagal mengirim feedback:", err);
    return { success: false, error: err.message };
  }
}

/**
 * Simpan Setelan ESP32 ke Firestore
 */
async function firebaseSaveHardwareSettings(settingsData) {
  if (!FirebaseState.db) return { success: true, localOnly: true };

  try {
    await FirebaseState.db.collection('settings').doc('esp32_hardware').set({
      ...settingsData,
      updatedAt: new Date().toISOString()
    }, { merge: true });
    return { success: true };
  } catch (err) {
    console.error("[Firestore] Gagal menyimpan setelan ESP32:", err);
    return { success: false, error: err.message };
  }
}

/**
 * Seed / Migrasi Seluruh Data Default ke Firestore dalam 1 Klik
 */
async function firebaseSeedDefaultData(defaultSubmissions, defaultAssessments, defaultStudents, defaultLogs) {
  if (!FirebaseState.db) {
    alert("Firebase belum terhubung. Harap masukkan API Key di web/firebase-keys.js terlebih dahulu.");
    return false;
  }

  try {
    const batch = FirebaseState.db.batch();

    // 1. Seed Submissions
    if (defaultSubmissions && defaultSubmissions.length) {
      defaultSubmissions.forEach(sub => {
        const ref = FirebaseState.db.collection('submissions').doc();
        batch.set(ref, { ...sub, createdAt: new Date().toISOString() });
      });
    }

    // 2. Seed Assessments (Penilaian Materi)
    if (defaultAssessments && defaultAssessments.length) {
      defaultAssessments.forEach(ass => {
        const ref = FirebaseState.db.collection('assessments').doc(ass.id || undefined);
        batch.set(ref, { ...ass, createdAt: new Date().toISOString() });
      });
    }

    // 3. Seed Students (Leaderboard)
    if (defaultStudents && defaultStudents.length) {
      defaultStudents.forEach(stu => {
        const ref = FirebaseState.db.collection('students').doc();
        batch.set(ref, { ...stu, createdAt: new Date().toISOString() });
      });
    }

    // 4. Seed Hardware Settings
    const settingsRef = FirebaseState.db.collection('settings').doc('esp32_hardware');
    batch.set(settingsRef, {
      baudRate: 115200,
      port: "COM3",
      tolerancePx: 18,
      dfplayerVolume: 22,
      audioMode: "speech_and_melody",
      vcc: 4.98,
      updatedAt: new Date().toISOString()
    });

    await batch.commit();
    console.log("[Firestore] Seluruh data awal berhasil disinkronkan ke Firestore!");
    return true;
  } catch (err) {
    console.error("[Firestore] Gagal melakukan seeder:", err);
    alert("Gagal migrasi data: " + err.message);
    return false;
  }
}

// Inisialisasi otomatis
window.addEventListener('DOMContentLoaded', () => {
  initFirebase();
});
