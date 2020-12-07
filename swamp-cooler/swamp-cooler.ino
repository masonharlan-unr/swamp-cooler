//Team 46: Mason Harlan & Prim Wandeevong
//swamp-cooler

//included libraries
#include <DS1307RTC.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <SimpleDHT.h>
#include <TimeLib.h>
#include <Wire.h>

//initialize lcd screen 
LiquidCrystal lcd(7,8,9,10,11,12);

//initialize servo vent
Servo vent;
int vent_position = 1;

//port registers
volatile unsigned char* myPORTA = (unsigned char*) 0x22; 
volatile unsigned char* myDDRA  = (unsigned char*) 0x21;
volatile unsigned char* myPINA  = (unsigned char*) 0x20;

volatile unsigned char* myPORTB = (unsigned char*) 0x25; 
volatile unsigned char* myDDRB  = (unsigned char*) 0x24;
volatile unsigned char* myPINB  = (unsigned char*) 0x23;

volatile unsigned char* myPORTE = (unsigned char*) 0x2E; 
volatile unsigned char* myDDRE  = (unsigned char*) 0x2D;
volatile unsigned char* myPINE  = (unsigned char*) 0x2C;

volatile unsigned char* myPORTG = (unsigned char*) 0x34; 
volatile unsigned char* myDDRG  = (unsigned char*) 0x33;
volatile unsigned char* myPING  = (unsigned char*) 0x32;

volatile unsigned char* myPORTJ  = (unsigned char *) 0x105;
volatile unsigned char* myDDRJ   = (unsigned char *) 0x104;
volatile unsigned char* myPINJ   = (unsigned char *) 0x103;

//timer registers
volatile unsigned int  *myTCNT1  = (unsigned  int *) 0x84;
volatile unsigned char *myTCCR1A = (unsigned char *) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char *) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char *) 0x82;
volatile unsigned char *myTIFR1  = (unsigned char *) 0x36;
volatile unsigned char *myTIMSK1 = (unsigned char *) 0x6F;

//adc registers for water sensor
volatile unsigned char* myADCSRA = (unsigned char*) 0x7A;
volatile unsigned char* myADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* myADMUX  = (unsigned char*) 0x7C;
volatile unsigned int* myADCDATA = (unsigned int*)  0x78;

//pin change interrupt registers
volatile unsigned char* myPCMSK1 = (unsigned char *) 0x6C;
volatile unsigned char* myPCICR  = (unsigned char *) 0x68;

bool button1 = false; //on/off button
bool button2 = false; //vent position button

//dht11 read ready flag
bool read_dht11 = false;

//humidity sensor
int pinDHT11 = 27;
SimpleDHT11 dht11;

//water sensor
int water_level_pin = 8;
int level_history = 0;

//time config
tmElements_t tm;
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

//super states
bool on = false;
//substates
bool error_state = false;
bool idle_state = false;
bool running_state = false;

//pin change interrupt 1
ISR (PCINT1_vect){ //to avoid bouncing, perform action on release
  if (*myPINJ == 1){ //state for on/off button down
    button1 = true; 
  } 
  if (*myPINJ == 2){ //state for vent position button down
    button2 = true;  
  }
  if(*myPINJ == 3){ //this means a button was released
     //check which button was released
    if(button1){ //on/off button
      if(on){ //if on turn off
        if(running_state){
          //stop fan
          *myPORTA &= 0b11110111;
          //print time stamp
          off_stamp();
        }
        //clear display
        clear_lcd();
        //turn yellow LED on and others off
        *myPORTE = 0b00010000; //pin 5 (PE3) LOW, pin 3 (PE5) LOW, pin 2 (PE4) HIGH
        *myPORTG = 0b00000000; //pin 4 (PG5) LOW
        //change state to off
        on = false;
        running_state = false;
        idle_state - false;  
      }
      else if (error_state){
        //turn yellow LED off and red LED on
        *myPORTE = 0b00000000; //pin 2 (PE4) LOW
        *myPORTG = 0b00100000; //pin 4 (PG5) HIGH
        //print error message
        error_message();
        //change state to on
        on = true;
      }
      else{ //if off turn on
        //turn yellow LED off and green LED on
        *myPORTE = 0b00100000;
        //change state to on
        on = true;
        idle_state = true;
      }
      button1 = false;  
    }  
    if(button2){ //cycle vent position button
      if(on){
        vent_position++;
        vent.write(vent_position * 30);
        if (vent_position > 3){
          vent_position = 1;  
        } 
      } 
    }
  }
}

