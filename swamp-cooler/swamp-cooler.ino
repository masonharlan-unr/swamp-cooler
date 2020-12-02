//Team 46: Mason Harlan & Prim Wandeevong
//swamp-cooler

//included libraries
#include <LiquidCrystal.h>

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

void GPIO_setup(){
  //digital pins
  //blue    = pin 5 (PE3)
  //red     = pin 4 (PG5)
  //green   = pin 3 (PE5)
  //yellow  = pin 2 (PE4)
  //fan     

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
  //GPIO_setup();

  //test fan on pin 23, 24, 25
  pinMode(25,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(24,OUTPUT);
  Serial.begin(9600);

  lcd.print("Hello, World!");
}

void loop() {
   digitalWrite(25,LOW); // enable on=HIGH
   digitalWrite(23,LOW); //one way
   digitalWrite(24,HIGH);
}
