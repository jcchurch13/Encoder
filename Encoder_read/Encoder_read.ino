



const int ledPin = 13; //LED connected to digital pin 13
const int clockPin = 7; //output to clock
const int CSnPin = 6; //output to chip select
const int inputPin = 2; //read AS5040

int inputstream = 0; //one bit read from pin
long packeddata = 0; //two bytes concatenated from inputstream
long angle = 0; //holds processed angle value
float anglefloat = 0; 
//long anglemask = 65472; //0x1111111111000000: mask to obtain first 10 digits with position info
long anglemask = 262080; // 0x111111111111000000: mask to obtain first 12 digits with position info
long statusmask = 63; //0x000000000111111; mask to obtain last 6 digits containing status info
long statusbits; //holds status/error information
int DECn; //bit holding decreasing magnet field error data
int INCn; //bit holding increasing magnet field error data
int OCF; //bit holding startup-valid bit
int COF; //bit holding cordic DSP processing error data
int LIN; //bit holding magnet field displacement error data
int debug = 0; //SET THIS TO 0 TO DISABLE PRINTING OF ERROR CODES
int shortdelay = 100; // this is the microseconds of delay in the data clock
int longdelay = 10; // this is the milliseconds between readings

void setup()
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT); // visual signal of I/O to chip
  pinMode(clockPin, OUTPUT); // SCK
  pinMode(CSnPin, OUTPUT); // CSn -- has to toggle high and low to signal chip to start data transfer
  pinMode(inputPin, INPUT); // SDA
}

void loop()
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
  anglefloat = angle * 0.08789; // angle * (360/4096) == actual degrees
  Serial.print("angle: "); // and, finally, print it.
  Serial.println(anglefloat, DEC);
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
}








//const int ledPin = 13; // LED connected to digital pin 13, used as a heartbeat
//const int clockPin = 7; // output to clock
//const int CSnPin = 6; // output to chip select
//const int inputPin = 2; // read AS5045
//
//int inputstream = 0; // one bit read from pin
//long packeddata = 0; // two bytes concatenated from inputstream
//long angle = 0; // holds processed angle value
//long anglemask = 262080; // 0x111111111111000000: mask to obtain first 12 digits with position info
//long statusmask = 63; // 0x000000000000111111; mask to obtain last 6 digits containing status info
//long statusbits; // holds status/error information
//int DECn; // bit holding decreasing magnet field error data
//int INCn; // bit holding increasing magnet field error data
//int OCF; // bit holding startup-valid bit
//int COF; // bit holding cordic DSP processing error data
//int LIN; // bit holding magnet field displacement error data
//int debug = 1; // SET THIS TO 0 TO DISABLE PRINTING OF ERROR CODES
//
//void setup()
//{
//Serial.begin(9600);
//pinMode(ledPin, OUTPUT); // visual signal of I/O to chip: heartbeat
//pinMode(clockPin, OUTPUT); // SCK
//pinMode(CSnPin, OUTPUT); // CSn -- has to toggle high and low to signal chip to start data transfer
//pinMode(inputPin, INPUT); // SDA
//}
//
//void loop()
//{
//// CSn needs to cycle from high to low to initiate transfer. Then clock cycles. As it goes high
//// again, data will appear on sda
//digitalWrite(CSnPin, HIGH); // CSn high
//digitalWrite(clockPin, HIGH); // CLK high
//delay(1000); // wait for 1 second for no particular reason
//digitalWrite(ledPin, HIGH); // signal start of transfer with LED
//digitalWrite(CSnPin, LOW); // CSn low: start of transfer
//delay(100); // delay for chip -- 1000x as long as it needs to be
//digitalWrite(clockPin, LOW); // CLK goes low: start clocking
//delay(10); // hold low for 10 ms
//for (int x=0; x < 18; x++) // clock signal, 18 transitions, output to clock pin
//{
//digitalWrite(clockPin, HIGH); // clock goes high
//delay(10); // wait 10ms
//inputstream = digitalRead(inputPin); // read one bit of data from pin
////Serial.print(inputstream, DEC); // useful if you want to see the actual bits
//packeddata = ((packeddata << 1) + inputstream); // left-shift summing variable, add pin value
//digitalWrite(clockPin, LOW);
//delay(10); // end of one clock cycle
//} // end of entire clock cycle
////Serial.println(" ");
//digitalWrite(ledPin, LOW); // signal end of transmission
//// lots of diagnostics for verifying bitwise operations
////Serial.print("packed:");
////Serial.println(packeddata,DEC);
////Serial.print("pack bin: ");
//// Serial.println(packeddata,BIN);
//angle = packeddata & anglemask; // mask rightmost 6 digits of packeddata to zero, into angle.
////Serial.print("mask: ");
////Serial.println(anglemask, BIN);
////Serial.print("bin angle:");
////Serial.println(angle, BIN);
////Serial.print("angle: ");
////Serial.println(angle, DEC);
//angle = (angle >> 6); // shift 18-digit angle right 6 digits to form 12-digit value
////Serial.print("angleshft:");
////Serial.println(angle, BIN);
////Serial.print("angledec: ");
////Serial.println(angle, DEC);
//angle = angle * 0.08789; // angle * (360/4096) == actual degrees
//Serial.print("angle: "); // and, finally, print it.
//Serial.println(angle, DEC);
////Serial.println("--------------------");
////Serial.print("raw: "); // this was the prefix for the bit-by-bit diag output inside the loop.
//if (debug)
//{
//statusbits = packeddata & statusmask;
//DECn = statusbits & 2; // goes high if magnet moved away from IC
//INCn = statusbits & 4; // goes high if magnet moved towards IC
//LIN = statusbits & 8; // goes high for linearity alarm
//COF = statusbits & 16; // goes high for cordic overflow: data invalid
//OCF = statusbits & 32; // this is 1 when the chip startup is finished.
//if (DECn && INCn) { Serial.println("magnet moved out of range"); }
//else
//{
//if (DECn) { Serial.println("magnet moved away from chip"); }
//if (INCn) { Serial.println("magnet moved towards chip"); }
//}
//if (LIN) { Serial.println("linearity alarm: magnet misaligned? Data questionable."); }
//if (COF) { Serial.println("cordic overflow: magnet misaligned? Data invalid."); }
//}
//
//packeddata = 0; // reset both variables to zero so they don't just accumulate
//angle = 0;
//}
