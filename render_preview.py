import trimesh
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import numpy as np
import os
from matplotlib.patches import FancyBboxPatch, Rectangle, Circle
from PIL import Image

artifact_dir = r"C:\Users\cokvr\.gemini\antigravity\brain\f157119b-0846-4ce9-850e-bbbf528be1df"

def render_views():
    top_mesh = trimesh.load("casing_atas.stl")
    bottom_mesh = trimesh.load("tutup_bawah.stl")
    
    fig = plt.figure(figsize=(16, 12), dpi=150)
    
    # 1. 3D Inside View of Top Case (Highlighting FPC Notch Relief Pocket & Standoffs)
    ax1 = fig.add_subplot(2, 2, 1, projection='3d')
    ax1.set_title("Tampak Dalam Casing Atas: Ceruk Bezel Kaca + Relief Notch FPC", fontsize=11, fontweight='bold', pad=15)
    
    top_poly = Poly3DCollection(top_mesh.vertices[top_mesh.faces], alpha=0.75, facecolor='#38bdf8', edgecolor='#0284c7', linewidths=0.1)
    ax1.add_collection3d(top_poly)
    
    ax1.set_xlim([-80, 80])
    ax1.set_ylim([-55, 55])
    ax1.set_zlim([0, 35])
    ax1.set_xlabel("Panjang X (150 mm)", fontsize=9)
    ax1.set_ylabel("Lebar Y (100 mm)", fontsize=9)
    ax1.set_zlabel("Tinggi Z (30 mm)", fontsize=9)
    # Looking from underneath inside the top case
    ax1.view_init(elev=-60, azim=-65)
    
    # 2. Detailed Technical Diagram of LCD Mounting Area (Active Area + Glass + FPC Notch)
    ax2 = fig.add_subplot(2, 2, 2)
    ax2.set_title("Detail Rekayasa Dudukan LCD: Kesesuaian Kaca & Notch FPC Touch", fontsize=11, fontweight='bold')
    
    # Outer Case section background
    rect_bg = Rectangle((-60, -35), 120, 70, facecolor='#f8fafc', edgecolor='#cbd5e1', linewidth=1)
    ax2.add_patch(rect_bg)
    
    # LCD Module PCB outline (98 x 56.34)
    rect_pcb = Rectangle((-49, -28.17), 98, 56.34, fill=False, edgecolor='#ef4444', linestyle='--', linewidth=1.5, label="Modul PCB (98 x 56.34 mm)")
    ax2.add_patch(rect_pcb)
    
    # 4 Mounting Standoffs (92 x 49.5 mm)
    for lx in [-46.0, 46.0]:
        for ly in [-24.75, 24.75]:
            c_post = Circle((lx, ly), 3.2, facecolor='#bae6fd', edgecolor='#0284c7', linewidth=1.2)
            c_hole = Circle((lx, ly), 1.25, facecolor='#0284c7', edgecolor='none')
            ax2.add_patch(c_post)
            ax2.add_patch(c_hole)
    ax2.plot([], [], 'o', color='#0284c7', markersize=6, label="4 Standoff Dudukan PCB (92 x 49.5 mm)")
    
    # Glass Outline (84.96 x 55.5 mm)
    rect_glass = Rectangle((-42.48, -27.75), 84.96, 55.5, fill=False, edgecolor='#64748b', linestyle='-', linewidth=1.5, label="Kaca LCD (85.0 x 55.5 mm)")
    ax2.add_patch(rect_glass)
    
    # NEW: FPC Ribbon Notch Relief Pocket in the casing (94.0 x 20.0 mm)
    rect_fpc_relief = FancyBboxPatch((-47.0, -10.0), 94.0, 20.0, boxstyle="round,pad=0,rounding_size=2", facecolor='#fef08a', edgecolor='#ca8a04', alpha=0.5, linewidth=1.5, label="Ceruk Pelega Notch FPC (94 x 20 mm)")
    ax2.add_patch(rect_fpc_relief)
    
    # FPC Ribbon Tab on right side (as in user's photo!)
    rect_fpc_tab = FancyBboxPatch((41.0, -6.0), 5.5, 12.0, boxstyle="round,pad=0,rounding_size=1", facecolor='#eab308', edgecolor='#a16207', linewidth=1.2, label="Kabel Pita / Tab FPC Touch (Foto User)")
    ax2.add_patch(rect_fpc_tab)
    ax2.text(43.5, 0, "FPC\nTAB", ha='center', va='center', fontsize=6.5, fontweight='bold', color='#713f12')

    # Front Cutout Window (78.0 x 51.5 mm)
    rect_win = FancyBboxPatch((-39.0, -25.75), 78.0, 51.5, boxstyle="round,pad=0,rounding_size=2", facecolor='#0f172a', edgecolor='#38bdf8', linewidth=2, label="Jendela Tampak Layar (78 x 51.5 mm)")
    ax2.add_patch(rect_win)
    
    # Active Area (73.44 x 48.96 mm, offset by 2.33 mm towards left)
    rect_aa = Rectangle((-39.05, -24.48), 73.44, 48.96, fill=False, edgecolor='#22c55e', linestyle=':', linewidth=1.5, label="Display Active Area (73.4 x 49 mm)")
    ax2.add_patch(rect_aa)
    
    ax2.set_xlim([-55, 55])
    ax2.set_ylim([-35, 35])
    ax2.set_aspect('equal')
    ax2.grid(True, linestyle=':', alpha=0.4)
    ax2.legend(loc='lower center', bbox_to_anchor=(0.5, -0.32), ncol=3, fontsize=7.5)

    # 3. Top-Down Layout Diagram
    ax3 = fig.add_subplot(2, 2, 3)
    ax3.set_title("Top View (Layout Lengkap 150 x 100 mm)", fontsize=11, fontweight='bold')
    
    rect_outer = FancyBboxPatch((-75, -50), 150, 100, boxstyle="round,pad=0,rounding_size=5", facecolor='#e0f2fe', edgecolor='#0284c7', linewidth=2, label="Outer Case (150 x 100 mm)")
    ax3.add_patch(rect_outer)
    
    rect_lcd = FancyBboxPatch((-39, -25.75), 78, 51.5, boxstyle="round,pad=0,rounding_size=2", facecolor='#0f172a', edgecolor='#0284c7', linewidth=1.5, label="LCD Window (78 x 51.5 mm)")
    ax3.add_patch(rect_lcd)
    
    mic_cluster = Circle((0, 38), 3.2, facecolor='#fbbf24', edgecolor='#d97706', linewidth=1.5, label="INMP441 Mic (9x Ø1.3mm)")
    ax3.add_patch(mic_cluster)
    ax3.text(0, 42.5, "INMP441 MIC", ha='center', va='bottom', fontsize=8, fontweight='bold', color='#b45309')
    
    # USB Port on left side
    rect_usb = FancyBboxPatch((-76.5, -6.25), 3, 12.5, boxstyle="round,pad=0,rounding_size=1", facecolor='#10b981', edgecolor='#047857', linewidth=1.5, label="USB Charger (12.5 x 6.5 mm)")
    ax3.add_patch(rect_usb)
    ax3.text(-74, 9, "USB CHARGER", ha='right', va='bottom', fontsize=7.5, fontweight='bold', color='#047857')
    
    # Slide Switch on right side
    rect_sw = FancyBboxPatch((73.5, -4.25), 3, 8.5, boxstyle="round,pad=0,rounding_size=1", facecolor='#ef4444', edgecolor='#b91c1c', linewidth=1.5, label="Saklar Geser (8.5 x 4.5 mm)")
    ax3.add_patch(rect_sw)
    ax3.text(74, 8, "SAKLAR GESER", ha='left', va='bottom', fontsize=7.5, fontweight='bold', color='#b91c1c')

    for cx in [-66.5, 66.5]:
        for cy in [-41.5, 41.5]:
            ax3.plot(cx, cy, 'x', color='#0284c7', markersize=6, markeredgewidth=2)
    ax3.plot([], [], 'x', color='#0284c7', label="M3 Screws (x4)")
    
    ax3.set_xlim([-85, 85])
    ax3.set_ylim([-60, 60])
    ax3.set_aspect('equal')
    ax3.grid(True, linestyle=':', alpha=0.5)
    ax3.legend(loc='lower center', bbox_to_anchor=(0.5, -0.32), ncol=3, fontsize=7.5)
    
    # 4. Blueprint Reference Preview (Thumbnail of official datasheet)
    ax4 = fig.add_subplot(2, 2, 4)
    ax4.set_title("Referensi Blueprint Resmi MSP3520 (LCDWiki)", fontsize=11, fontweight='bold')
    
    try:
        bp_img = Image.open("msp3520_size.png")
        ax4.imshow(bp_img)
        ax4.axis('off')
    except Exception as e:
        ax4.text(0.5, 0.5, "Blueprint Loaded", ha='center', va='center')
    
    plt.tight_layout()
    
    out_workspace = "casing_preview.png"
    out_artifact = os.path.join(artifact_dir, "casing_preview.png")
    plt.savefig(out_workspace, dpi=150, bbox_inches='tight')
    plt.savefig(out_artifact, dpi=150, bbox_inches='tight')
    print(f"Preview updated: {out_workspace} and {out_artifact}")

if __name__ == '__main__':
    render_views()
