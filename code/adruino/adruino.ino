#include <SoftwareSerial.h>
#include <AFMotor.h>
#include <LiquidCrystal_I2C.h>
#include <Password.h>
#include <Wire.h>
//===================================================
#define SCL_PIN 3
#define SDO_PIN 2
//===================================================
#define rxPin 10
#define txPin 11
SoftwareSerial sSerial(rxPin, txPin);
//===================================================
String RxBuffer = "";
String receivedSensorData = "";
byte receivedButtonData1, receivedButtonData2;
byte speed1, speed2;
uint8_t indexOfA, indexOfB, indexOfC;
//int data1, data2, data3, data4;
byte tem, hum;
byte lastSentData1, lastSentData2;
byte press, blin;
//===================================================
#define buzzer 13
#define Temp 0
#define Humid 1
#define Press 2
#define Bli 3
//===================================================
AF_DCMotor motor4(4);
AF_DCMotor motor1(3);
byte PWM1,PWM2;
LiquidCrystal_I2C lcd(0x27, 16, 2);
//===================================================
String newPasswordString; //hold the new password
char newPassword[6]; //charater string of newPasswordString
byte a = 5;
bool value = true;
Password password = Password("123456"); //Enter your password
byte maxPasswordLength = 6;
byte currentPasswordLength = 0;
byte keyPressed = 0, previousKey = 0;
char zkey[11] = {'0','1','2','3','4','5','6','7','8','9'};
//==================================================================
//==================================================================
typedef struct Linker{
  char MenuID;
  char List1[17];
  struct Linker *menulist1; void(*Actuator1) (char, char);
  char List2[17];
  struct Linker *menulist2; void(*Actuator2) (char, char);
} Menu;
  Menu *menu;
//===================================================================
byte threshold,threshold1,threshold2 = 100,threshold3 = 100;
byte thresh,thresh1;
byte th,th1;
byte ko,ko1;
void adjust(char id){
  switch(id){
    case Temp:
    adjust_temperature(); 
    adjust_temperature1(); break;
    case Humid:
    adjust_humidity1();
    adjust_humidity();  break;
    case Press:
    adjust_pressure();  break;
    case Bli:
    adjust_blinder();  break;
  }
}
byte take(char id, byte value){
  byte data;
  switch(id){
    case Temp: 
    data = value;  break;
    case Humid:
    data = value;  break;
  }
  return data;
}
byte take1(char id, byte value){
  byte data;
  switch(id){
    case Temp: 
    data = value; break;
    case Humid:
    data = value;  break;
  }
  return data;
}
//===================================================
void Act(char id, char hey)
{
  static boolean state = false;
  static boolean state1 = false;
  switch(id){
    case Temp:       th = take(Temp, threshold);
                     ko = take1(Temp, thresh);
                      lcd.setCursor(1,0);
                      lcd.print("Set Up         ");
                      lcd.setCursor(2,1);
                      lcd.print("Ready, Sir     ");
                      break;
    case Humid:      th1 = take(Humid, threshold1);
                     ko1 = take1(Humid, thresh1);
                      lcd.setCursor(1,0);
                      lcd.print("Set Up is      ");
                      lcd.setCursor(2,1);
                      lcd.print("Ready, Sir     ");
                      break;
    case Press:      
                    if (hey == 1){  
                      PWM1 = threshold2;
                      lcd.setCursor(12,1);
                      lcd.print("OK!");
                    }
                    else{
                      if ( state )
                        {
                            motor1.setSpeed(PWM1);
                            motor1.run(FORWARD);
                            lcd.setCursor(12,0);
                            lcd.print("ON ");
                            }
                      else
                        {
                            motor1.setSpeed(0);
                            motor1.run(RELEASE);
                            lcd.setCursor(12,0);
                            lcd.print("OFF");
                            }
                      state = ! state;
                    }
                      break;
    case Bli:        if(hey == 1){
                      PWM2 = threshold3;
                      lcd.setCursor(12,1);
                      lcd.print("OK!");
                      }
                      else{
                      if ( state1 )
                        {
                            motor4.setSpeed(PWM2);
                            motor4.run(FORWARD);
                            lcd.setCursor(12,0);
                            lcd.print("ON ");
                            }
                      else
                        {
                            motor4.setSpeed(0);
                            motor4.run(RELEASE);
                            lcd.setCursor(12,0);
                            lcd.print("OFF");
                            }
                      state1 = !state1;
                      }
                      break;
  }
}
void PrintL(const char *str, unsigned char line, unsigned char col){
    lcd.setCursor(col,line);
    lcd.print(str);
}

void MenuDisplay(Menu *menu, unsigned char select)
{
  PrintL(menu->List1,0,0);
  PrintL(menu->List2,1,0);
  PrintL(">", select, 1 );
}

