#include "ui_bali.h"
#include "ui_home.h"
#include "ui_learn.h"
#include "firebase_sync.h"
#include <stdio.h>

static const AppCallbacks* bali_cbs = nullptr;
static lv_obj_t* scr_bali = nullptr;

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

// Aksara Bali Data (Wreastra - 18 Konsonan Lengkap)
struct PathNodeData {
  const char* name;
  const char* symbol;
  int state; // 0 = Selesai, 1 = Aktif, 2 = Peti/Bonus, 3 = Terkunci
};

// 6 Halaman, masing-masing 3 node (total 18 node Wreastra lengkap)
static const PathNodeData BALI_PAGES[6][3] = {
  // Babak 1
  {
    { "Ha", "\xE1\xAC\xB3", 0 },
    { "Na", "\xE1\xAC\xA6", 0 },
    { "Ca", "\xE1\xAC\x98", 1 }
  },
  // Babak 2
  {
    { "Ra", "\xE1\xAC\xAD", 3 },
    { "Ka", "\xE1\xAC\x93", 3 },
    { "Da", "\xE1\xAC\xA4", 3 }
  },
  // Babak 3
  {
    { "Ta", "\xE1\xAC\xA2", 3 },
    { "Sa", "\xE1\xAC\xB2", 3 },
    { "Wa", "\xE1\xAC\xAF", 3 }
  },
  // Babak 4
  {
    { "La", "\xE1\xAC\xAE", 3 },
    { "Ma", "\xE1\xAC\xAB", 3 },
    { "Ga", "\xE1\xAC\x95", 3 }
  },
  // Babak 5
  {
    { "Ba", "\xE1\xAC\xA9", 3 },
    { "Nga", "\xE1\xAC\x97", 3 },
    { "Pa", "\xE1\xAC\xA7", 3 }
  },
  // Babak 6
  {
    { "Ja", "\xE1\xAC\x9A", 3 },
    { "Ya", "\xE1\xAC\xAC", 3 },
    { "Nya", "\xE1\xAC\x9C", 3 }
  }
};

// Canvas Nulis Aksara Bali
static lv_obj_t* write_target_title = nullptr;
static lv_obj_t* write_target_sub = nullptr;
static lv_obj_t* write_watermark_lbl = nullptr;
static lv_obj_t* write_guide_track = nullptr;
static lv_obj_t* write_feedback_lbl = nullptr;
static lv_obj_t* write_preview_lbl = nullptr;
static int active_write_index = 2; // Default ke aksara "Ca"
static TraceState bali_tab_trace;
static lv_point_precise_t bali_guide_pts[100];

// 18 Aksara Bali Wreastra Lengkap
static const char* BALI_NAMES[18] = {
  "Ha", "Na", "Ca", "Ra", "Ka", "Da",
  "Ta", "Sa", "Wa", "La", "Ma", "Ga",
  "Ba", "Nga", "Pa", "Ja", "Ya", "Nya"
};
static const char* BALI_AKSARA[18] = {
  "\xE1\xAC\xB3", "\xE1\xAC\xA6", "\xE1\xAC\x98", "\xE1\xAC\xAD", "\xE1\xAC\x93", "\xE1\xAC\xA4",
  "\xE1\xAC\xA2", "\xE1\xAC\xB2", "\xE1\xAC\xAF", "\xE1\xAC\xAE", "\xE1\xAC\xAB", "\xE1\xAC\x95",
  "\xE1\xAC\xA9", "\xE1\xAC\x97", "\xE1\xAC\xA7", "\xE1\xAC\x9A", "\xE1\xAC\xAC", "\xE1\xAC\x9C"
};

