/*
 * SPDX-License-Identifier: MIT
 *
 * Input processor that rotates relative pointer (trackball) X/Y movement
 * in 24 discrete 15-degree steps. The step index is shared global state
 * (there is normally only one trackball), adjustable at runtime by the
 * companion zmk,behavior-rotate-step behavior (see behavior_rotate_step.c).
 */

#define DT_DRV_COMPAT zmk_input_processor_rotate

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/logging/log.h>
#include <drivers/input_processor.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>

LOG_MODULE_REGISTER(rotate_ip, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#define ROTATE_SCALE 1000
#define ROTATE_STEPS 24
#define ROTATE_STEP_DEGREES (360 / ROTATE_STEPS)

/* cos()/sin() * 1000 for 0, 15, 30, ..., 345 degrees */
static const int32_t rotate_cos[ROTATE_STEPS] = {1000, 966, 866, 707, 500,  259,  0,    -259,
                                                  -500, -707, -866, -966, -1000, -966, -866, -707,
                                                  -500, -259, 0,    259,  500,  707,  866,  966};
static const int32_t rotate_sin[ROTATE_STEPS] = {0,    259,  500,  707,  866,  966,  1000, 966,
                                                  866,  707,  500,  259,  0,    -259, -500, -707,
                                                  -866, -966, -1000, -966, -866, -707, -500, -259};

/* Shared rotation step (0 to ROTATE_STEPS-1), mutated by the rotate-step behavior. */
static atomic_t rotate_step_idx = ATOMIC_INIT(17);

void zmk_rotate_step_adjust(int32_t delta) {
    int32_t cur = (int32_t)atomic_get(&rotate_step_idx);
    int32_t next = ((cur + delta) % ROTATE_STEPS + ROTATE_STEPS) % ROTATE_STEPS;
    atomic_set(&rotate_step_idx, next);
    LOG_INF("trackball rotation set to %d degrees", (int)(next * ROTATE_STEP_DEGREES));
}

struct rotate_data {
    int32_t last_x;
    int32_t last_y;
};

static int rotate_handle_event(const struct device *dev, struct input_event *event,
                                uint32_t param1, uint32_t param2,
                                struct zmk_input_processor_state *state) {
    ARG_UNUSED(param1);
    ARG_UNUSED(param2);
    ARG_UNUSED(state);

    if (event->type != INPUT_EV_REL) {
        return 0;
    }
    if (event->code != INPUT_REL_X && event->code != INPUT_REL_Y) {
        return 0;
    }

    struct rotate_data *data = dev->data;
    int32_t idx = (int32_t)atomic_get(&rotate_step_idx);

    if (idx == 0) {
        /* 0 degrees: pass through unchanged, just track raw values. */
        if (event->code == INPUT_REL_X) {
            data->last_x = event->value;
        } else {
            data->last_y = event->value;
        }
        return 0;
    }

    const int32_t c = rotate_cos[idx];
    const int32_t s = rotate_sin[idx];

    if (event->code == INPUT_REL_X) {
        const int32_t x = event->value;
        const int32_t y = data->last_y;
        data->last_x = x;
        event->value = (x * c - y * s) / ROTATE_SCALE;
    } else {
        const int32_t y = event->value;
        const int32_t x = data->last_x;
        data->last_y = y;
        event->value = (x * s + y * c) / ROTATE_SCALE;
    }

    return 0;
}

static const struct zmk_input_processor_driver_api rotate_driver_api = {
    .handle_event = rotate_handle_event,
};

#define ROTATE_INST_INIT(n)                                                                        \
    static struct rotate_data rotate_data_##n = {0};                                               \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &rotate_data_##n, NULL, POST_KERNEL, 50,                   \
                           &rotate_driver_api);

DT_INST_FOREACH_STATUS_OKAY(ROTATE_INST_INIT)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