Menu Temperature = {
  Temp,
  "  Up TH:     ",NULL,&Act,
  "  Down TH:   ",NULL,&Act
};

Menu Humidity = {
  Humid,
  "  Up TH      ",NULL,&Act,
  "  Down TH:   ",NULL,&Act
};


Menu PressurePump = {
  Press,
  "  Cfg Mode: ",NULL,&Act,
  "  PWM:      ",NULL,&Act
};

Menu Blinder = {
  Bli,
  "  Cfg Mode: ",NULL,&Act,
  "  PWM:      ",NULL,&Act
};

Menu Actuators = {
  NULL,
  "  PressurePump  ",&PressurePump,NULL,
  "  Blinder       ",&Blinder,NULL
};

Menu Sensors = {
  NULL,
  "  Temperature   ",&Temperature,NULL,
  "  Humidity      ",&Humidity,NULL
};

Menu SubMenu = {
  NULL,
  "  Sensors       ",&Sensors,NULL,
  "  Actuators     ",&Actuators,NULL
};

Menu MainMenu = {
  NULL,
  "Welcome Client  ",&SubMenu,NULL,
  "Press any key to",&SubMenu,NULL
};
//===================================================================
//===================================================================
void warning(){
    if( tem > th || tem < ko) digitalWrite(buzzer, HIGH); delay(1000); digitalWrite(buzzer, LOW); delay(1000);
    if( hum > th1 || hum < ko1) digitalWrite(buzzer, HIGH); delay(1000); digitalWrite(buzzer, LOW); delay(1000);
}
//===================================================================
void database1(byte state, byte speed){
  if(state){
      motor1.setSpeed(speed);
      motor1.run(FORWARD);
      delay(50);
  }else
  {
      motor1.setSpeed(0);
      motor1.run(RELEASE);
      delay(50);
  }
}