// Waypoint Jalur Aksara Bali (Tersinkronisasi Presisi dengan Font Asli 160px di Tengah Kanvas 308x160)
static const StrokeWaypoint BALI_GUIDE_HA[] = {
  { 81, 69 }, { 82, 62 }, { 85, 55 }, { 89, 49 }, { 95, 45 }, 
  { 102, 42 }, { 109, 43 }, { 106, 50 }, { 103, 57 }, { 101, 64 }, 
  { 99, 71 }, { 97, 78 }, { 97, 85 }, { 96, 92 }, { 96, 99 }, 
  { 97, 106 }, { 101, 112 }, { 107, 116 }, { 113, 120 }, { 120, 121 }, 
  { 127, 120 }, { 133, 116 }, { 140, 113 }, { 144, 107 }, { 145, 100 }, 
  { 145, 93 }, { 145, 86 }, { 145, 79 }, { 144, 71 }, { 144, 64 }, 
  { 144, 57 }, { 146, 50 }, { 152, 45 }, { 159, 42 }, { 166, 41 }, 
  { 173, 44 }, { 180, 48 }, { 185, 53 }, { 189, 59 }, { 192, 66 }, 
  { 193, 73 }, { 194, 80 }, { 195, 87 }, { 195, 94 }, { 195, 101 }, 
  { 195, 108 }, { 195, 115 }, { 194, 122 }, { 194, 115 }, { 195, 108 }, 
  { 196, 101 }, { 196, 94 }, { 196, 87 }, { 195, 80 }, { 194, 73 }, 
  { 194, 66 }, { 194, 59 }, { 195, 52 }, { 195, 45 }, { 202, 43 }, 
  { 209, 45 }, { 216, 47 }, { 222, 51 }, { 225, 58 }, { 225, 65 }, 
  { 225, 72 }, { 225, 79 }, { 225, 86 }, { 225, 93 }, { 225, 100 }, 
  { 226, 107 }, { 227, 114 }, { 227, 121 }
};
static const StrokeWaypoint BALI_GUIDE_NA[] = {
  { 94, 80 }, { 92, 73 }, { 89, 66 }, { 89, 59 }, { 92, 52 }, 
  { 96, 46 }, { 103, 43 }, { 110, 43 }, { 117, 45 }, { 121, 51 }, 
  { 121, 58 }, { 121, 65 }, { 121, 72 }, { 121, 79 }, { 123, 86 }, 
  { 123, 93 }, { 123, 100 }, { 122, 107 }, { 121, 114 }, { 117, 120 }, 
  { 110, 119 }, { 103, 117 }, { 100, 110 }, { 100, 103 }, { 105, 98 }, 
  { 112, 94 }, { 119, 93 }, { 126, 92 }, { 133, 91 }, { 140, 91 }, 
  { 147, 91 }, { 154, 91 }, { 161, 93 }, { 168, 94 }, { 175, 97 }, 
  { 181, 101 }, { 185, 107 }, { 185, 114 }, { 180, 119 }, { 173, 119 }, 
  { 166, 118 }, { 159, 116 }, { 154, 111 }, { 151, 104 }, { 147, 98 }, 
  { 147, 88 }, { 146, 81 }, { 147, 74 }, { 149, 67 }, { 151, 60 }, 
  { 154, 53 }, { 160, 49 }, { 165, 44 }, { 173, 43 }, { 180, 43 }, 
  { 187, 42 }, { 194, 43 }, { 200, 47 }, { 205, 52 }, { 211, 56 }, 
  { 214, 63 }, { 215, 70 }, { 215, 77 }, { 217, 84 }, { 219, 91 }, 
  { 219, 98 }, { 218, 105 }, { 218, 112 }, { 219, 119 }
};
static const StrokeWaypoint BALI_GUIDE_CA[] = {
  { 92, 80 }, { 90, 64 }, { 90, 49 }, { 101, 38 }, { 116, 44 }, 
  { 123, 60 }, { 125, 76 }, { 125, 91 }, { 127, 107 }, { 118, 120 }, 
  { 103, 122 }, { 98, 107 }, { 107, 93 }, { 123, 96 }, { 136, 104 }, 
  { 147, 116 }, { 163, 118 }, { 178, 118 }, { 190, 107 }, { 192, 91 }, 
  { 183, 78 }, { 170, 67 }, { 163, 51 }, { 178, 47 }, { 194, 49 }, 
  { 210, 56 }, { 216, 71 }, { 218, 87 }, { 218, 102 }, { 218, 118 }
};
static const StrokeWaypoint BALI_GUIDE_RA[] = {
  { 98, 55 }, { 103, 49 }, { 107, 43 }, { 114, 41 }, { 121, 41 }, 
  { 127, 45 }, { 132, 50 }, { 133, 57 }, { 133, 64 }, { 133, 71 }, 
  { 127, 79 }, { 123, 87 }, { 117, 93 }, { 111, 101 }, { 105, 107 }, 
  { 98, 111 }, { 105, 111 }, { 112, 114 }, { 119, 115 }, { 126, 117 }, 
  { 134, 118 }, { 141, 119 }, { 147, 115 }, { 152, 110 }, { 157, 104 }, 
  { 154, 97 }, { 154, 90 }, { 152, 83 }, { 147, 77 }, { 143, 71 }, 
  { 141, 64 }, { 139, 57 }, { 143, 51 }, { 150, 47 }, { 157, 44 }, 
  { 164, 42 }, { 172, 40 }, { 179, 40 }, { 186, 42 }, { 192, 46 }, 
  { 198, 51 }, { 203, 56 }, { 205, 63 }, { 207, 70 }, { 208, 78 }, 
  { 210, 85 }, { 207, 92 }, { 208, 99 }, { 208, 107 }, { 208, 116 }, 
  { 208, 123 }
};
static const StrokeWaypoint BALI_GUIDE_KA[] = {
  { 78, 80 }, { 75, 73 }, { 74, 66 }, { 73, 59 }, { 75, 52 }, 
  { 79, 46 }, { 86, 43 }, { 93, 43 }, { 100, 43 }, { 103, 50 }, 
  { 104, 57 }, { 105, 64 }, { 105, 71 }, { 107, 78 }, { 105, 85 }, 
  { 106, 92 }, { 106, 99 }, { 106, 108 }, { 103, 117 }, { 96, 120 }, 
  { 89, 116 }, { 84, 111 }, { 84, 104 }, { 87, 97 }, { 94, 92 }, 
  { 102, 91 }, { 109, 91 }, { 119, 90 }, { 126, 90 }, { 133, 90 }, 
  { 141, 90 }, { 149, 93 }, { 156, 96 }, { 161, 101 }, { 164, 109 }, 
  { 162, 117 }, { 155, 117 }, { 146, 116 }, { 141, 111 }, { 135, 106 }, 
  { 130, 99 }, { 130, 92 }, { 128, 85 }, { 129, 76 }, { 131, 67 }, 
  { 132, 59 }, { 137, 53 }, { 143, 47 }, { 149, 43 }, { 157, 42 }, 
  { 165, 41 }, { 173, 42 }, { 182, 45 }, { 189, 50 }, { 194, 56 }, 
  { 198, 63 }, { 201, 70 }, { 203, 77 }, { 204, 84 }, { 204, 91 }, 
  { 204, 98 }, { 205, 105 }, { 204, 112 }, { 204, 119 }, { 203, 112 }, 
  { 204, 105 }, { 204, 98 }, { 202, 91 }, { 202, 84 }, { 202, 77 }, 
  { 201, 70 }, { 200, 63 }, { 201, 56 }, { 203, 49 }, { 204, 42 }, 
  { 211, 41 }, { 218, 42 }, { 224, 46 }, { 231, 49 }, { 233, 56 }, 
  { 235, 63 }, { 234, 70 }, { 233, 77 }, { 233, 84 }, { 234, 91 }, 
  { 233, 98 }, { 234, 105 }, { 236, 112 }, { 236, 119 }
};
static const StrokeWaypoint BALI_GUIDE_DA[] = {
  { 91, 68 }, { 93, 61 }, { 93, 54 }, { 97, 48 }, { 103, 44 }, 
  { 110, 42 }, { 117, 41 }, { 124, 43 }, { 129, 48 }, { 124, 54 }, 
  { 120, 60 }, { 116, 67 }, { 112, 74 }, { 109, 81 }, { 105, 87 }, 
  { 101, 93 }, { 99, 100 }, { 100, 107 }, { 104, 113 }, { 110, 117 }, 
  { 117, 115 }, { 124, 111 }, { 129, 106 }, { 135, 102 }, { 142, 99 }, 
  { 148, 95 }, { 155, 94 }, { 162, 93 }, { 169, 94 }, { 172, 101 }, 
  { 171, 108 }, { 169, 115 }, { 162, 117 }, { 155, 115 }, { 150, 110 }, 
  { 146, 104 }, { 145, 97 }, { 141, 91 }, { 139, 83 }, { 142, 74 }, 
  { 143, 67 }, { 146, 60 }, { 150, 53 }, { 157, 47 }, { 165, 43 }, 
  { 172, 41 }, { 179, 41 }, { 186, 41 }, { 193, 43 }, { 199, 47 }, 
  { 204, 53 }, { 209, 58 }, { 212, 65 }, { 214, 72 }, { 214, 79 }, 
  { 214, 86 }, { 212, 93 }, { 213, 100 }, { 214, 107 }, { 214, 114 }, 
  { 215, 121 }
};
static const StrokeWaypoint BALI_GUIDE_TA[] = {
  { 82, 69 }, { 83, 62 }, { 84, 55 }, { 88, 49 }, { 93, 43 }, 
  { 100, 42 }, { 107, 42 }, { 114, 43 }, { 111, 50 }, { 107, 56 }, 
  { 105, 63 }, { 104, 70 }, { 102, 77 }, { 99, 84 }, { 99, 91 }, 
  { 100, 98 }, { 101, 105 }, { 103, 112 }, { 109, 116 }, { 116, 119 }, 
  { 123, 119 }, { 130, 118 }, { 137, 118 }, { 144, 118 }, { 151, 118 }, 
  { 157, 114 }, { 161, 108 }, { 161, 101 }, { 160, 94 }, { 157, 87 }, 
  { 152, 82 }, { 145, 79 }, { 138, 76 }, { 132, 72 }, { 132, 65 }, 
  { 134, 58 }, { 139, 53 }, { 144, 48 }, { 150, 44 }, { 157, 43 }, 
  { 164, 45 }, { 171, 47 }, { 178, 49 }, { 182, 55 }, { 186, 61 }, 
  { 189, 68 }, { 190, 75 }, { 191, 82 }, { 192, 89 }, { 191, 96 }, 
  { 191, 103 }, { 191, 110 }, { 193, 117 }, { 191, 110 }, { 191, 103 }, 
  { 192, 96 }, { 192, 89 }, { 192, 82 }, { 192, 75 }, { 191, 68 }, 
  { 190, 61 }, { 191, 54 }, { 192, 47 }, { 197, 42 }, { 204, 44 }, 
  { 211, 45 }, { 218, 48 }, { 221, 55 }, { 221, 62 }, { 221, 69 }, 
  { 221, 76 }, { 223, 83 }, { 222, 90 }, { 222, 97 }, { 223, 104 }, 
  { 224, 111 }, { 224, 118 }
};

