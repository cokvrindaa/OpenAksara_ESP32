#include "ui_common.h"

static lv_obj_t* toast = nullptr;
static lv_obj_t* toastLbl = nullptr;

static void anim_opa_cb(void* obj, int32_t v) {
  lv_obj_set_style_opa((lv_obj_t*)obj, (lv_opa_t)v, 0);
}

void ui_toast_init() {
  lv_obj_t* top = lv_layer_top();
  
  toast = lv_obj_create(top);
  lv_obj_remove_style_all(toast);
  lv_obj_set_style_radius(toast, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(toast, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(toast, lv_color_hex(0x181C26), 0);
  lv_obj_set_style_border_width(toast, 1, 0);
  lv_obj_set_style_border_color(toast, C_JAWA_1, 0);
  lv_obj_set_style_pad_hor(toast, 18, 0);
  lv_obj_set_style_pad_ver(toast, 7, 0);
  lv_obj_set_size(toast, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_max_width(toast, 440, 0);
  lv_obj_clear_flag(toast, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(toast, LV_OBJ_FLAG_SCROLLABLE);

  toastLbl = lv_label_create(toast);
  lv_label_set_text(toastLbl, "");
  lv_obj_set_style_text_font(toastLbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(toastLbl, C_TEXT, 0);
  lv_obj_center(toastLbl);

  lv_obj_align(toast, LV_ALIGN_TOP_MID, 0, 8);
  lv_obj_set_style_opa(toast, LV_OPA_TRANSP, 0);
}

void ui_show_toast(const char* text) {
  if (!toast || !toastLbl) return;
  lv_label_set_text(toastLbl, text);
  lv_obj_update_layout(toast);
  lv_obj_align(toast, LV_ALIGN_TOP_MID, 0, 8);

  lv_anim_del(toast, anim_opa_cb);
  lv_obj_set_style_opa(toast, LV_OPA_COVER, 0);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, toast);
  lv_anim_set_exec_cb(&a, anim_opa_cb);
  lv_anim_set_values(&a, 255, 0);
  lv_anim_set_delay(&a, 1800); // 1.8 detik agar sempat dibaca
  lv_anim_set_duration(&a, 300);
  lv_anim_start(&a);
}

