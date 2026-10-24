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
    
    # 1. Isometric Assembly View
    ax1 = fig.add_subplot(2, 2, 1, projection='3d')
    ax1.set_title("3D Isometric View (USB di Kiri, Saklar di Kanan)", fontsize=11, fontweight='bold', pad=15)
    
    top_poly = Poly3DCollection(top_mesh.vertices[top_mesh.faces], alpha=0.9, facecolor='#2563eb', edgecolor='#1e40af', linewidths=0.1)
    ax1.add_collection3d(top_poly)
    
    bot_poly = Poly3DCollection(bottom_mesh.vertices[bottom_mesh.faces], alpha=0.9, facecolor='#475569', edgecolor='#334155', linewidths=0.1)
    ax1.add_collection3d(bot_poly)
    
    ax1.set_xlim([-80, 80])
    ax1.set_ylim([-55, 55])
    ax1.set_zlim([0, 35])
    ax1.set_xlabel("Panjang X (150 mm)", fontsize=9)
    ax1.set_ylabel("Lebar Y (100 mm)", fontsize=9)
    ax1.set_zlabel("Tinggi Z (30 mm)", fontsize=9)
    ax1.view_init(elev=26, azim=45) # Angle showing the right side with the switch!
    
    # 2. Tampak Samping Kanan (Right Side View: Saklar 20x15mm)
    ax2 = fig.add_subplot(2, 2, 2)
    ax2.set_title("Tampak Samping Kanan: Lubang Saklar Switch 2-Pin 20x15mm", fontsize=11, fontweight='bold')
    
    rect_side_bot = FancyBboxPatch((-50, 0), 100, 3, boxstyle="round,pad=0,rounding_size=1", facecolor='#64748b', edgecolor='#334155', linewidth=1.5, label="Tutup Bawah (3mm)")
    ax2.add_patch(rect_side_bot)
    
    rect_side_top = FancyBboxPatch((-50, 3), 100, 27, boxstyle="round,pad=0,rounding_size=3", facecolor='#dbeafe', edgecolor='#1d4ed8', linewidth=2, label="Casing Atas (27mm)")
    ax2.add_patch(rect_side_top)
    
    # Switch Hole: Width Y = 19.4 mm (-9.7 to 9.7), Height Z = 13.0 mm (8.5 to 21.5)
    rect_sw_side = FancyBboxPatch((-9.7, 8.5), 19.4, 13.0, boxstyle="round,pad=0,rounding_size=1", facecolor='#0f172a', edgecolor='#ef4444', linewidth=2, label="Lubang Saklar KCD1 (19.4 x 13.0 mm)")
    ax2.add_patch(rect_sw_side)
    
    # Dimension arrows & labels
    ax2.annotate("", xy=(-9.7, 23.5), xytext=(9.7, 23.5), arrowprops=dict(arrowstyle="<->", color="#dc2626", lw=1.5))
    ax2.text(0, 24.8, "Lebar Cutout: 19.4 mm", ha='center', va='bottom', fontsize=9, fontweight='bold', color="#dc2626")
    
    ax2.annotate("", xy=(13.0, 8.5), xytext=(13.0, 21.5), arrowprops=dict(arrowstyle="<->", color="#dc2626", lw=1.5))
    ax2.text(15.0, 15.0, "Tinggi: 13.0 mm", ha='left', va='center', fontsize=9, fontweight='bold', color="#dc2626")
    
    # Centerline from base
    ax2.plot([-35, 35], [15.0, 15.0], '--', color='#ef4444', lw=1.2)
    ax2.text(-37, 15.0, "Pusat Z = 15.0 mm (Pas di Tengah)", ha='right', va='center', fontsize=8, fontweight='bold', color='#dc2626')

    ax2.text(0, 15.0, "ON / OFF\n(KCD1-101)", ha='center', va='center', fontsize=8, fontweight='bold', color='#f87171')

    ax2.set_xlim([-58, 58])
    ax2.set_ylim([-3, 35])
    ax2.set_aspect('equal')
    ax2.grid(True, linestyle=':', alpha=0.5)
    ax2.set_xlabel("Lebar Casing Y (100 mm)", fontsize=9)
    ax2.set_ylabel("Tinggi Casing Z (30 mm)", fontsize=9)
    ax2.legend(loc='lower center', bbox_to_anchor=(0.5, -0.28), ncol=2, fontsize=8)

    # 3. Top-Down Layout Diagram (with LCD Cutout + Mic Array + USB + Switch)
    ax3 = fig.add_subplot(2, 2, 3)
    ax3.set_title("Top View (Layout Lengkap): LCD + Mic + USB + Saklar", fontsize=11, fontweight='bold')
    
    rect_outer = FancyBboxPatch((-75, -50), 150, 100, boxstyle="round,pad=0,rounding_size=5", facecolor='#e0f2fe', edgecolor='#0284c7', linewidth=2, label="Outer Case (150 x 100 mm)")
    ax3.add_patch(rect_outer)
    
    rect_lcd = FancyBboxPatch((-38, -25.5), 76, 51, boxstyle="round,pad=0,rounding_size=2", facecolor='#0f172a', edgecolor='#0284c7', linewidth=1.5, label="LCD Window (76 x 51 mm)")
    ax3.add_patch(rect_lcd)
    
    rect_pcb = Rectangle((-49, -28.17), 98, 56.34, fill=False, edgecolor='#f43f5e', linestyle='--', linewidth=1.5, label="ILI9488 PCB Outline")
    ax3.add_patch(rect_pcb)
    
    mic_cluster = Circle((0, 38), 3.2, facecolor='#fbbf24', edgecolor='#d97706', linewidth=1.5, label="INMP441 Mic (9x Ø1.3mm)")
    ax3.add_patch(mic_cluster)
    ax3.text(0, 42.5, "INMP441 MIC", ha='center', va='bottom', fontsize=8, fontweight='bold', color='#b45309')
    
    # USB Port on left side (horizontal slot in Y)
    rect_usb = FancyBboxPatch((-76.5, -6.25), 3, 12.5, boxstyle="round,pad=0,rounding_size=1", facecolor='#10b981', edgecolor='#047857', linewidth=1.5, label="USB Charger (12.5 x 6.5 mm)")
    ax3.add_patch(rect_usb)
    ax3.text(-74, 9, "USB CHARGER", ha='right', va='bottom', fontsize=7.5, fontweight='bold', color='#047857')
    
    # Switch on right side (cutout 19.4 mm along Y)
    rect_sw = FancyBboxPatch((73.5, -9.7), 3, 19.4, boxstyle="round,pad=0,rounding_size=1", facecolor='#ef4444', edgecolor='#b91c1c', linewidth=1.5, label="Saklar 20x15mm (19.4 x 13 mm)")
    ax3.add_patch(rect_sw)
    ax3.text(74, 12, "SAKLAR ON/OFF", ha='left', va='bottom', fontsize=7.5, fontweight='bold', color='#b91c1c')

    for cx in [-66.5, 66.5]:
        for cy in [-41.5, 41.5]:
            ax3.plot(cx, cy, 'x', color='#0284c7', markersize=6, markeredgewidth=2)
    ax3.plot([], [], 'x', color='#0284c7', label="M3 Screws (x4)")
    
    ax3.set_xlim([-85, 85])
    ax3.set_ylim([-60, 60])
    ax3.set_aspect('equal')
    ax3.grid(True, linestyle=':', alpha=0.5)
    ax3.legend(loc='lower center', bbox_to_anchor=(0.5, -0.32), ncol=3, fontsize=7.5)
    
    # 4. Bottom-Up View (Speaker Grill & M3 Screws Detail)
    ax4 = fig.add_subplot(2, 2, 4)
    ax4.set_title("Bottom View: Speaker Grill (35 Holes) & M3 Countersunk Screws", fontsize=11, fontweight='bold')
    
    rect_bot = FancyBboxPatch((-75, -50), 150, 100, boxstyle="round,pad=0,rounding_size=5", facecolor='#f1f5f9', edgecolor='#475569', linewidth=2, label="Bottom Lid")
    ax4.add_patch(rect_bot)
    
    spk_circle = Circle((35, 0), 16.5, fill=False, edgecolor='#64748b', linestyle=':', linewidth=1)
    ax4.add_patch(spk_circle)
    spk_holes = [(0, 0)]
    rings = [(4.5, 6), (9.0, 12), (13.5, 16)]
    for r, n in rings:
        angles = np.linspace(0, 2*np.pi, n, endpoint=False)
        for a in angles:
            spk_holes.append((r * np.cos(a), r * np.sin(a)))
    for hx, hy in spk_holes:
        dot = Circle((35 + hx, hy), 1.0, facecolor='#1e293b', edgecolor='none')
        ax4.add_patch(dot)
    ax4.plot([], [], 'o', color='#1e293b', markersize=4, label="Speaker Grill (35x Ø2.2mm)")
    ax4.text(35, -21, "SPEAKER GRILL", ha='center', va='top', fontsize=8, fontweight='bold', color='#334155')
    
    for cx in [-66.5, 66.5]:
        for cy in [-41.5, 41.5]:
            cs_outer = Circle((cx, cy), 3.1, fill=False, edgecolor='#dc2626', linewidth=1.5)
            cs_inner = Circle((cx, cy), 1.7, facecolor='#dc2626', edgecolor='none')
            ax4.add_patch(cs_outer)
            ax4.add_patch(cs_inner)
    ax4.plot([], [], 'o', color='#dc2626', markersize=6, label="M3 Countersunk Hole")
    
    ax4.set_xlim([-85, 85])
    ax4.set_ylim([-60, 60])
    ax4.set_aspect('equal')
    ax4.grid(True, linestyle=':', alpha=0.5)
    ax4.legend(loc='lower center', bbox_to_anchor=(0.5, -0.28), ncol=3, fontsize=7.5)
    
    plt.tight_layout()
    
    out_workspace = "casing_preview.png"
    out_artifact = os.path.join(artifact_dir, "casing_preview.png")
    plt.savefig(out_workspace, dpi=150, bbox_inches='tight')
    plt.savefig(out_artifact, dpi=150, bbox_inches='tight')
    print(f"Preview updated: {out_workspace} and {out_artifact}")

if __name__ == '__main__':
    render_views()