static const StrokeWaypoint BALI_GUIDE_DEF[] = {
  { 104, 115 }, { 120, 52 }, { 148, 100 }, { 178, 52 }, { 204, 115 }
};

static const StrokeWaypoint BALI_GUIDE_SA[] = {
  { 95, 78 }, { 91, 72 }, { 91, 65 }, { 91, 58 }, { 91, 51 }, 
  { 97, 47 }, { 103, 43 }, { 110, 43 }, { 117, 44 }, { 121, 50 }, 
  { 123, 57 }, { 124, 64 }, { 125, 71 }, { 126, 78 }, { 127, 85 }, 
  { 126, 92 }, { 126, 99 }, { 126, 106 }, { 125, 113 }, { 121, 119 }, 
  { 114, 121 }, { 108, 117 }, { 103, 112 }, { 101, 105 }, { 104, 98 }, 
  { 110, 94 }, { 117, 93 }, { 124, 93 }, { 130, 97 }, { 137, 100 }, 
  { 143, 104 }, { 148, 109 }, { 155, 112 }, { 160, 117 }, { 167, 117 }, 
  { 174, 119 }, { 181, 117 }, { 184, 110 }, { 185, 103 }, { 185, 96 }, 
  { 185, 89 }, { 185, 82 }, { 186, 75 }, { 186, 68 }, { 184, 61 }, 
  { 185, 54 }, { 184, 47 }, { 183, 40 }, { 190, 42 }, { 197, 42 }, 
  { 204, 44 }, { 211, 47 }, { 216, 52 }, { 217, 59 }, { 217, 66 }, 
  { 217, 73 }, { 217, 80 }, { 216, 87 }, { 217, 94 }, { 217, 101 }, 
  { 217, 108 }, { 217, 115 }, { 217, 122 }
};

static const StrokeWaypoint BALI_GUIDE_WA[] = {
  { 97, 68 }, { 98, 61 }, { 101, 54 }, { 105, 48 }, { 111, 43 }, 
  { 118, 41 }, { 125, 41 }, { 123, 48 }, { 121, 55 }, { 118, 62 }, 
  { 115, 69 }, { 115, 76 }, { 114, 83 }, { 114, 90 }, { 115, 97 }, 
  { 115, 104 }, { 119, 110 }, { 126, 113 }, { 133, 116 }, { 140, 117 }, 
  { 147, 118 }, { 154, 117 }, { 161, 117 }, { 168, 116 }, { 175, 112 }, 
  { 178, 105 }, { 181, 98 }, { 182, 91 }, { 180, 84 }, { 178, 77 }, 
  { 173, 72 }, { 167, 68 }, { 160, 65 }, { 153, 63 }, { 146, 62 }, 
  { 139, 61 }, { 145, 57 }, { 151, 53 }, { 157, 49 }, { 163, 45 }, 
  { 170, 44 }, { 177, 41 }, { 184, 41 }, { 191, 43 }, { 197, 47 }, 
  { 201, 53 }, { 205, 59 }, { 207, 66 }, { 207, 73 }, { 208, 80 }, 
  { 208, 87 }, { 207, 94 }, { 208, 101 }, { 208, 108 }, { 208, 115 }, 
  { 208, 122 }
};

static const StrokeWaypoint BALI_GUIDE_LA[] = {
  { 69, 67 }, { 69, 60 }, { 73, 53 }, { 77, 47 }, { 84, 43 }, 
  { 91, 43 }, { 98, 44 }, { 95, 51 }, { 91, 58 }, { 89, 65 }, 
  { 87, 72 }, { 86, 79 }, { 85, 86 }, { 84, 93 }, { 84, 100 }, 
  { 87, 107 }, { 91, 113 }, { 98, 115 }, { 105, 116 }, { 113, 115 }, 
  { 110, 108 }, { 109, 101 }, { 108, 94 }, { 107, 87 }, { 108, 80 }, 
  { 110, 73 }, { 109, 66 }, { 113, 60 }, { 116, 53 }, { 121, 47 }, 
  { 128, 44 }, { 135, 42 }, { 143, 41 }, { 150, 44 }, { 155, 49 }, 
  { 159, 55 }, { 160, 62 }, { 160, 69 }, { 160, 76 }, { 161, 83 }, 
  { 160, 90 }, { 162, 97 }, { 163, 104 }, { 165, 111 }, { 170, 116 }, 
  { 177, 119 }, { 184, 119 }, { 191, 118 }, { 198, 117 }, { 202, 111 }, 
  { 206, 105 }, { 208, 98 }, { 208, 91 }, { 206, 84 }, { 205, 77 }, 
  { 205, 70 }, { 206, 63 }, { 206, 56 }, { 206, 49 }, { 207, 42 }, 
  { 214, 41 }, { 221, 44 }, { 228, 46 }, { 235, 49 }, { 235, 56 }, 
  { 235, 63 }, { 235, 70 }, { 236, 77 }, { 237, 84 }, { 237, 91 }, 
  { 238, 98 }, { 237, 105 }, { 236, 112 }, { 236, 119 }
};

static const StrokeWaypoint BALI_GUIDE_MA[] = {
  { 98, 68 }, { 101, 61 }, { 103, 54 }, { 107, 47 }, { 113, 43 }, 
  { 120, 41 }, { 128, 40 }, { 135, 40 }, { 142, 40 }, { 150, 40 }, 
  { 143, 42 }, { 136, 44 }, { 129, 46 }, { 125, 52 }, { 122, 59 }, 
  { 121, 66 }, { 126, 71 }, { 133, 74 }, { 140, 75 }, { 147, 74 }, 
  { 140, 78 }, { 133, 80 }, { 126, 83 }, { 121, 88 }, { 119, 95 }, 
  { 119, 102 }, { 121, 109 }, { 127, 113 }, { 134, 116 }, { 141, 118 }, 
  { 148, 118 }, { 155, 118 }, { 162, 117 }, { 169, 115 }, { 173, 108 }, 
  { 176, 101 }, { 177, 94 }, { 175, 87 }, { 176, 80 }, { 175, 73 }, 
  { 175, 66 }, { 175, 59 }, { 175, 52 }, { 176, 45 }, { 172, 38 }, 
  { 179, 39 }, { 186, 41 }, { 193, 43 }, { 200, 46 }, { 205, 51 }, 
  { 206, 58 }, { 206, 65 }, { 207, 72 }, { 208, 79 }, { 207, 86 }, 
  { 207, 93 }, { 207, 100 }, { 207, 107 }, { 207, 114 }, { 207, 121 }
};

