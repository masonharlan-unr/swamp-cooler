//www.elegoo.com
//2016.12.12

/************************
Exercise the motor using
the L293D chip
************************/

#define ENABLE 25
#define DIRA 23
#define DIRB 24

int i;
 
void setup() {
  //---set pin direction
  pinMode(ENABLE,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);
  Serial.begin(9600);
}

void loop() {
    digitalWrite(25,HIGH); // enable on
    digitalWrite(24,HIGH); //one way
    digitalWrite(23,LOW);
}
   
