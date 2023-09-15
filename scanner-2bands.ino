
#include <Arduino.h>
#include <Wire.h>
#include "si5351.h"
#include <EEPROM.h>

Si5351 si5351;

int32_t frequency;
int index = 0;
int32_t values[2];
boolean SetScan20;
boolean ScanCount20;
int chanset = 0;
int bandset = 0;
int clockset = 0;
int count = 0;
int channel1;
int channel2;
int channel3;
int channel4;
int channel5;
int channel6;
int channel7;
int channel8;
int channel9;
int channel10;
int clock10;
boolean SetShow = false;

void setup(void) {
  
  Serial.begin(57600);
  
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.set_correction(84600, SI5351_PLL_INPUT_XO);
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_4MA);
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_4MA);
    
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, OUTPUT);   // vhf laag
  pinMode(13, OUTPUT);   // vhf hoog
  pinMode(A0, OUTPUT);   // l led
  pinMode(A1, OUTPUT);   // h led
  pinMode(A3, INPUT);  // clock puls 

  SetScan20 = EEPROM.read(400);
  if (SetScan20 == 1){count = 1;}

  int EmptyProm = EEPROM.read(511);
  if (EmptyProm == 255){
    for (int i = 1; i <= 80; i++) 
    {
      byte myArray[4];
      myArray[0] = (13600000 >> 24) & 0xFF;
      myArray[1] = (13600000 >> 16) & 0xFF;
      myArray[2] = (13600000 >> 8) & 0xFF;
      myArray[3] = 13600000 & 0xFF;

      EEPROM.write((i * 4), myArray[0]);
      EEPROM.write((i * 4) + 1, myArray[1]);
      EEPROM.write((i * 4) + 2, myArray[2]);
      EEPROM.write((i * 4) + 3, myArray[3]);
    }
    EEPROM.write(511,1);
  }
}

void loop(void) {

//  Serial.print("count - ");
//  Serial.println(count);
//  Serial.print("chanset - ");
//  Serial.println(chanset);
  delay(10);
  
  channel1 = digitalRead(2);
  channel2 = digitalRead(3);
  channel3 = digitalRead(4);
  channel4 = digitalRead(5);
  channel5 = digitalRead(6);
  channel6 = digitalRead(7);
  channel7 = digitalRead(8);
  channel8 = digitalRead(9);
  channel9 = digitalRead(10);
  channel10 = digitalRead(11);
  clock10 = digitalRead(A3);

  if (clock10 == HIGH) {clockset = 1;}
  
  if (clock10 == LOW && clockset == 1) {
    if (SetScan20 == 0){
      count = count + 1;
      if (count > 3) {count = 0;}
      clockset = 0;
      chanset = 0;
    }
    else {
      if (count == 1) {
        count = 2;
        chanset = 0;
        clockset = 0; 
        return;   
      }
      if (count == 2){count = 1;}
      chanset = 0;   
      clockset = 0; 
    }
  }
   
  if (count == 0){
    digitalWrite(A0, LOW);  // Geen LED aan
    digitalWrite(A1, LOW);
    ReadDigChan();
  } 

  if (count == 1){
    digitalWrite(A0, HIGH);  // Alleen LED L aan
    digitalWrite(A1, LOW);
    ReadDigChan();
  } 

  if (count == 2){
    digitalWrite(A1, HIGH);   // Alleen LED H aan
    digitalWrite(A0, LOW);
    ReadDigChan();
  } 

  if (count == 3){
    digitalWrite(A0, HIGH);   // LED L en LED H aan
    digitalWrite(A1, HIGH);
    ReadDigChan();
  }
}

static void flush_input(void)
{
  while (Serial.available() > 0)
  Serial.read();
}

void serialEvent() {
  while(Serial.available())
  {
    char c = Serial.read(); 
    switch (c)
    {
    case 'm':
      ShowMenu();
      break;
    case '?':
      ShowMenu();
      break;
      
    case 'o':
      SetScan20 = true;
      EEPROM.write(400, SetScan20);
      count = 1;
      chanset = 0;
      Serial.println("20 kanalen");
      break;
    
    case 'u':
      SetScan20 = false;
      EEPROM.write(400, SetScan20);
      count = 0;
      chanset = 0;
      Serial.println("40 kanalen");
      break;
    case 'a':
      SetShow = !SetShow;
      break;
    case 'p':
//      while (Serial.available())
//        {
          for (index = 0; index < 2; index++)
            {
            values[index] = Serial.parseInt();     
            }
          index = 0;
          if ( values[0] >= 1 && values[0] <= 80){
            ProgChannels();
          }
          break;
 //       }
        
    case 'r':
      Serial.println();
//      while (Serial.available())
//        {
          for (index = 0; index < 2; index++)
            {
            values[index] = Serial.parseInt();     
            }
          index = 0;
          if ( values[0] >= 1 && values[1] <= 80){
            ShowChannels();
          }
          break;
//        }
      
      default:
      flush_input();
      return;  
    }
  }
}