void database2(byte state, byte speed){
  if(state){
      motor4.setSpeed(speed);
      motor4.run(FORWARD);
      delay(50);
  }else
  {
      motor4.setSpeed(0);
      motor4.run(RELEASE);
      delay(50);
  }
}
//====================================================================
boolean state3 = true;
void setup() {
  // put your setup code here, to run once:
    pinMode(SCL_PIN, OUTPUT);  
    pinMode(SDO_PIN, INPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);

    Serial.begin(115200);
    sSerial.begin(9600);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(1, 0);
    lcd.print("WELCOME TO THE");
    lcd.setCursor(3, 1);
    lcd.print("GARDEN, SIR");
    delay(2000);
    lcd.clear();
    delay(10);

    

    menu = &MainMenu;
    motor1.setSpeed(200);
    motor1.run(RELEASE);
    motor4.setSpeed(200);
    motor4.run(RELEASE);
    //digitalWrite(buzzer, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
    state3 = true;
    getData();
    database1(receivedButtonData1, speed1);
    database2(receivedButtonData2, speed2);
    //th = take(Temp, threshold);
    //ko = take1(Temp, thresh);
    //th1 = take(Humid, threshold1);
    //ko1 = take1(Humid, thresh1);
    lcd.setCursor(1, 0);
    lcd.print("Temperature:");
    lcd.setCursor(1, 1);
    lcd.print("Humidity:");
    lcd.setCursor(13, 0);
    lcd.print(tem);
    lcd.setCursor(13, 1);
    lcd.print(hum);
    //==================================================
    keyPressed = Read_Keypad();
    keyTone();
    /* If a key has been pressed output it to the serial port */
    if(keyPressed && keyPressed != previousKey)
    {
    Serial.print(keyPressed);
          if(keyPressed == 16){
            lcd.clear();
            delay(10);
              while(true){
                  keyPressed = Read_Keypad();
                  lcd.setCursor(2, 0);
                  lcd.print("ENTER KEY:");
                  keyTone();
                  if(keyPressed && keyPressed != previousKey){
                    switch (keyPressed){
                      case 11: checkPassword(); break;
                      case 12: resetPassword(); break;
                      case 13: resetPassword(); break;
                      case 14: resetPassword(); break;
                      case 15: state3 = false; break;
                      case 16: resetPassword(); break;
                      default: {
                        if(keyPressed==10) keyPressed=0;
                        processNumberKey(zkey[keyPressed]);}
                    }
                  }
                previousKey = keyPressed; keyPressed = 0;
                if(state3 == false) {break;}
              }
          }
    }
    previousKey = keyPressed; keyPressed = 0;
}
//========================================================
//===================================================
//===================================================
//========================================================
void processNumberKey(char key) {
  lcd.setCursor(a, 1);
  lcd.print("*");
  a++;
  if (a == 11) {
    a = 5;
  }
  
  currentPasswordLength++;
  password.append(key);

  if (currentPasswordLength == maxPasswordLength) {
    checkPassword();
  }
}
//========================================================
void checkPassword() {
  delay(500);
  if (password.evaluate()) {
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    lcd.setCursor(0, 0);
    lcd.print("CORRECT PASSWORD");
    lcd.setCursor(0, 1);
    lcd.print("ACCESS SYSTEM");
    delay(2000);
    lcd.clear();
    delay(50);
    a = 5;
    MenuDisplay(menu,2);
    unsigned char select=0;
        while(true)
        {   
            
            keyPressed = Read_Keypad();
            delay(10);
            if(keyPressed && keyPressed != previousKey)
              { Serial.print(keyPressed);
                keyTone();
                const int DesiredMenuIDs[] = {0, 1, 2, 3};
                for (int i = 0; i < 4; i++) {
                      if (menu->MenuID == DesiredMenuIDs[i]) {
                      adjust(menu->MenuID);
                        break; }
                      }
              //////////////////////////////////////////////
                if(keyPressed < 11){
                            if(keyPressed == 2){
                              select = ( select == 0 )? 1 : select - 1;
                              MenuDisplay(menu,select);
                            }
                              if(keyPressed == 10){
                              select = ( select == 1 )? 0 : select + 1;
                              MenuDisplay(menu,select);
                            }
                                if(keyPressed == 7){
                                  switch(select){
                                    case 0 : menu = (menu->menulist1 == NULL)? menu : menu->menulist1; break;
                                    case 1 : menu = (menu->menulist2 == NULL)? menu : menu->menulist2; break;
                                  }
                                  MenuDisplay(menu,select);
                                }
                                  if(keyPressed == 5){
                                    menu = &MainMenu;
                                    MenuDisplay(menu, 2);
                                  }
                                    if(keyPressed == 6){
                                      switch(select){
                                        case 0:
                                              if (menu->Actuator1 != NULL) menu->Actuator1(menu->MenuID, 0); break;
                                        case 1:
                                              if (menu->Actuator2 != NULL) menu->Actuator2(menu->MenuID, 1); break;
                                      }

                                    }
                                      if(keyPressed == 4){ break; }
                }
              }
            previousKey = keyPressed; keyPressed = 0;
        }







  } else {
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    lcd.setCursor(0, 0);
    lcd.print("WRONG PASSWORD!");
    lcd.setCursor(0, 1);
    lcd.print("PLEASE TRY AGAIN");
    delay(2000);
    lcd.clear();
    a = 5;
  }
  resetPassword();
}
//========================================================
//========================================================
void resetPassword() {
  password.reset();
  currentPasswordLength = 0;
  lcd.clear();
  a = 5;
}
//========================================================
byte Read_Keypad(void)
{
  byte Count;
  byte Key_State = 0;

  /* Pulse the clock pin 16 times (one for each key of the keypad) 
     and read the state of the data pin on each pulse */
  for(Count = 1; Count <= 16; Count++)
  {
    digitalWrite(SCL_PIN, LOW); 
      if (!digitalRead(SDO_PIN)){ 
        Key_State = Count;
      } 
    digitalWrite(SCL_PIN, HIGH);
  }  
  
  return Key_State; 
}
//========================================================
//void changePassword() {
//  newPasswordString = "1234";
//  newPasswordString.toCharArray(newPassword, newPasswordString.length() + 1); //convert string to char array
//  password.set(newPassword);
//  resetPassword();
//  lcd.clear();
//  lcd.setCursor(0, 0);
//  lcd.print("Password changed");
//  delay(1000);
//  lcd.clear();
//}

void keyTone()
{
  switch(keyPressed)
  {
    case 1 : tone(13,(keyPressed)*400,50); break;
    case 2 : tone(13,(keyPressed)*400,50); break;
    case 3 : tone(13,(keyPressed)*400,50); break;
    case 4 : tone(13,(keyPressed)*400,50); break;
    case 5 : tone(13,(keyPressed)*400,50); break;
    case 6 : tone(13,(keyPressed)*400,50); break;
    case 7 : tone(13,(keyPressed)*400,50); break;
    case 8 : tone(13,(keyPressed)*400,50); break;
    case 9 : tone(13,(keyPressed)*400,50); break;
    case 10: tone(13,(keyPressed)*400,50); break;
    case 11: tone(13,(keyPressed)*400,50); break;
    case 12: tone(13,(keyPressed)*400,50); break;
    case 13: tone(13,(keyPressed)*400,50); break;
    case 14: tone(13,(keyPressed)*400,50); break;
    case 15: tone(13,(keyPressed)*400,50); break;
    case 16: tone(13,(keyPressed)*400,50); break;
  }
}
char buf[4];
byte adjust_temperature(){
  if(keyPressed == 13)  {
        threshold = (threshold > 100)? 100 : threshold+=5;
        lcd.setCursor(12,0);
        sprintf(buf, "%3d", threshold);
        lcd.print(buf);
        }
  else if(keyPressed == 14) {
        threshold = (threshold == 0)? 0 : threshold-=5;
        lcd.setCursor(12,0);
        sprintf(buf, "%3d", threshold);
        lcd.print(buf);
        }
}
byte adjust_temperature1(){
  if(keyPressed == 15)  {
        thresh = (thresh > 100)? 100 : thresh +=5;
        lcd.setCursor(12,1);
        sprintf(buf, "%3d", thresh);
        lcd.print(buf);
        }
  else if(keyPressed == 16) {
        thresh = (thresh == 0)? 0 : thresh -= 5;
        lcd.setCursor(12,1);
        sprintf(buf, "%3d", thresh);
        lcd.print(buf);
        }
}

