/*


First attempt at closing the loop using AS5145 encoder, A4954 driver

Controlled via a serial terminal at 115200 baud.


____
    |
  13|-> LED 
  12|-> pulse         _
  11|-> clock          \
  10|->Chip Select      |---AS5145
   9|->data           _/
   8|->IN4             \
   7|->IN3              \
  ~6|->VREF2             \___A4954
  ~5|->VREF1             /           
   4|->IN2              /
  ~3|->IN1            _/
   2|
   1|
   0|
____|
  

Implemented commands are:

p  -  print [step count] , [assumed angle] , [encoder reading]

c  -  clear step count & assumed angle

s  -  step

d  -  dir toggle

z  -  seek zero position

g  -  Go! steps around 400 times

w  -  Same as go, but stores encoder angles to EEPROM

r  -  returns EEPROM contents

a  -  prompts user to enter angle 



*/


#include <EEPROM.h>
#include <math.h>
#include <avr/pgmspace.h>




const int ledPin = 13; //LED connected to digital pin 13
const int clockPin = 11; //output to clock
const int CSnPin = 10; //output to chip select
const int inputPin = 9; //read AS5040

//const PROGMEM  long  lookup[] = {};
float  lookup[] = {};

long l_index = 0; 

int dir = 1;
int step_state = 1;



int inputstream = 0; //one bit read from pin
long packeddata = 0; //two bytes concatenated from inputstream
long angle = 0; //holds processed angle value
long angletemp;
float anglefloat = 0; 

int a = 0;  //angle value in zero routine
int b = 0;
float c = 0.0;
float l = 0.0;
float m = 0.0;

float offset = 0.000000000000000; //zero-offest of closest full step

//long anglemask = 65472; //0x1111111111000000: mask to obtain first 10 digits with position info
long anglemask = 262080; // 0x111111111111000000: mask to obtain first 12 digits with position info
long statusmask = 63; //0x000000000111111; mask to obtain last 6 digits containing status info
long statusbits; //holds status/error information
int DECn; //bit holding decreasing magnet field error data
int INCn; //bit holding increasing magnet field error data
int OCF; //bit holding startup-valid bit
int COF; //bit holding cordic DSP processing error data
int LIN; //bit holding magnet field displacement error data
int debug = 1; //SET THIS TO 0 TO DISABLE PRINTING OF ERROR CODES
int shortdelay = 100; // this is the microseconds of delay in the data clock
int longdelay = 10; // this is the milliseconds between readings


int i_step = 0; // step index
int i_w = 0;// write index
int i_r = 0; // read index

//___________________________________

const float pi = 3.14159;
const int  half = 134;//128;

float new_angle = 0.0; //input angle
float current_angle = 0.0; //current angle
float diff_angle = 0.0;
int val1 = 0;
int val2 = 0;


int IN1 = 3;
int IN2 = 4;
int VREF1 = 5;
int VREF2 = 6;
int IN3 = 7;
int IN4 = 8;
int pulse = 12;



