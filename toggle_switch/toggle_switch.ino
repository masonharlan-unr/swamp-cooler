volatile unsigned char* myPCMSK1 = (unsigned char *) 0x6C;
volatile unsigned char* myPCICR  = (unsigned char *) 0x68;

volatile unsigned char* myPORTJ  = (unsigned char *) 0x105;
volatile unsigned char* myDDRJ   = (unsigned char *) 0x104;
volatile unsigned char* myPINJ   = (unsigned char *) 0x103;

byte value;

ISR (PCINT1_vect){
  value = *myPINJ;
  Serial.println(value);
  Serial.println("It Worked!");  
}

void setup() {
  //PJ0 = pin 15
  //PJ1 = pin 14
  *myDDRJ  = 0b00000000;
  *myPORTJ = 0b00000010;
  Serial.begin(9600);

  //set PCMSK1 pin 14 (PJ1)
  *myPCMSK1 = 0b00000100;
  
  //set PCIE1
  *myPCICR |= 0b00000010;

  //set sei
  sei();
}

void loop() {
  while(1);
}
