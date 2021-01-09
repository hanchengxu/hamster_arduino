#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SCoop.h>
#include "word.h"

#define OLED_RESET 4
#define SpeadPonit 2 //读取跑圈数字接口
#define T_READ A3 //温度读取口
#define FEED_SERVO 9  //自动喂食舵机
#define FEED_POINT 3 //自动喂食红外识别


Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

//------------全局变量-------------------------------

long lapCount = 0;//总圈数★
long singleLapCount = 0; //单次运动圈数

float totalRun = 0.0; //总里程★

float tempTime1 = 0.0; // 长期处于识别区超时
int lastSts = 1; //default 1

float endTime = 0; //用于计算平均速度
float notRunTime = 0; //用于识别处于未转动状态，超过1 秒则结束一次平均速度计算
boolean calSpeedFlg = false;//计算平均速度flg
float singleSpeed = 0.0; //实时平均速度★
float maxSingleSpeed = 0.0;//最大运动速度
//--------自动喂食用全局变量-------

defineTask(TaskTest1);//定义子线程1

void TaskTest1::setup()//线程1设定
{
  pinMode(SpeadPonit, INPUT);
  pinMode(FEED_SERVO,OUTPUT);
  pinMode(FEED_POINT,INPUT);

}
void TaskTest1::loop()//线程1循环
{
  int currSts = digitalRead(SpeadPonit);
  
  //一圈识别开始
  if (lastSts == 1 && currSts == 0) {
    tempTime1 = millis();//用于识别区超时
    if (calSpeedFlg == false) {
      calSpeedFlg = true; //在这里 开启calSpeedFlg Flg
      singleLapCount = 0; //单次运动圈数，从开启flg开始置为0
      singleSpeed = 0.0;
    }
  }
  //一圈识别结束
  if (lastSts == 0 && currSts == 1 ) {
    float tempTime2 = millis();//用于识别区超时，理论上状态跃迁是[1,0][0,0][0,0][0,1]
    
    //超时条件，如果一直处于识别区域超过1秒，则重置lastSts
    //强制转换为[1,1]状态进入到平均速度结算
    if ((tempTime2 - tempTime1) > 1000 ) {
      lastSts = 1;
    } else {
      //未超时，总圈数增加
      lapCount++;
      totalRun = lapCount * Perimeter;
      singleLapCount++;
      if (calSpeedFlg == true){
        if(singleLapCount > 1) {//第一圈速度不准，忽略
          float tempT = millis();
          float useTime = (tempT - notRunTime) / 1000.0;
          singleSpeed = Perimeter / useTime;
          if(maxSingleSpeed<singleSpeed){
            maxSingleSpeed = singleSpeed;
          }
        }
      }
    }
    notRunTime = millis();//每次完成一圈识别的下一个状态是[1,1],刷新notRunTime,用于计算平均速度
  }

  //运动结束变量清零
  if (lastSts == 1 && currSts == 1) {
    if (calSpeedFlg == true) {
      endTime = millis();
      if ((endTime - notRunTime) > 3000) {
        //大于3秒间隔，识别为一次连贯运动，可以进行平均速度计算了
        calSpeedFlg = false;//关闭计算平均速度flg
        singleSpeed = 0.0; //运动结束时 将实时速度清零
        endTime =0.0;
        notRunTime = 0.0;
        singleLapCount = 0;
      }
    }
  }

  //最后更新lastSts
  lastSts = currSts;
  
  //------------------------
  int feedFlg = digitalRead(FEED_POINT);
//  Serial.println(feedFlg);

  //0 不喂食 1 喂食
  //1:因为黑色食盆反射率，食盆空不会触发红外所有返回1，如果有食物反射率不同，则会返回0(这块与直觉不同)
  //拿掉食盆也会因为反射率原因返回0
  if(feedFlg == 1){

    int myangle1=map(130,0,180,500,2480);
    digitalWrite(FEED_SERVO,HIGH);//将舵机接口电平至高
    sleep(2.48);//延时脉宽值的微秒数
    digitalWrite(FEED_SERVO,LOW);//将舵机接口电平至低

//    sleep(500);
  }
  if(feedFlg == 0){
    int myangle1=map(90,0,180,500,2480);
    digitalWrite(FEED_SERVO,HIGH);//将舵机接口电平至高
    delayMicroseconds(myangle1);//延时脉宽值的微秒数
    digitalWrite(FEED_SERVO,LOW);//将舵机接口电平至低
  }
  
  
  
  sleep(10);
}

void setup() {
  Serial.begin(9600);
  mySCoop.start();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);//开像素点发光
  display.clearDisplay();//清屏
}

void loop() {
  yield();
  
  float temperature = analogRead(T_READ); //读取传感器获取温度模拟值
  temperature = temperature * (5.0 / 1023.0) * 100; //获取摄氏度
  //  Serial.println(Perimeter);
  display.clearDisplay();
  display.drawRect(0, 0, 128, 22, WHITE); //绘制边框
  display.drawBitmap(0, -4, cangshu, 32, 32, WHITE); //仓鼠图标

  display.drawBitmap(30, 7, wen, 12, 12, WHITE); //温度显示
  display.drawBitmap(42, 7, du, 12, 12, WHITE);
  display.setCursor(56, 5);
  display.setTextColor(WHITE);
  char t_char[6];
  dtostrf(temperature, 2, 1, t_char);
  display.setTextSize(2);
  display.print(t_char);
  display.drawBitmap(105, 8, sheshidu, 12, 12, WHITE);

  //运动速度相关显示
  display.drawBitmap(1, 25, zong, 12, 12, WHITE);
  display.drawBitmap(13, 25, li, 12, 12, WHITE);
  display.drawBitmap(25, 25, cheng, 12, 12, WHITE);
  display.setCursor(38, 30);
  display.setTextSize(1);
  char totalRun_char[6];
  if(totalRun>999){
  //超过999米变更为km单位
    dtostrf(totalRun/1000, 1, 3, totalRun_char);
    strcat(totalRun_char, " km");
    display.print(totalRun_char);
  }else{
    dtostrf(totalRun, 1, 1, totalRun_char);
    strcat(totalRun_char, " m");
    display.print(totalRun_char);
  }

  //最大速度
  display.setCursor(1, 38);
  display.print("Speed(MAX):");
  display.setCursor(67, 38);
  char maxSpeed_char[5];
  dtostrf(maxSingleSpeed, 1, 1, maxSpeed_char);
  strcat(maxSpeed_char, " m/s");
  display.print(maxSpeed_char);
  
  //实时速度
  display.setCursor(1, 46);
  display.print("Speed:");
  display.setCursor(40, 46);
  char currSpeed_char[5];
  dtostrf(singleSpeed, 1, 1, currSpeed_char);
  strcat(currSpeed_char, " m/s");
  display.print(currSpeed_char);

  display.setCursor(1, 54);
  display.print("lapCount:");
  display.setCursor(55, 54);
  display.print(lapCount);

  display.display();//屏幕显示

  sleep(1000);//每一秒刷新一次屏幕
}