void setup() {
  Serial.begin(115200);
  
  pinMode(VREF1, OUTPUT);
  pinMode(VREF2, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(pulse, OUTPUT);
  
  
  analogWrite(VREF1, 217);  
  analogWrite(VREF2, 217);  

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
 
  pinMode(ledPin, OUTPUT); // visual signal of I/O to chip
  pinMode(clockPin, OUTPUT); // SCK
  pinMode(CSnPin, OUTPUT); // CSn -- has to toggle high and low to signal chip to start data transfer
  pinMode(inputPin, INPUT); // SDA


}







void loop()
{
  while (Serial.available()) {

    char inChar = (char)Serial.read();
    
    if (inChar == 'p') {
      print_angle();
      delay(50);
    }
    
    else if (inChar == 's') {
      one_step();  
      print_angle();
    }
    
    else if (inChar == 'd') {
       if (dir == 1){
         dir = 0;
       }
       else {
         dir = 1;
       }
    }
    
    else if (inChar == 'c') {
       i_step = 0;
       i_w = 0;
    }
    
    else if (inChar == 'z') {
      a = readEncoder();
      anglefloat = a * 0.08789;
      while (anglefloat >= 0.9) {
        one_step();
        a = readEncoder();
        anglefloat = a * 0.08789;
        Serial.println(anglefloat,DEC);
      delay(50);        
      }
      delay(100);
      offset = readEncoder();
     }
     else if (inChar == 'g') {
       for(int x = 0; x < 400; x++){
        one_step();
        a = readEncoder();
        anglefloat = a * 0.08789;
        //Serial.print(i_step,DEC);
        //Serial.print(" , ");
        //Serial.print(i_step*0.9,DEC);
        //Serial.print(" , ");
        //Serial.println(a-offset, DEC);
        Serial.println(anglefloat, DEC);
       }
     } 
     
     else if (inChar == 'w') {
       for(int x = 0; x < 400; x++){
        one_step();
        a = readEncoder();
        i_w = 2*x;
        EEPROM.put(i_w,a);
        
        anglefloat = a * 0.08789;
        //Serial.print(i_step,DEC);
        //Serial.print(" , ");
        //Serial.print(i_step*0.9,DEC);
        //Serial.print(" , ");
        //Serial.println(a-offset, DEC);
        Serial.println(anglefloat, DEC);
       }
       
        for(long x = 0; x < 10; x++){
        i_r = 2*x;
        EEPROM.get(i_r,a);
        EEPROM.get(i_r+2,b);
        
        l = 1.0*a;
        m = 1.0*b;
        c = 1.0*(m-l);
        for(float y = 0.0; y < c; y = y+1.0){
//          Serial.println("------------------");
//          Serial.println(l,DEC);
//          Serial.println(m,DEC);
//          Serial.println(x,DEC);
//          Serial.println(y,DEC);
//          Serial.println(l+y,DEC);
//          Serial.println((y/c),DEC);
//          Serial.println((l+(y/c)),DEC);
          
          lookup[(int)(l+y)]= l + (y/c);
        }
         
         
       }
       Serial.println("lookup:");
       for(int z = 0; z < 100; z++){
      Serial.println(lookup[z]);
       }
     }    
      else if (inChar == 'r') {
       for(int x = 0; x < 400; x++){
        i_r = 2*x;
       
        EEPROM.get(i_r,a);
        
        anglefloat = a * 0.08789;
        //Serial.print(i_step,DEC);
        //Serial.print(" , ");
        //Serial.print(i_step*0.9,DEC);
        //Serial.print(" , ");
        //Serial.println(a-offset, DEC);
        Serial.println(a, DEC);
        //Serial.println(anglefloat, DEC);
       }
     }
   else if (inChar == 'a')  {
     Serial.println("Enter angle:");      //Prompt User for input
     while (Serial.available()==0)  {     //Wait for new angle
    }   
     update_angle();
    }    
       
     
     
}

}



float angle_to_encoder_reading(float);

float encoder_reading_to_angle(float);




void update_angle()
{
  new_angle=Serial.parseFloat();     
  diff_angle =(new_angle-current_angle);
  
  
  
  if (diff_angle > 0.05)  {
    while (diff_angle >= 0.05)  {
      current_angle +=0.1;
      
       digitalWrite(pulse, !digitalRead(pulse));
      
      val1 = 200*sin( (100*(current_angle*pi)/180) + (pi/4));
      analogWrite(VREF1, abs(val1));
      
      if (val1 >= 0)  {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2,LOW);
      }
      else  {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
      }
      val2 = 200*sin( (100*(current_angle*pi)/180) + (3*pi/4));
      analogWrite(VREF2, abs(val2));  
      
      if (val2 >= 0)  {
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4,LOW);
      }
      else  {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
      }
      
      
      //delay(1);
      Serial.print(current_angle);
      Serial.print(" , ");
      Serial.print(val1,DEC);
      Serial.print(" , ");
      Serial.println(val2,DEC);
      diff_angle =(new_angle-current_angle);  
    }

  }
  else if (diff_angle <= -0.05) {
        while (diff_angle <= -0.05)  {
      current_angle -=0.1;
       
       digitalWrite(pulse, !digitalRead(pulse));
      
      val1 = 200*sin( (100*(current_angle*pi)/180) + (pi/4));
      analogWrite(VREF1, abs(val1));
      
      if (val1 >= 0)  {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2,LOW);
      }
      else  {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
      }

      
      val2 = 200*sin( (100*(current_angle*pi)/180) + (3*pi/4));
      analogWrite(VREF2, abs(val2));  
      
      if (val2 >= 0)  {
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4,LOW);
      }
      else  {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
      }
      
      //delay(1);
      Serial.print(current_angle);
      Serial.print(" , ");
      Serial.print(val1,DEC);
      Serial.print(" , ");
      Serial.println(val2,DEC);
      diff_angle =(new_angle-current_angle);  
    }

    
    
  }

}








