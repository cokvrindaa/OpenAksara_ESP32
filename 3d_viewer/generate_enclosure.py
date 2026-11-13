"""
Updated Script to generate 3D Printable Enclosure with:
- Panjang: 180 mm (18 cm)
- Lebar: 100 mm (10 cm)
- Kedalaman / Tinggi: 50 mm (5.0 cm) -> Lebih ramping, proporsional, dan tetap sangat lega di dalam!
- TUTUP BAWAH 100% RATA MULUS (FLAT):
  * Tanpa bibir/pinggiran timbul keliling
  * Tanpa cincin/pinggiran timbul speaker
  * Dilengkapi 4 lubang baut M3 countersunk (rata permukaan)
- Top cutout untuk 3.5" ILI9488 LCD + ceruk bezel kaca + relief notch FPC Touch
- Speaker grill radial (35 lubang) tembus rata di pelat bawah
- Port USB charger horizontal di tengah dinding samping kiri (X = -90 mm, Z = 25.0 mm)
- Mini Slide Switch (saklar geser) di tengah dinding samping kanan (X = +90 mm, Z = 25.0 mm)
- Array mikrofon INMP441 di atas layar LCD
"""

import os
from manifold3d import CrossSection, Manifold
import trimesh
import numpy as np

def create_rounded_rect(length, width, radius, segments=32):
    dx = length / 2.0 - radius
    dy = width / 2.0 - radius
    c1 = CrossSection.circle(radius, segments).translate([dx, dy])
    c2 = CrossSection.circle(radius, segments).translate([-dx, dy])
    c3 = CrossSection.circle(radius, segments).translate([dx, -dy])
    c4 = CrossSection.circle(radius, segments).translate([-dx, -dy])
    return CrossSection.batch_hull([c1, c2, c3, c4])

def create_cylinder(radius, height, segments=32):
    return Manifold.cylinder(height, radius, radius, segments)

