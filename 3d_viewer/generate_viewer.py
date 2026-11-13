import trimesh
import json
import os

script_dir = os.path.dirname(os.path.abspath(__file__))
artifact_dir = r"C:\Users\cokvr\.gemini\antigravity\brain\7b259053-4c67-48f7-bd35-57a459999eab"

top_path = os.path.join(script_dir, "casing_atas.stl")
bot_path = os.path.join(script_dir, "tutup_bawah.stl")
if not os.path.exists(top_path):
    top_path = os.path.join(script_dir, "models", "casing_atas.stl")
if not os.path.exists(bot_path):
    bot_path = os.path.join(script_dir, "models", "tutup_bawah.stl")

top = trimesh.load(top_path)
bot = trimesh.load(bot_path)

top_data = {
    "verts": top.vertices.tolist(),
    "faces": top.faces.tolist(),
}

bot_data = {
    "verts": bot.vertices.tolist(),
    "faces": bot.faces.tolist(),
}

html_content = f"""<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <title>Interactive 3D Preview - OpenAksara Enclosure 18x10x5cm</title>
  <script src="https://www.gstatic.com/antigravity/web/dev/tailwindcss.min.js"></script>
  <style>
    body {{
      margin: 0;
      overflow: hidden;
      user-select: none;
      -webkit-user-select: none;
    }}
    canvas {{
      display: block;
      width: 100%;
      height: 100%;
      cursor: grab;
    }}
    canvas:active {{
      cursor: grabbing;
    }}
  </style>
</head>
<body class="bg-slate-950 text-slate-100 flex flex-col h-screen font-sans">
  <!-- Top Bar -->
  <header class="bg-slate-900 border-b border-slate-800 px-5 py-3 flex items-center justify-between z-10">
    <div>
      <h1 class="text-base font-bold text-white flex items-center gap-2">
        <span class="w-3 h-3 rounded-full bg-blue-500"></span>
        Enclosure 3D Model: OpenAksara ESP32 (180 × 100 × 50 mm)
      </h1>
      <p class="text-xs text-slate-400">Panjang 18 cm × Lebar 10 cm × Kedalaman 5.0 cm • Tutup Bawah Flat Rata Mulus</p>
    </div>
    <div class="flex items-center gap-2">
      <button id="btnReset" class="px-3 py-1.5 text-xs bg-slate-800 hover:bg-slate-700 text-slate-200 rounded-lg border border-slate-700 transition">Reset View</button>
      <button id="btnExplode" class="px-3 py-1.5 text-xs bg-blue-600 hover:bg-blue-500 text-white font-medium rounded-lg shadow-sm transition">Bongkar / Pasang</button>
    </div>
  </header>

  <!-- Main 3D Canvas Area -->
  <main class="relative flex-1 bg-gradient-to-b from-slate-950 via-slate-900 to-slate-950 overflow-hidden">
    <canvas id="view3d"></canvas>

    <!-- Overlay Info & Controls -->
    <div class="absolute bottom-5 left-5 bg-slate-900/90 backdrop-blur border border-slate-800 p-4 rounded-xl shadow-xl max-w-sm text-xs space-y-2 pointer-events-auto">
      <div class="font-semibold text-slate-200 text-sm flex items-center justify-between">
        <span>Dimensi & Fitur Terbaru</span>
        <span class="text-emerald-400 font-mono text-[11px] bg-emerald-950/60 px-2 py-0.5 rounded border border-emerald-800">100% Watertight</span>
      </div>
      <ul class="text-slate-300 space-y-1">
        <li>• <strong class="text-white">Panjang × Lebar:</strong> 180 mm × 100 mm (18 × 10 cm)</li>
        <li>• <strong class="text-white">Kedalaman / Tinggi:</strong> 50 mm (5.0 cm) - Ideal & Proporsional</li>
        <li>• <strong class="text-white">Tutup Bawah:</strong> Pelat Rata Mulus (Tanpa Bibir Keliling / Ring Speaker)</li>
        <li>• <strong class="text-white">LCD 3.5":</strong> Window 78×51.5mm + Relief Notch Touch FPC</li>
        <li>• <strong class="text-white">USB Charger:</strong> Horizontal di tengah (Z = 25.0 mm)</li>
        <li>• <strong class="text-white">Saklar Geser:</strong> Mini Slide Switch (8.5 × 4.5 mm)</li>
      </ul>
      <div class="pt-2 border-t border-slate-800 text-slate-400 text-[11px] flex justify-between items-center">
        <span>🖱️ Drag: Putar | Scroll: Zoom</span>
        <label class="flex items-center gap-1.5 cursor-pointer">
          <input type="checkbox" id="chkWireframe" class="rounded bg-slate-800 border-slate-700 text-blue-500">
          <span>Wireframe</span>
        </label>
      </div>
    </div>

    <!-- Explode Slider Floating -->
    <div class="absolute top-5 right-5 bg-slate-900/90 backdrop-blur border border-slate-800 px-4 py-3 rounded-xl shadow-lg flex items-center gap-3 text-xs">
      <span class="text-slate-300 font-medium">Jarak Lepas Tutup:</span>
      <input type="range" id="sliderExplode" min="0" max="70" value="0" class="w-32 accent-blue-500 cursor-pointer">
      <span id="txtExplodeVal" class="text-slate-400 font-mono w-10 text-right">0 mm</span>
    </div>
  </main>

  <script>
    const topData = {json.dumps(top_data)};
    const botData = {json.dumps(bot_data)};

    const canvas = document.getElementById('view3d');
    const ctx = canvas.getContext('2d');

    let width, height;
    function resize() {{
      width = canvas.clientWidth;
      height = canvas.clientHeight;
      canvas.width = width * window.devicePixelRatio;
      canvas.height = height * window.devicePixelRatio;
      ctx.scale(window.devicePixelRatio, window.devicePixelRatio);
      render();
    }}
    window.addEventListener('resize', resize);

    let rotX = -0.55;
    let rotZ = 0.78;
    let zoom = 3.2;
    let panX = 0, panY = 0;
    let explodeZ = 0;
    let isWireframe = false;

    let isDragging = false;
    let lastX = 0, lastY = 0;

    canvas.addEventListener('mousedown', (e) => {{
      isDragging = true;
      lastX = e.clientX;
      lastY = e.clientY;
    }});

    window.addEventListener('mouseup', () => isDragging = false);

    window.addEventListener('mousemove', (e) => {{
      if (!isDragging) return;
      const dx = e.clientX - lastX;
      const dy = e.clientY - lastY;
      lastX = e.clientX;
      lastY = e.clientY;

      rotZ += dx * 0.008;
      rotX += dy * 0.008;
      rotX = Math.max(-Math.PI / 2 + 0.01, Math.min(Math.PI / 2 - 0.01, rotX));
      render();
    }});

    canvas.addEventListener('wheel', (e) => {{
      e.preventDefault();
      zoom *= (e.deltaY < 0 ? 1.08 : 0.92);
      zoom = Math.max(1.0, Math.min(10.0, zoom));
      render();
    }}, {{ passive: false }});

    let lastTouchDist = 0;
    canvas.addEventListener('touchstart', (e) => {{
      if (e.touches.length === 1) {{
        isDragging = true;
        lastX = e.touches[0].clientX;
        lastY = e.touches[0].clientY;
      }} else if (e.touches.length === 2) {{
        lastTouchDist = Math.hypot(e.touches[0].clientX - e.touches[1].clientX, e.touches[0].clientY - e.touches[1].clientY);
      }}
    }});

    canvas.addEventListener('touchmove', (e) => {{
      if (e.touches.length === 1 && isDragging) {{
        const dx = e.touches[0].clientX - lastX;
        const dy = e.touches[0].clientY - lastY;
        lastX = e.touches[0].clientX;
        lastY = e.touches[0].clientY;
        rotZ += dx * 0.008;
        rotX += dy * 0.008;
        render();
      }} else if (e.touches.length === 2) {{
        const dist = Math.hypot(e.touches[0].clientX - e.touches[1].clientX, e.touches[0].clientY - e.touches[1].clientY);
        if (lastTouchDist > 0) {{
          zoom *= dist / lastTouchDist;
          render();
        }}
        lastTouchDist = dist;
      }}
    }});

    canvas.addEventListener('touchend', () => isDragging = false);

    const slider = document.getElementById('sliderExplode');
    const txtExplode = document.getElementById('txtExplodeVal');
    slider.addEventListener('input', (e) => {{
      explodeZ = parseFloat(e.target.value);
      txtExplode.textContent = Math.round(explodeZ) + ' mm';
      render();
    }});

    let isExploded = false;
    document.getElementById('btnExplode').addEventListener('click', () => {{
      isExploded = !isExploded;
      explodeZ = isExploded ? 40 : 0;
      slider.value = explodeZ;
      txtExplode.textContent = Math.round(explodeZ) + ' mm';
      render();
    }});

    document.getElementById('btnReset').addEventListener('click', () => {{
      rotX = -0.55;
      rotZ = 0.78;
      zoom = 3.2;
      explodeZ = 0;
      slider.value = 0;
      txtExplode.textContent = '0 mm';
      render();
    }});

    document.getElementById('chkWireframe').addEventListener('change', (e) => {{
      isWireframe = e.target.checked;
      render();
    }});

    function project(p, offsetZ) {{
      const x = p[0];
      const y = p[1];
      const z = p[2] + offsetZ - 25.0;

      const cz = Math.cos(rotZ), sz = Math.sin(rotZ);
      const x1 = x * cz - y * sz;
      const y1 = x * sz + y * cz;
      const z1 = z;

      const cx = Math.cos(rotX), sx = Math.sin(rotX);
      const x2 = x1;
      const y2 = y1 * cx - z1 * sx;
      const z2 = y1 * sx + z1 * cx;

      const sx_screen = width / 2 + x2 * zoom + panX;
      const sy_screen = height / 2 - z2 * zoom + panY;

      return {{ x: sx_screen, y: sy_screen, depth: y2 }};
    }}

    function render() {{
      ctx.clearRect(0, 0, width, height);

      const lx = 0.577, ly = 0.577, lz = 0.577;
      const polygons = [];

      for (let i = 0; i < topData.faces.length; i++) {{
        const f = topData.faces[i];
        const v0 = topData.verts[f[0]];
        const v1 = topData.verts[f[1]];
        const v2 = topData.verts[f[2]];

        const p0 = project(v0, 0);
        const p1 = project(v1, 0);
        const p2 = project(v2, 0);

        const cross = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
        if (cross <= 0) continue;

        const ax = v1[0] - v0[0], ay = v1[1] - v0[1], az = v1[2] - v0[2];
        const bx = v2[0] - v0[0], by = v2[1] - v0[1], bz = v2[2] - v0[2];
        let nx = ay * bz - az * by;
        let ny = az * bx - ax * bz;
        let nz = ax * by - ay * bx;
        const len = Math.hypot(nx, ny, nz) || 1;
        nx /= len; ny /= len; nz /= len;

        const diff = Math.max(0.18, (nx * lx + ny * ly + nz * lz));
        const depth = (p0.depth + p1.depth + p2.depth) / 3;

        polygons.push({{
          points: [p0, p1, p2],
          depth: depth,
          color: `hsl(217, 85%, ${{Math.round(28 + diff * 38)}}%)`,
          edgeColor: 'rgba(30, 64, 175, 0.25)',
          isTop: true
        }});
      }}

      for (let i = 0; i < botData.faces.length; i++) {{
        const f = botData.faces[i];
        const v0 = botData.verts[f[0]];
        const v1 = botData.verts[f[1]];
        const v2 = botData.verts[f[2]];

        const p0 = project(v0, -explodeZ);
        const p1 = project(v1, -explodeZ);
        const p2 = project(v2, -explodeZ);

        const cross = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
        if (cross <= 0) continue;

        const ax = v1[0] - v0[0], ay = v1[1] - v0[1], az = v1[2] - v0[2];
        const bx = v2[0] - v0[0], by = v2[1] - v0[1], bz = v2[2] - v0[2];
        let nx = ay * bz - az * by;
        let ny = az * bx - ax * bz;
        let nz = ax * by - ay * bx;
        const len = Math.hypot(nx, ny, nz) || 1;
        nx /= len; ny /= len; nz /= len;

        const diff = Math.max(0.18, (nx * lx + ny * ly + nz * lz));
        const depth = (p0.depth + p1.depth + p2.depth) / 3;

        polygons.push({{
          points: [p0, p1, p2],
          depth: depth,
          color: `hsl(215, 20%, ${{Math.round(25 + diff * 45)}}%)`,
          edgeColor: 'rgba(71, 85, 105, 0.3)',
          isTop: false
        }});
      }}

      polygons.sort((a, b) => a.depth - b.depth);

      for (let i = 0; i < polygons.length; i++) {{
        const poly = polygons[i];
        ctx.beginPath();
        ctx.moveTo(poly.points[0].x, poly.points[0].y);
        ctx.lineTo(poly.points[1].x, poly.points[1].y);
        ctx.lineTo(poly.points[2].x, poly.points[2].y);
        ctx.closePath();

        if (isWireframe) {{
          ctx.strokeStyle = poly.isTop ? '#60a5fa' : '#94a3b8';
          ctx.lineWidth = 0.8;
          ctx.stroke();
        }} else {{
          ctx.fillStyle = poly.color;
          ctx.fill();
          ctx.strokeStyle = poly.edgeColor;
          ctx.lineWidth = 0.3;
          ctx.stroke();
        }}
      }}
    }}

    setTimeout(resize, 50);
  </script>
</body>
</html>
"""

output_paths = [
    os.path.join(script_dir, "3d_viewer.html"),
    os.path.join(script_dir, "index.html")
]
if os.path.isdir(artifact_dir):
    output_paths.append(os.path.join(artifact_dir, "3d_viewer.html"))

for p in output_paths:
    with open(p, "w", encoding="utf-8") as f:
        f.write(html_content)

print(f"Interactive 3D Viewer updated at {output_paths[0]}")
