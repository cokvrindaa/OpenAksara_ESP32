const https = require('https');

function httpsJsonRequest(method, urlStr, payloadObj) {
  return new Promise((resolve, reject) => {
    const parsed = new URL(urlStr);
    const bodyStr = payloadObj ? JSON.stringify(payloadObj) : null;
    const headers = { 'User-Agent': 'OpenAksara-Vercel-Bridge' };
    if (bodyStr) {
      headers['Content-Type'] = 'application/json';
      headers['Content-Length'] = Buffer.byteLength(bodyStr);
    }

    const req = https.request(
      {
        hostname: parsed.hostname,
        port: 443,
        path: parsed.pathname + parsed.search,
        method,
        headers,
        timeout: 10000
      },
      (res) => {
        let data = '';
        res.on('data', (chunk) => (data += chunk));
        res.on('end', () => {
          try {
            const json = data ? JSON.parse(data) : {};
            resolve({ statusCode: res.statusCode || 200, json });
          } catch (e) {
            reject(new Error(`Invalid JSON response (${res.statusCode}): ${data.slice(0, 160)}`));
          }
        });
      }
    );

    req.on('timeout', () => {
      req.destroy(new Error('Request timeout'));
    });
    req.on('error', reject);
    if (bodyStr) req.write(bodyStr);
    req.end();
  });
}

function sendTelegram(botToken, chatId, text) {
  return httpsJsonRequest('POST', `https://api.telegram.org/bot${encodeURIComponent(botToken)}/sendMessage`, {
    chat_id: chatId,
    text,
    parse_mode: 'HTML',
    disable_web_page_preview: true
  }).then(({ json }) => {
    if (!json.ok) {
      throw new Error(json.description || 'Telegram API Error');
    }
    return json;
  });
}

function escapeHtml(val) {
  return String(val === undefined || val === null ? '' : val)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}

function fromFirestoreValue(val) {
  if (!val || typeof val !== 'object') return undefined;
  if ('stringValue' in val) return val.stringValue;
  if ('integerValue' in val) return parseInt(val.integerValue, 10);
  if ('doubleValue' in val) return Number(val.doubleValue);
  if ('booleanValue' in val) return Boolean(val.booleanValue);
  if ('timestampValue' in val) return val.timestampValue;
  return undefined;
}

function parseFirestoreDoc(doc) {
  const out = {
    id: doc.name ? doc.name.split('/').pop() : '',
    createTime: doc.createTime || ''
  };
  const fields = doc.fields || {};
  for (const [k, v] of Object.entries(fields)) {
    out[k] = fromFirestoreValue(v);
  }
  return out;
}

function getDocTimestampMs(doc) {
  const raw = doc.createdAt || doc.timestamp || doc.createTime || '';
  const ms = Date.parse(raw);
  return Number.isNaN(ms) ? 0 : ms;
}

function formatWitaLabel(isoStr) {
  const ms = Date.parse(isoStr || '');
  const d = Number.isNaN(ms) ? new Date() : new Date(ms);
  const wita = new Date(d.getTime() + 8 * 3600 * 1000);
  const months = ['Jan', 'Feb', 'Mar', 'Apr', 'Mei', 'Jun', 'Jul', 'Agu', 'Sep', 'Okt', 'Nov', 'Des'];
  const hh = String(wita.getUTCHours()).padStart(2, '0');
  const mm = String(wita.getUTCMinutes()).padStart(2, '0');
  return `${wita.getUTCDate()} ${months[wita.getUTCMonth()]} ${wita.getUTCFullYear()}, ${hh}:${mm} WITA`;
}

function formatFirestoreMessage(colName, item) {
  const waktu = escapeHtml(formatWitaLabel(item.createdAt || item.timestamp || item.createTime));
  if (colName === 'submissions') {
    return (
      `✍️ <b>LATIHAN AKSARA BARU • OPENAKSARA</b>\n` +
      `━━━━━━━━━━━━━━━━━━━━━━━\n` +
      `👤 <b>Siswa:</b> ${escapeHtml(item.name || 'Siswa')} (Kelas ${escapeHtml(item.class || '-')})\n` +
      `🔤 <b>Aksara:</b> ${escapeHtml(item.aksara || '-')} (${escapeHtml(item.script || 'Jawa')})\n` +
      `🎯 <b>Presisi:</b> ${escapeHtml(item.precision ?? '-')}% | 🏆 <b>Skor:</b> ${escapeHtml(item.score ?? '-')}\n` +
      `📌 <b>Status:</b> ${escapeHtml(item.status || 'Selesai')}\n` +
      `🕒 ${waktu}`
    );
  }
  if (colName === 'assessments') {
    return (
      `🏅 <b>RAPOR MATERI & XP • OPENAKSARA</b>\n` +
      `━━━━━━━━━━━━━━━━━━━━━━━\n` +
      `👤 <b>Siswa:</b> ${escapeHtml(item.studentName || 'Siswa')} (Kelas ${escapeHtml(item.class || '-')})\n` +
      `📚 <b>Materi:</b> ${escapeHtml(item.materiTitle || '-')} (${escapeHtml(item.script || '-')})\n` +
      `🏆 <b>Nilai:</b> ${escapeHtml(item.finalScore ?? '-')} (${escapeHtml(item.grade || '-')}) · +${escapeHtml(item.totalXp ?? 0)} XP\n` +
      (item.feedback ? `💬 <i>${escapeHtml(item.feedback)}</i>\n` : '') +
      `🕒 ${waktu}`
    );
  }
  return (
    `💬 <b>FEEDBACK GURU / PERANGKAT • OPENAKSARA</b>\n` +
    `━━━━━━━━━━━━━━━━━━━━━━━\n` +
    `📱 <b>Target:</b> ${escapeHtml(item.target || 'ESP32')}\n` +
    `📝 "${escapeHtml(item.text || '-')}"\n` +
    `🕒 ${waktu}`
  );
}