void one_step(){

  if (dir == 1) {
        i_step += 1;
        step_state += 1;
        if (step_state == 5){
          step_state = 1;
        }
  }
   else{
        i_step -= 1;
        step_state -= 1;
        if (step_state == 0){
          step_state = 4;
        }
  }
  Serial.println(dir,DEC);
  Serial.println(step_state,DEC);
  
      
  analogWrite(VREF1, 217);  
  analogWrite(VREF2, 217);  
    if (step_state == 1){
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
    }
    else if (step_state == 2){

        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
    }
    else if (step_state == 3){
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
    }
     else if (step_state == 4){
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        
     }
      delay(10);
}


void print_angle()
{
        a = readEncoder();
        anglefloat = a * 0.08789;
        Serial.print(i_step,DEC);
        Serial.print(" , ");
        Serial.print(i_step*0.9,DEC);
        Serial.print(" , ");
        Serial.print(a,DEC);
        Serial.print(" , ");
        //Serial.println(a-offset, DEC);
        Serial.println(anglefloat, DEC);
}

int readEncoder()
{
// CSn needs to cycle from high to low to initiate transfer. Then clock cycles. As it goes high
// again, data will appear on sda
  digitalWrite(CSnPin, HIGH); // CSn high
  digitalWrite(clockPin, HIGH); // CLK high
  delay(longdelay);// time between readings
  digitalWrite(ledPin, HIGH); // signal start of transfer with LED
  digitalWrite(CSnPin, LOW); // CSn low: start of transfer
  delayMicroseconds(shortdelay); // delay for chip initialization
  digitalWrite(clockPin, LOW); // CLK goes low: start clocking
  delayMicroseconds(shortdelay); // hold low
 // for (int x=0; x <16; x++) // clock signal, 16 transitions, output to clock pin
 for (int x=0; x <18; x++) // clock signal, 16 transitions, output to clock pin
  {
    digitalWrite(clockPin, HIGH); //clock goes high
    delayMicroseconds(shortdelay); //
    inputstream =digitalRead(inputPin); // read one bit of data from pin
//Serial.print(inputstream, DEC);
    packeddata = ((packeddata << 1) + inputstream);// left-shift summing variable, add pin value
    digitalWrite(clockPin, LOW);
    delayMicroseconds(shortdelay); // end of one clock cycle
  }
// end of entire clock cycle
//Serial.println(" ");
  digitalWrite(ledPin, LOW); // signal end of transmission
// lots of diagnostics for verifying bitwise operations
//Serial.print("packed:");
//Serial.println(packeddata,DEC);
//Serial.print("pack bin: ");
//Serial.println(packeddata,BIN);
  angle = packeddata & anglemask; // mask rightmost 6 digits of packeddata to zero, into angle.
//Serial.print("mask: ");
//Serial.println(anglemask, BIN);
//Serial.print("bin angle:");
//Serial.println(angle, BIN);
//Serial.print("angle: ");
//Serial.println(angle, DEC);
  angle = (angle >> 6); // shift 16-digit angle right 6 digits to form 10-digit value
//Serial.print("angleshft:");
//Serial.println(angle, BIN);
//Serial.print("angledec: ");
//Serial.println(angle, DEC);
//angle = angle * 0.3515; // angle * (360/1024) == actual degrees
  //anglefloat = angle * 0.08789; // angle * (360/4096) == actual degrees
  angletemp = angle;
  //Serial.print("angle: "); // and, finally, print it.
  
//  Serial.print(i_step,DEC);
//  Serial.print(" , ");
//  Serial.print(i_step*0.9,DEC);
//  Serial.print(" , ");
//  Serial.println(anglefloat-offset, DEC);
  
  
  
//Serial.println("--------------------");
//Serial.print("raw: "); // this was the prefix for the bit-by-bit diag output inside the loop.
  if (debug)
  {
    statusbits = packeddata & statusmask;
    DECn = statusbits & 2; // goes high if magnet moved away from IC
    INCn = statusbits & 4; // goes high if magnet moved towards IC
    LIN = statusbits & 8; // goes high for linearity alarm
    COF = statusbits & 16; // goes high for cordic overflow: data invalid
    OCF = statusbits & 32; // this is 1 when the chip startup is finished.
    if (DECn && INCn) { Serial.println("magnet moved out of range"); }
    else
    {
      if (DECn) { Serial.println("magnet moved away from chip"); }
      if (INCn) { Serial.println("magnet moved towards chip"); }
    }
    if (LIN) { Serial.println("linearity alarm: magnet misaligned? Data questionable."); }
    if (COF) { Serial.println("cordic overflow: magnet misaligned? Data invalid."); }
  }

  packeddata = 0; // reset both variables to zero so they don't just accumulate
  angle = 0;
  
//  return anglefloat;
return angletemp;
}