//timer 1 overflow interrupt
ISR (TIMER1_OVF_vect){
  *myTCCR1B = 0x00; //stop timer          
  *myTIFR1 |= 0x01; //reset interrupt

  read_dht11 = true;
  
  *myTCNT1  = (unsigned int) (34286); //set timer value for 2 second delay
  *myTCCR1B = 0x05; //start timer with prescalar
}

//setup led registers
void LED_setup(){
  //digital pins
  //blue     = pin 5 (PE3)
  //red      = pin 4 (PG5)
  //green    = pin 3 (PE5)
  //yellow   = pin 2 (PE4)

  //set write mode
  *myDDRE = 0b00111000; //pin 5 (PE3), pin 3 (PE5), pin2 (PE4) output
  *myDDRG = 0b00100000; //pin 4 (PG5) output

  //starts in off position
  *myPORTE = 0b00010000; //pin 2 (PE4) HIGHH
}

//setup button input registers
void button_setup(){
  //button1 = pin 14 (PJ1)
  //button2 = pin 15 (PJ0)
  //enable read for port 14 (PJ1) and 15 (PJ0)
  *myDDRJ  = 0b00000000;
  //enable pullup resistors for pin 14 (PJ1) and 15 (PJ0)
  *myPORTJ = 0b00000011;
}

//setup pin change interrupt registers
void PCINT_setup(){
  //digital GPIO interrupts
  //set PCMSK1 pin 14 (PJ1) PCINT10 and pin 15 (PJ0) PCINT9 
  *myPCMSK1 |= 0b00000110;  
  //set PCIE1 to enable interrupts
  *myPCICR |= 0b00000010;
}

//setup timer registers and timer interrupt
void timer_setup(){
  //enable timer overflow interrupt
  *myTIMSK1 |= 0b00000001;
  
  // init timer control registers
  *myTCCR1B = 0x00; //stop timer
  *myTCNT1  = (unsigned int) (34286); //set initial value for 2 second delay
  
  *myTCCR1A = 0x00; //normal timer
  *myTCCR1B = 0x05; //set prescalar to 1024 for 2 second delay and start
  *myTCCR1C = 0x00; //normal timer
}

//setup ADC registers for water sensor
void adc_setup(){
  //setup A register
  //set bit 7 to 1 to enable the ADC
  *myADCSRA |= 0b10000000;
  //set bit 5 to 0 to disable the ADC trigger mode
  *myADCSRA &= 0b11011111;
  //set bit 3 to 0 to disable the ADC interrupt
  *myADCSRA &= 0b11110111;
  //clear bits 2-0 to 0 to set rescaler selection to slow reading
  *myADCSRA &= 0b11111000;

  //setup B register
  //clear bit 3 to 0 to reset the channel and gain bit
  *myADCSRB &= 0b11110111;
  //clear bit 2-0 to 0 to set free running mode
  *myADCSRB &= 0b11111000;

  //setup the MUX register
  //clear bit 7 to 0 for AVCC analog refrence
  *myADMUX  &= 0b01111111;
  //set bit 6 to 1 for AVCC analog refrencce
  *myADMUX  |= 0b01000000;
  //clear bit 5 to 0 for right adjust result
  *myADMUX  &= 0b11011111;
  //clear bits 4-0 to 0 to reset the channel and gain bits
  *myADMUX  &= 0b11100000;
}

//setup registers for the fan
void fan_setup(){
  //on/off = pin 25 (PA3)
  //dira   = pin 24 (PA2)
  //dirb   = pin 23 (PA1)
  
  //set write mode
  *myDDRA  |= 0b00001110; //pin 25 (PA3), pin 24 (PA2), pin23 (PA1) output

  //set fan direction
  *myPORTA |= 0b00000100; //set pin 24 (PA2) HIGH
  *myPORTA &= 0b11111101; //set pin 23 (PA1) LOW 
}

//clear lcd completely
void clear_lcd(){
  lcd.setCursor(0,0);
  lcd.print("             ");
  lcd.setCursor(0,1);
  lcd.print("             ");
}

//clear lcd values displayed
void clear_values_lcd(){
  lcd.setCursor(6,0);
  lcd.print("     ");
  lcd.setCursor(9,1);
  lcd.print("     ");
}

void error_message(){
  clear_lcd();
  lcd.setCursor(0,0);
  lcd.print("LOW WATER");
  lcd.setCursor(0,1);
  lcd.print("FILL SOON");  
}

