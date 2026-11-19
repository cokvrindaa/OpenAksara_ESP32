#include "ui_jawa.h"
#include "ui_home.h"
#include "ui_learn.h"
#include "ui_settings.h"
#include "firebase_sync.h"
#include <stdio.h>

static const AppCallbacks* jawa_cbs = nullptr;
static lv_obj_t* scr_jawa = nullptr;

// Container 3 Tampilan
static lv_obj_t* view_path = nullptr;
static lv_obj_t* view_write = nullptr;
static lv_obj_t* view_settings = nullptr;

// Tombol Tab Bawah
static lv_obj_t* tab_btn_learn = nullptr;
static lv_obj_t* tab_btn_write = nullptr;
static lv_obj_t* tab_btn_settings = nullptr;
static lv_obj_t* tab_lbl_learn = nullptr;
static lv_obj_t* tab_lbl_write = nullptr;
static lv_obj_t* tab_lbl_settings = nullptr;

// State Tab Aktif: 0 = Belajar, 1 = Nulis Aksara, 2 = Setelan
static int current_tab = 0;

// Pagination Jalur Belajar (Tanpa Scroll, Anti Nge-lag)
static int current_page = 0; // 0 = Babak 1, 1 = Babak 2, 2 = Babak 3
static lv_obj_t* path_nodes_cont = nullptr;
static lv_obj_t* btn_page_up = nullptr;
static lv_obj_t* btn_page_down = nullptr;
static lv_obj_t* lbl_page_ind = nullptr;
static lv_obj_t* header_sub = nullptr;

// Aksara Jawa Data (Carakan - 20 Konsonan Lengkap)
struct PathNodeData {
  const char* name;
  const char* symbol;
  int state; // 0 = Selesai, 1 = Aktif, 2 = Peti/Bonus, 3 = Terkunci
};

// 7 Halaman, masing-masing 3 node (total 21 node: 20 Carakan + 1 Piala Juara)
static const PathNodeData JAWA_PAGES[7][3] = {
  // Babak 1
  {
    { "Ha", "\xEA\xA6\xB2", 0 },
    { "Na", "\xEA\xA6\xA4", 0 },
    { "Ca", "\xEA\xA6\x95", 1 }
  },
  // Babak 2
  {
    { "Ra", "\xEA\xA6\xAB", 3 },
    { "Ka", "\xEA\xA6\x8F", 3 },
    { "Da", "\xEA\xA6\xA2", 3 }
  },
  // Babak 3
  {
    { "Ta", "\xEA\xA6\xA0", 3 },
    { "Sa", "\xEA\xA6\xB1", 3 },
    { "Wa", "\xEA\xA6\xAE", 3 }
  },
  // Babak 4
  {
    { "La", "\xEA\xA6\xAD", 3 },
    { "Pa", "\xEA\xA6\xA5", 3 },
    { "Dha", "\xEA\xA6\x9D", 3 }
  },
  // Babak 5
  {
    { "Ja", "\xEA\xA6\x97", 3 },
    { "Ya", "\xEA\xA6\xAA", 3 },
    { "Nya", "\xEA\xA6\x9A", 3 }
  },
  // Babak 6
  {
    { "Ma", "\xEA\xA6\xA9", 3 },
    { "Ga", "\xEA\xA6\x92", 3 },
    { "Ba", "\xEA\xA6\xA7", 3 }
  },
  // Babak 7
  {
    { "Tha", "\xEA\xA6\x9B", 3 },
    { "Nga", "\xEA\xA6\x94", 3 },
    { "Piala", LV_SYMBOL_OK, 2 }
  }
};

// Canvas Nulis Aksara
static lv_obj_t* write_target_title = nullptr;
static lv_obj_t* write_target_sub = nullptr;
static lv_obj_t* write_watermark_lbl = nullptr;
static lv_obj_t* write_guide_track = nullptr;
static lv_obj_t* write_feedback_lbl = nullptr;
static lv_obj_t* write_preview_lbl = nullptr;
static int active_write_index = 2; // Default ke aksara "Ca"
static TraceState jawa_tab_trace;
static lv_point_precise_t jawa_guide_pts[100];

// 20 Aksara Jawa Carakan + 1 Piala Juara
static const char* JAWA_NAMES[21] = {
  "Ha", "Na", "Ca", "Ra", "Ka", "Da",
  "Ta", "Sa", "Wa", "La", "Pa", "Dha",
  "Ja", "Ya", "Nya", "Ma", "Ga", "Ba",
  "Tha", "Nga", "Piala"
};
static const char* JAWA_AKSARA[21] = {
  "\xEA\xA6\xB2", "\xEA\xA6\xA4", "\xEA\xA6\x95", "\xEA\xA6\xAB", "\xEA\xA6\x8F", "\xEA\xA6\xA2",
  "\xEA\xA6\xA0", "\xEA\xA6\xB1", "\xEA\xA6\xAE", "\xEA\xA6\xAD", "\xEA\xA6\xA5", "\xEA\xA6\x9D",
  "\xEA\xA6\x97", "\xEA\xA6\xAA", "\xEA\xA6\x9A", "\xEA\xA6\xA9", "\xEA\xA6\x92", "\xEA\xA6\xA7",
  "\xEA\xA6\x9B", "\xEA\xA6\x94", "Win"
};

