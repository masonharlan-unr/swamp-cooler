volatile unsigned char* myPCMSK1 = (unsigned char *) 0x6C;
volatile unsigned char* myPCICR  = (unsigned char *) 0x68;

volatile unsigned char* myPORTJ  = (unsigned char *) 0x105;
volatile unsigned char* myDDRJ   = (unsigned char *) 0x104;
volatile unsigned char* myPINJ   = (unsigned char *) 0x103;

bool button1 = false;
bool button2 = false;

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

void setup() {
  //PJ1 = pin 14
  //PJ0 = pin 15
  //enable read for port 14 and 15
  *myDDRJ  = 0b00000000;
  //enable pullup resistors for pin 14 and 15
  *myPORTJ = 0b00000011;

  //initialize serial port
  Serial.begin(9600);

  //set PCMSK1 pin 14 (PJ1) PCINT10
  *myPCMSK1 |= 0b00000100;

  //set PCMSK1 pin 15 (PJ0) PCINT9
  *myPCMSK1 |= 0b00000010;
  
  //set PCIE1
  *myPCICR |= 0b00000010;

  //set sei
  sei();
}

void loop() {
  while(1);
}