def build_top_case(switch_position='right'):
    # DIMENSIONS: 180mm (L) x 100mm (W) x 50mm (H)
    L = 180.0
    W = 100.0
    H_total = 50.0
    bottom_thickness = 3.0
    H_top = H_total - bottom_thickness # 47.0mm
    wall_t = 2.5
    corner_r = 5.0
    
    # 1. Outer body (from z = 3.0 to z = 50.0)
    outer_2d = create_rounded_rect(L, W, corner_r, 48)
    outer_solid = outer_2d.extrude(H_top).translate([0, 0, bottom_thickness])
    
    # 2. Inner cavity (from z = 3.0 to z = 47.5)
    inner_L = L - 2 * wall_t # 175.0mm
    inner_W = W - 2 * wall_t # 95.0mm
    inner_r = max(1.0, corner_r - wall_t) # 2.5mm
    inner_2d = create_rounded_rect(inner_L, inner_W, inner_r, 48)
    inner_cavity = inner_2d.extrude(H_top - wall_t + 0.1).translate([0, 0, bottom_thickness - 0.05])
    
    top_shell = outer_solid - inner_cavity
    
    # 3. Corner screw bosses (M3)
    cx = L / 2.0 - 8.5 # 180/2 - 8.5 = 81.5mm
    cy = W / 2.0 - 8.5 # 100/2 - 8.5 = 41.5mm
    boss_r = 4.5
    screw_hole_r = 1.5 # 3.0mm diameter
    
    bosses = []
    holes = []
    for sx in [-cx, cx]:
        for sy in [-cy, cy]:
            # Boss from z = 3.0 to z = 47.5
            boss = create_cylinder(boss_r, H_top - wall_t, 32).translate([sx, sy, bottom_thickness])
            bosses.append(boss)
            # Pilot hole depth 25mm into the boss
            hole = create_cylinder(screw_hole_r, 25.0, 32).translate([sx, sy, bottom_thickness - 0.1])
            holes.append(hole)
            
    for b in bosses:
        top_shell = top_shell + b
    for h in holes:
        top_shell = top_shell - h
        
    # 4. LCD Cutout Window (78.0 x 51.5 mm, centered at X=0, Y=0)
    win_2d = create_rounded_rect(78.0, 51.5, 2.0, 32)
    win_cut = win_2d.extrude(wall_t + 2.0).translate([0, 0, H_total - wall_t - 1.0])
    top_shell = top_shell - win_cut
    
    # 5. Inner LCD Glass Recess (86.5 x 56.5 x 1.4 mm)
    glass_2d = create_rounded_rect(86.5, 56.5, 2.0, 32)
    glass_recess = glass_2d.extrude(1.4).translate([0, 0, H_total - wall_t - 0.01])
    top_shell = top_shell - glass_recess
    
    # 5b. Touch Screen FPC Notch Relief Pocket (94.0 x 20.0 x 1.6 mm)
    fpc_notch_2d = create_rounded_rect(94.0, 20.0, 2.0, 32)
    fpc_notch_cut = fpc_notch_2d.extrude(1.6).translate([0, 0, H_total - wall_t - 0.21])
    top_shell = top_shell - fpc_notch_cut
    
    # 6. LCD Module Mounting Standoffs (92.0 x 49.5 mm)
    lcd_cx = 92.0 / 2.0 # 46.0mm
    lcd_cy = 49.5 / 2.0 # 24.75mm
    lcd_boss_r = 3.2
    lcd_hole_r = 1.25 # 2.5mm hole for M2.5 / M3
    standoff_h = 4.0
    
    lcd_bosses = []
    lcd_holes = []
    for lx in [-lcd_cx, lcd_cx]:
        for ly in [-lcd_cy, lcd_cy]:
            lboss = create_cylinder(lcd_boss_r, standoff_h, 24).translate([lx, ly, H_total - wall_t - standoff_h])
            lcd_bosses.append(lboss)
            lhole = create_cylinder(lcd_hole_r, standoff_h + 1.5, 24).translate([lx, ly, H_total - wall_t - standoff_h - 0.1])
            lcd_holes.append(lhole)
            
    for lb in lcd_bosses:
        top_shell = top_shell + lb
    for lh in lcd_holes:
        top_shell = top_shell - lh
        
    # 7. INMP441 Microphone Holes on Top Surface
    mic_x = 0.0
    mic_y = 38.0
    mic_r = 0.65 # diameter 1.3 mm
    mic_offsets = [
        (0.0, 0.0),
        (2.0, 0.0),
        (-2.0, 0.0),
        (0.0, 2.0),
        (0.0, -2.0),
        (1.4, 1.4),
        (-1.4, 1.4),
        (1.4, -1.4),
        (-1.4, -1.4)
    ]
    mic_holes = []
    for mdx, mdy in mic_offsets:
        mh = create_cylinder(mic_r, wall_t + 2.0, 16).translate([mic_x + mdx, mic_y + mdy, H_total - wall_t - 1.0])
        mic_holes.append(mh)
    for mh in mic_holes:
        top_shell = top_shell - mh

    # Inside guide pocket for INMP441 breakout board (14.5 x 12.5 mm, depth 1.2mm)
    inmp_guide = create_rounded_rect(14.5, 12.5, 1.0, 16).extrude(1.2).translate([mic_x, mic_y, H_total - wall_t - 0.01])
    top_shell = top_shell - inmp_guide
    
    # 8. HORIZONTAL & CENTERED USB Charger Port Cutout on Side Wall (Left side X = -90mm)
    # Centered vertically at Z = 25.0 mm (exact middle of 50mm height)
    z_center = H_total / 2.0 # 25.0 mm
    usb_w_y = 12.5
    usb_h_z = 6.5
    usb_r = 2.0
    dz_2d = usb_h_z / 2.0 - usb_r
    dy_2d = usb_w_y / 2.0 - usb_r
    c1 = CrossSection.circle(usb_r, 24).translate([dz_2d, dy_2d])
    c2 = CrossSection.circle(usb_r, 24).translate([-dz_2d, dy_2d])
    c3 = CrossSection.circle(usb_r, 24).translate([dz_2d, -dy_2d])
    c4 = CrossSection.circle(usb_r, 24).translate([-dz_2d, -dy_2d])
    usb_slot_2d = CrossSection.batch_hull([c1, c2, c3, c4])
    usb_cutter = usb_slot_2d.extrude(20.0).rotate([0, 90, 0]).translate([-(L/2.0 + 10.0), 0, z_center])
    top_shell = top_shell - usb_cutter

    # 9. MINI SLIDE SWITCH (Right side X = +90mm, Centered vertically at Z = 25.0 mm)
    sw_w_y = 8.5
    sw_h_z = 4.5
    sw_r = 1.2
    s_dz = sw_h_z / 2.0 - sw_r
    s_dy = sw_w_y / 2.0 - sw_r
    sc1 = CrossSection.circle(sw_r, 16).translate([s_dz, s_dy])
    sc2 = CrossSection.circle(sw_r, 16).translate([-s_dz, s_dy])
    sc3 = CrossSection.circle(sw_r, 16).translate([s_dz, -s_dy])
    sc4 = CrossSection.circle(sw_r, 16).translate([-s_dz, -s_dy])
    slide_slot_2d = CrossSection.batch_hull([sc1, sc2, sc3, sc4])

    pocket_2d = CrossSection.square([6.0, 10.5], center=True)
    
    if switch_position == 'right':
        slot_cutter = slide_slot_2d.extrude(20.0).rotate([0, 90, 0]).translate([L/2.0 - 10.0, 0, z_center])
        pocket_cutter = pocket_2d.extrude(1.4).rotate([0, 90, 0]).translate([L/2.0 - wall_t - 0.05, 0, z_center])
        h1 = Manifold.cylinder(2.5, 0.9, 0.9, 16).rotate([0, 90, 0]).translate([L/2.0 - wall_t - 0.5, 7.5, z_center])
        h2 = Manifold.cylinder(2.5, 0.9, 0.9, 16).rotate([0, 90, 0]).translate([L/2.0 - wall_t - 0.5, -7.5, z_center])
        top_shell = top_shell - slot_cutter - pocket_cutter - h1 - h2
    elif switch_position == 'left':
        slot_cutter = slide_slot_2d.extrude(20.0).rotate([0, 90, 0]).translate([-(L/2.0 + 10.0), 25.0, z_center])
        pocket_cutter = pocket_2d.extrude(1.4).rotate([0, 90, 0]).translate([-(L/2.0 - wall_t + 1.2), 25.0, z_center])
        h1 = Manifold.cylinder(2.5, 0.9, 0.9, 16).rotate([0, 90, 0]).translate([-(L/2.0 - wall_t + 0.5), 32.5, z_center])
        h2 = Manifold.cylinder(2.5, 0.9, 0.9, 16).rotate([0, 90, 0]).translate([-(L/2.0 - wall_t + 0.5), 17.5, z_center])
        top_shell = top_shell - slot_cutter - pocket_cutter - h1 - h2
        
    return top_shell

