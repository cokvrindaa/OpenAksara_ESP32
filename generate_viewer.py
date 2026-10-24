import trimesh
import json
import os

artifact_dir = r"C:\Users\cokvr\.gemini\antigravity\brain\f157119b-0846-4ce9-850e-bbbf528be1df"

top = trimesh.load("casing_atas.stl")
bot = trimesh.load("tutup_bawah.stl")

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
  <title>Interactive 3D Preview - OpenAksara Enclosure</title>
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
        Enclosure 3D Model: OpenAksara ESP32 (ILI9488 3.5")
      </h1>
      <p class="text-xs text-slate-400">150 mm (P) × 100 mm (L) × 30 mm (T) • Bawah Lepas-Pasang (M3 Screws)</p>
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
        <span>Dimensi & Fitur Utama</span>
        <span class="text-emerald-400 font-mono text-[11px] bg-emerald-950/60 px-2 py-0.5 rounded border border-emerald-800">100% Watertight</span>
      </div>
      <ul class="text-slate-300 space-y-1">
        <li>• <strong class="text-white">Panjang × Lebar:</strong> 150 mm × 100 mm (R = 5mm)</li>
        <li>• <strong class="text-white">Tinggi Total:</strong> 30 mm (Top 27mm + Base 3mm)</li>
        <li>• <strong class="text-white">Cutout LCD 3.5":</strong> 76 mm × 51 mm + Recess 86.5×56.5mm</li>
        <li>• <strong class="text-white">Mounting LCD:</strong> 4 Standoff M2.5/M3 (92 × 49.5 mm)</li>
        <li>• <strong class="text-white">Bongkar-Pasang:</strong> 4 Lubang Baut M3 Countersunk</li>
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
      <input type="range" id="sliderExplode" min="0" max="60" value="0" class="w-32 accent-blue-500 cursor-pointer">
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

    // Camera state
    let rotX = -0.55;
    let rotZ = 0.78;
    let zoom = 3.6;
    let panX = 0, panY = 0;
    let explodeZ = 0;
    let isWireframe = false;

    // Mouse interaction
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
      zoom = Math.max(1.0, Math.min(12.0, zoom));
      render();
    }}, {{ passive: false }});

    // Touch support
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

    // Controls
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
      explodeZ = isExploded ? 35 : 0;
      slider.value = explodeZ;
      txtExplode.textContent = Math.round(explodeZ) + ' mm';
      render();
    }});

    document.getElementById('btnReset').addEventListener('click', () => {{
      rotX = -0.55;
      rotZ = 0.78;
      zoom = 3.6;
      explodeZ = 0;
      slider.value = 0;
      txtExplode.textContent = '0 mm';
      render();
    }});

    document.getElementById('chkWireframe').addEventListener('change', (e) => {{
      isWireframe = e.target.checked;
      render();
    }});

    // 3D Math & Render
    function project(p, offsetZ) {{
      const x = p[0];
      const y = p[1];
      const z = p[2] + offsetZ - 15; // center around z=15

      // Rotate Z
      const cz = Math.cos(rotZ), sz = Math.sin(rotZ);
      const x1 = x * cz - y * sz;
      const y1 = x * sz + y * cz;
      const z1 = z;

      // Rotate X
      const cx = Math.cos(rotX), sx = Math.sin(rotX);
      const x2 = x1;
      const y2 = y1 * cx - z1 * sx;
      const z2 = y1 * sx + z1 * cx;

      // Isometric / Orthographic projection with zoom
      const sx_screen = width / 2 + x2 * zoom + panX;
      const sy_screen = height / 2 - z2 * zoom + panY;

      return {{ x: sx_screen, y: sy_screen, depth: y2 }};
    }}

    function render() {{
      ctx.clearRect(0, 0, width, height);

      // Light direction
      const lx = 0.577, ly = 0.577, lz = 0.577;

      const polygons = [];

      // Process Top Case
      for (let i = 0; i < topData.faces.length; i++) {{
        const f = topData.faces[i];
        const v0 = topData.verts[f[0]];
        const v1 = topData.verts[f[1]];
        const v2 = topData.verts[f[2]];

        const p0 = project(v0, 0);
        const p1 = project(v1, 0);
        const p2 = project(v2, 0);

        // Backface culling in screen space
        const cross = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
        if (cross <= 0) continue;

        // Normal in world space
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

      // Process Bottom Lid (with explode offset)
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

      // Sort by depth (painter's algorithm)
      polygons.sort((a, b) => a.depth - b.depth);

      // Draw
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

    // Initial render
    setTimeout(resize, 50);
  </script>
</body>
</html>
"""

output_path = os.path.join(artifact_dir, "3d_viewer.html")
with open(output_path, "w", encoding="utf-8") as f:
    f.write(html_content)

print(f"Interactive 3D Viewer generated at {output_path}")
