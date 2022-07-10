#define GPIOPINOUT ESP32_FORUM_PINOUT
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

const int defaultBrightness = (50*255)/100;

void setup() {
  Serial.begin(115200);
  
  matrix.addLayer(&backgroundLayer); 
  matrix.begin();

  matrix.setBrightness(defaultBrightness);
}

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

void random_noise(const uint duration) {
  const uint16_t speed = 5;
  const uint16_t scale = 400;

  const uint16_t max_dimension = (kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight;
  uint8_t noise[max_dimension][max_dimension];
  uint16_t z = random16();

  unsigned long currentMillis = millis();
  Serial.print("b");

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

void loop() {
  random_noise(60000);
}
