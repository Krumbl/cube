#include <avr/pgmspace.h>

#define CUBESIZE 3
#define PLANESIZE CUBESIZE*CUBESIZE
#define PLANETIME 3333          // time each plane is displayed in us -> 100 Hz refresh
#define TIMECONST 20          // multiplies DisplayTime to get ms - why not =100?

//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 11;

int layer1 = 2;
int layer2 = 3;
int layer3 = 4;

int pin9 =  13;

int displayLayer = 0;  // current layer being displayed (0,1,2)

int PlanePin[] = {2, 3, 4};

// 3 4 5
// 2 9 6
// 1 8 7
// 12345678
int seq2[] = {
 B10101010, B01010101, B10101010,
 B01010101, B10101010, B01010101,
};
// layer1, layer2, layer3, pin9[123], time
int seq[] = {
  B00000000, B11111111, B00000000,
  B11100000, B00010001, B00001110,
  B00010001, B00010001, B00010001,
  B00001110, B00010001, B11100000, 
};
int seqLayers = 4;
int currentSeq = 0; // current sequence step being displayed

int lseqA[] = {
//  B111, B111, B111, B111, B111, B111, B111, B111, B111, 2,
//  B000, B000, B000, B000, B000, B000, B000, B000, B000, 1,  
  B111, B000, B000, B000, B000, B000, B000, B000, B000, 1,
  B000, B111, B000, B000, B000, B000, B000, B000, B000, 1,
  B000, B000, B111, B000, B000, B000, B000, B000, B000, 1,
  B000, B000, B000, B111, B000, B000, B000, B000, B000, 1,
  B000, B000, B000, B000, B111, B000, B000, B000, B000, 1,
  B000, B000, B000, B000, B000, B111, B000, B000, B000, 1,
  B000, B000, B000, B000, B000, B000, B111, B000, B000, 1,
  B000, B000, B000, B000, B000, B000, B000, B111, B000, 1,
  B000, B000, B000, B000, B000, B000, B000, B000, B111, 1,
};
int lseq2[] = {
//  B100, B100, B100, B100, B100, B100, B100, B100, B100, 1,
//  B010, B010, B010, B010, B010, B010, B010, B010, B010, 1,
//  B001, B001, B001, B001, B001, B001, B001, B001, B001, 1,
  B111, B000, B000, B000, B000, B000, B000, B000, B000, 1,
};
int lseq[] = {
  B111, B000, B000, B000, B000, B000, B000, B000, B111, 1,
  B000, B000, B000, B000, B000, B000, B000, B000, B000, 1,
};
int lseqsize = 2;
int lseqsize2 = 1;
int lseqsizeA = 9;
prog_uchar PROGMEM PatternTable[] = {
  B10101010,
  // blink on and off
//    B111, B111, B111, B111, B111, B111, B111, B111, B111, 5,
//    B000, B000, B000, B000, B000, B000, B000, B000, B000, 1,
//    B111, B111, B111, B111, B111, B111, B111, B111, B111, 5,
//    B000, B000, B000, B000, B000, B000, B000, B000, B000, 1,
};

void setup() {
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(layer1, OUTPUT);
  pinMode(layer2, OUTPUT);
  pinMode(layer3, OUTPUT);
  pinMode(pin9, OUTPUT);
  
  // logging?
  Serial.begin(9600);
  Serial.println("Hello world");
  delay(2000);// Give reader a chance to see the output.
}

void loop() {
  long start = millis();
  log(start);
  loadLayer();
  log(start + lseq[currentSeq * (PLANESIZE + 1) + 9] * 1000);
  // 32767 overflows int!
  while (millis() < start + lseq[currentSeq * (PLANESIZE + 1) + 9] * 1000) {
    // rotate next layer on
    rotate();
  }
  log("next");
  currentSeq = ++currentSeq % lseqsize;
}

void log(int data) {
  Serial.println(data);
}
void log(long data) {
  Serial.println(data);
}
void log(String data) {
  Serial.println(data);
}

void off() {
  digitalWrite(layer1, HIGH);
  digitalWrite(layer2, HIGH);
  digitalWrite(layer3, HIGH);
}

void on() {
  switch (displayLayer) {
    case 0:
      digitalWrite(layer1, LOW);
      //digitalWrite(layer2, HIGH);
      //digitalWrite(layer3, HIGH);
      break;
    case 1:
      //digitalWrite(layer1, HIGH);
      digitalWrite(layer2, LOW);
      //digitalWrite(layer3, HIGH);
      break;
    case 2:
      //digitalWrite(layer1, HIGH);
      //digitalWrite(layer2, HIGH);
      digitalWrite(layer3, LOW);
      break;
    default:
      // uh oh turn it all off
      digitalWrite(layer1, HIGH);
      digitalWrite(layer2, HIGH);
      digitalWrite(layer3, HIGH);
      break;
  }
}

int layer[] = {B00000000, B00000000, B00000000};
int layer9[] = {0,0,0};
void loadLayer() {
  log("load");
  log(currentSeq);
  
  int current = 0;
  for (int i = 0; i < 8; i++) {
    int x = lseq[currentSeq * (PLANESIZE + 1) + i] & B100;
    x = x >> 2;
    //log(x);
    x = x << (7 - i);
    current = current | x;
  }
  layer[0] = current;
  int x = lseq[currentSeq * (PLANESIZE + 1) + i] & B100;
  x = x >> 2;
  layer9[0] = x;  
  log(x);
  
  for (int i = 0; i < 8; i++) {
    int x = lseq[currentSeq * (PLANESIZE + 1) + i] & B010;
    x = x >> 1;
    x = x << (7 - i);
    current = current | x;
  }
  layer[1] = current;
  layer9[1] = lseq[((currentSeq * (PLANESIZE + 1) + 8) & B010) >> 1];
  
  for (int i = 0; i < 8; i++) {
    int x = lseq[currentSeq * (PLANESIZE + 1) + i] & B001;
    x = x << (7 - i);
    current = current | x;
  }
  layer[2] = current;
  layer9[1] = lseq[((currentSeq * (PLANESIZE + 1) + 8) & B001)];
  
//  log(layer[0]);
//  log(layer[1]);
//  log(layer[2]);
//  log(layer9[0]);
//  log(layer9[1]);
//  log(layer9[2]);
}

// rotate next layer in
void rotate() {
  // switch layer
  displayLayer = ++displayLayer % CUBESIZE;
  // load value in
  // take the latchPin low so 
  // the LEDs don't change while you're sending in bits:
  digitalWrite(latchPin, LOW);
  // shift out the bits:
  //shiftOut(dataPin, clockPin, LSBFIRST, lseq[currentSeq * (CUBESIZE + 1) + displayLayer]);
  shiftOut(dataPin, clockPin, LSBFIRST, layer[displayLayer]);
  
  // turn off before changing values to avoid any flicker
  off();
  
  // any work while off will cause dimming!
  //take the latch pin high so the new values get stored in
  digitalWrite(latchPin, HIGH);

  //digitalWrite(pin9, LOW);
  digitalWrite(pin9, layer9[displayLayer]);
  
  // turn back on
  on();
}
