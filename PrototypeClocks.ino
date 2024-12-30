#include <DS3231.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>

//  Time Module Variables
DS3231 myRTC;
bool h12Flag;
bool century = false;
bool pmFlag;

//  Oled variables
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//  Button variables
#define BUTTON1 2
#define BUTTON2 3
uint32_t btnTimerDelay = 0;
bool flag_btn1 = false;
bool flag_btn2 = false;

//  Program variables
void (*func_ptr)();
byte modeSelect = 0;
bool SetMode = false;

//  Date data
//  hours, minutes, day, month, year
//  0    , 1      , 2  , 3    , 4
byte DateData[5] = {0, 0, 0, 0, 0};

const char monthName[][10] = {
  "January",
  "February",
  "March",
  "April",
  "May",
  "June",
  "July",
  "Agust",
  "Septemer",
  "October",
  "November",
  "December"
};

//  Functions
String shiftTimeCorrect(byte number);

void getData();
void setDataToMemory();

void button1Pressed()
{
  //Serial.println("Interrupt!");
  bool btnState = !digitalRead(BUTTON1);
  if(millis() - btnTimerDelay > 250 && flag_btn1 == false && btnState)
  {
    btnTimerDelay = millis();
    flag_btn1 = true;
  }
  if(millis() - btnTimerDelay > 100 && flag_btn1 == true && !btnState)
  {
    //Serial.println("Button #1 is pressed");
    func_ptr = &getData;
    modeSelect = 0;
    detachInterrupt(0);
    flag_btn1 = false;
  }
}


void setup() 
{

	Wire.begin();
  //Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();

  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.print("Created by M4rchy_S");

  display.display();

  pinMode(BUTTON1, INPUT_PULLUP);  
  pinMode(BUTTON2, INPUT_PULLUP);  

  attachInterrupt(0, button1Pressed, CHANGE);

  delay(1500);

  func_ptr = &PrintTimeMode;


}

void PrintTimeMode()
{
  //  Init
  display.clearDisplay();
  display.setCursor(36,4);
  display.setTextSize(2);

  //  Time
  display.print(shiftTimeCorrect( myRTC.getHour(h12Flag, pmFlag) ) );
	display.print(":");
	display.print(shiftTimeCorrect( myRTC.getMinute() ) );
	// display.print(":");
	// display.print(shiftTimeCorrect( myRTC.getSecond() ) );

  display.println();

  //  Date
  display.setCursor(18,24);
  display.setTextSize(1);

  display.print(myRTC.getDate(), DEC);
	display.print(" ");
  display.print(monthName[ myRTC.getMonth(century) ]);
  display.print(" ");
  display.print("20");
	display.print(myRTC.getYear(), DEC);
	display.print(' ');

  display.display();

  // delay(100);
  
}

void SetTimeMode()
{
  //detachInterrupt(0);
  //detachInterrupt(1);

  display.clearDisplay();
  display.setCursor(14,0);
  display.setTextSize(1);

  display.print("Setings mode");
  display.println();

  //  Date data
  //  hours, minutes, day, month, year
  //  0    , 1      , 2  , 3    , 4

  switch(modeSelect)
  {
    case 0: display.print("Hours:"); display.println(DateData[modeSelect] % 24);  
            break;
    case 1: display.print("Minutes:"); display.println(DateData[modeSelect] % 60);  
            break;
    case 2: display.print("Day:"); display.println(DateData[modeSelect] % 31);  
            break;
    case 3: display.print("Month:"); display.println(monthName[ DateData[modeSelect] % 12 ] ); 
            break;
    case 4: display.print("Year:"); display.println(DateData[modeSelect] % 100);  
            break;
    case 5: 
            attachInterrupt(0, button1Pressed, CHANGE);
            //attachInterrupt(1, button2Pressed, CHANGE);
            SetMode = false;
            setDataToMemory();
            func_ptr = &PrintTimeMode;
            return;
  }

  display.display();

  bool btnState = !digitalRead(BUTTON1);
  if(millis() - btnTimerDelay > 100 && flag_btn1 == false && btnState)
  {
    btnTimerDelay = millis();
    flag_btn1 = true;
  }
  if(millis() - btnTimerDelay > 100 && flag_btn1 == true && !btnState)
  {
      //Serial.print("[Detached] ");
      //Serial.println("button #1 is pressed");
      modeSelect++;
      flag_btn1 = false;
  }

  btnState = !digitalRead(BUTTON2);
  if(millis() - btnTimerDelay > 100 && flag_btn2 == false && btnState)
  {
    btnTimerDelay = millis();
    flag_btn2 = true;
  }
  if(millis() - btnTimerDelay > 100 && flag_btn2 == true && !btnState)
  {
    //Serial.println("button #2 is pressed");
    DateData[modeSelect] = DateData[modeSelect] + 1;
    flag_btn2 = false;
  }


}

void loop() {

  (*func_ptr)();
}

String shiftTimeCorrect(byte number)
{
  if(number < 10)
    return "0" + String( number );
  else
    return String( number );
}

void getData()
{
  //  Date data
  //  hours, minutes, day, month, year
  //  0    , 1      , 2  , 3    , 4
  DateData[0] = myRTC.getHour(h12Flag, pmFlag) ;
  DateData[1] = myRTC.getMinute();
  DateData[2] = myRTC.getDate();
  DateData[3] = myRTC.getMonth(century);
  DateData[4] = myRTC.getYear();

  func_ptr = &SetTimeMode;
}

void setDataToMemory()
{
    myRTC.setClockMode(false);  // set to 24h

    myRTC.setYear(DateData[4] % 100);
    myRTC.setMonth(DateData[3] % 12);
    myRTC.setDate(DateData[2] % 31);
    myRTC.setHour(DateData[0] % 24);
    myRTC.setMinute(DateData[1] % 60);
    myRTC.setSecond(0);
}
