#pragma once
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <Adafruit_SSD1306.h>

//#define SERIAL_OUTPUT_ON
//#define ADAFRUIT_OUTPUT_ON
#define LIQUID_CRYSTAL_OUTPUT_ON

#ifdef ADAFRUIT_OUTPUT_ON
  #if (SSD1306_LCDHEIGHT != 64)
  #error("Height incorrect, please fix Adafruit_SSD1306.h!");
  #endif
  Adafruit_SSD1306 _oled(4); // _led displej
#endif // _ADAFRUIT_OUTPUT_ON

#ifdef LIQUID_CRYSTAL_OUTPUT_ON
  LiquidCrystal_I2C lcd(0x27, 20, 4);

  
#endif // _LIQUID_CRYSTAL_OUTPUT_ON

// vtupne piny pre tlacidla
#define RIGHT 2
#define LEFT 3
#define RESET 4
#define START 5


bool _ready; // boolean hovori ci je casomiera pripravena na spustenie
volatile unsigned long _LStart;
volatile unsigned long _RStart;
volatile unsigned long _varL; // lavy terc dlzska v milisekundach
volatile unsigned long _varR; // pravy terc dlzska v milisekundach
volatile bool _LRunning; // lavy terc prave bezi
volatile bool _RRunning; // pravy terc prave bezi

void setup() {
  // povolenie prerušení
  attachInterrupt(digitalPinToInterrupt(RIGHT),f_interop_right,RISING); 
  attachInterrupt(digitalPinToInterrupt(LEFT),f_interop_left,RISING); 
 
  // nastavenie pinov na vstup a pripojenie pull-up rezistorov
  pinMode(RIGHT, INPUT);
  digitalWrite(RIGHT, LOW);
  pinMode(LEFT, INPUT);
  digitalWrite(LEFT, LOW);
  pinMode(RESET, INPUT_PULLUP);
  digitalWrite(RESET, HIGH);
  pinMode(START, INPUT_PULLUP);
  digitalWrite(START, HIGH);
  _ready = true;
  _varR = _varL = 0;
  _LRunning = false;
  _RRunning = false;

  // oled display inicializacia
#ifdef ADAFRUIT_OUTPUT_ON
  f_initPrint();
#endif // _ADAFRUIT_OUTPUT_ON

  // zaciatok komunikacie pozbernici (pre f_serialPrint ucely)
#ifdef SERIAL_OUTPUT_ON
  Serial.begin(9600);
#endif // _SERIAL_OUTPUT_ON
  // LED display inicializacia
#ifdef LIQUID_CRYSTAL_OUTPUT_ON
  lcd.init();
  lcd.backlight();
  lcd.clear();
  f_lcdPrint();
#endif // _LIQUID_CRYSTAL_OUTPUT_ON
}

void loop() {
  if (_ready)
  {
    while (digitalRead(START) == HIGH)
    {
      if (digitalRead(RESET) == LOW)
      {
        unsigned long startResetPressed = millis();
        while (digitalRead(RESET) == LOW)
        {
        }
        if (millis() - startResetPressed > 2000)
        {
          f_checkLights();
        }
      }
    }
    f_startWatch();
  }

  if (digitalRead(RESET) == LOW)
  {
    f_resetWatch();
  }

#ifdef ADAFRUIT_OUTPUT_ON
  f_digitalPrint();
#endif // _ADAFRUIT_OUTPUT_ON

#ifdef SERIAL_OUTPUT_ON
  f_serialPrint();
#endif // _SERIAL_OUTPUT_ON

#ifdef LIQUID_CRYSTAL_OUTPUT_ON
  f_lcdPrint();
#endif // _LIQUID_CRYSTAL_OUTPUT_ON
}

void f_checkLights() {
  f_stopWatch(RIGHT);
  f_stopWatch(LEFT);
  _varR = _varL = 0;
  _RStart = _LStart = 0;
  _ready = true;
  
  pinMode(LEFT, OUTPUT);
  digitalWrite(LEFT, HIGH);
  pinMode(RIGHT, OUTPUT);
  digitalWrite(RIGHT, HIGH);
  delay(3000);
  pinMode(RIGHT, INPUT_PULLUP);
  digitalWrite(RIGHT, LOW);
  pinMode(LEFT, INPUT_PULLUP);
  digitalWrite(LEFT, LOW);
}
  
void f_startWatch() {
  if (_ready)
  {
    _ready = false;
    _RStart = _LStart = millis();
    _LRunning = _RRunning = true;
  }
}

