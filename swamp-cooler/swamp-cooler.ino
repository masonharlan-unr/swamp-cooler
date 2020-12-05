//Team 46: Mason Harlan & Prim Wandeevong
//swamp-cooler

//included libraries
#include <LiquidCrystal.h>
#include <SimpleDHT.h>

//initialize lcd screen 
LiquidCrystal lcd(7,8,9,10,11,12);

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

 //usart registers
 volatile unsigned char* myUCSR0A = (unsigned char*)0x00C0;
 volatile unsigned char* myUCSR0B = (unsigned char*)0x00C1;
 volatile unsigned char* myUCSR0C = (unsigned char*)0x00C2;
 volatile unsigned int * myUBRR0  = (unsigned int*) 0x00C4;
 volatile unsigned char* myUDR0   = (unsigned char*)0x00C6;

//timer registers
volatile unsigned int  *myTCNT1  = (unsigned  int *) 0x84;
volatile unsigned char *myTCCR1A = (unsigned char *) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char *) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char *) 0x82;
volatile unsigned char *myTIFR1  = (unsigned char *) 0x36;
volatile unsigned char *myTIMSK1 = (unsigned char *) 0x6F;

//button registers
volatile unsigned char* myPCMSK1 = (unsigned char *) 0x6C;
volatile unsigned char* myPCICR  = (unsigned char *) 0x68;

volatile unsigned char* myPORTJ  = (unsigned char *) 0x105;
volatile unsigned char* myDDRJ   = (unsigned char *) 0x104;
volatile unsigned char* myPINJ   = (unsigned char *) 0x103;

bool button1 = false; //on/off button
bool button2 = false; //vent position button

//dht11 read ready flag
bool read_dht11 = false;

//humidity sensor
int pinDHT11 = 27;
SimpleDHT11 dht11;

//super states
bool on = false;
//substates
bool error_state = true;
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
        //stop fan
        *myPORTA &= 0b11110111;
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
      Serial.println("BUTTON 2 Released");
      button2 = false;  
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
  *myPORTE = 0b00010000; //pin 2 (PE4) HIGH

  //pin output refrence:
    //*myPORTE = 0b00111000; //pin 5 (PE3), pin 3 (PE5), pin 2 (PE4) HIGH
    //*myPORTG = 0b00100000; //pin 4 (PG5) HIGH
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
  //GPIO interrupts
  //set PCMSK1 pin 14 (PJ1) PCINT10
  *myPCMSK1 |= 0b00000100;

  //set PCMSK1 pin 15 (PJ0) PCINT9
  *myPCMSK1 |= 0b00000010;
  
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

//clear lccd completely
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

void setup() {
  //setup lcd (columns, rows)
  lcd.begin(16, 2);

  //setup LED registers
  LED_setup();

  //setup button registers
  button_setup();

  //setup GPIO pin change interrupts
  PCINT_setup();

  //setup timer1
  timer_setup();

  //setup fan registers
  fan_setup();

  //set sei
  sei();

  //setup serial communications
  Serial.begin(9600);
}

void loop() {
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
      if((!running_state) && (temperature > 20)){
        //turn fan on
        *myPORTA |= 0b00001000;
        //turn blue LED on and others off
        *myPORTE = 0b00001000;
        //change state to running
        idle_state = false;
        running_state = true;
      }
      //check if not already idle and if temp too low
      if((!idle_state) && (temperature <= 20)){
        //turn fan off
        *myPORTA &= 0b11110111;
        //turn green LED on and others off
        *myPORTE = 0b00100000;
        //change state to idle
        running_state = false;
        idle_state = true;  
      }
    }
    //signal that dht was read
    read_dht11 = false;
  }
}
