#define DT_DRV_COMPAT klink_behavior_indicator_refresh

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <drivers/behavior.h>

#include <zmk/battery.h>
#include <zmk/behavior.h>
#include <zmk/ble.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/hid_indicators.h>

#define CAPSLOCK_BIT BIT(1)
#define LED_GPIO_NODE_ID DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_leds)

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

BUILD_ASSERT(DT_NODE_EXISTS(DT_ALIAS(indicator_r)),
             "An alias for a red LED is not found for RGBLED_WIDGET");
BUILD_ASSERT(DT_NODE_EXISTS(DT_ALIAS(indicator_g)),
             "An alias for a green LED is not found for RGBLED_WIDGET");
BUILD_ASSERT(DT_NODE_EXISTS(DT_ALIAS(indicator_b)),
             "An alias for a blue LED is not found for RGBLED_WIDGET");

static const struct device *led_dev = DEVICE_DT_GET(LED_GPIO_NODE_ID);
static const uint8_t led_idx[] = {DT_NODE_CHILD_IDX(DT_ALIAS(indicator_r)),
                                  DT_NODE_CHILD_IDX(DT_ALIAS(indicator_g)),
                                  DT_NODE_CHILD_IDX(DT_ALIAS(indicator_b))};
static const uint8_t profile_color_bits[] = {0b011, 0b110, 0b101};

struct indicator_state_t {
    uint8_t keylock;
    uint8_t connection;
    uint8_t active_device;
    uint8_t battery;
    uint8_t flash_times;
};

static struct indicator_state_t indicator_state;

static void set_indicator_color(uint8_t bits) {
    static uint8_t last_bits;

    if (bits == last_bits) {
        return;
    }

    for (uint8_t pos = 0; pos < ARRAY_SIZE(led_idx); pos++) {
        if (bits & BIT(pos)) {
            led_on(led_dev, led_idx[pos]);
        } else {
            led_off(led_dev, led_idx[pos]);
        }
    }

    last_bits = bits;
}

static void get_lock_indicators(void) {
    indicator_state.keylock = zmk_hid_indicators_get_current_profile();
    LOG_DBG("LOCK LEDS: %d", indicator_state.keylock);
}

static void hid_indicators_status_update_cb(const zmk_event_t *eh) {
    ARG_UNUSED(eh);
    get_lock_indicators();
}

ZMK_LISTENER(widget_hid_indicators_status, hid_indicators_status_update_cb);
ZMK_SUBSCRIPTION(widget_hid_indicators_status, zmk_hid_indicators_changed);

static void ble_active_profile_update(void) {
    int profile_index = zmk_ble_active_profile_index();

    if (profile_index < 0 || profile_index >= (int)ARRAY_SIZE(profile_color_bits)) {
        LOG_WRN("Unsupported BLE profile index: %d", profile_index);
        indicator_state.connection = 0;
        return;
    }

    indicator_state.active_device = profile_index;
    if (zmk_ble_active_profile_is_connected()) {
        indicator_state.connection = 2;
        indicator_state.flash_times = 3 * 4;
    } else {
        indicator_state.connection = 1;
        indicator_state.flash_times = 15 * 4;
    }

    LOG_DBG("Device_BT%d, Connection State: %d", indicator_state.active_device + 1,
            indicator_state.connection);
}

static void ble_active_profile_update_cb(const zmk_event_t *eh) {
    ARG_UNUSED(eh);
    ble_active_profile_update();
}

ZMK_LISTENER(ble_active_profile_listener, ble_active_profile_update_cb);
ZMK_SUBSCRIPTION(ble_active_profile_listener, zmk_ble_active_profile_changed);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
static int indicator_refresh_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    ble_active_profile_update();
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api indicator_refresh_driver_api = {
    .binding_pressed = indicator_refresh_pressed,
    .locality = BEHAVIOR_LOCALITY_EVENT_SOURCE,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define INDICATOR_REFRESH_INST(n)                                                                  \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &indicator_refresh_driver_api);

DT_INST_FOREACH_STATUS_OKAY(INDICATOR_REFRESH_INST)
#endif

#if IS_ENABLED(CONFIG_ZMK_BATTERY_REPORTING)
static int led_battery_listener_cb(const zmk_event_t *eh) {
    indicator_state.battery = as_zmk_battery_state_changed(eh)->state_of_charge;
    return 0;
}

ZMK_LISTENER(led_battery_listener, led_battery_listener_cb);
ZMK_SUBSCRIPTION(led_battery_listener, zmk_battery_state_changed);
#endif

static void led_process_thread(void) {
    uint16_t led_timer_steps = 0;

    while (true) {
        k_sleep(K_MSEC(20));
        led_timer_steps++;

        if (indicator_state.connection > 0) {
            if (indicator_state.active_device >= ARRAY_SIZE(profile_color_bits)) {
                indicator_state.connection = 0;
                continue;
            }

            if ((led_timer_steps & 0xf) != 0xf) {
                continue;
            }

            uint8_t color_bits = profile_color_bits[indicator_state.active_device];
            switch ((led_timer_steps >> 4) & 0x3) {
            case 0:
                set_indicator_color(0);
                break;
            case 1:
                set_indicator_color(color_bits);
                break;
            case 2:
                if (indicator_state.connection != 2) {
                    set_indicator_color(0);
                }
                break;
            case 3:
                if (indicator_state.connection != 2) {
                    bt_addr_le_t *addr = zmk_ble_active_profile_addr();
                    set_indicator_color(bt_addr_le_eq(addr, BT_ADDR_LE_ANY) ? 0b001 : 0b100);
                }
                break;
            }

            if (indicator_state.flash_times > 0) {
                indicator_state.flash_times--;
            }
            if (indicator_state.flash_times == 0) {
                indicator_state.connection = 0;
            }
        } else if (indicator_state.battery < 10) {
            if ((led_timer_steps & 0x1f) == 0xf) {
                set_indicator_color(0b001);
            } else if ((led_timer_steps & 0x1f) == 0x1f) {
                set_indicator_color(0);
            }
        } else {
            set_indicator_color((indicator_state.keylock & CAPSLOCK_BIT) ? 0b101 : 0);
        }
    }
}

K_THREAD_DEFINE(led_process_tid, 1024, led_process_thread, NULL, NULL, NULL,
                K_LOWEST_APPLICATION_THREAD_PRIO, 0, 100);

static void klink_indicator_init_thread(void) {
    indicator_state.battery = 111;
    ble_active_profile_update();
}

K_THREAD_DEFINE(klink_indicator_init_tid, 1024, klink_indicator_init_thread, NULL, NULL, NULL,
                K_LOWEST_APPLICATION_THREAD_PRIO, 0, 200);
