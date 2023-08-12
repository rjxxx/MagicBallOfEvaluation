#include <Arduino_GFX_Library.h>
#include <EncButton.h>
#include <Adafruit_MPU6050.h>
#include <Wire.h>
#include "font.h"

EncButton<EB_TICK, 3> btn;
Adafruit_MPU6050 mpu;

Arduino_DataBus *bus = create_default_Arduino_DataBus();
//  CS:  9, DC:  8, RST:  7, BL:  6, SCK: 13, MOSI: 11, MISO: 12
Arduino_GFX *gfx = new Arduino_GC9A01(bus, 7 /* RST */, 0 /* rotation */, true /* IPS */);

uint8_t getPosition(char *num) {
    uint8_t pos = 0;
    int size = strlen(num);
    for (uint8_t i = 0; i < size; i++) {
        if (num[i] == 49) {
            pos += 14;
        } else if (num[i] == 45) {
            pos += 5;
        }
    }
    return 33 + (3 - size) * 29 + pos;
}

void printDays(uint16_t color, char *days) {
    gfx->setTextColor(color);
    gfx->setTextSize(3);
    gfx->setCursor(getPosition(days), 155);
    gfx->println(days);

}

void printSize(uint16_t color, char *size) {
    gfx->setTextColor(color);
    gfx->setTextSize(1);
    if (strlen(size) == 1) {
        gfx->setCursor(111, 30);
    } else {
        gfx->setCursor(102, 30);
    }
    gfx->println(size);
}

void printDebugLines() {
    for (int i = 0; i <= 48; i++) {
        gfx->drawLine(i * 5, 0, i * 5, 240, GREEN);
    }
    gfx->drawCircle(120, 120, 119, BLUE);
    gfx->drawLine(120, 0, 120, 240, BLUE);
    gfx->drawLine(0, 120, 240, 120, BLUE);
}

void setup(void) {
    Serial.begin(9600);

    randomSeed(analogRead(0));

    if (!gfx->begin()) {
        Serial.println("gfx->begin() failed!");
    }
    gfx->fillScreen(BLACK);
    gfx->setFont(&Font);


    mpu.begin();
    mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
    mpu.setMotionDetectionThreshold(50);
    mpu.setMotionDetectionDuration(100);
    mpu.setInterruptPinLatch(true);    // Keep it latched.  Will turn off when reinitialized.
    mpu.setInterruptPinPolarity(true);
    mpu.setMotionInterrupt(true);

    //printDebugLines();
}

void loop() {

    static bool needDraw = true;
    static int sizeIndex = 0;
    static int randNumber = 0;

    static uint16_t color = RGB565(0, 255, 0);
    static char days[5] = "-";
    static char *size = "XS";

    btn.tick();

    if (btn.click()) {
        if (++sizeIndex > 4) {
            sizeIndex = 0;
        }

        switch (sizeIndex) {
            case 0:
                size = "XS";
                color = RGB565(0, 255, 0);
                break;

            case 1:
                size = "S";
                color = RGB565(153, 255, 0);
                break;

            case 2:
                size = "M";
                color = RGB565(255, 255, 0);
                break;

            case 3:
                size = "L";
                color = RGB565(255, 102, 0);
                break;

            case 4:
                size = "XL";
                color = RGB565(255, 0, 0);
                break;
        }
        strcpy(days, "-");
        needDraw = true;
    }

    if (mpu.getMotionInterruptStatus()) {
        switch (sizeIndex) {
            case 0:
                randNumber = random(1, 3);
                itoa(randNumber, days, 10);
                break;

            case 1:
                randNumber = random(4, 10);
                itoa(randNumber, days, 10);
                break;

            case 2:
                randNumber = random(10, 20);
                itoa(randNumber, days, 10);
                break;

            case 3:
                randNumber = random(20, 50);
                itoa(randNumber, days, 10);
                break;

            case 4:
                randNumber = random(50, 300);
                itoa(randNumber, days, 10);
                break;
        }
        needDraw = true;
    }

    if (needDraw) {
        needDraw = false;
        gfx->fillRect(102, 0, 40, 40, BLACK);
        gfx->fillRect(33, 75, 177, 90, BLACK);
        gfx->fillArc(120, 120, 110, 104, -70, 250, color);
        printDays(color, days);
        printSize(color, size);
    }
}