static const StrokeWaypoint BALI_GUIDE_GA[] = {
  { 82, 69 }, { 84, 62 }, { 86, 55 }, { 89, 48 }, { 95, 44 }, 
  { 102, 42 }, { 109, 43 }, { 107, 50 }, { 105, 57 }, { 103, 64 }, 
  { 101, 71 }, { 99, 78 }, { 99, 85 }, { 99, 92 }, { 99, 99 }, 
  { 100, 106 }, { 103, 113 }, { 109, 117 }, { 116, 118 }, { 123, 117 }, 
  { 130, 117 }, { 126, 111 }, { 125, 104 }, { 122, 97 }, { 121, 90 }, 
  { 120, 83 }, { 120, 76 }, { 123, 69 }, { 126, 62 }, { 130, 56 }, 
  { 135, 51 }, { 141, 46 }, { 148, 42 }, { 155, 41 }, { 162, 42 }, 
  { 169, 45 }, { 176, 48 }, { 181, 53 }, { 185, 59 }, { 189, 65 }, 
  { 191, 72 }, { 192, 79 }, { 191, 86 }, { 191, 93 }, { 193, 100 }, 
  { 192, 107 }, { 192, 114 }, { 195, 121 }, { 195, 114 }, { 195, 107 }, 
  { 195, 100 }, { 194, 93 }, { 196, 86 }, { 197, 79 }, { 195, 72 }, 
  { 194, 65 }, { 193, 58 }, { 194, 51 }, { 195, 44 }, { 202, 43 }, 
  { 209, 45 }, { 216, 47 }, { 223, 50 }, { 226, 57 }, { 225, 64 }, 
  { 223, 71 }, { 223, 78 }, { 223, 85 }, { 223, 92 }, { 223, 99 }, 
  { 223, 106 }, { 223, 113 }, { 224, 120 }
};

static const StrokeWaypoint BALI_GUIDE_BA[] = {
  { 72, 69 }, { 73, 62 }, { 75, 55 }, { 78, 48 }, { 83, 43 }, 
  { 90, 41 }, { 97, 42 }, { 104, 45 }, { 99, 50 }, { 97, 57 }, 
  { 93, 63 }, { 91, 71 }, { 90, 78 }, { 89, 86 }, { 88, 94 }, 
  { 89, 101 }, { 91, 108 }, { 96, 114 }, { 103, 117 }, { 110, 117 }, 
  { 117, 117 }, { 124, 117 }, { 131, 118 }, { 138, 119 }, { 133, 114 }, 
  { 128, 108 }, { 124, 102 }, { 119, 97 }, { 116, 90 }, { 114, 83 }, 
  { 114, 76 }, { 113, 69 }, { 115, 62 }, { 118, 55 }, { 123, 49 }, 
  { 129, 45 }, { 136, 43 }, { 143, 43 }, { 148, 48 }, { 152, 54 }, 
  { 154, 61 }, { 159, 56 }, { 163, 50 }, { 168, 45 }, { 175, 44 }, 
  { 182, 43 }, { 188, 47 }, { 192, 53 }, { 196, 59 }, { 199, 66 }, 
  { 202, 73 }, { 203, 80 }, { 204, 87 }, { 203, 94 }, { 204, 101 }, 
  { 205, 108 }, { 203, 115 }, { 204, 122 }, { 201, 115 }, { 202, 108 }, 
  { 202, 101 }, { 203, 94 }, { 203, 87 }, { 202, 80 }, { 202, 73 }, 
  { 201, 66 }, { 203, 59 }, { 203, 52 }, { 203, 45 }, { 200, 38 }, 
  { 207, 41 }, { 214, 42 }, { 221, 42 }, { 226, 47 }, { 232, 51 }, 
  { 233, 58 }, { 235, 65 }, { 235, 72 }, { 235, 79 }, { 234, 86 }, 
  { 234, 93 }, { 235, 100 }, { 235, 107 }, { 235, 115 }, { 236, 122 }
};

static const StrokeWaypoint BALI_GUIDE_NGA[] = {
  { 90, 68 }, { 92, 61 }, { 94, 54 }, { 98, 48 }, { 104, 44 }, 
  { 113, 43 }, { 120, 43 }, { 114, 48 }, { 112, 55 }, { 110, 62 }, 
  { 109, 69 }, { 109, 76 }, { 108, 83 }, { 107, 90 }, { 107, 97 }, 
  { 107, 104 }, { 111, 111 }, { 116, 116 }, { 122, 120 }, { 129, 119 }, 
  { 136, 119 }, { 143, 119 }, { 150, 120 }, { 157, 121 }, { 151, 116 }, 
  { 145, 112 }, { 142, 105 }, { 139, 98 }, { 134, 93 }, { 133, 86 }, 
  { 133, 79 }, { 133, 72 }, { 133, 65 }, { 133, 58 }, { 135, 51 }, 
  { 139, 45 }, { 147, 43 }, { 154, 44 }, { 161, 46 }, { 168, 49 }, 
  { 173, 54 }, { 175, 61 }, { 180, 56 }, { 182, 49 }, { 187, 43 }, 
  { 194, 41 }, { 201, 40 }, { 208, 43 }, { 212, 49 }, { 214, 56 }, 
  { 216, 63 }, { 216, 70 }, { 216, 77 }, { 216, 84 }, { 216, 91 }, 
  { 216, 99 }, { 216, 106 }, { 216, 113 }, { 216, 120 }
};

static const StrokeWaypoint BALI_GUIDE_PA[] = {
  { 100, 69 }, { 100, 62 }, { 101, 55 }, { 104, 48 }, { 110, 44 }, 
  { 117, 43 }, { 124, 43 }, { 131, 46 }, { 126, 51 }, { 123, 58 }, 
  { 121, 65 }, { 117, 71 }, { 117, 78 }, { 117, 85 }, { 117, 92 }, 
  { 115, 99 }, { 119, 106 }, { 124, 111 }, { 128, 117 }, { 135, 118 }, 
  { 142, 120 }, { 149, 120 }, { 156, 117 }, { 163, 117 }, { 170, 116 }, 
  { 173, 109 }, { 175, 102 }, { 176, 95 }, { 174, 88 }, { 173, 81 }, 
  { 174, 74 }, { 175, 67 }, { 175, 60 }, { 177, 53 }, { 174, 46 }, 
  { 181, 44 }, { 188, 44 }, { 195, 46 }, { 202, 49 }, { 206, 55 }, 
  { 206, 62 }, { 206, 69 }, { 207, 76 }, { 208, 83 }, { 208, 90 }, 
  { 208, 97 }, { 208, 104 }, { 207, 111 }, { 207, 118 }
};

static const StrokeWaypoint BALI_GUIDE_JA[] = {
  { 102, 67 }, { 103, 60 }, { 105, 53 }, { 110, 48 }, { 117, 45 }, 
  { 124, 44 }, { 131, 43 }, { 129, 50 }, { 126, 57 }, { 122, 63 }, 
  { 121, 70 }, { 122, 77 }, { 122, 84 }, { 121, 91 }, { 121, 98 }, 
  { 121, 105 }, { 124, 112 }, { 129, 117 }, { 136, 119 }, { 143, 118 }, 
  { 150, 119 }, { 157, 120 }, { 164, 120 }, { 171, 118 }, { 166, 113 }, 
  { 161, 108 }, { 155, 104 }, { 150, 99 }, { 146, 93 }, { 145, 86 }, 
  { 144, 79 }, { 144, 72 }, { 146, 65 }, { 149, 58 }, { 151, 51 }, 
  { 158, 48 }, { 164, 44 }, { 171, 43 }, { 178, 43 }, { 185, 43 }, 
  { 192, 45 }, { 198, 49 }, { 202, 55 }, { 202, 62 }, { 196, 66 }, 
  { 189, 70 }, { 183, 75 }, { 176, 77 }, { 169, 79 }, { 167, 86 }, 
  { 174, 87 }, { 181, 88 }, { 188, 91 }, { 195, 93 }, { 200, 98 }, 
  { 203, 105 }, { 203, 112 }, { 204, 119 }
};

