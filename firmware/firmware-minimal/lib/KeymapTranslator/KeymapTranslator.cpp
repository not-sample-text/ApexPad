#include "KeymapTranslator.h"

// Hardcoded LUT mappings
struct KeyMapping {
    const char* label;
    uint8_t code;
};

// Aliases included to support natural language strings from the web UI
static const KeyMapping masterLut[] = {
    {"A", 4}, {"B", 5}, {"C", 6}, {"D", 7}, {"E", 8},
    {"F", 9}, {"G", 10}, {"H", 11}, {"I", 12}, {"J", 13},
    {"K", 14}, {"L", 15}, {"M", 16}, {"N", 17}, {"O", 18},
    {"P", 19}, {"Q", 20}, {"R", 21}, {"S", 22}, {"T", 23},
    {"U", 24}, {"V", 25}, {"W", 26}, {"X", 27}, {"Y", 28},
    {"Z", 29},
    {"1", 30}, {"2", 31}, {"3", 32}, {"4", 33}, {"5", 34},
    {"6", 35}, {"7", 36}, {"8", 37}, {"9", 38}, {"0", 39},
    {"ENTER", 40}, {"RETURN", 40},
    {"ESC", 41}, {"ESCAPE", 41},
    {"BACKSPACE", 42},
    {"TAB", 43},
    {"SPACE", 44},
    {"MINUS", 45}, {"-", 45},
    {"EQUAL", 46}, {"=", 46},
    {"LBRACKET", 47}, {"[", 47},
    {"RBRACKET", 48}, {"]", 48},
    {"BACKSLASH", 49}, {"\\", 49},
    {"SEMICOLON", 51}, {";", 51},
    {"QUOTE", 52}, {"'", 52},
    {"GRAVE", 53}, {"`", 53},
    {"COMMA", 54}, {",", 54},
    {"PERIOD", 55}, {".", 55},
    {"SLASH", 56}, {"/", 56},
    {"F1", 58}, {"F2", 59}, {"F3", 60}, {"F4", 61},
    {"F5", 62}, {"F6", 63}, {"F7", 64}, {"F8", 65},
    {"F9", 66}, {"F10", 67}, {"F11", 68}, {"F12", 69},
    {"F13", 104}, {"F14", 105}, {"F15", 106}, {"F16", 107},
    {"F17", 108}, {"F18", 109}, {"F19", 110}, {"F20", 111},
    {"F21", 112}, {"F22", 113}, {"F23", 114}, {"F24", 115}
};

void KeymapTranslator::init() {
    // Static arrays require no runtime initialization
}

static void applyToken(const String& t, HidCode& code) {
    if (t.length() == 0) return;

    // Modifier bitmasks mapping (allows natural terminology)
    if (t == "CTRL" || t == "CONTROL") { code.modifiers |= 0x01; return; }
    if (t == "SHIFT") { code.modifiers |= 0x02; return; }
    if (t == "ALT") { code.modifiers |= 0x04; return; }
    if (t == "GUI" || t == "META" || t == "WIN" || t == "CMD" || t == "WINDOWS") { code.modifiers |= 0x08; return; }

    // Search LUT
    for (size_t i = 0; i < sizeof(masterLut)/sizeof(masterLut[0]); ++i) {
        if (t == masterLut[i].label) {
            code.keycode = masterLut[i].code;
            return;
        }
    }
}

HidCode KeymapTranslator::translate(const String& value) {
    HidCode result = {0, 0};
    
    // Normalize string: Uppercase and remove all spaces
    String s = value;
    s.toUpperCase();
    s.replace(" ", ""); 

    // Split compound shortcuts by the '+' character
    int start = 0;
    int plusIdx = s.indexOf('+');
    
    while (plusIdx != -1) {
        String token = s.substring(start, plusIdx);
        applyToken(token, result);
        start = plusIdx + 1;
        plusIdx = s.indexOf('+', start);
    }
    
    // Process final token
    String finalToken = s.substring(start);
    if (finalToken.length() > 0) {
        applyToken(finalToken, result);
    }
    
    return result;
}
