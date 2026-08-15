/*
 * SPDX-License-Identifier: MIT
 *
 * Behavior that steps the shared trackball rotation angle (see
 * input_processor_rotate.c) by +-15 degrees each time it is pressed.
 * param1: 1 = clockwise (+15 degrees), -1 = counter-clockwise (-15 degrees).
 */

#define DT_DRV_COMPAT zmk_behavior_rotate_step

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

/* Implemented in input_processor_rotate.c */
extern void zmk_rotate_step_adjust(int32_t delta);

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    zmk_rotate_step_adjust((int32_t)binding->param1);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

/* Explicit list of the only two valid param1 values, so ZMK Studio (and other
 * keymap editors) know what this behavior accepts and can offer them as
 * named choices instead of hiding the behavior entirely. */
static const struct behavior_parameter_value_metadata rotate_step_param1_values[] = {
    {
        .display_name = "Clockwise",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = 1,
    },
    {
        .display_name = "Counter-Clockwise",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = (uint32_t)-1,
    },
};

static const struct behavior_parameter_metadata_set rotate_step_param_metadata_set[] = {{
    .param1_values = rotate_step_param1_values,
    .param1_values_len = ARRAY_SIZE(rotate_step_param1_values),
}};

static const struct behavior_parameter_metadata rotate_step_metadata = {
    .sets_len = ARRAY_SIZE(rotate_step_param_metadata_set),
    .sets = rotate_step_param_metadata_set,
};

#endif /* IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA) */

static const struct behavior_driver_api behavior_rotate_step_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &rotate_step_metadata,
#endif /* IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA) */
};

#define RS_INST(n)                                                                                 \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                             CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                  \
                             &behavior_rotate_step_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RS_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
