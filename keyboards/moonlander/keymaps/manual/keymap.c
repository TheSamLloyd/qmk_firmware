#include QMK_KEYBOARD_H
#include "version.h"
#include "raw_hid.h"
#include "rgblight_list.h"

#define MOON_LED_LEVEL LED_LEVEL
#define ____ KC_TRANSPARENT

/* thots :
we want to define a custom struct that has enough supporting functions that we basically are mapping a color to a keytype.
when we set a color, I want to be able to look at the keycode for the key I'm setting and then set the color based on the type of key (unless overridden)
like: setcolor(12) -> looks up what keycode is at position 12 -> it's L_CTRL (for instance) -> L_CTRL is in the MODIFIER group -> the key is set with a MODIFIER color
*/
enum custom_keycodes { RGB_SLD = ML_SAFE_RANGE, HSV_0_255_255, HSV_86_255_128, HSV_172_255_255, TOGGLE_GUI, TEST_HID };
enum layers { BASE, SYMB, MEDIA };

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [BASE]  = LAYOUT_moonlander(KC_EQUAL, KC_1, KC_2, KC_3, KC_4, KC_5, KC_LEFT, KC_RIGHT, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINUS, KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, TG(SYMB), TG(SYMB), KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLASH, KC_BSPACE, KC_A, KC_S, KC_D, KC_F, KC_G, KC_DELETE, KC_MEH, KC_H, KC_J, KC_K, KC_L, LT(MEDIA, KC_SCOLON), LGUI_T(KC_QUOTE), KC_LSPO, LGUI_T(KC_Z), KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMMA, KC_DOT, RCTL_T(KC_SLASH), KC_RSPC, LT(SYMB, KC_GRAVE), WEBUSB_PAIR, KC_LALT, KC_LEFT, KC_RIGHT, KC_LALT, LCTL_T(KC_ESCAPE), KC_UP, KC_DOWN, KC_LBRACKET, KC_RBRACKET, MO(SYMB), KC_SPACE, KC_BSPACE, KC_LCTRL, KC_LALT, KC_TAB, KC_ENTER),
    [SYMB]  = LAYOUT_moonlander(KC_ESCAPE, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, ____, ____, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, ____, KC_EXLM, KC_AT, KC_LCBR, KC_RCBR, KC_PIPE, ____, ____, KC_UP, KC_KP_7, KC_KP_8, KC_KP_9, KC_ASTR, KC_F12, ____, KC_HASH, KC_DLR, KC_LPRN, KC_RPRN, KC_GRAVE, ____, ____, KC_DOWN, KC_KP_4, KC_KP_5, KC_KP_6, KC_KP_PLUS, ____, ____, KC_PERC, KC_CIRC, KC_LBRACKET, KC_RBRACKET, KC_TILD, KC_AMPR, KC_KP_1, KC_KP_2, KC_KP_3, KC_BSLASH, ____, ____, KC_COMMA, HSV_0_255_255, HSV_86_255_128, HSV_172_255_255, RGB_MOD, RGB_TOG, ____, KC_DOT, KC_KP_0, KC_EQUAL, ____, RGB_VAD, RGB_VAI, TOGGLE_LAYER_COLOR, RGB_SLD, RGB_HUD, RGB_HUI),
    [MEDIA] = LAYOUT_moonlander(AU_TOG, TEST_HID, ____, ____, ____, ____, ____, ____, ____, ____, ____, ____, ____, RESET, MU_TOG, ____, ____, KC_MS_UP, ____, ____, ____, ____, ____, ____, ____, ____, ____, ____, MU_MOD, ____, KC_MS_LEFT, KC_MS_DOWN, KC_MS_RIGHT, ____, ____, ____, ____, ____, ____, ____, ____, KC_MEDIA_PLAY_PAUSE, ____, ____, ____, ____, ____, ____, ____, ____, KC_MEDIA_PREV_TRACK, KC_MEDIA_NEXT_TRACK, ____, ____, ____, ____, ____, KC_MS_BTN1, KC_MS_BTN2, TOGGLE_GUI, ____, KC_AUDIO_VOL_UP, KC_AUDIO_VOL_DOWN, KC_AUDIO_MUTE, ____, ____, ____, ____, ____, ____, ____, KC_WWW_BACK),
};

extern bool         g_suspend_state;
extern rgb_config_t rgb_matrix_config;
extern led_config_t g_led_config;

void keyboard_post_init_user(void) { rgb_matrix_enable(); }

uint8_t layerBaseColors[][3] = {[BASE] = {HSV_WHITE}, [SYMB] = {HSV_SPRINGGREEN}, [MEDIA] = {HSV_PURPLE}};