static const StrokeWaypoint BALI_GUIDE_YA[] = {
  { 81, 68 }, { 82, 61 }, { 85, 54 }, { 88, 47 }, { 93, 42 }, 
  { 100, 40 }, { 107, 40 }, { 114, 43 }, { 109, 49 }, { 106, 56 }, 
  { 103, 63 }, { 101, 70 }, { 99, 77 }, { 95, 83 }, { 94, 90 }, 
  { 94, 97 }, { 94, 104 }, { 96, 111 }, { 101, 117 }, { 108, 119 }, 
  { 115, 118 }, { 122, 115 }, { 127, 108 }, { 132, 102 }, { 134, 95 }, 
  { 137, 88 }, { 138, 81 }, { 138, 74 }, { 140, 67 }, { 141, 60 }, 
  { 143, 53 }, { 145, 46 }, { 145, 39 }, { 149, 45 }, { 149, 52 }, 
  { 150, 59 }, { 151, 66 }, { 153, 73 }, { 153, 80 }, { 154, 87 }, 
  { 156, 94 }, { 157, 101 }, { 159, 108 }, { 165, 113 }, { 172, 116 }, 
  { 179, 118 }, { 185, 114 }, { 191, 110 }, { 191, 103 }, { 192, 96 }, 
  { 193, 89 }, { 194, 80 }, { 192, 72 }, { 194, 64 }, { 192, 56 }, 
  { 192, 47 }, { 189, 39 }, { 196, 41 }, { 204, 42 }, { 211, 43 }, 
  { 218, 45 }, { 221, 52 }, { 224, 59 }, { 225, 66 }, { 224, 73 }, 
  { 224, 80 }, { 224, 87 }, { 224, 94 }, { 224, 101 }, { 224, 108 }, 
  { 223, 115 }
};

static const StrokeWaypoint BALI_GUIDE_NYA[] = {
  { 55, 68 }, { 56, 61 }, { 57, 54 }, { 62, 49 }, { 68, 45 }, 
  { 76, 42 }, { 83, 41 }, { 81, 48 }, { 77, 54 }, { 75, 61 }, 
  { 75, 68 }, { 74, 75 }, { 73, 82 }, { 71, 89 }, { 69, 96 }, 
  { 70, 103 }, { 74, 109 }, { 79, 114 }, { 89, 117 }, { 96, 116 }, 
  { 103, 116 }, { 110, 116 }, { 117, 117 }, { 113, 111 }, { 108, 106 }, 
  { 105, 99 }, { 102, 92 }, { 97, 87 }, { 96, 80 }, { 96, 73 }, 
  { 97, 66 }, { 97, 57 }, { 102, 51 }, { 107, 45 }, { 113, 41 }, 
  { 121, 40 }, { 127, 44 }, { 132, 50 }, { 136, 59 }, { 141, 51 }, 
  { 147, 47 }, { 154, 43 }, { 162, 41 }, { 170, 45 }, { 173, 52 }, 
  { 176, 59 }, { 177, 66 }, { 177, 73 }, { 177, 80 }, { 176, 87 }, 
  { 175, 94 }, { 175, 101 }, { 176, 108 }, { 176, 115 }, { 179, 108 }, 
  { 180, 101 }, { 179, 94 }, { 179, 87 }, { 180, 80 }, { 182, 73 }, 
  { 185, 66 }, { 188, 59 }, { 192, 53 }, { 197, 47 }, { 203, 43 }, 
  { 211, 42 }, { 218, 42 }, { 225, 43 }, { 231, 47 }, { 238, 51 }, 
  { 244, 55 }, { 248, 61 }, { 249, 68 }, { 250, 75 }, { 252, 82 }, 
  { 249, 89 }, { 247, 96 }, { 246, 103 }, { 241, 109 }, { 235, 113 }, 
  { 227, 117 }, { 219, 120 }, { 212, 121 }, { 204, 117 }, { 200, 111 }
};

static void get_bali_guide(int index, const StrokeWaypoint*& pts, uint8_t& count) {
  switch (index) {
    case 0:  pts = BALI_GUIDE_HA;  count = sizeof(BALI_GUIDE_HA)/sizeof(BALI_GUIDE_HA[0]); break;
    case 1:  pts = BALI_GUIDE_NA;  count = sizeof(BALI_GUIDE_NA)/sizeof(BALI_GUIDE_NA[0]); break;
    case 2:  pts = BALI_GUIDE_CA;  count = sizeof(BALI_GUIDE_CA)/sizeof(BALI_GUIDE_CA[0]); break;
    case 3:  pts = BALI_GUIDE_RA;  count = sizeof(BALI_GUIDE_RA)/sizeof(BALI_GUIDE_RA[0]); break;
    case 4:  pts = BALI_GUIDE_KA;  count = sizeof(BALI_GUIDE_KA)/sizeof(BALI_GUIDE_KA[0]); break;
    case 5:  pts = BALI_GUIDE_DA;  count = sizeof(BALI_GUIDE_DA)/sizeof(BALI_GUIDE_DA[0]); break;
    case 6:  pts = BALI_GUIDE_TA;  count = sizeof(BALI_GUIDE_TA)/sizeof(BALI_GUIDE_TA[0]); break;
    case 7:  pts = BALI_GUIDE_SA;  count = sizeof(BALI_GUIDE_SA)/sizeof(BALI_GUIDE_SA[0]); break;
    case 8:  pts = BALI_GUIDE_WA;  count = sizeof(BALI_GUIDE_WA)/sizeof(BALI_GUIDE_WA[0]); break;
    case 9:  pts = BALI_GUIDE_LA;  count = sizeof(BALI_GUIDE_LA)/sizeof(BALI_GUIDE_LA[0]); break;
    case 10: pts = BALI_GUIDE_MA;  count = sizeof(BALI_GUIDE_MA)/sizeof(BALI_GUIDE_MA[0]); break;
    case 11: pts = BALI_GUIDE_GA;  count = sizeof(BALI_GUIDE_GA)/sizeof(BALI_GUIDE_GA[0]); break;
    case 12: pts = BALI_GUIDE_BA;  count = sizeof(BALI_GUIDE_BA)/sizeof(BALI_GUIDE_BA[0]); break;
    case 13: pts = BALI_GUIDE_NGA; count = sizeof(BALI_GUIDE_NGA)/sizeof(BALI_GUIDE_NGA[0]); break;
    case 14: pts = BALI_GUIDE_PA;  count = sizeof(BALI_GUIDE_PA)/sizeof(BALI_GUIDE_PA[0]); break;
    case 15: pts = BALI_GUIDE_JA;  count = sizeof(BALI_GUIDE_JA)/sizeof(BALI_GUIDE_JA[0]); break;
    case 16: pts = BALI_GUIDE_YA;  count = sizeof(BALI_GUIDE_YA)/sizeof(BALI_GUIDE_YA[0]); break;
    case 17: pts = BALI_GUIDE_NYA; count = sizeof(BALI_GUIDE_NYA)/sizeof(BALI_GUIDE_NYA[0]); break;
    default: pts = BALI_GUIDE_DEF; count = sizeof(BALI_GUIDE_DEF)/sizeof(BALI_GUIDE_DEF[0]); break;
  }
}

