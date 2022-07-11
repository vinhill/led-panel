#pragma once

#include <MatrixHardware_ESP32_V0.h>
#include <SmartMatrix.h>
#include <FastLED.h>

#define COLOR_DEPTH 24  // 24 or 48
const uint16_t kMatrixWidth = 64;
const uint16_t kMatrixHeight = 32;
const uint8_t kRefreshDepth = 36;  // Tradeoff color quality vs refresh rate, max brightness, RAM usage. On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;  // not used on ESP32, leave default 4
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);
const uint8_t kBackgroundLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);

SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

const int defaultBrightness = (5*255)/100;

void random_pixels(const uint duration) {
    unsigned long currentMillis = millis();

    while (millis() - currentMillis < duration) {
        int x0, y0;

        rgb24 color;
        float fraction = ((float)millis() - currentMillis) / ((float)duration / 2);

        if (millis() - currentMillis < duration / 2) {
            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;
        }
        else {
            color.red = 0;
            color.green = 255 - 255.0 / 2 * (fraction - 1.0);
            color.blue = 255.0 * (fraction - 1.0);
        }

        for (int i = 0; i < 20; i++) {
            x0 = random(matrix.getScreenWidth());
            y0 = random(matrix.getScreenHeight());

            backgroundLayer.drawPixel(x0, y0, color);
        }
        backgroundLayer.swapBuffers();
    }
}

rgb24 to_rgb(uint8_t h, uint8_t s, uint8_t v) {
    CHSV hsv(h, s, v);
    CRGB rgb;
    hsv2rgb_rainbow(hsv, rgb);
    rgb24 color = {rgb.r, rgb.g, rgb.b};
    return color;
}

void perlin_noise(const uint duration) {
    const uint16_t speed = 5;
    const uint16_t scale = 400;

    const uint16_t max_dimension = (kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight;
    uint8_t noise[max_dimension][max_dimension];
    uint16_t z = random16();

    unsigned long currentMillis = millis();

    while (millis() - currentMillis < duration) {

        for(int i = 0; i < max_dimension; i++) {
        int ioffset = scale * i;
        for(int j = 0; j < max_dimension; j++) {
            int joffset = scale * j;
            noise[i][j] = inoise8(ioffset, joffset, z);
        }
        }
        z += speed;

        for(int i = 0; i < kMatrixWidth; i++) {
        for(int j = 0; j < kMatrixHeight; j++) {
            rgb24 color = to_rgb(noise[i][j] * 2 - 60, noise[i][j], noise[j][i]);
            backgroundLayer.drawPixel(i, j, color);
        }
        }

        backgroundLayer.swapBuffers();
    }
}

template<typename T>
int arr_indexof(const T arr[], const T value, const int size) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == value) {
            return i;
        }
    }
    return -1;
}

uint8_t random8range(const uint8_t min, const uint8_t max) {
    return min + (random8() % (max - min + 1) );
}

void game_of_life(const uint duration) {
    const int birth = 3;
    const int survive[] = {2, 3};
    uint8_t alive_factor = random8range(255*0.20, 255*0.5);
    const int generation_duration = 300;

    uint8_t gol[kMatrixWidth][kMatrixHeight];
    
    for (int i = 0; i < kMatrixWidth; i++) {
        for (int j = 0; j < kMatrixHeight; j++) {
            gol[i][j] = random8() > alive_factor;
        }
    }

    unsigned long currentMillis = millis();

    const uint16_t len_survive = sizeof(survive) / sizeof(survive[0]);
    while(millis() - currentMillis < duration) {

        for (int i = 0; i < kMatrixWidth; i++) {
            for (int j = 0; j < kMatrixHeight; j++) {
                int neighbors = 0;
                for (int k = -1; k <= 1; k++) {
                    for (int l = -1; l <= 1; l++) {
                        if (k == 0 && l == 0) {
                            continue;
                        }
                        if (i + k < 0 || i + k >= kMatrixWidth || j + l < 0 || j + l >= kMatrixHeight) {
                            continue;
                        }
                        if (gol[i + k][j + l]) {
                            neighbors++;
                        }
                    }
                }
                if (neighbors == birth) {
                    gol[i][j] = 1;
                }else if (gol[i][j] && arr_indexof(survive, neighbors, len_survive) != -1) {
                    gol[i][j] = 1;
                }else {
                    gol[i][j] = 0;
                }
            }
        }

        for (int i = 0; i < kMatrixWidth; i++) {
            for (int j = 0; j < kMatrixHeight; j++) {
                if (gol[i][j]) {
                    backgroundLayer.drawPixel(i, j, {255,255,255});
                }
                else {
                    backgroundLayer.drawPixel(i, j, {0,0,0});
                }
            }
        }

        backgroundLayer.swapBuffers();

        delay(generation_duration);
    }
}

void wolfram(const uint duration) {
    const float init_alive = 0.5;
    const uint32_t rule = random16() << 16 | random16();
    // 1 or 2 neighbours to the left as well as right
    const int n = (random8() > 128) + 1;
    const bool init_onepoint = random8() > 128;

    uint8_t* universe = (uint8_t*)malloc(kMatrixWidth);

    if (init_onepoint) {
        for (int i = 0; i < kMatrixWidth; i++) {
            universe[i] = 0;
        }
        universe[kMatrixWidth / 2] = 1;
    }
    else {
        const int alive_factor = 255 - (255 * init_alive);
        for (int i = 0; i < kMatrixWidth; i++) {
            universe[i] = random8() > alive_factor;
        }
    }

    backgroundLayer.fillScreen({0,0,0});
    for (int i = 0; i < kMatrixWidth; i++) {
        if (universe[i]) {
            backgroundLayer.drawPixel(i, 0, {255,255,255});
        }
    }

    for (int generation = 1; generation < kMatrixHeight; generation++) {
        uint8_t* last_universe = universe;
        universe = (uint8_t*)malloc(kMatrixWidth);

        for (int i = 0; i < kMatrixWidth; i++) {
            unsigned int rule_idx = 0;
            for (int k = -n; k <= n; k++) {
                if (i + k < 0 || i + k >= kMatrixWidth) {
                    continue;
                }
                rule_idx = (rule_idx << 1) | last_universe[i + k];
            }
            universe[i] = (rule >> rule_idx) & 1;
        }

        for (int i = 0; i < kMatrixWidth; i++) {
            if (universe[i]) {
                backgroundLayer.drawPixel(i, generation, {255,255,255});
            }
            else {
                backgroundLayer.drawPixel(i, generation, {0,0,0});
            }
        }

        backgroundLayer.swapBuffers();

        free(last_universe);
        delay(duration / (kMatrixHeight-1));
    }

    free(universe);
}

void setup_led_matrix() {
    matrix.addLayer(&backgroundLayer); 
    matrix.begin();

    matrix.setBrightness(defaultBrightness);
}