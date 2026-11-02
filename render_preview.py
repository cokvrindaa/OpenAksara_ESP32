import trimesh
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import numpy as np
import os
from matplotlib.patches import FancyBboxPatch, Rectangle, Circle

artifact_dir = r"C:\Users\cokvr\.gemini\antigravity\brain\f157119b-0846-4ce9-850e-bbbf528be1df"

def render_views():
    top_mesh = trimesh.load("casing_atas.stl")
    bottom_mesh = trimesh.load("tutup_bawah.stl")
    
    fig = plt.figure(figsize=(16, 12), dpi=150)
    
    # 1. 3D Isometric View (Assembly 180 x 100 x 50 mm)
    ax1 = fig.add_subplot(2, 2, 1, projection='3d')
    ax1.set_title("3D Isometric View (180 mm x 100 mm x 50 mm)", fontsize=11, fontweight='bold', pad=15)
    
    top_poly = Poly3DCollection(top_mesh.vertices[top_mesh.faces], alpha=0.9, facecolor='#2563eb', edgecolor='#1e40af', linewidths=0.1)
    ax1.add_collection3d(top_poly)
    
    bot_poly = Poly3DCollection(bottom_mesh.vertices[bottom_mesh.faces], alpha=0.9, facecolor='#475569', edgecolor='#334155', linewidths=0.1)
    ax1.add_collection3d(bot_poly)
    
    ax1.set_xlim([-95, 95])
    ax1.set_ylim([-55, 55])
    ax1.set_zlim([0, 55])
    ax1.set_xlabel("Panjang X (180 mm)", fontsize=9)
    ax1.set_ylabel("Lebar Y (100 mm)", fontsize=9)
    ax1.set_zlabel("Tinggi Z (50 mm)", fontsize=9)
    ax1.view_init(elev=28, azim=-135)
    
    # 2. Tampak Samping (Side Elevation View: Height 50 mm with centered USB)
    ax2 = fig.add_subplot(2, 2, 2)
    ax2.set_title("Tampak Samping Kiri: Tinggi 50 mm & Port USB Pas di Tengah (Z = 25.0 mm)", fontsize=11, fontweight='bold')
    
    # Bottom flat plate: Width Y = 100 mm, Height Z = 3 mm
    rect_side_bot = FancyBboxPatch((-50, 0), 100, 3, boxstyle="round,pad=0,rounding_size=1", facecolor='#64748b', edgecolor='#334155', linewidth=1.5, label="Tutup Bawah Rata (Tebal 3mm)")
    ax2.add_patch(rect_side_bot)
    
    # Top case side wall: Y = 100 mm, Z = 47 mm (from Z = 3 to 50)
    rect_side_top = FancyBboxPatch((-50, 3), 100, 47, boxstyle="round,pad=0,rounding_size=3", facecolor='#dbeafe', edgecolor='#1d4ed8', linewidth=2, label="Casing Atas (Tinggi 47mm)")
    ax2.add_patch(rect_side_top)
    
    # Horizontal USB port centered at Z = 25.0 mm
    rect_usb_side = FancyBboxPatch((-6.25, 21.75), 12.5, 6.5, boxstyle="round,pad=0,rounding_size=2", facecolor='#0f172a', edgecolor='#10b981', linewidth=2, label="USB Type-C (12.5 x 6.5 mm)")
    ax2.add_patch(rect_usb_side)
    
    # Centerline & dimensions
    ax2.plot([-40, 40], [25.0, 25.0], '--', color='#10b981', lw=1.2)
    ax2.text(-42, 25.0, "Pusat Z = 25.0 mm (Tengah)", ha='right', va='center', fontsize=8, fontweight='bold', color='#047857')

    ax2.annotate("", xy=(-25, 0), xytext=(-25, 21.75), arrowprops=dict(arrowstyle="<->", color="#64748b", lw=1))
    ax2.text(-27, 10.8, "21.75 mm ke dasar", ha='right', va='center', fontsize=7.5, color='#475569')

    ax2.annotate("", xy=(-25, 28.25), xytext=(-25, 50), arrowprops=dict(arrowstyle="<->", color="#64748b", lw=1))
    ax2.text(-27, 39.1, "21.75 mm ke atas", ha='right', va='center', fontsize=7.5, color='#475569')

    ax2.set_xlim([-58, 58])
    ax2.set_ylim([-3, 56])
    ax2.set_aspect('equal')
    ax2.grid(True, linestyle=':', alpha=0.5)
    ax2.set_xlabel("Lebar Casing Y (100 mm)", fontsize=9)
    ax2.set_ylabel("Tinggi Casing Z (50 mm)", fontsize=9)
    ax2.legend(loc='lower center', bbox_to_anchor=(0.5, -0.28), ncol=2, fontsize=8)

    # 3. Top-Down Layout Diagram (180 mm x 100 mm)
    ax3 = fig.add_subplot(2, 2, 3)
    ax3.set_title("Top View: Layout 180 mm x 100 mm (LCD + Mic + USB + Saklar)", fontsize=11, fontweight='bold')
    
    rect_outer = FancyBboxPatch((-90, -50), 180, 100, boxstyle="round,pad=0,rounding_size=5", facecolor='#e0f2fe', edgecolor='#0284c7', linewidth=2, label="Outer Case (180 x 100 mm)")
    ax3.add_patch(rect_outer)
    
    rect_lcd = FancyBboxPatch((-39, -25.75), 78, 51.5, boxstyle="round,pad=0,rounding_size=2", facecolor='#0f172a', edgecolor='#0284c7', linewidth=1.5, label="LCD Window (78 x 51.5 mm)")
    ax3.add_patch(rect_lcd)
    
    rect_pcb = Rectangle((-49, -28.17), 98, 56.34, fill=False, edgecolor='#f43f5e', linestyle='--', linewidth=1.5, label="ILI9488 PCB (98 x 56.3 mm)")
    ax3.add_patch(rect_pcb)
    
    mic_cluster = Circle((0, 38), 3.2, facecolor='#fbbf24', edgecolor='#d97706', linewidth=1.5, label="INMP441 Mic (9x Ø1.3mm)")
    ax3.add_patch(mic_cluster)
    ax3.text(0, 42.5, "INMP441 MIC", ha='center', va='bottom', fontsize=8, fontweight='bold', color='#b45309')
    
    # USB Port on left side (X = -90)
    rect_usb = FancyBboxPatch((-91.5, -6.25), 3, 12.5, boxstyle="round,pad=0,rounding_size=1", facecolor='#10b981', edgecolor='#047857', linewidth=1.5, label="USB Charger (12.5 x 6.5 mm)")
    ax3.add_patch(rect_usb)
    ax3.text(-89, 9, "USB", ha='right', va='bottom', fontsize=7.5, fontweight='bold', color='#047857')
    
    # Slide Switch on right side (X = +90)
    rect_sw = FancyBboxPatch((88.5, -4.25), 3, 8.5, boxstyle="round,pad=0,rounding_size=1", facecolor='#ef4444', edgecolor='#b91c1c', linewidth=1.5, label="Saklar Geser (8.5 x 4.5 mm)")
    ax3.add_patch(rect_sw)
    ax3.text(89, 8, "SAKLAR", ha='left', va='bottom', fontsize=7.5, fontweight='bold', color='#b91c1c')

    # Corner screws (X = +-81.5, Y = +-41.5)
    for cx in [-81.5, 81.5]:
        for cy in [-41.5, 41.5]:
            ax3.plot(cx, cy, 'x', color='#0284c7', markersize=6, markeredgewidth=2)
    ax3.plot([], [], 'x', color='#0284c7', label="M3 Screws (x4)")
    
    ax3.set_xlim([-100, 100])
    ax3.set_ylim([-60, 60])
    ax3.set_aspect('equal')
    ax3.grid(True, linestyle=':', alpha=0.5)
    ax3.legend(loc='lower center', bbox_to_anchor=(0.5, -0.32), ncol=3, fontsize=7.5)
    
    # 4. Bottom View: Completely Flat Lid (No Perimeter Rim, No Speaker Ring!)
    ax4 = fig.add_subplot(2, 2, 4)
    ax4.set_title("Bottom View: TUTUP BAWAH 100% RATA (Semua Pinggiran Dihapus)", fontsize=11, fontweight='bold')
    
    rect_bot = FancyBboxPatch((-90, -50), 180, 100, boxstyle="round,pad=0,rounding_size=5", facecolor='#f1f5f9', edgecolor='#475569', linewidth=2, label="Pelat Rata Mulus (180 x 100 mm)")
    ax4.add_patch(rect_bot)
    
    # Speaker grill at X = 40, Y = 0 (clean holes without any ring)
    spk_holes = [(0, 0)]
    rings = [(4.5, 6), (9.0, 12), (13.5, 16)]
    for r, n in rings:
        angles = np.linspace(0, 2*np.pi, n, endpoint=False)
        for a in angles:
            spk_holes.append((r * np.cos(a), r * np.sin(a)))
    for hx, hy in spk_holes:
        dot = Circle((40 + hx, hy), 1.1, facecolor='#1e293b', edgecolor='none')
        ax4.add_patch(dot)
    ax4.plot([], [], 'o', color='#1e293b', markersize=4, label="Speaker Grill (35x Ø2.2mm Rata)")
    ax4.text(40, -20, "SPEAKER GRILL (FLAT)", ha='center', va='top', fontsize=8, fontweight='bold', color='#334155')
    
    # Countersunk screw holes
    for cx in [-81.5, 81.5]:
        for cy in [-41.5, 41.5]:
            cs_outer = Circle((cx, cy), 3.1, fill=False, edgecolor='#dc2626', linewidth=1.5)
            cs_inner = Circle((cx, cy), 1.7, facecolor='#dc2626', edgecolor='none')
            ax4.add_patch(cs_outer)
            ax4.add_patch(cs_inner)
    ax4.plot([], [], 'o', color='#dc2626', markersize=6, label="M3 Countersunk Hole (x4)")
    
    ax4.text(0, 25, "✓ 100% RATA & MULUS (FLAT)\n(Bibir keliling & cincin speaker sudah dihapus)", ha='center', va='center', fontsize=9, fontweight='bold', color='#16a34a', bbox=dict(boxstyle="round,pad=0.4", fc="#dcfce7", ec="#86efac", lw=1))

    ax4.set_xlim([-100, 100])
    ax4.set_ylim([-60, 60])
    ax4.set_aspect('equal')
    ax4.grid(True, linestyle=':', alpha=0.5)
    ax4.legend(loc='lower center', bbox_to_anchor=(0.5, -0.32), ncol=3, fontsize=7.5)
    
    plt.tight_layout()
    
    out_workspace = "casing_preview.png"
    out_artifact = os.path.join(artifact_dir, "casing_preview.png")
    plt.savefig(out_workspace, dpi=150, bbox_inches='tight')
    plt.savefig(out_artifact, dpi=150, bbox_inches='tight')
    print(f"Preview updated: {out_workspace} and {out_artifact}")

if __name__ == '__main__':
    render_views()
