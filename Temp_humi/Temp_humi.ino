#include <Wire.h>
#include "ClosedCube_SHT31D.h"
#include <DHT.h>
#include <DHT_U.h>
#include <U8glib.h>

ClosedCube_SHT31D sht3xd;
U8GLIB_SH1106_128X64 u8g(13, 11, 10, 9);  // OLED: SCK = 13, MOSI = 11, CS = 10, A0 = 9

float temp;
float humi;

float hi_temp = 0.0;
float lo_temp = 0.0;

int buttonIn = 7;
bool lineMode = false;

float tempArr[27];
int tempLineIndex = 0;
float tempAvg[10];
int tempAvgIndex = 0;

void setup() {
  Wire.begin();
  Serial.begin(9600);

  pinMode(buttonIn, INPUT);

  u8g.setRot180();
  u8g.setColorIndex(1);  // Instructs the display to draw with a pixel on.

  u8g.setFont(u8g_font_helvR10);

  u8g.firstPage();
  do {
    u8g.drawStr(10, 20, "Starting");
  } while (u8g.nextPage());


  // Initialize device.
  sht3xd.begin(0x44);  // I2C address: 0x44 or 0x45
  sht3xd.readSerialNumber();
}

void loop() {



  // 读取温湿度计参数
  SHT31D result = sht3xd.readTempAndHumidity(SHT3XD_REPEATABILITY_LOW, SHT3XD_MODE_CLOCK_STRETCH, 50);

  if (result.error == SHT3XD_NO_ERROR) {
    temp = result.t;
    humi = result.rh;
    int tempForPrint = temp * 100;
    int humiForPrint = humi * 100;
    Serial.print(tempForPrint);
    Serial.print(",");
    Serial.println(humiForPrint);

    // 初始化
    if (hi_temp == 0.0) {
      initData();
    }

    // 判断最高最低温度是否需要替换
    getHiLoTemp();

    // 计算历史温度数据数组
    calcTemp();

    // 判断执行模式
    drawNums();
  }

  delay(1000);
}

void initData() {
  hi_temp = temp;
  lo_temp = temp;

  for (int i = 0; i < 32; i++) {
    tempArr[i] = temp;
  }
}

void getHiLoTemp() {

  if (hi_temp < temp) {
    hi_temp = temp;
  }

  if (lo_temp > temp) {
    lo_temp = temp;
  }
}

void calcTemp() {
  if (tempAvgIndex < 9) {
    tempAvg[tempAvgIndex] = temp;
    tempAvgIndex++;
  } else {
    tempAvg[tempAvgIndex] = temp;
    tempAvgIndex = 0;
    float total = 0.0;
    for (int i = 0; i < 10; i++) {
      total += tempAvg[i];
    }
    total /= 10.0;

    if (tempLineIndex < 26) {
      tempArr[tempLineIndex] = total;
      tempLineIndex++;
    } else {
      for (int i = 0; i < 26; i++) {
        tempArr[i] = tempArr[i + 1];
      }
      tempArr[26] = total;
    }
  }
}

void drawNums() {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_04b_24);
    u8g.setPrintPos(0, 10);  // 对应 x，y 轴值
    u8g.print("LO TEMP:");
    u8g.setPrintPos(32, 10);
    u8g.print(lo_temp);
    u8g.setPrintPos(65, 10);  // 对应 x，y 轴值
    u8g.print("HI TEMP:");
    u8g.setPrintPos(97, 10);
    u8g.print(hi_temp);

    u8g.setFont(u8g_font_helvR08);
    u8g.setPrintPos(0, 40);  // 对应 x，y 轴值
    u8g.print("TEMP(oC):");
    u8g.setPrintPos(73, 40);
    u8g.print(temp);
    u8g.setPrintPos(0, 55);  // 对应 x，y 轴值
    u8g.print("HUMI(%) :");
    u8g.setPrintPos(73, 55);
    u8g.print(humi);

  } while (u8g.nextPage());
}

void drawLine() {

  float unitTemp, midTemp, upBorder, loBorder;

  if (hi_temp == lo_temp) {
    unitTemp = 30;
    midTemp = hi_temp;
    upBorder = hi_temp + 1;
    loBorder = hi_temp - 1;
  } else {
    unitTemp = 60.0 / (hi_temp - lo_temp);
    midTemp = (hi_temp + lo_temp) / 2;
    upBorder = hi_temp;
    loBorder = lo_temp;
  }



  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_04b_24);
    u8g.setPrintPos(0, 6);  // 对应 x，y 轴值
    u8g.print(upBorder);
    u8g.setPrintPos(0, 32);
    u8g.print(midTemp);
    u8g.setPrintPos(0, 62);
    u8g.print(loBorder);

    u8g.drawLine(18, 2, 18, 62);
    u8g.drawLine(18, 62, 126, 62);
  } while (u8g.nextPage());
}