// Waypoint Jalur Aksara Jawa (Tersinkronisasi Presisi dengan Font Asli 72px di Tengah Kanvas 308x160)
static const StrokeWaypoint JAWA_GUIDE_HA[] = {
  { 87, 116 }, { 81, 112 }, { 74, 109 }, { 74, 102 }, { 73, 95 }, 
  { 73, 88 }, { 73, 81 }, { 74, 74 }, { 77, 67 }, { 74, 60 }, 
  { 77, 53 }, { 82, 48 }, { 89, 45 }, { 96, 43 }, { 103, 44 }, 
  { 108, 50 }, { 111, 57 }, { 114, 64 }, { 112, 72 }, { 113, 79 }, 
  { 113, 86 }, { 112, 93 }, { 112, 100 }, { 112, 107 }, { 115, 114 }, 
  { 122, 117 }, { 129, 117 }, { 136, 118 }, { 143, 118 }, { 150, 115 }, 
  { 157, 112 }, { 158, 105 }, { 158, 98 }, { 160, 91 }, { 161, 84 }, 
  { 160, 77 }, { 160, 70 }, { 159, 63 }, { 159, 56 }, { 160, 49 }, 
  { 166, 45 }, { 173, 42 }, { 180, 44 }, { 187, 47 }, { 193, 51 }, 
  { 197, 58 }, { 196, 65 }, { 195, 72 }, { 195, 79 }, { 196, 86 }, 
  { 194, 93 }, { 195, 100 }, { 195, 107 }, { 195, 114 }, { 195, 121 }, 
  { 196, 114 }, { 196, 107 }, { 196, 100 }, { 195, 93 }, { 196, 86 }, 
  { 196, 79 }, { 196, 72 }, { 194, 65 }, { 192, 58 }, { 198, 54 }, 
  { 200, 47 }, { 207, 45 }, { 214, 43 }, { 221, 43 }, { 227, 47 }, 
  { 230, 54 }, { 232, 61 }, { 232, 68 }, { 234, 75 }, { 231, 82 }, 
  { 231, 90 }, { 232, 97 }, { 232, 104 }, { 230, 111 }, { 230, 118 }
};
static const StrokeWaypoint JAWA_GUIDE_NA[] = {
  { 108, 119 }, { 101, 117 }, { 95, 113 }, { 93, 106 }, { 91, 99 }, 
  { 92, 92 }, { 92, 85 }, { 90, 78 }, { 91, 71 }, { 91, 64 }, 
  { 92, 57 }, { 95, 50 }, { 101, 46 }, { 106, 41 }, { 114, 42 }, 
  { 121, 45 }, { 127, 49 }, { 129, 56 }, { 130, 63 }, { 130, 70 }, 
  { 131, 77 }, { 129, 84 }, { 129, 91 }, { 130, 98 }, { 131, 105 }, 
  { 130, 112 }, { 130, 119 }, { 132, 112 }, { 133, 105 }, { 131, 98 }, 
  { 133, 91 }, { 140, 89 }, { 147, 90 }, { 154, 91 }, { 162, 90 }, 
  { 171, 93 }, { 179, 95 }, { 184, 100 }, { 186, 107 }, { 181, 112 }, 
  { 175, 116 }, { 168, 118 }, { 162, 113 }, { 158, 107 }, { 160, 100 }, 
  { 160, 93 }, { 160, 86 }, { 157, 79 }, { 158, 72 }, { 159, 65 }, 
  { 162, 58 }, { 166, 52 }, { 171, 47 }, { 176, 42 }, { 183, 42 }, 
  { 190, 43 }, { 197, 43 }, { 204, 45 }, { 210, 49 }, { 213, 56 }, 
  { 215, 63 }, { 216, 70 }, { 217, 77 }, { 217, 84 }, { 217, 91 }, 
  { 217, 98 }, { 218, 105 }, { 217, 112 }, { 216, 119 }
};
static const StrokeWaypoint JAWA_GUIDE_CA[] = {
  { 104, 119 }, { 97, 116 }, { 91, 112 }, { 89, 105 }, { 87, 98 }, 
  { 87, 91 }, { 88, 84 }, { 88, 77 }, { 90, 70 }, { 91, 63 }, 
  { 91, 56 }, { 95, 50 }, { 102, 48 }, { 109, 46 }, { 116, 45 }, 
  { 123, 44 }, { 130, 46 }, { 135, 51 }, { 139, 57 }, { 141, 64 }, 
  { 141, 71 }, { 141, 78 }, { 141, 85 }, { 141, 92 }, { 140, 99 }, 
  { 139, 106 }, { 136, 113 }, { 128, 116 }, { 121, 115 }, { 115, 111 }, 
  { 115, 104 }, { 119, 98 }, { 125, 94 }, { 133, 95 }, { 140, 98 }, 
  { 147, 100 }, { 154, 101 }, { 161, 102 }, { 165, 108 }, { 170, 113 }, 
  { 177, 116 }, { 184, 116 }, { 187, 109 }, { 188, 102 }, { 190, 95 }, 
  { 191, 88 }, { 190, 81 }, { 189, 74 }, { 187, 67 }, { 186, 60 }, 
  { 183, 53 }, { 179, 47 }, { 173, 43 }, { 166, 41 }, { 173, 42 }, 
  { 180, 41 }, { 187, 41 }, { 194, 41 }, { 201, 43 }, { 208, 45 }, 
  { 215, 48 }, { 217, 55 }, { 218, 62 }, { 221, 69 }, { 221, 76 }, 
  { 222, 83 }, { 222, 90 }, { 222, 97 }, { 222, 104 }, { 221, 111 }, 
  { 221, 118 }
};
static const StrokeWaypoint JAWA_GUIDE_RA[] = {
  { 109, 61 }, { 105, 55 }, { 100, 50 }, { 95, 45 }, { 90, 40 }, 
  { 97, 43 }, { 104, 42 }, { 111, 43 }, { 118, 41 }, { 125, 43 }, 
  { 132, 44 }, { 138, 48 }, { 141, 55 }, { 140, 62 }, { 141, 69 }, 
  { 139, 76 }, { 136, 83 }, { 133, 90 }, { 128, 95 }, { 123, 101 }, 
  { 117, 105 }, { 124, 105 }, { 131, 108 }, { 138, 112 }, { 145, 113 }, 
  { 152, 115 }, { 159, 116 }, { 165, 112 }, { 165, 105 }, { 166, 98 }, 
  { 166, 91 }, { 166, 84 }, { 166, 77 }, { 166, 70 }, { 166, 63 }, 
  { 166, 56 }, { 170, 50 }, { 175, 45 }, { 182, 44 }, { 190, 43 }, 
  { 197, 43 }, { 204, 46 }, { 210, 50 }, { 213, 57 }, { 214, 64 }, 
  { 215, 71 }, { 215, 79 }, { 215, 87 }, { 215, 95 }, { 215, 103 }, 
  { 214, 110 }, { 213, 117 }
};
static const StrokeWaypoint JAWA_GUIDE_KA[] = {
  { 88, 118 }, { 81, 114 }, { 76, 109 }, { 74, 102 }, { 73, 95 }, 
  { 74, 88 }, { 72, 81 }, { 72, 74 }, { 73, 67 }, { 72, 60 }, 
  { 75, 53 }, { 79, 47 }, { 86, 44 }, { 93, 43 }, { 100, 45 }, 
  { 106, 49 }, { 110, 55 }, { 111, 62 }, { 110, 69 }, { 110, 76 }, 
  { 110, 83 }, { 110, 90 }, { 111, 99 }, { 111, 106 }, { 110, 113 }, 
  { 108, 120 }, { 109, 113 }, { 109, 106 }, { 113, 99 }, { 116, 92 }, 
  { 121, 87 }, { 128, 87 }, { 135, 88 }, { 142, 89 }, { 149, 90 }, 
  { 156, 91 }, { 162, 95 }, { 165, 102 }, { 165, 109 }, { 160, 114 }, 
  { 154, 118 }, { 147, 116 }, { 142, 111 }, { 139, 104 }, { 141, 97 }, 
  { 141, 90 }, { 141, 83 }, { 140, 76 }, { 141, 69 }, { 142, 62 }, 
  { 143, 55 }, { 147, 49 }, { 154, 45 }, { 161, 43 }, { 168, 43 }, 
  { 175, 43 }, { 182, 45 }, { 188, 49 }, { 194, 53 }, { 196, 60 }, 
  { 199, 67 }, { 199, 74 }, { 199, 81 }, { 197, 88 }, { 197, 95 }, 
  { 198, 102 }, { 198, 109 }, { 198, 116 }, { 199, 123 }, { 198, 116 }, 
  { 196, 109 }, { 195, 102 }, { 195, 95 }, { 198, 88 }, { 199, 81 }, 
  { 198, 74 }, { 197, 67 }, { 199, 60 }, { 202, 53 }, { 207, 48 }, 
  { 214, 44 }, { 221, 42 }, { 227, 46 }, { 230, 53 }, { 233, 60 }, 
  { 233, 67 }, { 234, 74 }, { 236, 81 }, { 233, 88 }, { 234, 95 }, 
  { 234, 102 }, { 235, 109 }, { 235, 116 }, { 236, 123 }
};
static const StrokeWaypoint JAWA_GUIDE_DA[] = {
  { 102, 119 }, { 95, 117 }, { 89, 113 }, { 86, 106 }, { 86, 99 }, 
  { 87, 92 }, { 84, 85 }, { 82, 78 }, { 83, 71 }, { 86, 64 }, 
  { 88, 57 }, { 90, 50 }, { 95, 45 }, { 102, 44 }, { 109, 44 }, 
  { 116, 44 }, { 123, 44 }, { 130, 44 }, { 137, 43 }, { 144, 43 }, 
  { 137, 42 }, { 130, 45 }, { 126, 51 }, { 122, 57 }, { 118, 64 }, 
  { 116, 71 }, { 115, 78 }, { 113, 85 }, { 113, 92 }, { 114, 99 }, 
  { 115, 106 }, { 117, 113 }, { 124, 116 }, { 131, 114 }, { 138, 111 }, 
  { 145, 108 }, { 150, 103 }, { 157, 100 }, { 162, 95 }, { 169, 92 }, 
  { 176, 92 }, { 183, 93 }, { 188, 98 }, { 189, 105 }, { 188, 112 }, 
  { 183, 117 }, { 176, 117 }, { 169, 115 }, { 165, 109 }, { 164, 102 }, 
  { 163, 95 }, { 164, 88 }, { 163, 81 }, { 162, 73 }, { 164, 64 }, 
  { 166, 56 }, { 171, 50 }, { 178, 46 }, { 185, 45 }, { 192, 43 }, 
  { 199, 43 }, { 206, 45 }, { 213, 47 }, { 219, 51 }, { 222, 58 }, 
  { 222, 65 }, { 222, 72 }, { 222, 79 }, { 222, 86 }, { 223, 93 }, 
  { 222, 100 }, { 222, 107 }, { 224, 114 }, { 223, 121 }
};
static const StrokeWaypoint JAWA_GUIDE_TA[] = {
  { 93, 119 }, { 86, 118 }, { 81, 113 }, { 77, 107 }, { 76, 100 }, 
  { 74, 93 }, { 74, 86 }, { 76, 79 }, { 76, 72 }, { 77, 65 }, 
  { 77, 58 }, { 77, 51 }, { 82, 46 }, { 89, 43 }, { 96, 43 }, 
  { 103, 42 }, { 107, 48 }, { 111, 54 }, { 112, 61 }, { 112, 68 }, 
  { 112, 75 }, { 114, 82 }, { 115, 89 }, { 115, 96 }, { 115, 103 }, 
  { 113, 110 }, { 118, 115 }, { 125, 116 }, { 132, 117 }, { 139, 118 }, 
  { 146, 118 }, { 153, 117 }, { 160, 117 }, { 167, 114 }, { 171, 108 }, 
  { 173, 101 }, { 173, 94 }, { 167, 89 }, { 161, 85 }, { 155, 81 }, 
  { 149, 77 }, { 144, 72 }, { 139, 66 }, { 136, 59 }, { 136, 52 }, 
  { 140, 46 }, { 147, 43 }, { 154, 42 }, { 162, 41 }, { 169, 41 }, 
  { 176, 43 }, { 183, 46 }, { 189, 50 }, { 192, 57 }, { 193, 64 }, 
  { 195, 71 }, { 194, 78 }, { 193, 85 }, { 193, 92 }, { 193, 99 }, 
  { 193, 106 }, { 193, 113 }, { 194, 120 }, { 193, 113 }, { 195, 106 }, 
  { 197, 99 }, { 198, 92 }, { 197, 85 }, { 197, 78 }, { 196, 71 }, 
  { 198, 64 }, { 199, 57 }, { 200, 50 }, { 201, 43 }, { 208, 43 }, 
  { 215, 44 }, { 222, 44 }, { 228, 48 }, { 229, 55 }, { 229, 62 }, 
  { 230, 69 }, { 231, 76 }, { 231, 83 }, { 233, 90 }, { 234, 97 }, 
  { 235, 104 }, { 233, 111 }, { 232, 118 }
};

static const StrokeWaypoint JAWA_GUIDE_DEF[] = {
  { 104, 115 }, { 120, 52 }, { 148, 100 }, { 178, 52 }, { 204, 115 }
};