void on_stamp(){
  Serial.print("Fan ON: ");
  Serial.print(month());
  Serial.print("/");
  Serial.print(day());
  Serial.print("/");
  Serial.print(year());
  Serial.print(" ");
  Serial.print(hour());
  Serial.print(":");
  if (minute() < 10){
    Serial.print("0");  
  }
  Serial.println(minute());
}

void off_stamp(){
  Serial.print("Fan OFF: ");
  Serial.print(month());
  Serial.print("/");
  Serial.print(day());
  Serial.print("/");
  Serial.print(year());
  Serial.print(" ");
  Serial.print(hour());
  Serial.print(":");
  Serial.println(minute());  
}

void setup() {
  //setup lcd (columns, rows)
  lcd.begin(16, 2);

  //setup LED registers
  LED_setup();

  //setup adc registers
  adc_setup();

  //setup button registers
  button_setup();

  //setup GPIO pin change interrupts
  PCINT_setup();

  //setup timer1
  timer_setup();

  //setup fan registers
  fan_setup();

  //setup servo for vent on pin 13
  vent.attach(13);
  vent.write(30);

  //set sei
  sei();

  //setup serial communications
  Serial.begin(9600);

  //setup time
  if(getDate(__DATE__) && getTime(__TIME__)){
    RTC.write(tm);
  }
  setSyncProvider(RTC.get);
}

void loop() {
  if(on){
    //read water level value
    int water_level = adc_read(water_level_pin);
    //check if value is within history by ten
    if (((level_history >= water_level) && ((level_history - water_level) > 5)) || ((level_history < water_level) && ((water_level - level_history) > 5)))
    {
      //if water has been refilled
      if(error_state && (water_level >= 100)){
          //turn red LED off and green LED on
          *myPORTE = 0b00100000; 
          *myPORTG = 0b00000000; 
          //change state
          error_state = false;
          idle_state = true;
      }
      //if water is too low then go to error state
      else if ((!error_state) && (water_level < 100)){
        //turn off fan
        *myPORTA &= 0b11110111;
        //turn on red LED and other LEDs off
        *myPORTE = 0b00000000; //pin 2 (PE4) LOW
        *myPORTG = 0b00100000; //pin 4 (PG5) HIGH
        //print error message
        error_message();
        //print time stamp
        if(running_state){
          off_stamp();  
        }
        //change state
        error_state = true;
        idle_state = false;
        running_state = false;
      }
      //Serial.print("Water level is: ");
      //Serial.println(water_level);
      level_history = water_level;
    }
  }
  
  //if dht data ready to update
  if (read_dht11){
    byte temperature = 0;
    byte humidity = 0;
    byte data[40] = {0};
    
    //if read is successful, print to lcd screen
    if (!(dht11.read(pinDHT11, &temperature, &humidity, data)) && on && !error_state){
      clear_values_lcd();
      lcd.setCursor(0,0);
      lcd.print("Temp: ");
      lcd.print((int) temperature);
      lcd.print("*C");
      lcd.setCursor(0,1);
      lcd.print("Humidity: ");
      lcd.print((int) humidity);
      lcd.print("%");

      //check if not already runnning and if temp too high
      if((!running_state) && (temperature > 24)){
        //turn fan on
        *myPORTA |= 0b00001000;
        //turn blue LED on and others off
        *myPORTE = 0b00001000;
        //print time stamp
        on_stamp();
        //change state to running
        idle_state = false;
        running_state = true;
      }
      //check if not already idle and if temp too low
      if((!idle_state) && (temperature <= 24)){
        //turn fan off
        *myPORTA &= 0b11110111;
        //turn green LED on and others off
        *myPORTE = 0b00100000;
        //print time stamp
        off_stamp();
        //change state to idle
        running_state = false;
        idle_state = true;  
      }
    }
    //signal that dht was read
    read_dht11 = false;
  }
}

unsigned int adc_read(unsigned char adc_channel){
  //clear channel selecti0n bits (set 4-0 to 0)
  *myADMUX  &= 0b11100000;
  //clear MUX5 (set 3 to 0)
  *myADCSRB &= 0b11110111;

  //set selection bit(if greater than 7, nned to adjust)
  if(adc_channel > 7){
    adc_channel -= 8;

    //set MUX 5 to 0 (set 3 to 0)
    *myADCSRB |= 0b00001000;
  }
  //set the channel selection bit
  *myADMUX += adc_channel;

  //start the conversion (set 6 to 1)
  *myADCSRA |= 0b01000000; 
  //wait for the conversion to finish
  while((*myADCSRA & 0b01000000) != 0);
  return *myADCDATA; //return the result of the conversion
}

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}