async function syncFirestoreToTelegram({ botToken, chatId, projectId, apiKey }) {
  if (!botToken || !chatId) {
    throw new Error('TELEGRAM_BOT_TOKEN dan TELEGRAM_CHAT_ID belum diisi di Vercel Environment Variables.');
  }
  if (!projectId || !apiKey) {
    throw new Error('FIREBASE_PROJECT_ID dan FIREBASE_API_KEY belum diisi di Vercel Environment Variables.');
  }

  const base = `https://firestore.googleapis.com/v1/projects/${encodeURIComponent(projectId)}/databases/(default)/documents`;
  const cursorUrl = `${base}/settings/telegram_vercel_sync?key=${encodeURIComponent(apiKey)}`;

  let lastSyncMs = Date.now() - 180000; // Default baseline: 3 menit terakhir pada run perdana
  let isFirstBaseline = true;
  try {
    const cursorRes = await httpsJsonRequest('GET', cursorUrl);
    if (cursorRes.statusCode === 200 && cursorRes.json && cursorRes.json.fields) {
      const parsedCursor = parseFirestoreDoc(cursorRes.json);
      if (parsedCursor.lastSyncMs && Number(parsedCursor.lastSyncMs) > 0) {
        lastSyncMs = Number(parsedCursor.lastSyncMs);
        isFirstBaseline = false;
      }
    }
  } catch (_) {}

  const collections = ['submissions', 'assessments', 'feedbacks'];
  const newEvents = [];
  let maxSeenMs = lastSyncMs;

  for (const col of collections) {
    const colUrl = `${base}/${col}?pageSize=25&key=${encodeURIComponent(apiKey)}`;
    const colRes = await httpsJsonRequest('GET', colUrl);
    if (colRes.statusCode !== 200 || !colRes.json || !Array.isArray(colRes.json.documents)) {
      continue;
    }
    for (const rawDoc of colRes.json.documents) {
      const item = parseFirestoreDoc(rawDoc);
      const tsMs = getDocTimestampMs(item);
      if (tsMs > maxSeenMs) maxSeenMs = tsMs;
      if (tsMs > lastSyncMs) {
        newEvents.push({ col, item, tsMs });
      }
    }
  }

  newEvents.sort((a, b) => a.tsMs - b.tsMs);
  const toSend = newEvents.slice(-10); // Batasi maksimal 10 pesan per panggilan agar aman dari rate-limit
  const logs = [];

  for (const ev of toSend) {
    const msg = formatFirestoreMessage(ev.col, ev.item);
    const label =
      ev.col === 'submissions'
        ? `Latihan: ${ev.item.name || 'Siswa'} (${ev.item.aksara || '-'})`
        : ev.col === 'assessments'
        ? `Rapor: ${ev.item.studentName || 'Siswa'} (+${ev.item.totalXp || 0} XP)`
        : `Feedback: ${ev.item.target || 'ESP32'}`;
    try {
      await sendTelegram(botToken, chatId, msg);
      logs.push({
        id: 'TG-' + Date.now().toString().slice(-6),
        timestamp: new Date().toISOString(),
        type: ev.col,
        title: label,
        recipient: chatId,
        status: 'Delivered',
        is_dummy: false
      });
    } catch (err) {
      logs.push({
        id: 'TG-' + Date.now().toString().slice(-6),
        timestamp: new Date().toISOString(),
        type: ev.col,
        title: `${label} (${err.message})`,
        recipient: chatId,
        status: 'Failed',
        is_dummy: false
      });
    }
  }

  const nextCursorMs = Math.max(maxSeenMs, Date.now());
  try {
    await httpsJsonRequest('PATCH', cursorUrl, {
      fields: {
        lastSyncMs: { integerValue: String(nextCursorMs) },
        updatedAt: { timestampValue: new Date().toISOString() }
      }
    });
  } catch (_) {}

  return {
    isFirstBaseline,
    forwardedCount: logs.filter((l) => l.status === 'Delivered').length,
    totalNewFound: newEvents.length,
    logs
  };
}

