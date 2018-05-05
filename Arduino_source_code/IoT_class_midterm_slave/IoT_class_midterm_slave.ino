#include <SoftwareSerial.h>   // 引用UART程式庫
#include <Servo.h>     // 引用Servo motor程式庫
const uint8_t max_people = 5;  // maximum num of people can be detected
const int package_size = (3*max_people+2)*3;    // first 3 means (x,y,w). last 3 means three decimal digits.
const uint8_t yaw_stride = 3;     // Yaw-servo strides one step (in degree)
const uint8_t pitch_stride = 5;     // Pitch-servo strides one step (in degree)
bool user_button_flag = true;      // false is manual mode, true is auto mode
bool image_flag = false;           // click z-button one time, save image in python
bool video_flag = false;           // pressing z-button, save video in python 
const uint16_t debounce = 200;     // debounce time
const uint16_t long_press = 1000;     // long press time
const uint16_t velocity = 5;       // velocity of servo in manual mode(0~10)
volatile long user_button_previous_time = millis();
volatile long z_button_previous_time = millis();
volatile long manual_previous = millis();
bool first_press_flag = true;
bool video_cmd_be_sent = false;

SoftwareSerial debug(10, 11); // 接收腳, 傳送腳
Servo yaw_servo;  // create a Yaw-axis servo object
Servo pitch_servo;  // create a Pitch-axis servo object
#define YAW_PIN 9
#define PITCH_PIN 6
#define USER_BUTTON_PIN 2     // additional button for user used
#define JOYSTICKER_PIN_X A1    // JoySticker X-axis movement: mid(231+-5), left(<20), right(>450)
#define JOYSTICKER_PIN_Y A2    // JoySticker Y-axis movement  mid(238+-5), up(<20), down(>450)
#define JOYSTICKER_PIN_Z 3    // JoySticker Z-axis press
#define BaudRate 115200   // UART BaudRate
#define SLAVE_BORAD      // this code is for Slave-Board(connect among python and master UNO-board)
 
void setup() {
  Serial.begin(BaudRate);   // 與電腦序列埠連線
  debug.begin(BaudRate);  // connect with other UNO-board via UART
  pinMode(USER_BUTTON_PIN, INPUT_PULLUP);   // default: 1, pressed: 0
  pinMode(JOYSTICKER_PIN_Z, INPUT_PULLUP);    // default: 1, pressed: 0
  pinMode(13, OUTPUT);
  yaw_servo.attach(YAW_PIN); // attaches the yaw_servo on pin 9 to the servo object
  pitch_servo.attach(PITCH_PIN); // attaches the yaw_servo on pin 6 to the servo object
  yaw_servo.write(0);
  pitch_servo.write(0);
}

void sendData_master(int _data)    // send 3 bytes per each call
{
  if(_data < 256)
    if(_data > 99)
      Serial.print(_data, DEC);
    else if(_data > 9)
    {
      Serial.print(0, DEC);
      Serial.print(_data, DEC);
    }
    else
    {
      Serial.print(0, DEC);
      Serial.print(0, DEC);
      Serial.print(_data, DEC);
    }
  return;
}
void sendData_slave(int _data)    // send 3 bytes per each call
{
  if(_data < 256)
    if(_data > 99)
      debug.print(_data, DEC);
    else if(_data > 9)
    {
      debug.print(0, DEC);
      debug.print(_data, DEC);
    }
    else
    {
      debug.print(0, DEC);
      debug.print(0, DEC);
      debug.print(_data, DEC);
    }
  return;
}

void calibrate_cam(int des_x,int des_y)
{
  if(des_x==0 && des_y==0)  return;     // no face detected
  /*Yaw-servo movement*/
  if(des_x < 256)
  {
    if(des_x > 128)   // Right-side
    {
      int tmp = yaw_servo.read();  // read pos of yaw_servo
      if(des_x < 200)
      {
        if(tmp+yaw_stride < 180)
          yaw_servo.write(tmp + yaw_stride);
        else
          yaw_servo.write(180);
      }
      else                // because of too far, move faster
      {
        if(tmp+yaw_stride*1.5 < 180)
          yaw_servo.write(tmp + yaw_stride*1.5);
        else
          yaw_servo.write(180);
      }
    }
    else            // Left-side
    {
      int tmp = yaw_servo.read();   // read pos of yaw_servo
      if(des_x > 56)
      {
        if(tmp-yaw_stride > 0)
          yaw_servo.write(tmp - yaw_stride);
        else
          yaw_servo.write(0);
      }
      else                // because of too far, move faster
      {
         if(tmp-yaw_stride*1.5 > 0)
          yaw_servo.write(tmp - yaw_stride*1.5);
        else
          yaw_servo.write(0); 
      }
    } 
  }

  /*Pitch-servo movement*/
  if(des_y < 256)
  {
    if(des_y > 128)   // Down-side
    {
      int tmp = pitch_servo.read();   // read pos of pitch_servo
      if(des_y < 200)
      {
        if(tmp-pitch_stride > 0)
          pitch_servo.write(tmp - pitch_stride);
        else
          pitch_servo.write(0);
      }
      else                // because of too far, move faster
      {
        if(tmp-pitch_stride*1.5 > 0)
          pitch_servo.write(tmp - pitch_stride*1.5);
        else
          pitch_servo.write(0);
      }
    }
    else            // UP-side
    {
      int tmp = pitch_servo.read();   // read pos of pitch_servo
      if(des_y > 56)
      {
        if(tmp+pitch_stride < 180)
          pitch_servo.write(tmp + pitch_stride);
        else
          pitch_servo.write(180);
      }
      else                // because of too far, move faster
        if(tmp+pitch_stride*1.5 < 180)
          pitch_servo.write(tmp + pitch_stride*1.5);
        else
          pitch_servo.write(180);
    } 
  }
}
void manual_mode()
{
  if((millis() - manual_previous) > (11-velocity)*5)
  {
    uint16_t tmp_x = analogRead(JOYSTICKER_PIN_X);
    uint16_t tmp_y = analogRead(JOYSTICKER_PIN_Y);
    int des_x = 255*(float(tmp_x)/(1024));    // JoyStick_cal_x is mid_val of JOYSTICKER read, mapping it to 255
    int des_y = 255*(float(tmp_y)/(1024));    // JoyStick_cal_y is mid_val of JOYSTICKER read, mapping it to 255
    if(des_x < 200 && des_x > 56)   des_x = 256;          // when JoySticker's movement is too small,doing nothing in Yaw-servo
    if(des_y < 200 && des_y > 56)   des_y = 256;          // when JoySticker's movement is too small,doing nothing in Pitch-servo
    calibrate_cam(des_x, des_y);
    manual_previous = millis();
  }
}

