//Team 46: Mason Harlan & Prim Wandeevong
//swamp-cooler

//included libraries
#include <LiquidCrystal.h>
#include <SimpleDHT.h>

//initialize lcd screen 
LiquidCrystal lcd(7,8,9,10,11,12);

//port registers
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

bool button1 = false;
bool button2 = false;

//humidity sensor
int pinDHT11 = 27;
SimpleDHT11 dht11;

ISR (PCINT1_vect){ //to avoid bouncing, perform action on release
  if (*myPINJ == 1){ 
    Serial.println("BUTTON 1 Pressed");
    button1 = true; 
  } 
  if (*myPINJ == 2){
    Serial.println("BUTTON 2 pressed");
    button2 = true;  
  }
  if(*myPINJ == 3){
    if(button1){
      Serial.println("BUTTON 1 Released");
      button1 = false;  
    }  
    if(button2){
      Serial.println("BUTTON 2 Released");
      button2 = false;  
    }
  }
}

void GPIO_setup(){
  //digital pins
  //blue     = pin 5 (PE3)
  //red      = pin 4 (PG5)
  //green    = pin 3 (PE5)
  //yellow   = pin 2 (PE4)
  //fan enb  = pin 25
  //fan dirb = pin 24
  //fan dirn = pin 23

  *myDDRE = 0b00111000; //pin 5 (PE3), pin 3 (PE5), pin2 (PE4) output
  *myDDRG = 0b00100000; //pin 4 (PG5) output

  //test pin output
  *myPORTE = 0b00111000; //pin 5 (PE3), pin 3 (PE5), pin 2 (PE4) HIGH
  *myPORTG = 0b00100000; //pin 4 (PG5) HIGH
}

void setup() {
  //setup lcd (columns, rows)
  lcd.begin(16, 2);

  //setup GPIO registers
  GPIO_setup();

  //PJ1 = pin 14
  //PJ0 = pin 15
  //enable read for port 14 and 15
  *myDDRJ  = 0b00000000;
  //enable pullup resistors for pin 14 and 15
  *myPORTJ = 0b00000011;

  //GPIO interrupts
  //set PCMSK1 pin 14 (PJ1) PCINT10
  *myPCMSK1 |= 0b00000100;

  //set PCMSK1 pin 15 (PJ0) PCINT9
  *myPCMSK1 |= 0b00000010;
  
  //set PCIE1
  *myPCICR |= 0b00000010;

  //set sei
  sei();

  //test fan on pin 23, 24, 25
  pinMode(25,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(24,OUTPUT);
  digitalWrite(25,HIGH); // enable on=HIGH
  digitalWrite(23,LOW); //one way
  digitalWrite(24,HIGH);
  Serial.begin(9600);
}

void loop() {
  // read with raw sample data.
  byte temperature = 0;
  byte humidity = 0;
  byte data[40] = {0};
  dht11.read(pinDHT11, &temperature, &humidity, data);
  
  //clear lines first
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print((int) temperature);
  lcd.print("*C");
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print((int) humidity);
  lcd.print("%");
  
  // DHT11 sampling rate is 1HZ.
  delay(2000);
}
