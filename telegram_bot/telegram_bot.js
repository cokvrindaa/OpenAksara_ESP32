/**
 * OpenAksara - Standalone Telegram Bridge Worker
 * 
 * Sistem independen di luar web untuk mengambil data baru dari Cloud Firestore
 * dan langsung meneruskannya ke Bot Telegram Guru / Wali Murid secara real-time.
 * 
 * - Tanpa perlu membuka browser / web
 * - Zero dependency (murni menggunakan modul bawaan Node.js: https, fs, path)
 * - Otomatis mendeteksi submission latihan, rapor materi, dan feedback baru
 */

const https = require('https');
const fs = require('fs');
const path = require('path');

// 1. Baca Konfigurasi (.env atau secrets)
function loadConfig() {
  const candidates = [
    path.join(__dirname, '.env'),
    path.join(__dirname, '..', '.env'),
    path.join(__dirname, '..', '..', '.env')
  ];
  const env = {};
  for (const c of candidates) {
    if (fs.existsSync(c)) {
      const lines = fs.readFileSync(c, 'utf8').split('\n');
      for (let line of lines) {
        line = line.trim();
        if (!line || line.startsWith('#')) continue;
        const idx = line.indexOf('=');
        if (idx !== -1) {
          const key = line.substring(0, idx).trim();
          const val = line.substring(idx + 1).trim().replace(/(^["']|["']$)/g, '');
          env[key] = val;
          if (!process.env[key]) process.env[key] = val;
        }
      }
      break;
    }
  }

  // Fallback membaca projectId & apiKey dari web/firebase-keys.js jika di .env belum ada
  let projectId = process.env.FIREBASE_PROJECT_ID || env.FIREBASE_PROJECT_ID || '';
  let apiKey = process.env.FIREBASE_API_KEY || env.FIREBASE_API_KEY || '';

  if (!projectId || !apiKey) {
    const keysPath = path.join(__dirname, '..', 'web', 'firebase-keys.js');
    if (fs.existsSync(keysPath)) {
      const raw = fs.readFileSync(keysPath, 'utf8');
      const pMatch = raw.match(/projectId\s*:\s*["']([^"']+)["']/);
      const kMatch = raw.match(/apiKey\s*:\s*["']([^"']+)["']/);
      if (!projectId && pMatch) projectId = pMatch[1];
      if (!apiKey && kMatch) apiKey = kMatch[1];
    }
  }

  const botToken = (process.env.TELEGRAM_BOT_TOKEN || env.TELEGRAM_BOT_TOKEN || '').trim();
  const chatId = (process.env.TELEGRAM_CHAT_ID || env.TELEGRAM_CHAT_ID || '').trim();

  return {
    projectId: projectId || 'aksarabuddy-edu',
    apiKey,
    botToken,
    chatId,
    pollIntervalMs: parseInt(process.env.POLL_INTERVAL_MS || '5000', 10)
  };
}

const CONFIG = loadConfig();

function escapeHtml(str) {
  return String(str === undefined || str === null ? '' : str)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}

function formatTime(iso) {
  const d = iso ? new Date(iso) : new Date();
  if (isNaN(d.getTime())) return '-';
  const months = ['Jan', 'Feb', 'Mar', 'Apr', 'Mei', 'Jun', 'Jul', 'Agu', 'Sep', 'Okt', 'Nov', 'Des'];
  const hh = String(d.getHours()).padStart(2, '0');
  const mm = String(d.getMinutes()).padStart(2, '0');
  return `${d.getDate()} ${months[d.getMonth()]} ${d.getFullYear()}, ${hh}:${mm}`;
}

// 2. Kirim Pesan ke Telegram Bot via HTTPS
function sendTelegramMessage(botToken, chatId, htmlText) {
  return new Promise((resolve, reject) => {
    if (!botToken || !chatId) {
      return reject(new Error('Bot Token atau Chat ID belum diisi di .env!'));
    }

    const postData = JSON.stringify({
      chat_id: chatId,
      text: htmlText,
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
          const json = JSON.parse(body || '{}');
          if (json.ok) resolve(json);
          else reject(new Error(json.description || `HTTP ${res.statusCode}`));
        } catch (e) {
          reject(e);
        }
      });
    });

    req.on('error', reject);
    req.write(postData);
    req.end();
  });
}

