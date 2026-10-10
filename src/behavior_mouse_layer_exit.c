/*
 * SPDX-License-Identifier: MIT
 *
 * Behavior used inside the auto mouse layer: while any key bound to it is
 * held, nothing happens; once the last one is released, the configured
 * layer is turned off after `delay-ms`. Pressing it again before the delay
 * expires cancels the pending turn-off (so double clicks and drags work).
 *
 * Intended to be combined with a mouse button in a macro, e.g.
 *   press: &mlx, &mkp MB1 / release: &mkp MB1, &mlx
 */

#define DT_DRV_COMPAT zmk_behavior_mouse_layer_exit

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct mlx_config {
    uint8_t layer;
    uint32_t delay_ms;
};

struct mlx_data {
    const struct device *dev;
    struct k_work_delayable exit_work;
    int held;
};

static void mlx_exit_work_cb(struct k_work *work) {
    struct k_work_delayable *d_work = k_work_delayable_from_work(work);
    struct mlx_data *data = CONTAINER_OF(d_work, struct mlx_data, exit_work);
    const struct mlx_config *cfg = data->dev->config;

    if (data->held > 0) {
        return;
    }

    zmk_keymap_layer_id_t id = zmk_keymap_layer_index_to_id(cfg->layer);
    if (zmk_keymap_layer_active(id)) {
        LOG_DBG("mouse-layer-exit: deactivating layer %d", cfg->layer);
        zmk_keymap_layer_deactivate(id, false);
    }
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct mlx_data *data = dev->data;

    data->held++;
    k_work_cancel_delayable(&data->exit_work);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct mlx_data *data = dev->data;
    const struct mlx_config *cfg = dev->config;

    if (data->held > 0) {
        data->held--;
    }
    if (data->held == 0) {
        k_work_reschedule(&data->exit_work, K_MSEC(cfg->delay_ms));
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static int mlx_init(const struct device *dev) {
    struct mlx_data *data = dev->data;
    data->dev = dev;
    data->held = 0;
    k_work_init_delayable(&data->exit_work, mlx_exit_work_cb);
    return 0;
}

static const struct behavior_driver_api behavior_mouse_layer_exit_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define MLX_INST(n)                                                                                \
    static struct mlx_data mlx_data_##n;                                                           \
    static const struct mlx_config mlx_config_##n = {                                              \
        .layer = DT_INST_PROP(n, layer),                                                           \
        .delay_ms = DT_INST_PROP(n, delay_ms),                                                     \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, mlx_init, NULL, &mlx_data_##n, &mlx_config_##n, POST_KERNEL,        \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                   \
                            &behavior_mouse_layer_exit_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MLX_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