static const StrokeWaypoint JAWA_GUIDE_SA[] = {
  { 100, 119 }, { 93, 116 }, { 87, 112 }, { 83, 106 }, { 82, 99 }, 
  { 85, 91 }, { 85, 84 }, { 86, 77 }, { 86, 70 }, { 86, 63 }, 
  { 87, 56 }, { 89, 49 }, { 94, 44 }, { 101, 43 }, { 108, 43 }, 
  { 115, 42 }, { 122, 42 }, { 128, 46 }, { 132, 52 }, { 135, 59 }, 
  { 137, 66 }, { 137, 73 }, { 137, 80 }, { 138, 87 }, { 139, 94 }, 
  { 139, 101 }, { 137, 108 }, { 135, 115 }, { 130, 120 }, { 123, 118 }, 
  { 116, 115 }, { 115, 108 }, { 115, 101 }, { 119, 95 }, { 125, 91 }, 
  { 132, 92 }, { 139, 95 }, { 146, 97 }, { 154, 102 }, { 160, 106 }, 
  { 166, 110 }, { 172, 114 }, { 179, 118 }, { 184, 113 }, { 184, 106 }, 
  { 186, 99 }, { 187, 92 }, { 187, 85 }, { 187, 78 }, { 185, 71 }, 
  { 186, 64 }, { 187, 57 }, { 190, 50 }, { 195, 45 }, { 202, 43 }, 
  { 209, 43 }, { 215, 47 }, { 220, 52 }, { 222, 59 }, { 222, 66 }, 
  { 223, 73 }, { 224, 80 }, { 224, 87 }, { 225, 94 }, { 225, 101 }, 
  { 225, 108 }, { 225, 115 }, { 226, 122 }
};

static const StrokeWaypoint JAWA_GUIDE_WA[] = {
  { 111, 119 }, { 104, 117 }, { 97, 113 }, { 93, 107 }, { 92, 100 }, 
  { 92, 93 }, { 93, 86 }, { 93, 79 }, { 93, 72 }, { 92, 65 }, 
  { 93, 58 }, { 93, 51 }, { 97, 45 }, { 104, 43 }, { 111, 43 }, 
  { 118, 43 }, { 124, 47 }, { 126, 54 }, { 128, 61 }, { 129, 68 }, 
  { 129, 75 }, { 129, 82 }, { 129, 89 }, { 130, 96 }, { 130, 103 }, 
  { 131, 110 }, { 136, 115 }, { 142, 119 }, { 149, 119 }, { 156, 119 }, 
  { 163, 119 }, { 170, 118 }, { 177, 116 }, { 182, 111 }, { 182, 104 }, 
  { 184, 97 }, { 185, 90 }, { 186, 83 }, { 185, 76 }, { 184, 69 }, 
  { 182, 62 }, { 178, 55 }, { 173, 50 }, { 167, 46 }, { 160, 43 }, 
  { 167, 43 }, { 174, 43 }, { 181, 43 }, { 188, 43 }, { 195, 43 }, 
  { 202, 43 }, { 209, 45 }, { 211, 52 }, { 213, 59 }, { 214, 66 }, 
  { 213, 73 }, { 213, 80 }, { 213, 87 }, { 213, 95 }, { 214, 102 }, 
  { 214, 109 }, { 215, 116 }, { 214, 123 }
};

static const StrokeWaypoint JAWA_GUIDE_LA[] = {
  { 91, 118 }, { 84, 116 }, { 79, 111 }, { 75, 105 }, { 73, 98 }, 
  { 76, 91 }, { 76, 84 }, { 76, 77 }, { 76, 70 }, { 76, 63 }, 
  { 77, 56 }, { 79, 49 }, { 85, 45 }, { 92, 44 }, { 99, 45 }, 
  { 106, 47 }, { 110, 53 }, { 112, 60 }, { 112, 67 }, { 112, 74 }, 
  { 112, 81 }, { 111, 88 }, { 111, 95 }, { 111, 102 }, { 111, 109 }, 
  { 112, 116 }, { 112, 123 }, { 112, 116 }, { 111, 109 }, { 110, 102 }, 
  { 110, 95 }, { 110, 88 }, { 112, 81 }, { 113, 74 }, { 115, 67 }, 
  { 115, 60 }, { 116, 53 }, { 118, 46 }, { 125, 43 }, { 132, 44 }, 
  { 139, 45 }, { 145, 49 }, { 146, 56 }, { 148, 63 }, { 148, 70 }, 
  { 150, 77 }, { 149, 84 }, { 149, 91 }, { 151, 98 }, { 149, 105 }, 
  { 149, 112 }, { 156, 117 }, { 163, 118 }, { 170, 119 }, { 177, 119 }, 
  { 184, 119 }, { 191, 119 }, { 194, 112 }, { 194, 105 }, { 194, 98 }, 
  { 195, 91 }, { 195, 84 }, { 195, 77 }, { 196, 70 }, { 198, 63 }, 
  { 197, 56 }, { 200, 49 }, { 205, 44 }, { 212, 43 }, { 220, 43 }, 
  { 227, 44 }, { 229, 51 }, { 230, 58 }, { 231, 65 }, { 232, 72 }, 
  { 234, 79 }, { 231, 86 }, { 231, 93 }, { 231, 100 }, { 231, 107 }, 
  { 231, 114 }, { 233, 121 }
};

static const StrokeWaypoint JAWA_GUIDE_PA[] = {
  { 111, 118 }, { 104, 116 }, { 97, 113 }, { 95, 106 }, { 92, 99 }, 
  { 92, 92 }, { 92, 85 }, { 92, 78 }, { 92, 71 }, { 93, 64 }, 
  { 94, 57 }, { 95, 50 }, { 100, 45 }, { 107, 42 }, { 114, 41 }, 
  { 120, 45 }, { 126, 49 }, { 129, 56 }, { 128, 63 }, { 129, 70 }, 
  { 130, 77 }, { 129, 84 }, { 130, 91 }, { 130, 98 }, { 130, 105 }, 
  { 134, 111 }, { 138, 117 }, { 145, 119 }, { 153, 120 }, { 160, 120 }, 
  { 167, 118 }, { 174, 115 }, { 176, 108 }, { 178, 101 }, { 178, 94 }, 
  { 178, 87 }, { 178, 80 }, { 176, 73 }, { 176, 66 }, { 177, 59 }, 
  { 178, 52 }, { 180, 45 }, { 187, 43 }, { 194, 43 }, { 201, 42 }, 
  { 208, 45 }, { 212, 51 }, { 213, 58 }, { 213, 65 }, { 214, 72 }, 
  { 214, 79 }, { 213, 86 }, { 215, 93 }, { 213, 100 }, { 212, 107 }, 
  { 211, 114 }, { 212, 121 }
};

static const StrokeWaypoint JAWA_GUIDE_DHA[] = {
  { 102, 119 }, { 95, 117 }, { 89, 113 }, { 86, 106 }, { 85, 99 }, 
  { 86, 92 }, { 85, 85 }, { 85, 78 }, { 85, 71 }, { 85, 64 }, 
  { 85, 57 }, { 87, 50 }, { 92, 45 }, { 99, 42 }, { 106, 41 }, 
  { 113, 44 }, { 119, 48 }, { 121, 55 }, { 122, 62 }, { 122, 69 }, 
  { 122, 76 }, { 122, 83 }, { 123, 90 }, { 123, 97 }, { 124, 104 }, 
  { 125, 111 }, { 128, 118 }, { 135, 118 }, { 142, 118 }, { 149, 116 }, 
  { 152, 109 }, { 154, 102 }, { 155, 95 }, { 157, 102 }, { 158, 109 }, 
  { 160, 116 }, { 166, 120 }, { 173, 118 }, { 180, 116 }, { 185, 111 }, 
  { 183, 104 }, { 185, 97 }, { 186, 90 }, { 187, 83 }, { 188, 76 }, 
  { 187, 69 }, { 187, 62 }, { 187, 55 }, { 189, 48 }, { 195, 44 }, 
  { 202, 42 }, { 209, 41 }, { 216, 43 }, { 220, 49 }, { 223, 56 }, 
  { 222, 63 }, { 222, 70 }, { 223, 77 }, { 223, 84 }, { 224, 91 }, 
  { 224, 98 }, { 224, 105 }, { 224, 112 }, { 224, 119 }
};

static const StrokeWaypoint JAWA_GUIDE_JA[] = {
  { 107, 120 }, { 100, 117 }, { 94, 112 }, { 92, 105 }, { 92, 98 }, 
  { 91, 91 }, { 92, 84 }, { 91, 77 }, { 91, 70 }, { 89, 63 }, 
  { 90, 56 }, { 94, 50 }, { 99, 45 }, { 106, 44 }, { 113, 44 }, 
  { 120, 46 }, { 127, 48 }, { 129, 55 }, { 128, 62 }, { 128, 69 }, 
  { 128, 76 }, { 129, 83 }, { 130, 90 }, { 130, 97 }, { 131, 104 }, 
  { 131, 111 }, { 135, 118 }, { 142, 121 }, { 149, 120 }, { 156, 119 }, 
  { 163, 118 }, { 170, 118 }, { 177, 118 }, { 184, 119 }, { 179, 114 }, 
  { 173, 110 }, { 167, 106 }, { 162, 101 }, { 160, 94 }, { 158, 87 }, 
  { 157, 80 }, { 156, 73 }, { 157, 66 }, { 159, 59 }, { 164, 54 }, 
  { 168, 48 }, { 174, 44 }, { 181, 42 }, { 188, 42 }, { 195, 42 }, 
  { 202, 42 }, { 209, 43 }, { 216, 44 }, { 213, 51 }, { 206, 54 }, 
  { 200, 58 }, { 195, 63 }, { 189, 67 }, { 182, 70 }, { 175, 73 }, 
  { 168, 76 }, { 162, 80 }, { 155, 83 }, { 161, 87 }, { 168, 87 }, 
  { 175, 87 }, { 182, 87 }, { 189, 87 }, { 196, 88 }, { 203, 90 }, 
  { 210, 91 }, { 211, 98 }, { 214, 105 }, { 215, 112 }, { 215, 119 }
};