// 3. Ambil Dokumen dari Firestore REST API
function fetchFirestoreDocuments(projectId, apiKey, collectionName, pageToken) {
  return new Promise((resolve, reject) => {
    let qs = `pageSize=100&key=${encodeURIComponent(apiKey)}`;
    if (pageToken) qs += `&pageToken=${encodeURIComponent(pageToken)}`;

    const options = {
      hostname: 'firestore.googleapis.com',
      port: 443,
      path: `/v1/projects/${encodeURIComponent(projectId)}/databases/(default)/documents/${encodeURIComponent(collectionName)}?${qs}`,
      method: 'GET'
    };

    const req = https.request(options, (res) => {
      let body = '';
      res.on('data', chunk => body += chunk);
      res.on('end', () => {
        try {
          const json = JSON.parse(body || '{}');
          if (res.statusCode >= 200 && res.statusCode < 300) {
            resolve(json);
          } else {
            reject(new Error((json.error && json.error.message) || `Firestore HTTP ${res.statusCode}`));
          }
        } catch (e) {
          reject(e);
        }
      });
    });

    req.on('error', reject);
    req.setTimeout(8000, () => req.destroy(new Error('Timeout koneksi ke Firestore')));
    req.end();
  });
}

function decodeFields(fields) {
  const out = {};
  if (!fields) return out;
  for (const [k, v] of Object.entries(fields)) {
    if (v.stringValue !== undefined) out[k] = v.stringValue;
    else if (v.integerValue !== undefined) out[k] = Number(v.integerValue);
    else if (v.doubleValue !== undefined) out[k] = Number(v.doubleValue);
    else if (v.booleanValue !== undefined) out[k] = Boolean(v.booleanValue);
    else if (v.timestampValue !== undefined) out[k] = v.timestampValue;
  }
  return out;
}

// 4. Format Template Notifikasi Telegram
function buildMessage(collection, doc) {
  const d = decodeFields(doc.fields);
  const timeStr = formatTime(d.createdAt || doc.createTime);

  if (collection === 'submissions') {
    const score = d.score || 0;
    const isPass = score >= 75;
    const emoji = score >= 90 ? '🌟' : (isPass ? '✅' : '⚠️');
    return (
      `${emoji} <b>OPENAKSARA • HASIL LATIHAN SISWA</b>\n` +
      `━━━━━━━━━━━━━━━━━━━━━━━\n` +
      `👤 <b>Nama Siswa:</b> ${escapeHtml(d.name || 'Siswa')}\n` +
      `🏫 <b>Kelas:</b> ${escapeHtml(d.class || '-')}\n` +
      `✍️ <b>Aksara:</b> ${escapeHtml(d.aksara || '-')} (${escapeHtml(d.script || 'Nusantara')})\n` +
      `🎯 <b>Akurasi Goresan:</b> ${escapeHtml(d.precision || '-')}% \n` +
      `🏆 <b>Skor:</b> <b>${escapeHtml(score)} / 100</b> (${escapeHtml(d.status || (isPass ? 'Lulus' : 'Remedial'))})\n` +
      `🕒 <b>Waktu:</b> ${timeStr}\n` +
      `━━━━━━━━━━━━━━━━━━━━━━━\n` +
      `🤖 <i>Dikirim otomatis dari Cloud Firestore ESP32</i>`
    );
  }

  if (collection === 'assessments') {
    return (
      `🏅 <b>OPENAKSARA • EVALUASI MATERI SELESAI</b>\n` +
      `━━━━━━━━━━━━━━━━━━━━━━━\n` +
      `👤 <b>Nama Siswa:</b> ${escapeHtml(d.studentName || 'Siswa')} (${escapeHtml(d.class || '-')})\n` +
      `📚 <b>Materi:</b> ${escapeHtml(d.materiTitle || '-')}\n` +
      `🏆 <b>Nilai Akhir:</b> <b>${escapeHtml(d.finalScore || '-')}</b> (${escapeHtml(d.grade || '-')})\n` +
      `⚡ <b>Perolehan XP:</b> +${escapeHtml(d.totalXp || 0)} XP\n` +
      `💡 <b>Catatan:</b> <i>${escapeHtml(d.feedback || 'Tuntas.')}</i>\n` +
      `🕒 <b>Waktu:</b> ${timeStr}\n` +
      `━━━━━━━━━━━━━━━━━━━━━━━\n` +
      `🤖 <i>Dikirim otomatis dari Cloud Firestore ESP32</i>`
    );
  }

  return (
    `💬 <b>OPENAKSARA • PESAN & CATATAN WALI</b>\n` +
    `━━━━━━━━━━━━━━━━━━━━━━━\n` +
    `📱 <b>Target:</b> ${escapeHtml(d.target || 'Sabak Siswa')}\n` +
    `📝 <b>Isi Pesan:</b> "${escapeHtml(d.text || '-')}"\n` +
    `🕒 <b>Waktu:</b> ${timeStr}\n` +
    `━━━━━━━━━━━━━━━━━━━━━━━\n` +
    `🤖 <i>Dikirim otomatis dari Cloud Firestore ESP32</i>`
  );
}

