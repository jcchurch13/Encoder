/* PWM Test for Stepper Driver
*/


#include <math.h>

const float pi = 3.14159;
const int  half = 134;//128;

float new_angle = 0.0; //input angle
float current_angle = 0.0; //current angle
float diff_angle = 0.0;
int val1 = 0;
int val2 = 0;


int IN1 = 3;
int IN2 = 4;
int ledPin1 = 5;
int ledPin2 = 6;
int IN3 = 7;
int IN4 = 8;



void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(halfpin, OUTPUT);
  analogWrite(ledPin1, 217);  
  analogWrite(ledPin2, 217);  
  analogWrite(halfpin, half);
}




void loop() {
  Serial.println("Enter angle:");      //Prompt User for input
  
  while (Serial.available()==0)  {     //Wait for new angle
  }  
  
  
  update_angle();

}





void update_angle()
{
  new_angle=Serial.parseFloat();     
  diff_angle =(new_angle-current_angle);
  if (diff_angle > 0.05)  {
    while (diff_angle >= 0.05)  {
      current_angle +=0.1;
      
      val1 = 128+ 127*sin( (100*(current_angle*pi)/180) + (pi/4));
      analogWrite(ledPin1, val1);  
      
      val2 = 128+ 127*sin( (100*(current_angle*pi)/180) + (3*pi/4));
      analogWrite(ledPin2, val2);  
      
      
      
      delay(10);
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
      
      val1 = 128+ 127*sin( (100*(current_angle*pi)/180) + (pi/4));
      analogWrite(ledPin1, val1);  
      
      val2 = 128+ 127*sin( (100*(current_angle*pi)/180) + (3*pi/4));
      analogWrite(ledPin2, val2);  
      
      
      delay(10);
      Serial.print(current_angle);
      Serial.print(" , ");
      Serial.print(val1,DEC);
      Serial.print(" , ");
      Serial.println(val2,DEC);
      diff_angle =(new_angle-current_angle);  
    }

    
    
  }

}