static const StrokeWaypoint JAWA_GUIDE_MA[] = {
  { 110, 118 }, { 103, 119 }, { 97, 115 }, { 95, 108 }, { 93, 101 }, 
  { 93, 94 }, { 94, 87 }, { 95, 80 }, { 94, 73 }, { 92, 66 }, 
  { 93, 59 }, { 95, 52 }, { 97, 45 }, { 104, 44 }, { 111, 43 }, 
  { 118, 41 }, { 125, 40 }, { 132, 40 }, { 139, 42 }, { 146, 42 }, 
  { 153, 42 }, { 146, 45 }, { 139, 45 }, { 132, 47 }, { 128, 53 }, 
  { 126, 60 }, { 128, 67 }, { 132, 73 }, { 139, 76 }, { 146, 77 }, 
  { 153, 78 }, { 146, 79 }, { 140, 83 }, { 133, 85 }, { 128, 90 }, 
  { 126, 97 }, { 125, 104 }, { 130, 110 }, { 132, 117 }, { 139, 118 }, 
  { 146, 120 }, { 153, 120 }, { 160, 118 }, { 167, 117 }, { 174, 114 }, 
  { 176, 107 }, { 178, 100 }, { 178, 93 }, { 177, 86 }, { 177, 79 }, 
  { 176, 72 }, { 177, 65 }, { 178, 58 }, { 179, 51 }, { 184, 46 }, 
  { 191, 44 }, { 198, 43 }, { 205, 43 }, { 211, 47 }, { 215, 53 }, 
  { 215, 60 }, { 216, 67 }, { 216, 74 }, { 216, 81 }, { 216, 88 }, 
  { 216, 95 }, { 216, 102 }, { 216, 109 }, { 215, 116 }, { 215, 123 }
};

static const StrokeWaypoint JAWA_GUIDE_GA[] = {
  { 110, 82 }, { 103, 78 }, { 99, 72 }, { 96, 65 }, { 97, 58 }, 
  { 99, 51 }, { 104, 46 }, { 112, 42 }, { 119, 45 }, { 125, 49 }, 
  { 130, 54 }, { 131, 61 }, { 132, 68 }, { 133, 75 }, { 132, 82 }, 
  { 131, 89 }, { 131, 96 }, { 131, 103 }, { 132, 110 }, { 132, 117 }, 
  { 129, 110 }, { 129, 103 }, { 131, 96 }, { 131, 89 }, { 131, 82 }, 
  { 132, 75 }, { 133, 68 }, { 133, 61 }, { 135, 54 }, { 139, 48 }, 
  { 146, 45 }, { 153, 44 }, { 160, 46 }, { 167, 48 }, { 173, 53 }, 
  { 174, 60 }, { 173, 67 }, { 172, 74 }, { 173, 81 }, { 173, 88 }, 
  { 172, 95 }, { 172, 102 }, { 173, 109 }, { 173, 116 }, { 173, 123 }, 
  { 174, 116 }, { 173, 109 }, { 173, 102 }, { 174, 95 }, { 176, 88 }, 
  { 173, 81 }, { 173, 74 }, { 173, 67 }, { 175, 60 }, { 174, 53 }, 
  { 178, 47 }, { 185, 44 }, { 192, 42 }, { 199, 41 }, { 206, 44 }, 
  { 210, 50 }, { 214, 56 }, { 213, 63 }, { 215, 70 }, { 215, 77 }, 
  { 214, 84 }, { 214, 91 }, { 213, 98 }, { 213, 105 }, { 215, 112 }, 
  { 214, 119 }
};

static const StrokeWaypoint JAWA_GUIDE_THA[] = {
  { 109, 107 }, { 102, 105 }, { 96, 101 }, { 93, 94 }, { 92, 87 }, 
  { 92, 80 }, { 92, 73 }, { 94, 66 }, { 93, 59 }, { 91, 52 }, 
  { 92, 45 }, { 95, 38 }, { 100, 33 }, { 108, 33 }, { 115, 31 }, 
  { 122, 33 }, { 128, 37 }, { 129, 45 }, { 128, 52 }, { 129, 59 }, 
  { 131, 66 }, { 128, 73 }, { 128, 80 }, { 130, 87 }, { 131, 94 }, 
  { 131, 101 }, { 137, 105 }, { 144, 106 }, { 151, 106 }, { 158, 105 }, 
  { 165, 105 }, { 171, 101 }, { 175, 95 }, { 177, 88 }, { 173, 82 }, 
  { 166, 80 }, { 159, 81 }, { 155, 87 }, { 153, 94 }, { 151, 101 }, 
  { 149, 108 }, { 149, 115 }, { 152, 122 }, { 156, 128 }, { 161, 133 }, 
  { 168, 131 }, { 174, 127 }, { 181, 126 }, { 165, 59 }, { 157, 56 }, 
  { 154, 49 }, { 154, 42 }, { 155, 35 }, { 161, 31 }, { 168, 31 }, 
  { 175, 34 }, { 180, 40 }, { 184, 47 }, { 187, 40 }, { 191, 34 }, 
  { 198, 31 }, { 205, 32 }, { 211, 36 }, { 213, 43 }, { 215, 50 }, 
  { 217, 58 }, { 216, 65 }, { 215, 72 }, { 215, 79 }, { 215, 86 }, 
  { 214, 93 }, { 216, 101 }, { 217, 108 }
};

static const StrokeWaypoint JAWA_GUIDE_NGA[] = {
  { 109, 118 }, { 102, 118 }, { 98, 112 }, { 93, 107 }, { 93, 100 }, 
  { 93, 93 }, { 93, 86 }, { 93, 79 }, { 93, 72 }, { 94, 65 }, 
  { 94, 58 }, { 95, 51 }, { 100, 46 }, { 107, 44 }, { 114, 42 }, 
  { 121, 44 }, { 126, 49 }, { 127, 56 }, { 127, 63 }, { 128, 70 }, 
  { 128, 77 }, { 128, 84 }, { 128, 91 }, { 129, 98 }, { 129, 105 }, 
  { 131, 112 }, { 136, 117 }, { 143, 117 }, { 150, 117 }, { 157, 118 }, 
  { 164, 118 }, { 171, 118 }, { 178, 119 }, { 173, 114 }, { 169, 108 }, 
  { 165, 102 }, { 161, 96 }, { 153, 92 }, { 163, 70 }, { 157, 66 }, 
  { 155, 59 }, { 155, 52 }, { 159, 46 }, { 166, 44 }, { 173, 45 }, 
  { 179, 49 }, { 183, 55 }, { 183, 62 }, { 185, 55 }, { 189, 49 }, 
  { 194, 43 }, { 201, 42 }, { 208, 46 }, { 212, 52 }, { 214, 59 }, 
  { 215, 66 }, { 215, 73 }, { 215, 80 }, { 215, 87 }, { 215, 94 }, 
  { 215, 101 }, { 215, 108 }, { 215, 115 }, { 215, 122 }
};

static const StrokeWaypoint JAWA_GUIDE_YA[] = {
  { 84, 118 }, { 78, 112 }, { 76, 102 }, { 76, 86 }, { 76, 70 },
  { 78, 56 }, { 84, 46 }, { 94, 43 }, { 104, 45 }, { 111, 54 },
  { 113, 68 }, { 113, 86 }, { 114, 104 }, { 118, 116 }, { 130, 119 },
  { 144, 118 }, { 154, 110 }, { 156, 94 }, { 156, 76 }, { 157, 58 },
  { 162, 46 }, { 172, 43 }, { 184, 45 }, { 190, 56 }, { 191, 74 },
  { 191, 94 }, { 193, 112 }, { 200, 119 }, { 212, 118 }, { 218, 106 },
  { 220, 88 }, { 220, 68 }, { 222, 50 }, { 228, 43 }, { 236, 48 },
  { 238, 66 }, { 238, 90 }, { 238, 118 }
};

static const StrokeWaypoint JAWA_GUIDE_NYA[] = {
  { 76, 118 }, { 70, 110 }, { 68, 96 }, { 68, 76 }, { 70, 56 },
  { 78, 44 }, { 90, 43 }, { 100, 48 }, { 105, 62 }, { 106, 82 },
  { 106, 102 }, { 108, 118 }, { 112, 92 }, { 116, 64 }, { 124, 46 },
  { 136, 43 }, { 148, 46 }, { 154, 58 }, { 156, 78 }, { 156, 100 },
  { 162, 116 }, { 174, 119 }, { 186, 114 }, { 190, 96 }, { 192, 72 },
  { 196, 50 }, { 206, 43 }, { 218, 45 }, { 224, 58 }, { 226, 80 },
  { 226, 102 }, { 228, 120 }
};