void Master()
{
  if(!digitalRead(JOYSTICKER_PIN_Z))
  {
    if(first_press_flag)
    {
      z_button_previous_time = millis();
      first_press_flag = false;
    }
    else if((millis()-z_button_previous_time) > debounce/2.0 && (millis()-z_button_previous_time) < long_press)    // debounce < t < long_press (touch down)
    {
      image_flag = true;
      video_flag = false; 
    }
    else if((millis()-z_button_previous_time) > long_press)    // t > long_press (long press)
    {
      if(!video_cmd_be_sent)
      {
        debug.print(2, DEC);
        video_cmd_be_sent = true;
        video_flag = true;
        image_flag = false;
      }
    }
  }
  else
  {
    if(image_flag && !video_flag)
    {
      debug.print(1, DEC);
      image_flag = false;
      video_flag = false;
      first_press_flag = true;
    }
    else if(!image_flag && video_flag)
    {
      debug.print(0, DEC);
      image_flag = false;
      video_flag = false;
      first_press_flag = true;
      video_cmd_be_sent = false;
    }
  }
  
  if(!digitalRead(USER_BUTTON_PIN))
  {
    if((millis() - user_button_previous_time) > debounce)
    {
      user_button_flag = !user_button_flag;
      if(user_button_flag)  digitalWrite(13, LOW);
      user_button_previous_time = millis();
    }
  }
  if(!user_button_flag)
  {
    manual_mode();      // Using Joy-sticker to controll the CAM
    digitalWrite(13, HIGH);
  }
  if(debug.available())
  {
    int count = 0;
    int tmp[package_size] = {0};  
    while(count<package_size)
    {
      int _tmp = debug.read();
      if(_tmp != -1)
      {
        tmp[count] = _tmp;
        count++;
      }
    }
    int count2 = 0;
    int val[int(package_size/3)] = {0};                  // +2 means it contains start & stop bytes. 3 means (x,y,w).
    int max_width = 0;
    int destination_x = 0;
    int destination_y = 0;
    for(int i=0;i<package_size;i++)
    {
      if(i%3==2)
      {
        val[count2] = (tmp[i-2]-48)*100 + (tmp[i-1]-48)*10 + (tmp[i]-48);
        if(count2%3 == 0)
        {
          if(count2 != 0)
          {
            if(val[count2] > max_width)
            {
              max_width = val[count2];
              destination_x = val[count2-2];
              destination_y = val[count2-1];
            }
          }
        }
        count2++;
      }
    }

    if(user_button_flag) calibrate_cam(destination_x, destination_y);  // calibrate CAM to NEAREST face
    
    if(val[int(package_size/3) -1]==255)
    {
      sendData_master(val[0]);   // start byte
      for(int i=1;i<int(package_size/3) -1;i++)
      {
        sendData_master(val[i]);
      }
      sendData_master(val[int(package_size/3) -1]);    // stop byte
      val[int(package_size/3) -1] = 0;
    }
    delete tmp,val;
  }
}
void Slave()
{
  if(debug.available())
  {
    int tmp_read = debug.read()-48;
    if(tmp_read > -1)
      Serial.println(tmp_read, DEC);
  }
  if(Serial.available())
  {
    int count = 0;
    int tmp[package_size] = {0};  
    while(count<package_size)
    {
      int _tmp = Serial.read();
      if(_tmp != -1)
      {
        tmp[count] = _tmp;
        count++;
      }
    }
    int count2 = 0;
    int val[int(package_size/3)] = {0};                  // +2 means it contains start & stop bytes. 3 means (x,y,w).
    for(int i=0;i<package_size;i++)
    {
      if(i%3==2)
      {
        val[count2] = (tmp[i-2]-48)*100 + (tmp[i-1]-48)*10 + (tmp[i]-48); 
        count2++;
      }
    }
    if(val[int(package_size/3) -1]==255)
    {
      sendData_slave(val[0]);   // start byte
      for(int i=1;i<int(package_size/3) -1;i++)
      {
        sendData_slave(val[i]);
      }
      sendData_slave(val[int(package_size/3) -1]);    // stop byte
      val[int(package_size/3) -1] = 0;
    }
    delete tmp,val;
  }
}

void loop() {
  #ifdef MASTER_BORAD
    Master();
  #else 
    #ifdef SLAVE_BORAD
      Slave();
    #endif
  #endif
}
