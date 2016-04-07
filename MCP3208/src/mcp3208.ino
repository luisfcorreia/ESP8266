// Modified to work with a NodeMCU v1.0 dev board from http://playground.arduino.cc/Code/MCP3208
//

// NodeMCU v1.0 SPI Pinout
// 
// HCS   -> GPIO15 - D8 
// HMOSI -> GPIO13 - D7
// HMISO -> GPIO12 - D6
// HSCLK -> GPIO14 - D5
//

// MCP3208 Pinout:
//        ___ 
// CH0 1 | u | 16 VCC
// CH1 2 |   | 15 VREF
// CH2 3 |   | 14 AGND
// CH3 4 |   | 13 CLK
// CH4 5 |   | 12 MISO
// CH5 6 |   | 11 MOSI
// CH6 7 |   | 10 CS
// CH7 8 |___| 9  DGND



// NodeMCU v1.0 Pinout from pins_arduino.h
static const uint8_t SDA = 4;
static const uint8_t SCL = 5;

static const uint8_t SS    = 15;
static const uint8_t MOSI  = 13;
static const uint8_t MISO  = 12;
static const uint8_t SCK   = 14;

static const uint8_t LED_BUILTIN = 16;
static const uint8_t BUILTIN_LED = 16;

static const uint8_t A0 = 17;

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;


#define SELPIN D8    //CS
#define DATAOUT D7   //MOSI
#define DATAIN D6    //MISO
#define SPICLOCK D5  //SCLK
int readvalue;

void setup(){
 //set pin modes
 pinMode(SELPIN, OUTPUT);
 pinMode(DATAOUT, OUTPUT);
 pinMode(DATAIN, INPUT);
 pinMode(SPICLOCK, OUTPUT);
 //disable device to start with
 digitalWrite(SELPIN,HIGH);
 digitalWrite(DATAOUT,LOW);
 digitalWrite(SPICLOCK,LOW);

 Serial.begin(115200);
}

int read_adc(int channel){
  int adcvalue = 0;
  byte commandbits = B11000000; //command bits - start, mode, chn (3), dont care (3)

  //allow channel selection
  commandbits|=((channel-1)<<3);

  digitalWrite(SELPIN,LOW); //Select adc
  // setup bits to be written
  for (int i=7; i>=3; i--){
    digitalWrite(DATAOUT,commandbits&1<<i);
    //cycle clock
    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);
  }

  digitalWrite(SPICLOCK,HIGH);    //ignores 2 null bits
  digitalWrite(SPICLOCK,LOW);
  digitalWrite(SPICLOCK,HIGH);
  digitalWrite(SPICLOCK,LOW);

  //read bits from adc
  for (int i=11; i>=0; i--){
    adcvalue+=digitalRead(DATAIN)<<i;
    //cycle clock
    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);
  }
  digitalWrite(SELPIN, HIGH); //turn off device
  return adcvalue;
}


void loop() {
 readvalue = read_adc(1);
 Serial.println(readvalue,DEC);
 readvalue = read_adc(2);
 Serial.println(readvalue,DEC);
 Serial.println(" ");
 delay(250);
}