static void update_bali_write_guide() {
  if (write_watermark_lbl) {
    if (active_write_index >= 0 && active_write_index < 18) {
      lv_label_set_text(write_watermark_lbl, BALI_AKSARA[active_write_index]);
      lv_obj_align(write_watermark_lbl, LV_ALIGN_CENTER, 0, 29);
    }
  }

  if (!write_guide_track) return;
  const StrokeWaypoint* guide_pts = nullptr;
  uint8_t count = 0;
  get_bali_guide(active_write_index, guide_pts, count);

  for (int i = 0; i < count && i < 100; i++) {
    bali_guide_pts[i].x = guide_pts[i].x;
    bali_guide_pts[i].y = guide_pts[i].y;
  }
  lv_line_set_points(write_guide_track, bali_guide_pts, count > 100 ? 100 : count);

  // Inisialisasi checkpoints urutan titik P1 .. Pn (R=18px, Koridor=22px, Batas Gagal Parah=34px)
  ui_trace_set_checkpoints(&bali_tab_trace, guide_pts, count, 28, 35, 60);
  ui_trace_clear(&bali_tab_trace);
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
  if (current_page < 5) {
    audio_play(SOUND_PAGE);
    current_page++;
    active_write_index = current_page * 3;
    update_path_page();
  }
}

static void on_cycle_write_aksara_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  active_write_index = (active_write_index + 1) % 18;
  switch_tab(1);
}

static void on_node_click(lv_event_t* e) {
  int node_idx = (int)(intptr_t)lv_event_get_user_data(e);
  const PathNodeData& node = BALI_PAGES[current_page][node_idx];

  char buf[64];
  int active_idx = current_page * 3 + node_idx;
  active_write_index = active_idx;

  if (node.state == 0) {
    audio_play(SOUND_NODE);
    snprintf(buf, sizeof(buf), "Aksara Bali %s: Latihan Ulang!", node.name);
    ui_show_toast(buf);
    ui_learn_start(AKSARA_BALI, active_idx);
  } else if (node.state == 1) {
    audio_play(SOUND_NODE);
    snprintf(buf, sizeof(buf), "Level Aktif: Aksara %s!", node.name);
    ui_show_toast(buf);
    ui_learn_start(AKSARA_BALI, active_idx);
  } else if (node.state == 2) {
    audio_play(SOUND_REWARD);
    ui_show_toast("Peti Hadiah Sastra: +50 XP!");
  } else {
    audio_play(SOUND_NODE);
    snprintf(buf, sizeof(buf), "Mulai Belajar Aksara %s!", node.name);
    ui_show_toast(buf);
    ui_learn_start(AKSARA_BALI, active_idx);
  }

  if (bali_cbs && bali_cbs->onSelectAksara) {
    bali_cbs->onSelectAksara(AKSARA_BALI, active_idx);
  }
}

static void on_clear_canvas_click(lv_event_t* e) {
  (void)e;
  audio_play(SOUND_CLICK);
  ui_trace_clear(&bali_tab_trace);
  ui_show_toast("Kanvas dibersihkan");
}

