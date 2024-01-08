#include <Arduino_GFX_Library.h>
#include <EncButton.h>
#include <MPU6050.h>
#include <Wire.h>
#include "font.h"

#define ENABLED_PIN 4

#define MPU_INTERRUPT 3
#define ROLL_THRESHOLD 10000
#define SHAKE_PERIOD 500

#define BATTARY_PIN A3
#define BATTARY_X 109
#define BATTARY_Y 200

Button btn(14, INPUT);
MPU6050 mpu;
Arduino_DataBus *bus = create_default_Arduino_DataBus();
//  CS:  9, DC:  8, RST:  7, BL:  6, SCK: 13, MOSI: 11, MISO: 12
Arduino_GFX *gfx = new Arduino_GC9A01(bus, 7 /* RST */, 3 /* rotation */, true /* IPS */);

unsigned long lastActionTime;

uint8_t getPosition(char *num)
{
    uint8_t pos = 0;
    int size = strlen(num);
    for (uint8_t i = 0; i < size; i++)
    {
        if (num[i] == 49)
        {
            pos += 14;
        }
        else if (num[i] == 45)
        {
            pos += 5;
        }
    }
    return 33 + (3 - size) * 29 + pos;
}

void printDays(uint16_t color, char *days)
{
    gfx->setTextColor(color);
    gfx->setTextSize(3);
    gfx->setCursor(getPosition(days), 155);
    gfx->println(days);
}

void printSize(uint16_t color, char *size)
{
    gfx->setTextColor(color);
    gfx->setTextSize(1);
    if (strlen(size) == 1)
    {
        gfx->setCursor(111, 30);
    }
    else
    {
        gfx->setCursor(102, 30);
    }
    gfx->println(size);
}

void printDebugLines()
{
    for (int i = 0; i <= 48; i++)
    {
        gfx->drawLine(i * 5, 0, i * 5, 240, GREEN);
    }
    gfx->drawCircle(120, 120, 119, BLUE);
    gfx->drawLine(120, 0, 120, 240, BLUE);
    gfx->drawLine(0, 120, 240, 120, BLUE);
}

void printBattary()
{
    gfx->drawRect(BATTARY_X, BATTARY_Y, 25, 14, WHITE);
    gfx->fillRect(BATTARY_X + 25, BATTARY_Y + 4, 2, 6, WHITE);
    gfx->fillRect(BATTARY_X + 3, BATTARY_Y + 3, 5, 8, RED);
}

void clearBattary()
{
    gfx->fillRect(BATTARY_X - 4, BATTARY_Y - 4, 35, 22, BLACK);
}

void setup(void)
{
    analogReference(INTERNAL);
    pinMode(ENABLED_PIN, OUTPUT);
    digitalWrite(ENABLED_PIN, HIGH);
    lastActionTime = millis();

    btn.setBtnLevel(HIGH);
    Serial.begin(9600);

    randomSeed(analogRead(0));

    if (!gfx->begin())
    {
        Serial.println("gfx->begin() failed!");
    }
    gfx->fillScreen(BLACK);
    gfx->setFont(&Font);

    Wire.begin();
    mpu.initialize();
    Serial.println(mpu.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);

    mpu.setXAccelOffset(-5803);
    mpu.setYAccelOffset(-378);
    mpu.setZAccelOffset(1850);
    mpu.setXGyroOffset(19);
    mpu.setYGyroOffset(7);
    mpu.setZGyroOffset(-34);

    // printDebugLines();
}

void loop()
{
    static bool needDraw = true;
    static int sizeIndex = 0;
    static int randNumber = 0;

    static uint16_t color = RGB565(0, 255, 0);
    static char days[5] = "-";
    static char *size = "XS";

    static int16_t ax, ay, az, gx, gy, gz;
    static float X, Y, Z, totalAccel;

    static uint32_t shakeCount = 0;
    static unsigned long shakeTimer;

    static bool isLowBattary = false;

    btn.tick();

    if (btn.hold())
    {
        digitalWrite(ENABLED_PIN, LOW);
        gfx->fillScreen(BLACK);
    }

    if (btn.click())
    {
        lastActionTime = millis();

        if (++sizeIndex > 4)
        {
            sizeIndex = 0;
        }

        switch (sizeIndex)
        {
        case 0:
            size = "XS";
            color = RGB565(0, 255, 0);
            break;

        case 1:
            size = "S";
            color = RGB565(127, 245, 66);
            break;

        case 2:
            size = "M";
            color = RGB565(255, 235, 132);
            break;

        case 3:
            size = "L";
            color = RGB565(255, 118, 66);
            break;

        case 4:
            size = "XL";
            color = RGB565(255, 0, 0);
            break;
        }
        strcpy(days, "-");
        needDraw = true;
    }

    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    X = 0;
    Y = 0;
    Z = 0;
    for (int i = 0; i < 10; i++)
    {
        X += ax;
        Y += ay;
        Z += az;
        delay(1);
    }
    X /= 10;
    Y /= 10;
    Z /= 10;

    totalAccel = sqrt(X * X + Y * Y + Z * Z) - 16384;

    if (totalAccel > ROLL_THRESHOLD)
    {
        shakeCount++;
    }

    if (shakeCount == 1)
    {
        shakeTimer = millis();
        shakeCount++;
    }

    if (shakeCount == 6)
    {
        switch (sizeIndex)
        {
        case 0:
            randNumber = random(1, 4);
            itoa(randNumber, days, 10);
            break;

        case 1:
            randNumber = random(4, 8);
            itoa(randNumber, days, 10);
            break;

        case 2:
            randNumber = random(8, 15);
            itoa(randNumber, days, 10);
            break;

        case 3:
            randNumber = random(15, 30);
            itoa(randNumber, days, 10);
            break;

        case 4:
            randNumber = random(30, 100);
            itoa(randNumber, days, 10);
            break;
        }

        needDraw = true;
        lastActionTime = millis();
        shakeCount = 0;
    }

    if (shakeCount > 0 && millis() - shakeTimer >= SHAKE_PERIOD)
    {
        shakeCount = 0;
    }

    if (needDraw)
    {
        needDraw = false;
        gfx->fillRect(102, 0, 40, 40, BLACK);
        gfx->fillRect(33, 75, 177, 90, BLACK);
        gfx->fillArc(120, 120, 110, 104, 290, 250, color);

        printDays(color, days);
        printSize(color, size);
    }

    if (millis() - lastActionTime > 120000)
    {
        digitalWrite(ENABLED_PIN, LOW);
    }

    if (analogRead(BATTARY_PIN) < 118)
    {
        if (!isLowBattary)
        {
            isLowBattary = true;
            printBattary();
        }
    }
    else
    {
        if (isLowBattary)
        {
            isLowBattary = false;
            clearBattary();
        }
       
    }
}