void f_stopWatch(int pWatch) {
  if (pWatch == RIGHT)
  {
    if (_RRunning) {
      _RRunning = false;
      _varR = millis() - _RStart;
    }
  }
  else if (pWatch == LEFT)
  {
    if (_LRunning) {
      _LRunning = false;
      _varL = millis() - _LStart;
    }
  }
}

void f_interop_left() {
  if (digitalRead(LEFT) == HIGH && _LRunning)
  {
    f_stopWatch(LEFT);
    pinMode(LEFT, OUTPUT);
    digitalWrite(LEFT, HIGH);
  }
}

void f_interop_right() {
  if (digitalRead(RIGHT) == HIGH && _RRunning)
  {
    f_stopWatch(RIGHT);
    pinMode(RIGHT, OUTPUT);
    digitalWrite(RIGHT, HIGH);
  }
}

void f_resetWatch() {
  f_stopWatch(RIGHT);
  f_stopWatch(LEFT);
  _varR = _varL = 0;
  _RStart = _LStart = 0;
  _ready = true;
  pinMode(RIGHT, INPUT_PULLUP);
  digitalWrite(RIGHT, LOW);
  pinMode(LEFT, INPUT_PULLUP);
  digitalWrite(LEFT, LOW);

#ifdef LIQUID_CRYSTAL_OUTPUT_ON
  lcd.clear();
#endif
}

#ifdef ADAFRUIT_OUTPUT_ON
  void f_initPrint()
{
  _oled.begin(SSD1306_SWITCHCAPVCC, 0x3C, true);
  _oled.setTextSize(1);
  _oled.setTextColor(WHITE);
  _oled.setCursor(0, 15);
  _oled.display();
  delay(2000);
  _oled.clearDisplay();
  _oled.setCursor(0, 40);
  for (unsigned int i = 0; i < 10; i++) {
    delay(300);
    _oled.print(" .");
    _oled.display();
  }
  delay(300);
}

void f_digitalPrint()
{
  _oled.clearDisplay();
  _oled.setCursor(0, 15);
  if (_res) {
    _oled.print("  Lavy terc:  0.000\n\n\n  Pravy terc: 0.000");
  } else {
    _oled.print("  Lavy terc:  ");
    if (_LRunning) {
      _oled.print((millis() - _LStart) / 1000.0, 3);
    } else {
      _oled.print(_varL / 1000.0, 3);
    }
    _oled.print("\n\n\n  Pravy terc: ");
    if (_RRunning) {
      _oled.print((millis() - _RStart) / 1000.0, 3);      
    } else {
      _oled.print(_varR / 1000.0, 3);  
    }
  }
  _oled.display();
}
#endif // _ADAFRUIT_OUTPUT_ON

#ifdef LIQUID_CRYSTAL_OUTPUT_ON
  void f_lcdPrint() {
    lcd.setCursor(0, 0);
    lcd.print("+------------------+");
    lcd.setCursor(0, 3);
    lcd.print("+------------------+");

    lcd.setCursor(0, 1);
    lcd.print("|");
    lcd.setCursor(19, 1);
    lcd.print("|");

    lcd.setCursor(0, 2);
    lcd.print("|");
    lcd.setCursor(19, 2);
    lcd.print("|");

    lcd.setCursor(1, 1);
    lcd.print(" L:     ");
    if (_LRunning) {
      lcd.print((millis() - _LStart) / 1000.0, 3);
    } else {
      lcd.print(_varL / 1000.0, 3);
    }

    lcd.setCursor(1, 2);
    lcd.print(" P:     ");
    if (_RRunning) {
      lcd.print((millis() - _RStart) / 1000.0, 3);      
    } else {
      lcd.print(_varR / 1000.0, 3);  
    }
}
#endif // _LIQUID_CRYSTAL_OUTPUT_ON

#ifdef SERIAL_OUTPUT_ON
  void f_serialPrint()
  {
  if (_ready) {
    Serial.print("  Lavy terc: 0.000  Pravy terc: 0.000");
  } else {
    Serial.print("  Lavy terc:  ");
    if (_LRunning) {
      Serial.print((millis() - _LStart) / 1000.0, 3);
    } else {
      Serial.print(_varL / 1000.0, 3);
    }
    Serial.print("  Pravy terc: ");
    if (_RRunning) {
      Serial.print((millis() - _RStart) / 1000.0, 3);      
    } else {
      Serial.print(_varR / 1000.0, 3);  
    }
  }
  Serial.println("");
}
#endif // _SERIAL_OUTPUT_ON