// 5. Polling Loop Utama
const COLLECTIONS = ['submissions', 'assessments', 'feedbacks'];
const seenDocs = new Set();
let isBaselineDone = false;

async function checkNewData() {
  for (const col of COLLECTIONS) {
    try {
      const res = await fetchFirestoreDocuments(CONFIG.projectId, CONFIG.apiKey, col);
      const docs = res.documents || [];

      if (!isBaselineDone) {
        // Pada putaran awal pertama, catat semua data lama agar tidak membanjiri Telegram
        docs.forEach(doc => { if (doc.name) seenDocs.add(doc.name); });
        continue;
      }

      // Cari dokumen yang belum pernah dikirim
      const freshDocs = docs
        .filter(doc => doc.name && !seenDocs.has(doc.name))
        .sort((a, b) => Date.parse(a.createTime || 0) - Date.parse(b.createTime || 0));

      for (const doc of freshDocs) {
        seenDocs.add(doc.name);
        const msg = buildMessage(col, doc);
        console.log(`[${new Date().toLocaleTimeString()}] 📨 Mendeteksi data baru di '${col}', mengirim ke Telegram...`);

        try {
          await sendTelegramMessage(CONFIG.botToken, CONFIG.chatId, msg);
          console.log(`[${new Date().toLocaleTimeString()}] ✅ Berhasil terkirim ke Telegram (Chat ID: ${CONFIG.chatId})`);
        } catch (sendErr) {
          console.error(`[${new Date().toLocaleTimeString()}] ❌ Gagal kirim ke Telegram:`, sendErr.message);
        }
      }
    } catch (fetchErr) {
      // Abaikan error jaringan sementara agar bot tetap hidup
    }
  }

  if (!isBaselineDone) {
    isBaselineDone = true;
    console.log(`[${new Date().toLocaleTimeString()}] 🟢 Baseline selesai (${seenDocs.size} data lama dicatat). Siap memantau data baru secara realtime!`);
  }
}

// 6. Jalankan Worker
console.log('================================================================');
console.log('🤖 OpenAksara - Standalone Telegram Bridge Worker');
console.log('================================================================');
console.log(`🔗 Project Firestore : ${CONFIG.projectId}`);
console.log(`🔑 Bot Token         : ${CONFIG.botToken ? 'Terisi (Siap Kirim)' : 'KOSONG! Harap isi TELEGRAM_BOT_TOKEN di .env'}`);
console.log(`💬 Chat ID Tujuan    : ${CONFIG.chatId || 'KOSONG! Harap isi TELEGRAM_CHAT_ID di .env'}`);
console.log(`⏱️ Interval Polling  : ${CONFIG.pollIntervalMs / 1000} detik`);
console.log('================================================================');

checkNewData();
setInterval(checkNewData, CONFIG.pollIntervalMs);