function getAction(req) {
  try {
    const u = new URL(req.url || '/api/telegram', 'http://localhost');
    const qAction = (u.searchParams.get('action') || '').trim().toLowerCase();
    if (qAction) return qAction;
    const parts = u.pathname.replace(/\/+$/, '').split('/').filter(Boolean);
    const last = parts[parts.length - 1] || '';
    if (last && last !== 'telegram' && last !== 'telegram.js') {
      return last.toLowerCase();
    }
  } catch (_) {}
  return '';
}

function readBodyPayload(req) {
  if (!req.body) return {};
  if (typeof req.body === 'string') {
    try {
      return JSON.parse(req.body || '{}');
    } catch (_) {
      return {};
    }
  }
  return req.body;
}

module.exports = async (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'OPTIONS') {
    return res.status(204).end();
  }

  const envToken = (process.env.TELEGRAM_BOT_TOKEN || '').trim();
  const envChatId = (process.env.TELEGRAM_CHAT_ID || '').trim();
  const envBotName = (process.env.TELEGRAM_BOT_NAME || '@OpenAksaraBot').trim();
  const envProjectId = (process.env.FIREBASE_PROJECT_ID || '').trim();
  const envApiKey = (process.env.FIREBASE_API_KEY || '').trim();
  const action = getAction(req);
  const payload = readBodyPayload(req);

  // 1. GET/POST /api/telegram/config (atau ?action=config)
  if (action === 'config' || (req.method === 'GET' && !action)) {
    if (req.method === 'POST') {
      const targetToken = (payload.bot_token || envToken).trim();
      const targetChat = (payload.chat_id || envChatId).trim();
      return res.status(200).json({
        success: true,
        mode: 'vercel',
        message: 'Pengaturan Telegram disimpan untuk sesi web Vercel.',
        has_token: !!targetToken,
        is_dummy_mode: !(targetToken && targetChat)
      });
    }

    return res.status(200).json({
      success: true,
      mode: 'vercel',
      config: {
        mode: 'vercel',
        watcher_active: false,
        is_dummy_mode: !(envToken && envChatId),
        firebase_auto_forward: true,
        has_token: !!envToken,
        chat_id: envChatId,
        bot_name: envBotName,
        firebase_ready: !!(envProjectId && envApiKey)
      }
    });
  }

  // 2. GET /api/telegram/logs (atau ?action=logs)
  if (action === 'logs' && req.method === 'GET') {
    return res.status(200).json({
      success: true,
      mode: 'vercel',
      logs: []
    });
  }

  // 3. GET/POST /api/telegram/sync (Tarik data baru Firestore -> Telegram langsung di Vercel)
  if (action === 'sync') {
    try {
      const targetToken = (payload.bot_token || envToken).trim();
      const targetChat = (payload.chat_id || envChatId).trim();
      const projectId = (payload.project_id || envProjectId).trim();
      const apiKey = (payload.api_key || envApiKey).trim();
      const syncResult = await syncFirestoreToTelegram({
        botToken: targetToken,
        chatId: targetChat,
        projectId,
        apiKey
      });
      return res.status(200).json({ success: true, mode: 'vercel', ...syncResult });
    } catch (err) {
      return res.status(400).json({ success: false, mode: 'vercel', error: err.message });
    }
  }

  // 4. POST /api/telegram/test atau /api/telegram/send
  if (req.method === 'POST' && (action === 'test' || action === 'send' || !action)) {
    try {
      const targetToken = (payload.bot_token || envToken).trim();
      const targetChat = (payload.chat_id || envChatId).trim();

      if (!targetToken || !targetChat) {
        return res.status(400).json({
          success: false,
          error: 'Bot Token atau Chat ID belum dikonfigurasi (isi di Web UI atau Vercel Environment Variables).'
        });
      }

      const message =
        action === 'test'
          ? `✅ <b>TES KONEKSI OPENAKSARA WEB (VERCEL) ↔ TELEGRAM BERHASIL!</b>\n` +
            `━━━━━━━━━━━━━━━━━━━━━━━\n` +
            `🌐 <b>Platform:</b> Web EduCloud Vercel Bridge\n` +
            `🔥 <b>Integrasi:</b> Firebase Firestore → Telegram Real-time\n` +
            `🕒 <b>Waktu Tes:</b> ${escapeHtml(formatWitaLabel(new Date().toISOString()))}`
          : payload.message || payload.text;

      if (!message) {
        return res.status(400).json({ success: false, error: 'Isi pesan Telegram tidak boleh kosong.' });
      }

      const result = await sendTelegram(targetToken, targetChat, message);
      const logEntry = {
        id: 'TG-' + Date.now().toString().slice(-6),
        timestamp: new Date().toISOString(),
        type: payload.type || (action === 'test' ? 'test_connection' : 'web_relay'),
        title: payload.title || (action === 'test' ? 'Tes Koneksi Telegram (Vercel)' : 'Notifikasi OpenAksara Web'),
        recipient: targetChat,
        status: 'Delivered',
        is_dummy: false
      };
      return res.status(200).json({ success: true, mode: 'vercel', log: logEntry, result });
    } catch (err) {
      return res.status(500).json({ success: false, mode: 'vercel', error: err.message });
    }
  }

  return res.status(405).json({ success: false, error: 'Method or action not allowed' });
};