static const StrokeWaypoint JAWA_GUIDE_BA[] = {
  { 84, 118 }, { 78, 110 }, { 75, 96 }, { 75, 76 }, { 77, 56 },
  { 84, 45 }, { 96, 43 }, { 108, 47 }, { 114, 58 }, { 115, 76 },
  { 115, 98 }, { 116, 116 }, { 118, 92 }, { 122, 64 }, { 130, 47 },
  { 142, 43 }, { 154, 47 }, { 160, 60 }, { 161, 80 }, { 162, 102 },
  { 168, 116 }, { 180, 119 }, { 192, 115 }, { 196, 98 }, { 198, 74 },
  { 200, 52 }, { 208, 43 }, { 220, 45 }, { 226, 58 }, { 228, 80 },
  { 228, 102 }, { 228, 120 }
};

static void get_jawa_guide(int index, const StrokeWaypoint*& pts, uint8_t& count) {
  switch (index) {
    case 0:  pts = JAWA_GUIDE_HA;  count = sizeof(JAWA_GUIDE_HA)/sizeof(JAWA_GUIDE_HA[0]); break;
    case 1:  pts = JAWA_GUIDE_NA;  count = sizeof(JAWA_GUIDE_NA)/sizeof(JAWA_GUIDE_NA[0]); break;
    case 2:  pts = JAWA_GUIDE_CA;  count = sizeof(JAWA_GUIDE_CA)/sizeof(JAWA_GUIDE_CA[0]); break;
    case 3:  pts = JAWA_GUIDE_RA;  count = sizeof(JAWA_GUIDE_RA)/sizeof(JAWA_GUIDE_RA[0]); break;
    case 4:  pts = JAWA_GUIDE_KA;  count = sizeof(JAWA_GUIDE_KA)/sizeof(JAWA_GUIDE_KA[0]); break;
    case 5:  pts = JAWA_GUIDE_DA;  count = sizeof(JAWA_GUIDE_DA)/sizeof(JAWA_GUIDE_DA[0]); break;
    case 6:  pts = JAWA_GUIDE_TA;  count = sizeof(JAWA_GUIDE_TA)/sizeof(JAWA_GUIDE_TA[0]); break;
    case 7:  pts = JAWA_GUIDE_SA;  count = sizeof(JAWA_GUIDE_SA)/sizeof(JAWA_GUIDE_SA[0]); break;
    case 8:  pts = JAWA_GUIDE_WA;  count = sizeof(JAWA_GUIDE_WA)/sizeof(JAWA_GUIDE_WA[0]); break;
    case 9:  pts = JAWA_GUIDE_LA;  count = sizeof(JAWA_GUIDE_LA)/sizeof(JAWA_GUIDE_LA[0]); break;
    case 10: pts = JAWA_GUIDE_PA;  count = sizeof(JAWA_GUIDE_PA)/sizeof(JAWA_GUIDE_PA[0]); break;
    case 11: pts = JAWA_GUIDE_DHA; count = sizeof(JAWA_GUIDE_DHA)/sizeof(JAWA_GUIDE_DHA[0]); break;
    case 12: pts = JAWA_GUIDE_JA;  count = sizeof(JAWA_GUIDE_JA)/sizeof(JAWA_GUIDE_JA[0]); break;
    case 13: pts = JAWA_GUIDE_YA;  count = sizeof(JAWA_GUIDE_YA)/sizeof(JAWA_GUIDE_YA[0]); break;
    case 14: pts = JAWA_GUIDE_NYA; count = sizeof(JAWA_GUIDE_NYA)/sizeof(JAWA_GUIDE_NYA[0]); break;
    case 15: pts = JAWA_GUIDE_MA;  count = sizeof(JAWA_GUIDE_MA)/sizeof(JAWA_GUIDE_MA[0]); break;
    case 16: pts = JAWA_GUIDE_GA;  count = sizeof(JAWA_GUIDE_GA)/sizeof(JAWA_GUIDE_GA[0]); break;
    case 17: pts = JAWA_GUIDE_BA;  count = sizeof(JAWA_GUIDE_BA)/sizeof(JAWA_GUIDE_BA[0]); break;
    case 18: pts = JAWA_GUIDE_THA; count = sizeof(JAWA_GUIDE_THA)/sizeof(JAWA_GUIDE_THA[0]); break;
    case 19: pts = JAWA_GUIDE_NGA; count = sizeof(JAWA_GUIDE_NGA)/sizeof(JAWA_GUIDE_NGA[0]); break;
    default: pts = JAWA_GUIDE_DEF; count = sizeof(JAWA_GUIDE_DEF)/sizeof(JAWA_GUIDE_DEF[0]); break;
  }
}

static void update_jawa_write_guide() {
  if (write_watermark_lbl) {
    if (active_write_index >= 0 && active_write_index < 20) {
      lv_label_set_text(write_watermark_lbl, JAWA_AKSARA[active_write_index]);
      lv_obj_align(write_watermark_lbl, LV_ALIGN_CENTER, 0, 29);
    }
  }

  if (!write_guide_track) return;
  const StrokeWaypoint* guide_pts = nullptr;
  uint8_t count = 0;
  get_jawa_guide(active_write_index, guide_pts, count);

  for (int i = 0; i < count && i < 100; i++) {
    jawa_guide_pts[i].x = guide_pts[i].x;
    jawa_guide_pts[i].y = guide_pts[i].y;
  }
  lv_line_set_points(write_guide_track, jawa_guide_pts, count > 100 ? 100 : count);

  // Inisialisasi checkpoints urutan titik (R=20px, Koridor=24px, Batas Gagal Parah=38px)
  ui_trace_set_checkpoints(&jawa_tab_trace, guide_pts, count, 28, 35, 60);
  ui_trace_clear(&jawa_tab_trace);
}

// Deklarasi fungsi navigasi
static void switch_tab(int tab_index);
static void update_path_page();

// ---------- Event Handlers ----------
static void on_back_home_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  ui_home_show();
}

static void on_tab_click(lv_event_t* e) {
  int target_tab = (int)(intptr_t)lv_event_get_user_data(e);
  audio_play(SOUND_CLICK);
  switch_tab(target_tab);
}

static void on_page_up_click(lv_event_t* e) {
  (void)e;
  if (current_page > 0) {
    audio_play(SOUND_PAGE);
    current_page--;
    active_write_index = current_page * 3;
    update_path_page();
  }
}

static void on_page_down_click(lv_event_t* e) {
  (void)e;
  if (current_page < 6) {
    audio_play(SOUND_PAGE);
    current_page++;
    active_write_index = current_page * 3;
    update_path_page();
  }
}

static void on_cycle_write_aksara_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  active_write_index = (active_write_index + 1) % 20;
  switch_tab(1);
}

static void on_node_click(lv_event_t* e) {
  int node_idx = (int)(intptr_t)lv_event_get_user_data(e);
  const PathNodeData& node = JAWA_PAGES[current_page][node_idx];

  char buf[64];
  int active_idx = current_page * 3 + node_idx;
  active_write_index = active_idx;

  if (node.state == 2) {
    audio_play(SOUND_REWARD);
    ui_show_toast("Piala Juara: Selamat! 20 Aksara Carakan Selesai!");
  } else if (node.state == 0) {
    audio_play(SOUND_NODE);
    snprintf(buf, sizeof(buf), "Aksara %s: Latihan Ulang!", node.name);
    ui_show_toast(buf);
    ui_learn_start(AKSARA_JAWA, active_idx);
  } else if (node.state == 1) {
    audio_play(SOUND_NODE);
    snprintf(buf, sizeof(buf), "Level Aktif: Aksara %s!", node.name);
    ui_show_toast(buf);
    ui_learn_start(AKSARA_JAWA, active_idx);
  } else {
    audio_play(SOUND_NODE);
    snprintf(buf, sizeof(buf), "Mulai Belajar Aksara %s!", node.name);
    ui_show_toast(buf);
    ui_learn_start(AKSARA_JAWA, active_idx);
  }

  if (jawa_cbs && jawa_cbs->onSelectAksara) {
    jawa_cbs->onSelectAksara(AKSARA_JAWA, active_idx);
  }
}

static void on_clear_canvas_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  ui_trace_clear(&jawa_tab_trace);
  ui_show_toast("Kanvas dibersihkan");
}

static void on_check_write_click(lv_event_t* e) {
  (void)e;
  StrokeEvalResult eval = ui_eval_stroke_trace(&jawa_tab_trace);
  if (eval.success) {
    if (!jawa_tab_trace.celebration_layer) {
      ui_trace_set_completed_visuals(&jawa_tab_trace);
    }
    audio_play(SOUND_SUCCESS);
    audio_play(SOUND_REWARD);
    if (write_feedback_lbl) {
      lv_label_set_text(write_feedback_lbl, "BENAR!");
      lv_obj_set_style_text_color(write_feedback_lbl, lv_color_hex(0x22C55E), 0);
    }
    ui_show_toast(eval.message);

    // Kirim hasil latihan goresan ke Firebase Cloud Firestore & Serial USB
    int prec = 96;
    if (jawa_tab_trace.checkpoint_count > 0) {
      prec = (jawa_tab_trace.active_checkpoint_idx * 100) / jawa_tab_trace.checkpoint_count;
      if (prec > 100) prec = 100;
      if (prec < 80) prec = 88;
    }
    char aksara_label[48];
    if (active_write_index >= 0 && active_write_index < 20) {
      snprintf(aksara_label, sizeof(aksara_label), "%s (%s)", JAWA_NAMES[active_write_index], JAWA_AKSARA[active_write_index]);
    } else {
      strncpy(aksara_label, "Ca (ꦕ)", sizeof(aksara_label) - 1);
    }
    firebase_sync_send_submission("I Putu Raditya", "5A", "Jawa", aksara_label, prec, 95, "Sempurna");
    if (!firebase_sync_is_connected()) {
      ui_show_toast("Wi-Fi mati: hasil disimpan di Sesi Sementara");
    }
  } else {
    audio_play(SOUND_ERROR);
    if (write_feedback_lbl) {
      lv_label_set_text(write_feedback_lbl, "BELUM SELESAI");
      lv_obj_set_style_text_color(write_feedback_lbl, lv_color_hex(0xEF4444), 0);
    }
    ui_show_toast(eval.message);
  }
}