static void on_check_write_click(lv_event_t* e) {
  (void)e;
  StrokeEvalResult eval = ui_eval_stroke_trace(&bali_tab_trace);
  if (eval.success) {
    if (!bali_tab_trace.celebration_layer) {
      ui_trace_set_completed_visuals(&bali_tab_trace);
    }
    audio_play(SOUND_SUCCESS);
    audio_play(SOUND_REWARD);
    if (write_feedback_lbl) {
      lv_label_set_text(write_feedback_lbl, "BENAR!");
      lv_obj_set_style_text_color(write_feedback_lbl, lv_color_hex(0x22C55E), 0);
    }
    ui_show_toast(eval.message);

    // Kirim hasil latihan goresan Aksara Bali ke Firebase Cloud Firestore & Serial USB
    int prec = 94;
    if (bali_tab_trace.checkpoint_count > 0) {
      prec = (bali_tab_trace.active_checkpoint_idx * 100) / bali_tab_trace.checkpoint_count;
      if (prec > 100) prec = 100;
      if (prec < 80) prec = 91;
    }
    char aksara_label[48];
    if (active_write_index >= 0 && active_write_index < 18) {
      snprintf(aksara_label, sizeof(aksara_label), "%s (%s)", BALI_NAMES[active_write_index], BALI_AKSARA[active_write_index]);
    } else {
      strncpy(aksara_label, "Na (ᬦ)", sizeof(aksara_label) - 1);
    }
    firebase_sync_send_submission("Ni Kadek Ayu", "5B", "Bali", aksara_label, prec, 92, "Lulus");
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
    "Buddy: Rahajeng! Semangat nulis Aksara Bali!",
    "Buddy: Luar biasa, yuk teruskan langkahmu!",
    "Buddy: Setiap aksara punya keindahan tersendiri!",
    "Buddy: Keren banget progress belajarmu!"
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
                                 lv_color_hex(0x10B981), lv_color_hex(0x059669), 11);
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
  snprintf(ind_buf, sizeof(ind_buf), "%d / 6", current_page + 1);
  lv_label_set_text(lbl_page_ind, ind_buf);

  char sub_buf[32];
  snprintf(sub_buf, sizeof(sub_buf), "Jalur Belajar • Babak %d / 6", current_page + 1);
  if (header_sub) lv_label_set_text(header_sub, sub_buf);

  // Update tombol panah atas/bawah
  if (current_page == 0) {
    lv_obj_set_style_opa(btn_page_up, LV_OPA_30, 0);
  } else {
    lv_obj_set_style_opa(btn_page_up, LV_OPA_COVER, 0);
  }

  if (current_page == 5) {
    lv_obj_set_style_opa(btn_page_down, LV_OPA_30, 0);
  } else {
    lv_obj_set_style_opa(btn_page_down, LV_OPA_COVER, 0);
  }

  // Koordinat 3 Node berbeda untuk setiap babak (variasi jalur berliku dinamis melengkung kiri/kanan)
  static const int POS[6][3][2] = {
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
    { { 185, 16 }, { 280, 82 }, { 195, 148 } }
  };

  // Titik jalur penghubung (S-curve sesuai babak)
  create_trail_dots(path_nodes_cont, POS[current_page][0][0], POS[current_page][0][1], POS[current_page][1][0], POS[current_page][1][1]);
  create_trail_dots(path_nodes_cont, POS[current_page][1][0], POS[current_page][1][1], POS[current_page][2][0], POS[current_page][2][1]);

  // Gambar 3 Node halaman ini dengan posisi khusus babak tersebut
  for (int i = 0; i < 3; i++) {
    const PathNodeData& node = BALI_PAGES[current_page][i];
    lv_color_t c_top, c_dark;
    const char* txt = node.symbol;
    const lv_font_t* f = &font_bali_22;

    if (node.state == 0) {
      // Selesai (Emerald/Teal)
      c_top = lv_color_hex(0x059669);
      c_dark = lv_color_hex(0x047857);
    } else if (node.state == 1) {
      // Sedang Aktif (Emas/Amber Bali)
      c_top = C_BALI_1;
      c_dark = C_BALI_2;
    } else if (node.state == 2) {
      // Peti / Piala
      c_top = lv_color_hex(0xEA580C);
      c_dark = lv_color_hex(0xC2410C);
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

  static const BabakMeta BALI_META[6] = {
    { "Babak 1: Dasar", "Ha • Na • Ca", "Level 1 - 3", C_BALI_1 },
    { "Babak 2: Wreastra", "Ra • Ka • Da", "Level 4 - 6", lv_color_hex(0x10B981) },
    { "Babak 3: Lanjutan", "Ta • Sa • Wa", "Level 7 - 9", lv_color_hex(0x38BDF8) },
    { "Babak 4: Menengah", "La • Ma • Ga", "Level 10 - 12", lv_color_hex(0xA855F7) },
    { "Babak 5: Terampil", "Ba • Nga • Pa", "Level 13 - 15", lv_color_hex(0xEC4899) },
    { "Babak 6: Mahir", "Ja • Ya • Nya", "Tamat 18 Aksara!", lv_color_hex(0xF59E0B) }
  };

  const BabakMeta& meta = BALI_META[current_page];
  lv_obj_t* t1 = ui_make_label(infoCard, meta.title, &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(t1, 10, 6);
  lv_obj_t* t2 = ui_make_label(infoCard, meta.letters, &lv_font_montserrat_14, meta.color);
  lv_obj_set_pos(t2, 10, 26);
  lv_obj_t* t3 = ui_make_label(infoCard, meta.desc, &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(t3, 10, 46);

  // Maskot Buddy berposisi di sisi berlawanan dari node
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
  lv_obj_clear_flag(view_path, LV_OBJ_FLAG_SCROLLABLE); // Tanpa scroll
  lv_obj_set_style_bg_color(view_path, C_BG, 0);
  lv_obj_set_style_bg_opa(view_path, LV_OPA_COVER, 0);

  // Area Node Aksara
  path_nodes_cont = lv_obj_create(view_path);
  lv_obj_remove_style_all(path_nodes_cont);
  lv_obj_set_pos(path_nodes_cont, 0, 0);
  lv_obj_set_size(path_nodes_cont, 396, 224);
  lv_obj_clear_flag(path_nodes_cont, LV_OBJ_FLAG_SCROLLABLE);

  // Panel Tombol Navigasi Panah Atas & Bawah
  lv_obj_t* nav_side = ui_make_card(view_path, 400, 8, 70, 208, C_CARD, C_CARD, 16);
  lv_obj_clear_flag(nav_side, LV_OBJ_FLAG_SCROLLABLE);

  // Tombol Panah Atas (▲)
  btn_page_up = ui_make_card(nav_side, 8, 10, 54, 52, C_BALI_1, C_BALI_2, 14);
  ui_set_pressed(btn_page_up, C_BALI_2, C_BALI_2);
  lv_obj_add_event_cb(btn_page_up, on_page_up_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* upIc = ui_make_label(btn_page_up, LV_SYMBOL_UP, &lv_font_montserrat_20, lv_color_hex(0xFFFFFF));
  lv_obj_center(upIc);

  // Indikator Halaman ("1 / 6")
  lv_obj_t* ind_box = ui_make_card(nav_side, 6, 80, 58, 36, C_CARD_PR, C_CARD_PR, 10);
  lv_obj_clear_flag(ind_box, LV_OBJ_FLAG_CLICKABLE);
  lbl_page_ind = ui_make_label(ind_box, "1 / 6", &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(lbl_page_ind);

  // Tombol Panah Bawah (▼)
  btn_page_down = ui_make_card(nav_side, 8, 134, 54, 52, C_BALI_1, C_BALI_2, 14);
  ui_set_pressed(btn_page_down, C_BALI_2, C_BALI_2);
  lv_obj_add_event_cb(btn_page_down, on_page_down_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* downIc = ui_make_label(btn_page_down, LV_SYMBOL_DOWN, &lv_font_montserrat_20, lv_color_hex(0xFFFFFF));
  lv_obj_center(downIc);

  // Gambar konten halaman pertama
  update_path_page();
}

// ---------- 2. Tampilan Nulis Aksara Bali (Writing Canvas) ----------
static void build_write_view(lv_obj_t* parent) {
  view_write = lv_obj_create(parent);
  lv_obj_remove_style_all(view_write);
  lv_obj_set_pos(view_write, 0, 0);
  lv_obj_set_size(view_write, 480, 224);
  lv_obj_clear_flag(view_write, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(view_write, LV_OBJ_FLAG_HIDDEN);

  // Kartu Target Aksara Bali (Kiri)
  lv_obj_t* cardTarget = ui_make_card(view_write, 12, 8, 140, 208, C_CARD, C_CARD, 16);
  write_target_title = ui_make_label(cardTarget, "Aksara Ca", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(write_target_title, 14, 14);

  write_target_sub = ui_make_label(cardTarget, "Wreastra Bali\nBunyi: /tja/", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(write_target_sub, 14, 40);

  // Preview Kotak Huruf (Ketuk untuk ganti aksara langsung di tab Nulis)
  lv_obj_t* boxHuruf = ui_make_card(cardTarget, 14, 84, 112, 110, C_BALI_1, C_BALI_2, 14);
  ui_set_pressed(boxHuruf, C_BALI_2, C_BALI_2);
  lv_obj_add_event_cb(boxHuruf, on_cycle_write_aksara_click, LV_EVENT_CLICKED, NULL);
  write_preview_lbl = ui_make_label(boxHuruf, "Ca", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF));
  lv_obj_center(write_preview_lbl);

  // Area Kanvas Tracing ala Contoh Gambar User (308x160 px)
  lv_obj_t* canvasCard = ui_make_card(view_write, 160, 8, 308, 160, lv_color_hex(0x0A0F1D), lv_color_hex(0x0A0F1D), 16);
  lv_obj_set_style_border_color(canvasCard, C_BALI_1, 0);
  lv_obj_set_style_border_width(canvasCard, 2, 0);

  // Watermark Glif Aksara Asli Transparan (Background Guide 160px JUMBO)
  write_watermark_lbl = ui_make_label(canvasCard, "\xE1\xAC\x98", &font_bali_160, lv_color_hex(0x38BDF8));
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

  ui_trace_attach(&bali_tab_trace, canvasCard);
  bali_tab_trace.feedback_lbl = write_feedback_lbl;
  bali_tab_trace.watermark_lbl = write_watermark_lbl;
  bali_tab_trace.guide_track = write_guide_track;

  // Tombol Bersihkan & Periksa
  lv_obj_t* btnClear = ui_make_card(view_write, 160, 174, 146, 42, C_CARD, C_CARD, 12);
  ui_set_pressed(btnClear, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btnClear, on_clear_canvas_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* btnClearLbl = ui_make_label(btnClear, LV_SYMBOL_TRASH "  Bersihkan", &lv_font_montserrat_14, C_TEXT);
  lv_obj_center(btnClearLbl);

  lv_obj_t* btnCheck = ui_make_card(view_write, 316, 174, 152, 42, C_BALI_1, C_BALI_2, 12);
  ui_set_pressed(btnCheck, C_BALI_2, C_BALI_2);
  lv_obj_add_event_cb(btnCheck, on_check_write_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* btnCheckLbl = ui_make_label(btnCheck, LV_SYMBOL_OK "  Cek Goresan", &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
  lv_obj_center(btnCheckLbl);
}

// ---------- 3. Tampilan Setelan Aksara Bali ----------
static void build_settings_view(lv_obj_t* parent) {
  view_settings = lv_obj_create(parent);
  lv_obj_remove_style_all(view_settings);
  lv_obj_set_pos(view_settings, 0, 0);
  lv_obj_set_size(view_settings, 480, 224);
  lv_obj_clear_flag(view_settings, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(view_settings, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t* cardSet = ui_make_card(view_settings, 16, 8, 448, 208, C_CARD, C_CARD, 16);

  lv_obj_t* sTitle = ui_make_label(cardSet, "Setelan Pembelajaran Bali", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(sTitle, 16, 12);

  // Baris 1: Panduan Goresan
  lv_obj_t* row1 = ui_make_card(cardSet, 16, 46, 416, 44, C_CARD_PR, C_CARD_PR, 10);
  lv_obj_t* r1Lbl = ui_make_label(row1, "Tampilkan Garis Panduan Tipis", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(r1Lbl, 14, 12);
  lv_obj_t* sw1 = lv_switch_create(row1);
  lv_obj_set_pos(sw1, 350, 8);
  lv_obj_add_state(sw1, LV_STATE_CHECKED);

  // Baris 2: Efek Suara
  lv_obj_t* row2 = ui_make_card(cardSet, 16, 96, 416, 44, C_CARD_PR, C_CARD_PR, 10);
  lv_obj_t* r2Lbl = ui_make_label(row2, "Audio Pelafalan Aksara Bali", &lv_font_montserrat_14, C_TEXT);
  lv_obj_set_pos(r2Lbl, 14, 12);
  lv_obj_t* sw2 = lv_switch_create(row2);
  lv_obj_set_pos(sw2, 350, 8);
  lv_obj_add_state(sw2, LV_STATE_CHECKED);

  // Tombol Reset Kemajuan
  lv_obj_t* btnReset = ui_make_card(cardSet, 16, 150, 416, 42, lv_color_hex(0x3B151E), lv_color_hex(0x270B13), 10);
  ui_set_pressed(btnReset, lv_color_hex(0x4C1D27), lv_color_hex(0x4C1D27));
  lv_obj_t* rstLbl = ui_make_label(btnReset, LV_SYMBOL_REFRESH "  Reset Ulang Kemajuan Belajar Bali",
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
    ui_trace_clear(&bali_tab_trace);
  }

  if (tab_index == 0) {
    // Tab Belajar
    lv_obj_clear_flag(view_path, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(tab_btn_learn, C_CARD_PR, 0);
    lv_obj_set_style_text_color(tab_lbl_learn, C_BALI_1, 0);
  } else if (tab_index == 1) {
    // Tab Nulis Aksara
    lv_obj_clear_flag(view_write, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(tab_btn_write, C_CARD_PR, 0);
    lv_obj_set_style_text_color(tab_lbl_write, C_BALI_1, 0);

    // Update target aksara sesuai pilihan aktif
    if (active_write_index >= 0 && active_write_index < 18) {
      char b[32];
      snprintf(b, sizeof(b), "Aksara %s", BALI_NAMES[active_write_index]);
      lv_label_set_text(write_target_title, b);
      if (write_target_sub) {
        char sub_b[48];
        snprintf(sub_b, sizeof(sub_b), "Wreastra Bali\nBunyi: /%s/", BALI_NAMES[active_write_index]);
        lv_label_set_text(write_target_sub, sub_b);
      }
      lv_obj_set_style_text_font(write_preview_lbl, &font_bali_38, 0);
      lv_label_set_text(write_preview_lbl, BALI_AKSARA[active_write_index]);
      update_bali_write_guide();
    }
  } else {
    // Tab Setelan
    lv_obj_clear_flag(view_settings, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(tab_btn_settings, C_CARD_PR, 0);
    lv_obj_set_style_text_color(tab_lbl_settings, C_BALI_1, 0);
  }
}

// Bangun Layar Aksara Bali secara Lazy (hemat RAM heap saat belum dibuka)
static void build_bali_screen() {
  if (scr_bali) return;

  scr_bali = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_bali, C_BG, 0);
  lv_obj_set_style_bg_opa(scr_bali, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr_bali, LV_OBJ_FLAG_SCROLLABLE);

  // ===== TOP HEADER BAR =====
  lv_obj_t* header = ui_make_card(scr_bali, 0, 0, 480, 44, C_BG, C_BG, 0);

  // Tombol Kembali (< Back)
  lv_obj_t* btnBack = ui_make_card(header, 10, 5, 40, 34, C_CARD, C_CARD, 10);
  ui_set_pressed(btnBack, C_CARD_PR, C_CARD_PR);
  lv_obj_add_event_cb(btnBack, on_back_home_click, LV_EVENT_CLICKED, NULL);
  lv_obj_t* backIc = ui_make_label(btnBack, LV_SYMBOL_LEFT, &lv_font_montserrat_20, C_TEXT);
  lv_obj_center(backIc);

  // Judul & Subjudul
  lv_obj_t* title = ui_make_label(header, "Aksara Bali", &lv_font_montserrat_20, C_TEXT);
  lv_obj_set_pos(title, 58, 3);

  header_sub = ui_make_label(header, "Jalur Belajar • Babak 1 / 3", &lv_font_montserrat_14, C_SUB);
  lv_obj_set_pos(header_sub, 58, 23);

  // Badge Status XP Kanan
  lv_obj_t* badgeXp = ui_make_card(header, 366, 7, 104, 30, lv_color_hex(0x451A03), lv_color_hex(0x270E02), 15);
  lv_obj_t* badgeLbl = ui_make_label(badgeXp, "140 XP", &lv_font_montserrat_14, lv_color_hex(0xFBBF24));
  lv_obj_center(badgeLbl);

  // ===== KONTEN UTAMA (Tengah) =====
  lv_obj_t* content_area = lv_obj_create(scr_bali);
  lv_obj_remove_style_all(content_area);
  lv_obj_set_pos(content_area, 0, 44);
  lv_obj_set_size(content_area, 480, 226);
  lv_obj_clear_flag(content_area, LV_OBJ_FLAG_SCROLLABLE);

  build_path_view(content_area);
  build_write_view(content_area);
  build_settings_view(content_area);

  // ===== BOTTOM NAVIGATION BAR (3 OPSI) =====
  lv_obj_t* botNav = ui_make_card(scr_bali, 0, 270, 480, 50, C_CARD, C_CARD, 0);
  lv_obj_set_style_border_color(botNav, lv_color_hex(0x232938), 0);
  lv_obj_set_style_border_width(botNav, 1, 0);

  // Opsi 1: Belajar
  tab_btn_learn = ui_make_card(botNav, 4, 3, 154, 44, C_CARD_PR, C_CARD_PR, 12);
  lv_obj_add_event_cb(tab_btn_learn, on_tab_click, LV_EVENT_CLICKED, (void*)(intptr_t)0);
  tab_lbl_learn = ui_make_label(tab_btn_learn, LV_SYMBOL_HOME "  Belajar", &lv_font_montserrat_14, C_BALI_1);
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

void ui_bali_init(const AppCallbacks* callbacks) {
  bali_cbs = callbacks;
}

void ui_bali_show() {
  if (!scr_bali) {
    build_bali_screen();
  }
  if (scr_bali) {
    switch_tab(0); // Buka tab Belajar secara default
    if (lv_screen_active() != scr_bali) {
      lv_scr_load_anim(scr_bali, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
    }
  }
}