byte adjust_humidity(){
  if(keyPressed == 13)  {
        threshold1 = (threshold1 > 100)? 100 : threshold1 += 5;
        lcd.setCursor(12,0);
        sprintf(buf, "%3d", threshold1);
        lcd.print(buf);
        }
  else if(keyPressed == 14) {
        threshold1 = (threshold1 == 0)? 0 : threshold1 -= 5;
        lcd.setCursor(12,0);
        sprintf(buf, "%3d", threshold1);
        lcd.print(buf);
        }      
}
byte adjust_humidity1(){
  if(keyPressed == 15)  {
        thresh1 = (thresh1 > 100)? 100 : thresh1 += 5;
        lcd.setCursor(12,1);
        sprintf(buf, "%3d", thresh1);
        lcd.print(buf);
        }
  else if(keyPressed == 16) {
        thresh1 = (thresh1 == 0)? 0 : thresh1 -= 5;
        lcd.setCursor(12,1);
        sprintf(buf, "%3d", thresh1);
        lcd.print(buf);
        }
}


byte adjust_pressure(){
  if(keyPressed == 13)  {
        threshold2 = (threshold2 == 255)? 255 : threshold2 += 15;
        lcd.setCursor(12,1);
        sprintf(buf, "%3d", threshold2);
        lcd.print(buf);
        }
  else if(keyPressed == 14) {
        threshold2 = (threshold2 == 0)? 0 : threshold2 -= 15;
        lcd.setCursor(12,1);
        sprintf(buf, "%3d", threshold2);
        lcd.print(buf);
        }
}

byte adjust_blinder(){
  if(keyPressed == 13) {
        threshold3 = (threshold3 == 255)? 255 : threshold3 += 15;
        lcd.setCursor(12,1);
        sprintf(buf, "%3d", threshold3);
        lcd.print(buf);
        }
  else if(keyPressed == 14) {
        threshold3 = (threshold3 == 0)? 0 : threshold3 -= 15;
        lcd.setCursor(12,1);
        sprintf(buf, "%3d", threshold3);
        lcd.print(buf);
        }
}
//===================================================
//===================================================
/*void sendData(){
  if (PWM1 != lastSentData1) { // if new data is different from last sent data
    sSerial.println(PWM1); // send new data
    lastSentData1 = PWM1;      // update last sent data
  }
  if (PWM2 != lastSentData2) { // if new data is different from last sent data
    sSerial.println(PWM2); // send new data
    lastSentData2 = PWM2;      // update last sent data
  }
}*/
//===================================================
//===================================================
void getData()
{
  while (sSerial.available() > 0) {
        char c = sSerial.read();
        if (c == '\n') {
            Serial.println(RxBuffer);
            if (RxBuffer[0] == 'S') { // Check for sensor data identifier
                receivedSensorData = RxBuffer.substring(1);
                indexOfA = receivedSensorData.indexOf('A');
                tem = receivedSensorData.substring(0, indexOfA).toInt();
                hum = receivedSensorData.substring(indexOfA + 1).toInt();
                  }
                    else if (RxBuffer[0] == 'L') { // Check for button data identifier
                      receivedButtonData1 = RxBuffer.substring(1).toInt();
                          }
                          else if (RxBuffer[0] == 'B') { // Check for button data identifier
                          receivedButtonData2 = RxBuffer.substring(1).toInt();
                            }
                            else if (RxBuffer[0] == 'D') { // Check for button data identifier
                              speed1 = RxBuffer.substring(1).toInt();
                              Serial.println(speed1);
                                }
                                else if (RxBuffer[0] == 'C') { // Check for button data identifier
                                  speed2 = RxBuffer.substring(1).toInt();
                                  Serial.println(speed2);
                                    }
            RxBuffer = "";
        } else {
            RxBuffer += c;
        }
    }
}
//===================================================
//////////////////////////////////////////////////
//===================================================