static void mascot_bounce_anim_cb(void* var, int32_t val) {
  lv_image_set_scale((lv_obj_t*)var, val);
}

static void on_mascot_click(lv_event_t* e) {
  audio_play(SOUND_BUDDY);
  lv_obj_t* target = (lv_obj_t*)lv_event_get_target(e);
  if (target) {
    lv_anim_del(target, mascot_bounce_anim_cb);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, target);
    lv_anim_set_exec_cb(&a, mascot_bounce_anim_cb);
    lv_anim_set_values(&a, 256, 292); // Sedikit membesar (~114%) lalu kembali ke 256 (100%)
    lv_anim_set_duration(&a, 120);
    lv_anim_set_reverse_duration(&a, 130);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
  }

  static int mascot_quote_idx = 0;
  const char* quotes[] = {
    "Buddy: Semangat belajar aksaranya!",
    "Buddy: Kamu hebat, yuk selesaikan babak ini!",
    "Buddy: Setiap goresan bikin kamu makin mahir!",
    "Buddy: Aksara Jawa itu indah banget!"
  };
  ui_show_toast(quotes[mascot_quote_idx % 4]);
  mascot_quote_idx++;
}

// Helper membuat Node 3D ala Duolingo
static lv_obj_t* create_duo_node(lv_obj_t* parent, int x, int y, int size,
                                 lv_color_t color_top, lv_color_t color_dark,
                                 const char* text, const lv_font_t* font,
                                 bool is_active, int node_idx) {
  lv_obj_t* cont = lv_obj_create(parent);
  lv_obj_remove_style_all(cont);
  lv_obj_set_pos(cont, x, y);
  lv_obj_set_size(cont, size, size + 8);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

  // Lapisan dasar 3D
  lv_obj_t* base = ui_make_card(cont, 0, 6, size, size, color_dark, color_dark, LV_RADIUS_CIRCLE);
  lv_obj_clear_flag(base, LV_OBJ_FLAG_CLICKABLE);

  // Lapisan tombol atas
  lv_obj_t* btn = ui_make_card(cont, 0, 0, size, size, color_top, color_top, LV_RADIUS_CIRCLE);
  ui_set_pressed(btn, color_dark, color_dark);
  lv_obj_add_event_cb(btn, on_node_click, LV_EVENT_CLICKED, (void*)(intptr_t)node_idx);

  // Icon atau Label aksara
  lv_obj_t* lbl = ui_make_label(btn, text, font, lv_color_hex(0xFFFFFF));
  lv_obj_center(lbl);

  // Floating pill "MULAI" di atas node aktif
  if (is_active) {
    lv_obj_t* pill = ui_make_card(parent, x - 2, y - 24, size + 4, 22,
                                 lv_color_hex(0x22C55E), lv_color_hex(0x16A34A), 11);
    lv_obj_clear_flag(pill, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t* pillLbl = ui_make_label(pill, "MULAI", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
    lv_obj_center(pillLbl);
  }

  return cont;
}

// Titik penghubung jalur
static void create_trail_dots(lv_obj_t* parent, int x1, int y1, int x2, int y2) {
  for (int step = 1; step <= 2; step++) {
    float t = step / 3.0f;
    int px = (int)(x1 + t * (x2 - x1));
    int py = (int)(y1 + t * (y2 - y1));
    lv_obj_t* dot = ui_make_card(parent, px + 26, py + 26, 10, 10,
                                 lv_color_hex(0x475569), lv_color_hex(0x475569), LV_RADIUS_CIRCLE);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);
  }
}

// Perbarui Konten Halaman Aktif (Tanpa Scroll)
static void update_path_page() {
  if (!path_nodes_cont) return;

  // Hapus elemen halaman sebelumnya
  lv_obj_clean(path_nodes_cont);

  // Update teks indikator halaman
  char ind_buf[16];
  snprintf(ind_buf, sizeof(ind_buf), "%d / 7", current_page + 1);
  lv_label_set_text(lbl_page_ind, ind_buf);

  char sub_buf[32];
  snprintf(sub_buf, sizeof(sub_buf), "Jalur Belajar • Babak %d / 7", current_page + 1);
  if (header_sub) lv_label_set_text(header_sub, sub_buf);

  // Update tombol panah atas/bawah (disable jika di ujung)
  if (current_page == 0) {
    lv_obj_set_style_opa(btn_page_up, LV_OPA_30, 0);
  } else {
    lv_obj_set_style_opa(btn_page_up, LV_OPA_COVER, 0);
  }

  if (current_page == 6) {
    lv_obj_set_style_opa(btn_page_down, LV_OPA_30, 0);
  } else {
    lv_obj_set_style_opa(btn_page_down, LV_OPA_COVER, 0);
  }

  // Koordinat 3 Node berbeda untuk setiap babak (variasi jalur berliku dinamis melengkung kiri/kanan)
  static const int POS[7][3][2] = {
    // Babak 1: Melengkung ke Kiri (Node di X=70..165, Info Card di KANAN X=246)
    { { 165, 16 }, { 70,  82 }, { 155, 148 } },
    // Babak 2: Melengkung ke Kanan (Node di X=185..280, Info Card di KIRI X=14)
    { { 185, 16 }, { 280, 82 }, { 195, 148 } },
    // Babak 3: Jalur Gelombang Kiri
    { { 165, 16 }, { 70,  82 }, { 155, 148 } },
    // Babak 4: Jalur Gelombang Kanan
    { { 185, 16 }, { 280, 82 }, { 195, 148 } },
    // Babak 5: Jalur Gelombang Kiri
    { { 165, 16 }, { 70,  82 }, { 155, 148 } },
    // Babak 6: Jalur Gelombang Kanan
    { { 185, 16 }, { 280, 82 }, { 195, 148 } },
    // Babak 7: Puncak Melengkung ke Kiri
    { { 165, 16 }, { 70,  82 }, { 155, 148 } }
  };

  // Titik jalur penghubung (S-curve sesuai babak)
  create_trail_dots(path_nodes_cont, POS[current_page][0][0], POS[current_page][0][1], POS[current_page][1][0], POS[current_page][1][1]);
  create_trail_dots(path_nodes_cont, POS[current_page][1][0], POS[current_page][1][1], POS[current_page][2][0], POS[current_page][2][1]);

  // Gambar 3 Node halaman ini dengan posisi khusus babak tersebut
  for (int i = 0; i < 3; i++) {
    const PathNodeData& node = JAWA_PAGES[current_page][i];
    lv_color_t c_top, c_dark;
    const char* txt = node.symbol;
    const lv_font_t* f = &font_jawa_22;

    if (node.state == 0) {
      // Selesai (Centang biru)
      c_top = lv_color_hex(0x0284C7);
      c_dark = lv_color_hex(0x0369A1);
    } else if (node.state == 1) {
      // Sedang Aktif (Ungu Jawa)
      c_top = C_JAWA_1;
      c_dark = C_JAWA_2;
    } else if (node.state == 2) {
      // Peti Hadiah / Piala Juara
      c_top = lv_color_hex(0xF59E0B);
      c_dark = lv_color_hex(0xB45309);
      f = &lv_font_montserrat_14;
      txt = "Win";
    } else {
      // Terkunci
      c_top = lv_color_hex(0x334155);
      c_dark = lv_color_hex(0x1E293B);
    }

    create_duo_node(path_nodes_cont, POS[current_page][i][0], POS[current_page][i][1], 62,
                    c_top, c_dark, txt, f, (node.state == 1), i);
  }

  // Info Card & Maskot menyesuaikan ruang kosong agar tidak tabrakan dengan node:
  bool is_right_curve = (current_page % 2 == 1);
  int card_x = is_right_curve ? 14 : 244;
  int card_y = 10;
  lv_obj_t* infoCard = ui_make_card(path_nodes_cont, card_x, card_y, 146, 70, C_CARD, C_CARD, 12);
  lv_obj_clear_flag(infoCard, LV_OBJ_FLAG_CLICKABLE);

  struct BabakMeta {
    const char* title;
    const char* letters;
    const char* desc;
    lv_color_t color;
  };

  static const BabakMeta JAWA_META[7] = {
    { "Babak 1: Dasar", "Ha • Na • Ca", "Level 1 - 3", C_JAWA_1 },
    { "Babak 2: Carakan", "Ra • Ka • Da", "Level 4 - 6", lv_color_hex(0x10B981) },
    { "Babak 3: Lanjutan", "Ta • Sa • Wa", "Level 7 - 9", lv_color_hex(0x38BDF8) },
    { "Babak 4: Menengah", "La • Pa • Dha", "Level 10 - 12", lv_color_hex(0xA855F7) },
    { "Babak 5: Terampil", "Ja • Ya • Nya", "Level 13 - 15", lv_color_hex(0xEC4899) },
    { "Babak 6: Mahir", "Ma • Ga • Ba", "Level 16 - 18", lv_color_hex(0xF59E0B) },
    { "Babak 7: Puncak", "Tha • Nga • Juara", "Tamat 20 Aksara!", lv_color_hex(0x22C55E) }
  };

  const BabakMeta& meta = JAWA_META[current_page];
  lv_obj_t* t1 = ui_make_label(infoCard, meta.title, &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(t1, 10, 6);
  lv_obj_t* t2 = ui_make_label(infoCard, meta.letters, &lv_font_montserrat_14, meta.color);
  lv_obj_set_pos(t2, 10, 26);
  lv_obj_t* t3 = ui_make_label(infoCard, meta.desc, &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(t3, 10, 46);

  // Variasi Maskot Buddy untuk setiap babak
  const lv_image_dsc_t* buddy_src = (current_page % 3 == 0) ? &BuddyMaskot : ((current_page % 3 == 1) ? &Buddy3 : &Buddy4);
  int mascot_x = is_right_curve ? 36 : 268;
  int mascot_y = 86;

  lv_obj_t* mascot = lv_image_create(path_nodes_cont);
  lv_image_set_src(mascot, buddy_src);
  lv_image_set_pivot(mascot, buddy_src->header.w / 2, buddy_src->header.h / 2);
  lv_obj_set_pos(mascot, mascot_x, mascot_y);
  lv_obj_add_flag(mascot, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(mascot, on_mascot_click, LV_EVENT_CLICKED, NULL);
}

// ---------- 1. Tampilan Jalur Belajar Berhalaman (Paging) ----------
static void build_path_view(lv_obj_t* parent) {
  view_path = lv_obj_create(parent);
  lv_obj_remove_style_all(view_path);
  lv_obj_set_pos(view_path, 0, 0);
  lv_obj_set_size(view_path, 480, 224);
  lv_obj_clear_flag(view_path, LV_OBJ_FLAG_SCROLLABLE); // Tanpa scroll = Anti nge-lag
  lv_obj_set_style_bg_color(view_path, C_BG, 0);
  lv_obj_set_style_bg_opa(view_path, LV_OPA_COVER, 0);

  // Area Node Aksara (Kiri & Tengah)
  path_nodes_cont = lv_obj_create(view_path);
  lv_obj_remove_style_all(path_nodes_cont);
  lv_obj_set_pos(path_nodes_cont, 0, 0);
  lv_obj_set_size(path_nodes_cont, 396, 224);
  lv_obj_clear_flag(path_nodes_cont, LV_OBJ_FLAG_SCROLLABLE);

  // Panel Tombol Navigasi Panah Atas & Bawah (Sisi Kanan Layar)
  lv_obj_t* nav_side = ui_make_card(view_path, 400, 8, 70, 208, C_CARD, C_CARD, 16);
  lv_obj_clear_flag(nav_side, LV_OBJ_FLAG_SCROLLABLE);

  // Tombol Panah Atas (▲)
  btn_page_up = ui_make_card(nav_side, 8, 10, 54, 52, C_JAWA_1, C_JAWA_2, 14);
  ui_set_pressed(btn_page_up, C_JAWA_2, C_JAWA_2);
  lv_obj_add_event_cb(btn_page_up, on_page_up_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* upIc = ui_make_label(btn_page_up, LV_SYMBOL_UP, &lv_font_montserrat_20, lv_color_hex(0xFFFFFF));
  lv_obj_center(upIc);

  // Indikator Halaman (Contoh: "1 / 7")
  lv_obj_t* ind_box = ui_make_card(nav_side, 6, 80, 58, 36, C_CARD_PR, C_CARD_PR, 10);
  lv_obj_clear_flag(ind_box, LV_OBJ_FLAG_CLICKABLE);
  lbl_page_ind = ui_make_label(ind_box, "1 / 7", &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(lbl_page_ind);

  // Tombol Panah Bawah (▼)
  btn_page_down = ui_make_card(nav_side, 8, 134, 54, 52, C_JAWA_1, C_JAWA_2, 14);
  ui_set_pressed(btn_page_down, C_JAWA_2, C_JAWA_2);
  lv_obj_add_event_cb(btn_page_down, on_page_down_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* downIc = ui_make_label(btn_page_down, LV_SYMBOL_DOWN, &lv_font_montserrat_20, lv_color_hex(0xFFFFFF));
  lv_obj_center(downIc);

  // Gambar konten halaman pertama
  update_path_page();
}

// ---------- 2. Tampilan Nulis Aksara (Writing Canvas) ----------
static void build_write_view(lv_obj_t* parent) {
  view_write = lv_obj_create(parent);
  lv_obj_remove_style_all(view_write);
  lv_obj_set_pos(view_write, 0, 0);
  lv_obj_set_size(view_write, 480, 224);
  lv_obj_clear_flag(view_write, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(view_write, LV_OBJ_FLAG_HIDDEN);

  // Kartu Target Aksara (Kiri)
  lv_obj_t* cardTarget = ui_make_card(view_write, 12, 8, 140, 208, C_CARD, C_CARD, 16);
  write_target_title = ui_make_label(cardTarget, "Aksara Ca", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(write_target_title, 14, 14);

  write_target_sub = ui_make_label(cardTarget, "Goresan 1 dari 3\nBunyi: /ca/", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(write_target_sub, 14, 40);

  // Preview Kotak Huruf (Ketuk untuk ganti aksara langsung di tab Nulis)
  lv_obj_t* boxHuruf = ui_make_card(cardTarget, 14, 84, 112, 110, C_JAWA_1, C_JAWA_2, 14);
  ui_set_pressed(boxHuruf, C_JAWA_2, C_JAWA_2);
  lv_obj_add_event_cb(boxHuruf, on_cycle_write_aksara_click, LV_EVENT_CLICKED, NULL);
  write_preview_lbl = ui_make_label(boxHuruf, "Ca", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF));
  lv_obj_center(write_preview_lbl);

  // Area Kanvas Tracing ala Contoh Gambar User (308x160 px)
  lv_obj_t* canvasCard = ui_make_card(view_write, 160, 8, 308, 160, lv_color_hex(0x0A0F1D), lv_color_hex(0x0A0F1D), 16);
  lv_obj_set_style_border_color(canvasCard, C_JAWA_1, 0);
  lv_obj_set_style_border_width(canvasCard, 2, 0);

  // Watermark Glif Aksara Asli Transparan (Background Guide 160px JUMBO)
  write_watermark_lbl = ui_make_label(canvasCard, "\xEA\xA6\x95", &font_jawa_160, lv_color_hex(0x38BDF8));
  lv_obj_set_style_text_opa(write_watermark_lbl, LV_OPA_30, 0); // 30% opasitas lembut transparan
  lv_obj_align(write_watermark_lbl, LV_ALIGN_CENTER, 0, 29);    // Offset +29px agar tengah visual pas di Y=80
  lv_obj_clear_flag(write_watermark_lbl, LV_OBJ_FLAG_CLICKABLE);

  // Label Feedback di pojok kanan atas (seperti "BENAR!" pada gambar contoh user)
  write_feedback_lbl = ui_make_label(canvasCard, "Mulai di P1 -> Pn", &lv_font_montserrat_14, lv_color_hex(0x38BDF8));
  lv_obj_align(write_feedback_lbl, LV_ALIGN_TOP_RIGHT, -12, 8);

  // Jalur Biru Tipis & Halus (Line Track)
  write_guide_track = lv_line_create(canvasCard);
  lv_obj_clear_flag(write_guide_track, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_line_color(write_guide_track, lv_color_hex(0x2563EB), 0); // Biru panduan
  lv_obj_set_style_line_width(write_guide_track, 4, 0);                      // Tebal 4px rapi (tidak menutupi huruf)
  lv_obj_set_style_line_opa(write_guide_track, LV_OPA_60, 0);                // 60% opasitas
  lv_obj_set_style_line_rounded(write_guide_track, true, 0);

  ui_trace_attach(&jawa_tab_trace, canvasCard);
  jawa_tab_trace.feedback_lbl = write_feedback_lbl;
  jawa_tab_trace.watermark_lbl = write_watermark_lbl;
  jawa_tab_trace.guide_track = write_guide_track;

  // Tombol Bersihkan & Periksa
  lv_obj_t* btnClear = ui_make_card(view_write, 160, 174, 146, 42, C_CARD, C_CARD, 12);
  ui_set_pressed(btnClear, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btnClear, on_clear_canvas_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* btnClearLbl = ui_make_label(btnClear, LV_SYMBOL_TRASH "  Bersihkan", &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(btnClearLbl);

  lv_obj_t* btnCheck = ui_make_card(view_write, 316, 174, 152, 42, C_JAWA_1, C_JAWA_2, 12);
  ui_set_pressed(btnCheck, C_JAWA_2, C_JAWA_2);
  lv_obj_add_event_cb(btnCheck, on_check_write_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* btnCheckLbl = ui_make_label(btnCheck, LV_SYMBOL_OK "  Cek Goresan", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(btnCheckLbl);
}

static void on_open_device_settings(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  ui_settings_show();
}

// ---------- 3. Tampilan Setelan Aksara Jawa ----------
static void build_settings_view(lv_obj_t* parent) {
  view_settings = lv_obj_create(parent);
  lv_obj_remove_style_all(view_settings);
  lv_obj_set_pos(view_settings, 0, 0);
  lv_obj_set_size(view_settings, 480, 224);
  lv_obj_clear_flag(view_settings, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(view_settings, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t* cardSet = ui_make_card(view_settings, 16, 8, 448, 208, C_CARD, C_CARD, 16);

  lv_obj_t* sTitle = ui_make_label(cardSet, "Setelan Pembelajaran Jawa", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(sTitle, 16, 12);

  lv_obj_t* btnDevSet = ui_make_card(cardSet, 300, 10, 132, 28, C_JAWA_1, C_JAWA_2, 8);
  ui_set_pressed(btnDevSet, C_JAWA_2, C_JAWA_2);
  lv_obj_add_event_cb(btnDevSet, on_open_device_settings, LV_EVENT_CLICKED, NULL);
  lv_obj_t* devSetLbl = ui_make_label(btnDevSet, LV_SYMBOL_WIFI " Wi-Fi & Daya", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(devSetLbl);

  // Baris 1: Panduan Goresan
  lv_obj_t* row1 = ui_make_card(cardSet, 16, 46, 416, 44, C_CARD_PR, C_CARD_PR, 10);
  lv_obj_t* r1Lbl = ui_make_label(row1, "Tampilkan Garis Panduan Tipis", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(r1Lbl, 14, 12);
  lv_obj_t* sw1 = lv_switch_create(row1);
  lv_obj_set_pos(sw1, 350, 8);
  lv_obj_add_state(sw1, LV_STATE_CHECKED);

  // Baris 2: Efek Suara
  lv_obj_t* row2 = ui_make_card(cardSet, 16, 96, 416, 44, C_CARD_PR, C_CARD_PR, 10);
  lv_obj_t* r2Lbl = ui_make_label(row2, "Audio Pelafalan Aksara Otomatis", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(r2Lbl, 14, 12);
  lv_obj_t* sw2 = lv_switch_create(row2);
  lv_obj_set_pos(sw2, 350, 8);
  lv_obj_add_state(sw2, LV_STATE_CHECKED);

  // Tombol Reset Kemajuan
  lv_obj_t* btnReset = ui_make_card(cardSet, 16, 150, 416, 42, lv_color_hex(0x3B151E), lv_color_hex(0x270B13), 10);
  ui_set_pressed(btnReset, lv_color_hex(0x4C1D27), lv_color_hex(0x4C1D27));
  lv_obj_t* rstLbl = ui_make_label(btnReset, LV_SYMBOL_REFRESH "  Reset Ulang Kemajuan Belajar Jawa",
                                   &lv_font_montserrat_14, lv_color_hex(0xFB7185));
  lv_obj_center(rstLbl);
}

// Navigasi Pergantian Tab Bawah (Belajar / Nulis / Setelan)
static void switch_tab(int tab_index) {
  current_tab = tab_index;

  // Sembunyikan semua view
  lv_obj_add_flag(view_path, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(view_write, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(view_settings, LV_OBJ_FLAG_HIDDEN);

  // Reset warna tombol tab bawah
  lv_obj_set_style_bg_color(tab_btn_learn, C_CARD, 0);
  lv_obj_set_style_bg_color(tab_btn_write, C_CARD, 0);
  lv_obj_set_style_bg_color(tab_btn_settings, C_CARD, 0);
  lv_obj_set_style_text_color(tab_lbl_learn, C_SUB, 0);
  lv_obj_set_style_text_color(tab_lbl_write, C_SUB, 0);
  lv_obj_set_style_text_color(tab_lbl_settings, C_SUB, 0);

  if (tab_index != 1) {
    // Bebaskan memori objek garis coretan & efek selebrasi saat tidak di tab Nulis
    ui_trace_clear(&jawa_tab_trace);
  }

  if (tab_index == 0) {
    // Tab Belajar
    lv_obj_clear_flag(view_path, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(tab_btn_learn, C_CARD_PR, 0);
    lv_obj_set_style_text_color(tab_lbl_learn, C_JAWA_1, 0);
  } else if (tab_index == 1) {
    // Tab Nulis Aksara
    lv_obj_clear_flag(view_write, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(tab_btn_write, C_CARD_PR, 0);
    lv_obj_set_style_text_color(tab_lbl_write, C_JAWA_1, 0);

    // Update target aksara sesuai pilihan aktif
    if (active_write_index >= 0 && active_write_index < 21) {
      char b[32];
      snprintf(b, sizeof(b), "Aksara %s", JAWA_NAMES[active_write_index]);
      lv_label_set_text(write_target_title, b);
      if (write_target_sub) {
        char sub_b[48];
        snprintf(sub_b, sizeof(sub_b), "Carakan Jawa\nBunyi: /%s/", JAWA_NAMES[active_write_index]);
        lv_label_set_text(write_target_sub, sub_b);
      }
      lv_obj_set_style_text_font(write_preview_lbl, (active_write_index == 20) ? &lv_font_montserrat_20 : &font_jawa_38, 0);
      lv_label_set_text(write_preview_lbl, JAWA_AKSARA[active_write_index]);
      update_jawa_write_guide();
    }
  } else {
    // Tab Setelan
    lv_obj_clear_flag(view_settings, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(tab_btn_settings, C_CARD_PR, 0);
    lv_obj_set_style_text_color(tab_lbl_settings, C_JAWA_1, 0);
  }
}

// Bangun Layar Aksara Jawa secara Lazy (hemat RAM heap saat belum dibuka)
static void build_jawa_screen() {
  if (scr_jawa) return;

  scr_jawa = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_jawa, C_BG, 0);
  lv_obj_set_style_bg_opa(scr_jawa, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr_jawa, LV_OBJ_FLAG_SCROLLABLE);

  // ===== TOP HEADER BAR =====
  lv_obj_t* header = ui_make_card(scr_jawa, 0, 0, 480, 44, C_BG, C_BG, 0);

  // Tombol Kembali (< Back)
  lv_obj_t* btnBack = ui_make_card(header, 10, 5, 40, 34, C_CARD, C_CARD, 10);
  ui_set_pressed(btnBack, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btnBack, on_back_home_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* backIc = ui_make_label(btnBack, LV_SYMBOL_LEFT, &lv_font_montserrat_20, C_TEXT);
  lv_obj_center(backIc);

  // Judul & Subjudul
  lv_obj_t* title = ui_make_label(header, "Aksara Jawa", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 58, 3);

  header_sub = ui_make_label(header, "Jalur Belajar • Babak 1 / 3", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(header_sub, 58, 23);

  // Badge Status XP Kanan
  lv_obj_t* badgeXp = ui_make_card(header, 366, 7, 104, 30, lv_color_hex(0x2E1065), lv_color_hex(0x1E1B4B), 15);
  lv_obj_t* badgeLbl = ui_make_label(badgeXp, "120 XP", &lv_font_montserrat_14, lv_color_hex(0xC084FC));
  lv_obj_center(badgeLbl);

  // ===== KONTEN UTAMA (Tengah) =====
  lv_obj_t* content_area = lv_obj_create(scr_jawa);
  lv_obj_remove_style_all(content_area);
  lv_obj_set_pos(content_area, 0, 44);
  lv_obj_set_size(content_area, 480, 226);
  lv_obj_clear_flag(content_area, LV_OBJ_FLAG_SCROLLABLE);

  build_path_view(content_area);
  build_write_view(content_area);
  build_settings_view(content_area);

  // ===== BOTTOM NAVIGATION BAR (3 OPSI) =====
  lv_obj_t* botNav = ui_make_card(scr_jawa, 0, 270, 480, 50, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(botNav, lv_color_hex(0x232938), 0);
  lv_obj_set_style_border_width(botNav, 1, 0);

  // Opsi 1: Belajar
  tab_btn_learn = ui_make_card(botNav, 4, 3, 154, 44, C_CARD_PR, C_CARD_PR, 12);
  lv_obj_add_event_cb(tab_btn_learn, on_tab_click, LV_EVENT_CLICKED, (void*)(intptr_t)0);
  tab_lbl_learn = ui_make_label(tab_btn_learn, LV_SYMBOL_HOME "  Belajar", &lv_font_montserrat_14, C_JAWA_1);
  lv_obj_center(tab_lbl_learn);

  // Opsi 2: Nulis Aksara
  tab_btn_write = ui_make_card(botNav, 162, 3, 156, 44, C_CARD, C_CARD, 12);
  lv_obj_add_event_cb(tab_btn_write, on_tab_click, LV_EVENT_CLICKED, (void*)(intptr_t)1);
  tab_lbl_write = ui_make_label(tab_btn_write, LV_SYMBOL_EDIT "  Nulis Aksara", &lv_font_montserrat_14, C_SUB);
  lv_obj_center(tab_lbl_write);

  // Opsi 3: Setelan
  tab_btn_settings = ui_make_card(botNav, 322, 3, 154, 44, C_CARD, C_CARD, 12);
  lv_obj_add_event_cb(tab_btn_settings, on_tab_click, LV_EVENT_CLICKED, (void*)(intptr_t)2);
  tab_lbl_settings = ui_make_label(tab_btn_settings, LV_SYMBOL_SETTINGS "  Setelan", &lv_font_montserrat_14, C_SUB);
  lv_obj_center(tab_lbl_settings);
}

void ui_jawa_init(const AppCallbacks* callbacks) {
  jawa_cbs = callbacks;
}

void ui_jawa_show() {
  if (!scr_jawa) {
    build_jawa_screen();
  }
  if (scr_jawa) {
    switch_tab(0); // Buka tab Belajar secara default
    if (lv_screen_active() != scr_jawa) {
      lv_scr_load_anim(scr_jawa, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
    }
  }
}
