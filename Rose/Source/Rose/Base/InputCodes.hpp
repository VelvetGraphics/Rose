#pragma once

namespace Rose {
    // From glfw3.h

    namespace Key {
        enum Key
        {
            HAT_CENTERED = 0,
            HAT_UP = 1,
            HAT_RIGHT = 2,
            HAT_DOWN = 4,
            HAT_LEFT = 8,
            HAT_RIGHT_UP = HAT_RIGHT | HAT_UP,
            HAT_RIGHT_DOWN = HAT_RIGHT | HAT_DOWN,
            HAT_LEFT_UP = HAT_LEFT | HAT_UP,
            HAT_LEFT_DOWN = HAT_LEFT | HAT_DOWN,

            KEY_UNKNOWN = -1,

            /* Printable keys */
            SPACE = 32,
            APOSTROPHE = 39, /* ' */
            COMMA = 44, /* , */
            MINUS = 45, /* - */
            PERIOD = 46, /* . */
            SLASH = 47, /* / */
            KEY_0 = 48,
            KEY_1 = 49,
            KEY_2 = 50,
            KEY_3 = 51,
            KEY_4 = 52,
            KEY_5 = 53,
            KEY_6 = 54,
            KEY_7 = 55,
            KEY_8 = 56,
            KEY_9 = 57,
            SEMICOLON = 59, /* ; */
            EQUAL = 61, /* = */
            A = 65,
            B = 66,
            C = 67,
            D = 68,
            E = 69,
            F = 70,
            G = 71,
            H = 72,
            I = 73,
            J = 74,
            K = 75,
            L = 76,
            M = 77,
            N = 78,
            O = 79,
            P = 80,
            Q = 81,
            R = 82,
            S = 83,
            T = 84,
            U = 85,
            V = 86,
            W = 87,
            X = 88,
            Y = 89,
            Z = 90,
            LEFT_BRACKET = 91, /* [ */
            BACKSLASH = 92, /* \ */
            RIGHT_BRACKET = 93, /* ] */
            GRAVE_ACCENT = 96, /* ` */
            WORLD_1 = 161, /* non-US #1 */
            WORLD_2 = 162, /* non-US #2 */

            /* Function keys */
            ESCAPE = 256,
            ENTER = 257,
            TAB = 258,
            BACKSPACE = 259,
            INSERT = 260,
            KEY_DELETE = 261,
            RIGHT = 262,
            LEFT = 263,
            DOWN = 264,
            UP = 265,
            PAGE_UP = 266,
            PAGE_DOWN = 267,
            HOME = 268,
            END = 269,
            CAPS_LOCK = 280,
            SCROLL_LOCK = 281,
            NUM_LOCK = 282,
            PRINT_SCREEN = 283,
            PAUSE = 284,
            F1 = 290,
            F2 = 291,
            F3 = 292,
            F4 = 293,
            F5 = 294,
            F6 = 295,
            F7 = 296,
            F8 = 297,
            F9 = 298,
            F10 = 299,
            F11 = 300,
            F12 = 301,
            F13 = 302,
            F14 = 303,
            F15 = 304,
            F16 = 305,
            F17 = 306,
            F18 = 307,
            F19 = 308,
            F20 = 309,
            F21 = 310,
            F22 = 311,
            F23 = 312,
            F24 = 313,
            F25 = 314,
            KEY_KP_0 = 320,
            KEY_KP_1 = 321,
            KEY_KP_2 = 322,
            KEY_KP_3 = 323,
            KEY_KP_4 = 324,
            KEY_KP_5 = 325,
            KEY_KP_6 = 326,
            KEY_KP_7 = 327,
            KEY_KP_8 = 328,
            KEY_KP_9 = 329,
            KEY_KP_DECIMAL = 330,
            KEY_KP_DIVIDE = 331,
            KEY_KP_MULTIPLY = 332,
            KEY_KP_SUBTRACT = 333,
            KEY_KP_ADD = 334,
            KEY_KP_ENTER = 335,
            KEY_KP_EQUAL = 336,
            KEY_LEFT_SHIFT = 340,
            KEY_LEFT_CONTROL = 341,
            KEY_LEFT_ALT = 342,
            KEY_LEFT_SUPER = 343,
            KEY_RIGHT_SHIFT = 344,
            KEY_RIGHT_CONTROL = 345,
            KEY_RIGHT_ALT = 346,
            KEY_RIGHT_SUPER = 347,
            KEY_MENU = 348,

            KEY_LAST = KEY_MENU,
        };

        enum Mod
        {

            SHIFT = 0x0001,
            CONTROL = 0x0002,
            ALT = 0x0004,
            SUPER = 0x0008,
            MOD_CAPS_LOCK = 0x0010,
            MOD_NUM_LOCK = 0x0020,
        };
    } // namespace Key

    namespace Mouse {
        enum Mouse
        {
            BUTTON_1 = 0,
            BUTTON_2 = 1,
            BUTTON_3 = 2,
            BUTTON_4 = 3,
            BUTTON_5 = 4,
            BUTTON_6 = 5,
            BUTTON_7 = 6,
            BUTTON_8 = 7,
            LAST = BUTTON_8,
            LEFT = BUTTON_1,
            RIGHT = BUTTON_2,
            MIDDLE = BUTTON_3,
        };
    }
} // namespace Rose
