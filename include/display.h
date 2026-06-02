#pragma once
// ST7789 1.47" landscape 320x172, driven by TFT_eSPI
#include <TFT_eSPI.h>

// --- RGB565 colour helper ---
inline uint16_t C(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Palette
static const uint16_t COL_BG     = C(12,  12,  18);
static const uint16_t COL_CARD   = C(26,  26,  36);
static const uint16_t COL_BORDER = C(50,  50,  70);
static const uint16_t COL_PURPLE = C(127, 119, 221);
static const uint16_t COL_TEAL   = C(29,  158, 117);
static const uint16_t COL_CORAL  = C(216, 90,  48);
static const uint16_t COL_AMBER  = C(239, 159, 39);
static const uint16_t COL_GRAY   = C(136, 135, 128);
static const uint16_t COL_DIM    = C(55,  55,  75);
static const uint16_t COL_WHITE  = TFT_WHITE;
static const uint16_t COL_BLACK  = TFT_BLACK;

// Layout (landscape 320x172)
static const int16_t LCD_W = 320;
static const int16_t LCD_H = 172;
static const int16_t HDR_H = 18;
static const int16_t CY    = 19;
static const int16_t PAD   = 8;

// --- Shared structs (used by main.cpp too) ---
struct MagData { float bx, by, bz, magnitude, temp; };
struct Striker { char name[24]; float magnitude; };

// --- Helpers ---
static void drawHeader(TFT_eSPI& tft, const char* title, uint8_t dotIdx, uint8_t dotTotal) {
    tft.fillRect(0, 0, LCD_W, HDR_H, COL_CARD);
    tft.drawFastHLine(0, HDR_H, LCD_W, COL_BORDER);
    tft.setTextColor(COL_PURPLE, COL_CARD);
    tft.setTextSize(1);
    tft.setCursor(PAD, 5);
    tft.print(title);
    for (uint8_t i = 0; i < dotTotal; i++) {
        uint16_t c = (i == dotIdx) ? COL_PURPLE : COL_DIM;
        tft.fillRect(LCD_W - PAD - (dotTotal - i) * 9, 7, 5, 5, c);
    }
}

static void drawBar(TFT_eSPI& tft, int16_t x, int16_t y, int16_t w, int16_t h,
                    float pct, uint16_t colFg, uint16_t colBg) {
    pct = pct < 0 ? 0 : (pct > 1 ? 1 : pct);
    int16_t filled = (int16_t)(w * pct);
    if (filled > 0) tft.fillRect(x,         y, filled,     h, colFg);
    if (filled < w) tft.fillRect(x + filled, y, w - filled, h, colBg);
}

// --- Screens ---

inline void drawBoot(TFT_eSPI& tft) {
    tft.fillScreen(COL_BG);
    int16_t cx = LCD_W / 2, cy = LCD_H / 2 - 14;
    // Horseshoe magnet shape
    tft.fillRect(cx - 36, cy - 14, 14, 32, COL_PURPLE);
    tft.fillRect(cx + 22, cy - 14, 14, 32, COL_PURPLE);
    tft.fillRect(cx - 36, cy - 14, 72, 14, COL_PURPLE);
    tft.setTextSize(1);
    tft.setTextColor(COL_CORAL, COL_BG);
    tft.setCursor(cx - 32, cy + 22); tft.print("N");
    tft.setTextColor(COL_TEAL, COL_BG);
    tft.setCursor(cx + 24, cy + 22); tft.print("S");

    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setTextSize(2);
    const char* t1 = "KLASK Magnet Tester";
    tft.setCursor((LCD_W - (int16_t)strlen(t1) * 12) / 2, LCD_H - 42);
    tft.print(t1);
    tft.setTextSize(1);
    tft.setTextColor(COL_DIM, COL_BG);
    tft.setCursor((LCD_W - 4 * 6) / 2, LCD_H - 20);
    tft.print("v2.0");
    delay(1800);
}

inline void drawWifiInfo(TFT_eSPI& tft, const char* ssid, const char* ip, const char* mdns) {
    tft.fillScreen(COL_BG);
    drawHeader(tft, "Wi-Fi", 0, 1);

    // Labels at size 1, values at size 2
    int16_t lx = PAD, vx = PAD + 40, y = CY + 6;

    tft.setTextSize(1);
    tft.setTextColor(COL_GRAY, COL_BG);
    tft.setCursor(lx, y + 4); tft.print("SSID");
    tft.setTextSize(2);
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setCursor(vx, y); tft.print(ssid);

    y += 30;
    tft.setTextSize(1);
    tft.setTextColor(COL_GRAY, COL_BG);
    tft.setCursor(lx, y + 4); tft.print("IP");
    tft.setTextSize(2);
    tft.setTextColor(COL_TEAL, COL_BG);
    tft.setCursor(vx, y); tft.print(ip);

    y += 30;
    tft.setTextSize(1);
    tft.setTextColor(COL_GRAY, COL_BG);
    tft.setCursor(lx, y + 4); tft.print("URL");
    tft.setTextSize(2);
    tft.setTextColor(COL_PURPLE, COL_BG);
    tft.setCursor(vx, y);
    tft.print("http://"); tft.print(mdns); tft.print(".local");
}

inline void drawError(TFT_eSPI& tft, const char* msg) {
    tft.fillScreen(COL_BG);
    tft.fillRect(PAD, CY + 10, LCD_W - PAD*2, 30, C(60, 10, 10));
    tft.drawRect(PAD, CY + 10, LCD_W - PAD*2, 30, COL_CORAL);
    tft.setTextColor(COL_CORAL, C(60, 10, 10));
    tft.setTextSize(1);
    tft.setCursor(PAD + 4, CY + 20);
    tft.print(msg);
}

inline void drawLive(TFT_eSPI& tft, const MagData& d, float peak) {
    tft.fillRect(0, CY, LCD_W, LCD_H - CY, COL_BG);
    drawHeader(tft, "Live", 0, 2);

    int16_t y0 = CY + 2;   // = 21, tight to header
    const float MAX_MT = 60.0f;
    char buf[16];

    // ── Row 1: big |B| (left) + x/y/z axis bars (right) ─────────────────
    // The three axis bars show each axis' fraction of |B| magnitude.
    snprintf(buf, sizeof(buf), "%.1f", d.magnitude);
    tft.setTextSize(4);                          // 32 px tall
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setCursor(PAD, y0);
    tft.print(buf);
    int16_t numW = (int16_t)strlen(buf) * 24;   // 24 px per char at size 4
    tft.setTextSize(1);
    tft.setTextColor(COL_GRAY, COL_BG);
    tft.setCursor(PAD + numW + 2, y0 + 18);     // mid-height subscript
    tft.print("mT");

    float norm  = (d.magnitude > 0.5f) ? d.magnitude : MAX_MT;
    float axV[] = {fabsf(d.bx), fabsf(d.by), fabsf(d.bz)};
    const char* axL[] = {"x", "y", "z"};
    for (uint8_t i = 0; i < 3; i++) {
        int16_t vy = y0 + i * 14;
        tft.setTextSize(1);
        tft.setTextColor(COL_GRAY, COL_BG);
        tft.setCursor(140, vy + 1);
        tft.print(axL[i]);
        float pct = axV[i] / norm;
        if (pct > 1.0f) pct = 1.0f;
        tft.fillRect(152, vy + 1, 160, 8, COL_DIM);
        if (pct > 0.0f) tft.fillRect(152, vy + 1, (int16_t)(160 * pct), 8, COL_PURPLE);
    }

    // ── Row 2: full-width progress bar ───────────────────────────────────
    int16_t bar_y = y0 + 44;                    // = 65
    int16_t bar_w = LCD_W - PAD * 2;            // = 304 px
    float   pct   = d.magnitude / MAX_MT;
    uint16_t bar_c = (pct < 0.6f) ? COL_TEAL : (pct < 0.85f ? COL_AMBER : COL_CORAL);
    drawBar(tft, PAD, bar_y, bar_w, 14, pct, bar_c, COL_DIM);
    tft.drawRect(PAD, bar_y, bar_w, 14, COL_BORDER);
    tft.setTextSize(1);
    tft.setTextColor(COL_DIM, COL_BG);
    tft.setCursor(PAD,                    bar_y + 16); tft.print("0");
    tft.setCursor(PAD + bar_w / 2 - 6,    bar_y + 16); tft.print("30");
    tft.setCursor(PAD + bar_w - 24,        bar_y + 16); tft.print("60mT");

    // ── Row 3: Bx / By / Bz in three equal columns (size 2 values) ───────
    int16_t bxyz_y = bar_y + 30;               // = 95
    int16_t col_w  = (LCD_W - PAD * 2) / 3;   // = 101 px
    const char* lbl[] = {"Bx", "By", "Bz"};
    float vals[] = {d.bx, d.by, d.bz};
    for (uint8_t i = 0; i < 3; i++) {
        int16_t cx = PAD + i * col_w;
        tft.setTextSize(1);
        tft.setTextColor(COL_GRAY, COL_BG);
        tft.setCursor(cx, bxyz_y + 4);         // vertically centred label
        tft.print(lbl[i]);
        snprintf(buf, sizeof(buf), "%+.1f", vals[i]);
        tft.setTextSize(2);
        tft.setTextColor(COL_WHITE, COL_BG);
        tft.setCursor(cx + 18, bxyz_y);
        tft.print(buf);
        tft.setTextSize(1);
        tft.setTextColor(COL_GRAY, COL_BG);
        tft.print("mT");
    }

    // ── Row 4: Temp (left) + Peak (right) — large size 3 values ─────────
    int16_t tp_y = bxyz_y + 30;               // = 125
    tft.setTextSize(1);
    tft.setTextColor(COL_GRAY, COL_BG);
    tft.setCursor(PAD, tp_y);
    tft.print("Temp");
    snprintf(buf, sizeof(buf), "%.1f", d.temp);
    tft.setTextSize(3);
    tft.setTextColor(COL_AMBER, COL_BG);
    tft.setCursor(PAD, tp_y + 10);
    tft.print(buf);
    tft.setTextSize(1);
    tft.setTextColor(COL_GRAY, COL_BG);
    tft.print(" C");

    int16_t px = LCD_W / 2;
    tft.setTextSize(1);
    tft.setTextColor(COL_GRAY, COL_BG);
    tft.setCursor(px, tp_y);
    tft.print("Peak");
    snprintf(buf, sizeof(buf), "%.1f", peak);
    tft.setTextSize(3);
    tft.setTextColor(COL_PURPLE, COL_BG);
    tft.setCursor(px, tp_y + 10);
    tft.print(buf);
    tft.setTextSize(1);
    tft.setTextColor(COL_GRAY, COL_BG);
    tft.print("mT");
}

inline void drawStrikers(TFT_eSPI& tft, Striker* strikers, uint8_t count) {
    tft.fillRect(0, CY, LCD_W, LCD_H - CY, COL_BG);
    drawHeader(tft, "Striker", 1, 2);

    if (count == 0) {
        tft.setTextColor(COL_GRAY, COL_BG);
        tft.setCursor((LCD_W - 11 * 6) / 2, 80); tft.print("Keine Daten");
        tft.setTextColor(COL_DIM, COL_BG);
        tft.setCursor((LCD_W - 11 * 6) / 2, 96); tft.print("SAVE:<name>");
        return;
    }

    float best = strikers[0].magnitude;
    int16_t y  = CY + 4;
    char buf[12];

    for (uint8_t i = 0; i < count && i < 5; i++) {   // max 5 rows at larger size
        if (y + 26 > LCD_H - 2) break;
        bool is_best = (i == 0);
        // Badge: 14×14 instead of 10×10
        tft.fillRect(PAD, y + 1, 14, 14, is_best ? COL_PURPLE : COL_DIM);
        tft.setTextColor(COL_BLACK, is_best ? COL_PURPLE : COL_DIM);
        tft.setCursor(PAD + 3, y + 4); tft.print(i + 1);
        // Name at size 1
        char nameShort[11]; strncpy(nameShort, strikers[i].name, 10); nameShort[10] = '\0';
        tft.setTextSize(1);
        tft.setTextColor(is_best ? COL_WHITE : COL_GRAY, COL_BG);
        tft.setCursor(PAD + 18, y + 5); tft.print(nameShort);
        // Value at size 2, right-aligned
        snprintf(buf, sizeof(buf), "%.1f", strikers[i].magnitude);
        tft.setTextSize(2);
        tft.setTextColor(COL_WHITE, COL_BG);
        int16_t vx = LCD_W - PAD - (int16_t)strlen(buf) * 12;
        tft.setCursor(vx, y);
        tft.print(buf);
        tft.setTextSize(1);
        tft.setTextColor(COL_GRAY, COL_BG);
        tft.print("mT");
        // Proportional bar
        int16_t bar_x = PAD + 18 + 66;
        int16_t bw    = vx - bar_x - 6;
        if (bw > 4)
            drawBar(tft, bar_x, y + 18, bw, 5,
                    strikers[i].magnitude / best,
                    is_best ? COL_PURPLE : COL_TEAL, COL_DIM);
        y += 26;
    }
}

inline void drawCalConfirm(TFT_eSPI& tft, float ox, float oy, float oz) {
    tft.fillRect(0, CY, LCD_W, LCD_H - CY, COL_BG);
    drawHeader(tft, "Kalibrierung", 0, 1);
    tft.fillRect(PAD, CY + 6, LCD_W - PAD*2, 26, COL_CARD);
    tft.drawRect(PAD, CY + 6, LCD_W - PAD*2, 26, COL_TEAL);
    tft.setTextColor(COL_TEAL, COL_CARD);
    tft.setCursor((LCD_W - 17*6)/2, CY + 10); tft.print("Nullpunkt gesetzt");
    tft.setTextColor(COL_WHITE, COL_CARD);
    tft.setCursor((LCD_W - 13*6)/2, CY + 22); tft.print("Offsets aktiv");
    char buf[16];
    int16_t col_w = (LCD_W - PAD*2) / 3;
    const char* lbl[] = {"ox", "oy", "oz"};
    float ovals[] = {ox, oy, oz};
    for (uint8_t i = 0; i < 3; i++) {
        int16_t cx = PAD + i * col_w;
        tft.setTextColor(COL_GRAY, COL_BG);
        tft.setCursor(cx, CY + 42); tft.print(lbl[i]);
        snprintf(buf, sizeof(buf), "%.2fmT", ovals[i]);
        tft.setTextColor(COL_WHITE, COL_BG);
        tft.setCursor(cx, CY + 54); tft.print(buf);
    }
}