def build_bottom_lid():
    # DIMENSIONS: 180mm (L) x 100mm (W) x 3mm (Thickness)
    L = 180.0
    W = 100.0
    bottom_t = 3.0
    corner_r = 5.0
    
    # 1. Base plate: 180 x 100 x 3mm completely flat
    base_2d = create_rounded_rect(L, W, corner_r, 48)
    lid = base_2d.extrude(bottom_t)
    
    # 2. Countersunk Screw Holes (M3)
    cx = L / 2.0 - 8.5 # 81.5mm
    cy = W / 2.0 - 8.5 # 41.5mm
    for sx in [-cx, cx]:
        for sy in [-cy, cy]:
            through_hole = create_cylinder(1.7, bottom_t + 3.0, 32).translate([sx, sy, -0.5])
            cs_hole = create_cylinder(3.1, 1.6 + 0.1, 32).translate([sx, sy, -0.05])
            cone = Manifold.cylinder(1.0, 3.1, 1.7, 32).translate([sx, sy, 1.55])
            lid = lid - through_hole
            lid = lid - cs_hole
            lid = lid - cone
            
    # 3. Speaker Grill on Bottom Lid (Clean through-holes without any raised ring)
    spk_cx = 40.0
    spk_cy = 0.0
    spk_hole_r = 1.1 # 2.2mm diameter
    
    spk_holes = [(0, 0)]
    rings = [(4.5, 6), (9.0, 12), (13.5, 16)]
    for r, n in rings:
        angles = np.linspace(0, 2*np.pi, n, endpoint=False)
        for a in angles:
            spk_holes.append((r * np.cos(a), r * np.sin(a)))
            
    spk_cutters = []
    for shx, shy in spk_holes:
        hole = create_cylinder(spk_hole_r, bottom_t + 2.0, 20).translate([spk_cx + shx, spk_cy + shy, -1.0])
        spk_cutters.append(hole)
        
    for sc in spk_cutters:
        lid = lid - sc

    return lid

if __name__ == '__main__':
    print("Generating Updated 3D Models with 180mm Length, 50mm (5cm) Depth, and Flat Bottom Lid...")
    top = build_top_case(switch_position='right')
    print("Top case generated. Vertices:", top.num_vert(), "Triangles:", top.num_tri())
    
    bottom = build_bottom_lid()
    print("Bottom lid generated. Vertices:", bottom.num_vert(), "Triangles:", bottom.num_tri())
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    models_dir = os.path.join(script_dir, "models")
    os.makedirs(models_dir, exist_ok=True)

    def export_stl(manifold_obj, filename):
        mesh_data = manifold_obj.to_mesh()
        mesh = trimesh.Trimesh(vertices=mesh_data.vert_properties, faces=mesh_data.tri_verts)
        target1 = os.path.join(script_dir, filename)
        target2 = os.path.join(models_dir, filename)
        mesh.export(target1)
        mesh.export(target2)
        print(f"Exported {filename} (Watertight: {mesh.is_watertight}, Volume: {mesh.volume/1000:.1f} cm3)")

    export_stl(top, "casing_atas.stl")
    export_stl(bottom, "tutup_bawah.stl")
    
    assembly = top + bottom
    export_stl(assembly, "casing_lengkap_assembly.stl")
    print("All STL files updated successfully in 3d_viewer/ and 3d_viewer/models/!")
