//Team 46: Mason Harlan & Prim Wandeevong
//swamp-cooler

//included libraries
#include <LiquidCrystal.h>

//initialize lcd screen 
LiquidCrystal lcd(7,8,9,10,11,12);

//port b registers
volatile unsigned char* myPORTB = (unsigned char*) 0x25; 
volatile unsigned char* myDDRB  = (unsigned char*) 0x24;  

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

void setup() {
  //setup lcd (columns, rows)
  lcd.begin(16, 2);

  lcd.print("Hello, World!");
}

void loop() {

}