#define classes 3
#define maxKeys 4
enum keyTypes { arrowKeys, modifierKeys, deleteKeys, numpadKeys, mouseKeys, specialKeys, multiKeys };
const uint8_t   keyClassColors[][3] = {{HSV_CYAN}, {HSV_YELLOW}, {HSV_RED}, {HSV_CYAN}, {HSV_CYAN}, {HSV_ORANGE}, {HSV_CORAL}};
const uint16_t  arrows[maxKeys]     = {KC_LEFT, KC_RIGHT, KC_UP, KC_DOWN};
const uint16_t  modifiers[maxKeys]  = {KC_TAB, KC_LSPO, KC_RSPC};
const uint16_t  deletes[maxKeys]    = {KC_BSPACE, KC_DELETE};
const uint16_t *keyClasses[classes] = {arrows, modifiers, deletes};

uint16_t getKeycode(int layer, int addr) {
    for (int i = 0; i < MATRIX_ROWS; i++) {
        for (int j = 0; j < MATRIX_COLS; j++) {
            if (g_led_config.matrix_co[i][j] == addr) {
                return keymaps[layer][i][j];
            }
        }
    }
    return 0;
};
int getKeyCategory(uint16_t keycode) {
    if (keycode == 0) {
        return -1;
    }
    // QMK category detection
    if (IS_MOD(keycode)) return modifierKeys;
    if (keycode >= 0x54 && keycode <= 0x85) return numpadKeys;
    if (IS_MOUSEKEY(keycode)) return mouseKeys;
    if ((keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_MOD_MAX) || keycode == TOGGLE_GUI) return specialKeys;
    if ((keycode >= QK_MACRO && keycode <= QK_MACRO_MAX) || (keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX)) return multiKeys;
    for (int class = 0; class < sizeof(keyClasses) / sizeof(*keyClasses); class ++) {
        for (int key = 0; key < maxKeys; key++) {
            if (keycode == keyClasses[class][key]) {
                return class;
            } else if (keyClasses[class][key] == 0) {
                break;
            }
        }
    }
    return -1;
}

void set_layer_color(int layer) {
    float f      = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
    HSV   hsv    = {.h = layerBaseColors[layer][0], .s = layerBaseColors[layer][1], .v = layerBaseColors[layer][2]};
    RGB   rgbDef = hsv_to_rgb(hsv);
    for (int i = 0; i < DRIVER_LED_TOTAL; i++) {
        int class = getKeyCategory(getKeycode(layer, i));
        if (class != -1) {
            HSV hsv = {
                .h = keyClassColors[class][0],
                .s = keyClassColors[class][1],
                .v = keyClassColors[class][2],
            };
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, f * rgb.r, f * rgb.g, f * rgb.b);
        } else {
            rgb_matrix_set_color(i, f * rgbDef.r, f * rgbDef.g, f * rgbDef.b);
        }
    }
}

void rgb_matrix_indicators_user(void) {
    if (g_suspend_state || keyboard_config.disable_layer_led) {
        return;
    }
    switch (biton32(layer_state)) {
        case BASE:
            set_layer_color(BASE);
            break;
        case SYMB:
            set_layer_color(SYMB);
            break;
        case MEDIA:
            set_layer_color(MEDIA);
            break;
        default:
            if (rgb_matrix_get_flags() == LED_FLAG_NONE) rgb_matrix_set_color_all(0, 0, 0);
            break;
    }
}

bool    GUI_ENABLED = true;
uint8_t testdata[]  = {255, 128, 0};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case RGB_SLD:
            if (record->event.pressed) {
                rgblight_mode(1);
            }
            return false;
        case HSV_0_255_255:
            if (record->event.pressed) {
                rgblight_mode(1);
                rgblight_sethsv(0, 255, 255);
            }
            return false;
        case HSV_86_255_128:
            if (record->event.pressed) {
                rgblight_mode(1);
                rgblight_sethsv(86, 255, 128);
            }
            return false;
        case HSV_172_255_255:
            if (record->event.pressed) {
                rgblight_mode(1);
                rgblight_sethsv(172, 255, 255);
            }
            return false;
        case TOGGLE_GUI:
            if (record->event.pressed) {
                switch (GUI_ENABLED) {
                    case true:
                        process_magic(GUI_OFF, record);
                        GUI_ENABLED = false;
                        return false;
                    case false:
                        process_magic(GUI_ON, record);
                        GUI_ENABLED = true;
                        return false;
                }
            }
        case TEST_HID:
            if (record->event.pressed) {
                raw_hid_receive(testdata, 3);
                return false;
            }
    }
    return true;
}
const int LEDS[] = {11, 7, 12, 17};
void      raw_hid_receive(uint8_t *data, uint8_t length) {
    // on data receive from HID info
    raw_hid_send(data, length);
    // send back data as ACK
    // want 1 uint8_t data in [0 255] to set RGB for WASD
    switch (length) {
        case 1:
            for (int led = 0; led < 4; led++) {
                rgb_matrix_set_color(LEDS[led], 1.0 - (float)*data / 255.0, (float)*data / 255.0, 0);
            }
            break;
        case 2:
            break;
        case 3:
            for (int led = 0; led < 4; led++) {
                rgb_matrix_set_color(LEDS[led], (float)data[0] / 255.0, (float)data[1] / 255.0, (float)data[2] / 255.0);
            }
            break;
    }
};
