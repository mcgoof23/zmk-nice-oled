/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/services/bas.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/wpm.h>

#include "uiacat.h"

#define SRC(array) (const void **)array, sizeof(array) / sizeof(lv_img_dsc_t *)

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

LV_IMG_DECLARE(cat_idle);
LV_IMG_DECLARE(cat_spin_1);
LV_IMG_DECLARE(cat_spin_2);
LV_IMG_DECLARE(cat_spin_3);
LV_IMG_DECLARE(cat_spin_4);
LV_IMG_DECLARE(cat_spin_5);
LV_IMG_DECLARE(cat_spin_6);
LV_IMG_DECLARE(cat_spin_7);
LV_IMG_DECLARE(cat_spin_8);
LV_IMG_DECLARE(cat_spin_9);

// #define ANIMATION_SPEED_IDLE 10000
#define ANIMATION_SPEED_IDLE 960
const lv_img_dsc_t *idle_imgs[] = {
    &cat_idle,
    &cat_idle,
};

const lv_img_dsc_t *spin_imgs[] = {
    &cat_spin_1,
    &cat_spin_2,
    &cat_spin_3,
    &cat_spin_4,
    &cat_spin_5,
    &cat_spin_6,
    &cat_spin_7,
    &cat_spin_8,
    &cat_spin_9,
};
// #define ANIMATION_SPEED_SLOW 2000
#define ANIMATION_SPEED_SLOW 2000
// #define ANIMATION_SPEED_MID 500
#define ANIMATION_SPEED_MID 600

#define ANIMATION_SPEED_FAST 10

struct uiacat_wpm_status_state {
    uint8_t wpm;
};

enum anim_state {
    anim_state_none,
    anim_state_idle,
    anim_state_slow,
    anim_state_mid,
    anim_state_fast
} current_anim_state;

static void set_animation(lv_obj_t *animing, struct uiacat_wpm_status_state state) {
    if (state.wpm < 5) { // def: 5
        if (current_anim_state != anim_state_idle) {
            lv_animimg_set_src(animing, SRC(idle_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_IDLE);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_idle;
        }
    } else {
        if (current_anim_state != anim_state_fast) {
            lv_animimg_set_src(animing, SRC(spin_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_FAST);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_fast;
        }
    }
}

struct uiacat_wpm_status_state uiacat_wpm_status_get_state(const zmk_event_t *eh) {
    struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
    return (struct uiacat_wpm_status_state){.wpm = ev->state};
};

void uiacat_wpm_status_update_cb(struct uiacat_wpm_status_state state) {
    struct zmk_widget_uiacat *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_animation(widget->obj, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_uiacat, struct uiacat_wpm_status_state, uiacat_wpm_status_update_cb,
    uiacat_wpm_status_get_state)

ZMK_SUBSCRIPTION(widget_uiacat, zmk_wpm_state_changed);

int zmk_widget_uiacat_init(struct zmk_widget_uiacat *widget, lv_obj_t *parent) {
    widget->obj = lv_animimg_create(parent);
    lv_obj_center(widget->obj);

    sys_slist_append(&widgets, &widget->node);

    widget_uiacat_init();

    return 0;
}

lv_obj_t *zmk_widget_uiacat_obj(struct zmk_widget_uiacat *widget) { return widget->obj; }