void ShowMenu() { 
  Serial.println();
  Serial.println(F("Kristal(compie)scanner by PA7AP"));
  Serial.println();
  Serial.println(F("rxx,yy - laat de kanalen zien van xx tot en met yy"));
  Serial.println(F("pxx,yyyyyyyy - programmeer kanaal xx met de freq yyyyyyyyy (frequentie in 10hz nauwkeurig opgeven)"));
  Serial.println(F("o - scanner 20 kanalen 11-20 / 21-30 "));
  Serial.println(F("u - scanner 40 kanalen 1-40"));
  Serial.println(F("a - aan/uit het laten zien van de actieve frequentie"));
  Serial.println(F("m of ? - laat dit menu zien"));
  Serial.println();
}

void ShowChannels() {
  for (int i = values[0]; i <= values[1]; i++){
  byte a,b,c,d;
 
  a = EEPROM.read(i * 4);
  b = EEPROM.read(i * 4 + 1);
  c = EEPROM.read(i * 4 + 2);
  d = EEPROM.read(i * 4 + 3);
 
  frequency = a;
  frequency = (frequency << 8) | b;
  frequency = (frequency << 8) | c;
  frequency = (frequency << 8) | d;
  Serial.print(i); 
  Serial.print(" - ");
  if (frequency != -1){Serial.println(frequency);}
    else {Serial.println();}
  }
}

void ProgChannels(){
  if (values[1] >= 6600000 && values[1] <= 8800000 || values[1] >= 13600000 && values[1] <= 17400000 || values[1] >= 38000000 && values[1] <= 51200000){
    byte myArray[4];
    
    myArray[0] = (values[1] >> 24) & 0xFF;
    myArray[1] = (values[1] >> 16) & 0xFF;
    myArray[2] = (values[1] >> 8) & 0xFF;
    myArray[3] = values[1] & 0xFF;

    EEPROM.write((values[0] * 4), myArray[0]);
    EEPROM.write((values[0] * 4) + 1, myArray[1]);
    EEPROM.write((values[0] * 4) + 2, myArray[2]);
    EEPROM.write((values[0] * 4) + 3, myArray[3]);
    Serial.print(values[0]);
    Serial.print(" - ");
    Serial.println(values[1]);
  }    
}

void ShowSet() {
  if (SetShow == true){ 
    Serial.print(((count * 10) + chanset));
    Serial.print(" - ");
    Serial.println(frequency);
  }
}

void CalcFrq() {
  
  int i = (count * 10) + chanset;
  
  byte a,b,c,d;
 
  a = EEPROM.read(i * 4);
  b = EEPROM.read(i * 4 + 1);
  c = EEPROM.read(i * 4 + 2);
  d = EEPROM.read(i * 4 + 3);
 
  frequency = a;
  frequency = (frequency << 8) | b;
  frequency = (frequency << 8) | c;
  frequency = (frequency << 8) | d;

  ShowSet();
  
  if (frequency < 10000000){
    frequency = frequency + 1070000;
    frequency = frequency * 10;
    frequency = frequency / 2;
    if (bandset != 1) {
      si5351.output_enable(SI5351_CLK0, 1);
      si5351.output_enable(SI5351_CLK1, 0);
      digitalWrite(12, HIGH);
      digitalWrite(13, LOW);
      bandset = 1;
    }
    si5351.set_freq(frequency * 100ULL, SI5351_CLK0); 
  }
  else {
    frequency = frequency + 1070000;
    frequency = frequency * 10;
    frequency = frequency / 3;
    if (bandset != 2) {
      si5351.output_enable(SI5351_CLK0, 0);
      si5351.output_enable(SI5351_CLK1, 1);
      digitalWrite(12, LOW);
      digitalWrite(13, HIGH);
      bandset = 2;
    }
    si5351.set_freq(frequency * 100ULL, SI5351_CLK1); 
  }   
}


void ReadDigChan() {
  if (channel1 == LOW && chanset != 1 && clock10 == LOW) {
    chanset = 1;
    CalcFrq();
    }
  if (channel2 == LOW && chanset != 2) {
    chanset = 2;
    CalcFrq();
    }
  if (channel3 == LOW && chanset != 3) {
    chanset = 3;
    CalcFrq();
    }
  if (channel4 == LOW && chanset != 4) {
    chanset = 4;
    CalcFrq();
    }
  if (channel5 == LOW && chanset != 5) {
    chanset = 5;
    CalcFrq();
    }
  if (channel6 == LOW && chanset != 6) {
    chanset = 6;
    CalcFrq();
    }
  if (channel7 == LOW && chanset != 7) {
    chanset = 7;
    CalcFrq();
    }
  if (channel8 == LOW && chanset != 8) {
    chanset = 8;
    CalcFrq();
    }
  if (channel9 == LOW && chanset != 9) {
    chanset = 9;
    CalcFrq();
    }
  if (channel10 == LOW && chanset != 10) {
    chanset = 10;
    CalcFrq();
    }
}
