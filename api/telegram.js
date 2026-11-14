const https = require('https');

function sendTelegram(botToken, chatId, text) {
  return new Promise((resolve, reject) => {
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
          if (json.ok) resolve(json);
          else reject(new Error(json.description || 'Telegram API Error'));
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

module.exports = async (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'OPTIONS') {
    return res.status(204).end();
  }

  const botToken = (process.env.TELEGRAM_BOT_TOKEN || '').trim();
  const chatId = (process.env.TELEGRAM_CHAT_ID || '').trim();

  // GET: Cek config
  if (req.method === 'GET') {
    return res.status(200).json({
      success: true,
      config: {
        is_dummy_mode: !(botToken && chatId),
        has_token: !!botToken,
        chat_id: chatId,
        bot_name: process.env.TELEGRAM_BOT_NAME || '@OpenAksaraBot'
      }
    });
  }

  // POST: Kirim atau tes
  if (req.method === 'POST') {
    try {
      const payload = typeof req.body === 'string' ? JSON.parse(req.body || '{}') : (req.body || {});
      const targetToken = (payload.bot_token || botToken).trim();
      const targetChat = (payload.chat_id || chatId).trim();
      const message = payload.message || payload.text || '✅ <b>Tes Notifikasi OpenAksara Vercel</b>';

      if (!targetToken || !targetChat) {
        return res.status(400).json({ success: false, error: 'Token atau Chat ID belum dikonfigurasi.' });
      }

      const result = await sendTelegram(targetToken, targetChat, message);
      return res.status(200).json({ success: true, result });
    } catch (err) {
      return res.status(500).json({ success: false, error: err.message });
    }
  }

  return res.status(405).json({ error: 'Method not allowed' });
};
