#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SCoop.h>

#define OLED_RESET 4
#define T_READ A3

Adafruit_SSD1306 display(128, 64, &Wire,OLED_RESET);

//画面显示文字
static const uint8_t PROGMEM wen[] = {
0x0F,0x80,0x88,0x80,0x4F,0x80,0x08,0x80,0x0F,0x80,0x80,0x00,0x5F,0xC0,0x15,0x40,
0x35,0x40,0x55,0x40,0x95,0x40,0x3F,0xE0};
static const uint8_t PROGMEM du[] = {
0x02,0x00,0x7F,0xE0,0x48,0x80,0x7F,0xE0,0x48,0x80,0x4F,0x80,0x40,0x00,0x5F,0xC0,
0x48,0x40,0x44,0x80,0x43,0x00,0x9C,0xE0};
static const uint8_t PROGMEM sheshidu[] ={
0x40,0x00,0xA7,0x40,0x48,0xC0,0x10,0x40,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x20,
0x08,0x40,0x07,0x80,0x00,0x00,0x00,0x00};
static const uint8_t PROGMEM cangshu[] ={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x01,0xE0,0x07,0x80,0x07,0x30,0x0C,0xE0,0x0C,0x18,0x18,0x30,0x08,0x08,0x30,0x10,
0x10,0x04,0x20,0x08,0x18,0x04,0x20,0x08,0x08,0x03,0xC0,0x30,0x04,0x0C,0x30,0x20,
0x03,0x10,0x08,0xC0,0x00,0xE0,0x07,0x00,0x00,0x40,0x02,0x00,0x00,0x80,0x01,0x00,
0x01,0x00,0x00,0x80,0x02,0x20,0x04,0x40,0x04,0x00,0x00,0x20,0x04,0x03,0xC0,0x20,
0x08,0x01,0x80,0x10,0x08,0x08,0x10,0x10,0x04,0x04,0x20,0x20,0x04,0x07,0xE0,0x20,
0x02,0x05,0xA0,0x40,0x01,0x05,0xA0,0x80,0x1E,0xFF,0xFF,0x78,0x03,0x80,0x01,0xC0,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t PROGMEM zong[] ={
0x20,0x80,0x11,0x00,0x7F,0xC0,0x40,0x40,0x40,0x40,0x40,0x40,0x7F,0xC0,0x00,0x00,
0x04,0x40,0x52,0x20,0x50,0xA0,0x8F,0x80};
static const uint8_t PROGMEM li[] ={
0x7F,0xC0,0x44,0x40,0x44,0x40,0x7F,0xC0,0x44,0x40,0x44,0x40,0x7F,0xC0,0x04,0x00,
0x7F,0xC0,0x04,0x00,0x04,0x00,0xFF,0xE0};
static const uint8_t PROGMEM cheng[] ={
0x17,0xC0,0xE4,0x40,0x24,0x40,0x27,0xC0,0xF0,0x00,0x27,0xC0,0x71,0x00,0x69,0x00,
0xA7,0xC0,0x21,0x00,0x21,0x00,0x2F,0xE0};

float temperature;//温度
char t_char[10];//为画面显示温度准备的char数组

//----------------------------------------------------
const int SpeadPonit = 2;//读取跑圈数字接口
float static const PROGMEM Perimeter = 0.502;//周长 单位m

int lapCount = 0;//总圈数★
int singleLapCount =0;//单次运动圈数

float totalRunTime = 0.0;//总运动时间，用于计算平均速度
float totalRun = 0.0; //总里程★

int tempTime1 =0 ,tempTime2 =0;// 用于控制计算里程超时
int lastSts =1; //default 1

int timeStart = 0, timeEnd = 0; //用于计算平均速度
int timeNotRun =0;//用于识别处于未转动状态，超过2秒则结束一次平均速度计算
boolean calSpeed = false;//是否正在计算平均速度
float aveSpeed = 0.0; //平均速度★
float singleSpeed = 0.0; //实时平均速度★

defineTask(TaskTest1);//定义子线程1

void TaskTest1::setup()//线程1设定
{
  pinMode(SpeadPonit,INPUT);
}
void TaskTest1::loop()//线程1循环
{
  int currSts = digitalRead(SpeadPonit);
    //一圈识别开始
  if(lastSts == 1 && currSts == 0){
    tempTime1 = millis();
  }
  //一圈识别结束
  if(lastSts == 0 && currSts == 1){
    tempTime2 = millis();
    timeNotRun = millis();//每次完成一圈识别的下一个状态是[1,1],刷新timeNotRun,用于计算平均速度
    //超时条件，如果一直处于识别区域超过1秒，则重置lastSts
    if((tempTime2 - tempTime1)> 1000 ){
      lastSts=1;
    }else{
      lapCount++;
      singleLapCount++;
      totalRun = lapCount*Perimeter;
      if(calSpeed == true){
        int tempT = millis();
        float useTime = (tempT-timeStart)/1000.0;
        singleSpeed = (singleLapCount*Perimeter)/useTime;
      }
    }
  }
  //平均速度计算
  if(lastSts == 1 && currSts == 1){
    if(calSpeed == true){
      int tempT = millis();
      timeEnd= millis();
      if((tempT-timeNotRun)>3000){
        //大于3秒间隔，识别为一次连贯运动，可以进行平均速度计算了
        calSpeed = false;
        //开始计算平均速度
        int useTime = (timeEnd - timeStart)/1000;
        aveSpeed = totalRun/(totalRunTime + useTime);
        singleSpeed =0.0;
      }
    }
  }else{
    if(calSpeed == false){
      calSpeed =true;//在这里 开启calSpeed Flg
      singleLapCount =0;//单次运动圈数，从开启flg开始置为0
      timeStart = millis();
    }
  }
  //最后更新lastSts
  lastSts = currSts;
  sleep(5);
}

void setup() {
  Serial.begin(9600);
  mySCoop.start();
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
  display.setTextColor(WHITE);//开像素点发光
  display.clearDisplay();//清屏
  
}

void loop() {
  yield();
  
  temperature=analogRead(T_READ);//读取传感器获取温度模拟值
  temperature=temperature*(5.0/1023.0)*100;//获取摄氏度
//  Serial.println(temperature);
  display.clearDisplay();
  display.drawRect(0,0,128,22,WHITE);//绘制边框
  display.drawBitmap(0,-4,cangshu,32,32,WHITE);//仓鼠图标
  
  display.drawBitmap(30,7,wen,12,12,WHITE);//温度显示
  display.drawBitmap(42,7,du,12,12,WHITE);
  display.setCursor(56,5);
  display.setTextColor(WHITE);
  dtostrf(temperature,2,1,t_char);
  display.setTextSize(2);
  display.print(t_char);
  display.drawBitmap(105,8,sheshidu,12,12,WHITE);

  //运动速度相关显示
  display.drawBitmap(1,25,zong,12,12,WHITE);
  display.drawBitmap(13,25,li,12,12,WHITE);
  display.drawBitmap(25,25,cheng,12,12,WHITE);
  display.setCursor(38,30);
  display.setTextSize(1);
  char totalRun_char[6];
  dtostrf(totalRun,1,1,totalRun_char);
  strcat(totalRun_char," m");
  display.print(totalRun_char);
  
  display.setCursor(1,38);
  display.print("Speed(AVG):");
  display.setCursor(67,38);
  char aveSpeed_char[5];
  dtostrf(aveSpeed,1,1,aveSpeed_char);
  strcat(aveSpeed_char," m/s");
  display.print(aveSpeed_char);
  
  display.setCursor(1,46);
  display.print("Speed:");
  display.setCursor(40,46);
  char currSpeed_char[5];
  dtostrf(singleSpeed,1,1,currSpeed_char);
  strcat(currSpeed_char," m/s");
  display.print(currSpeed_char);
  
  display.setCursor(1,54);
  display.print("lapCount:");
  display.setCursor(55,54);
  display.print(lapCount);
  
  display.display();//屏幕显示

  
  sleep(1000);//每一秒刷新一次屏幕
}
