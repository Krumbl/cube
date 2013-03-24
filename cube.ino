#define CUBESIZE 3
#define PLANESIZE CUBESIZE*CUBESIZE


//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
//Pin connected to DS of 74HC595
int dataPin = 11;

int layer1 = 2;
int layer2 = 3;
int layer3 = 4;

int pin9 =  13;

int displayLayer = 0;  // current layer being displayed (0,1,2)
int currentSeq = 0; // current sequence step being displayed

int PlanePin[] = {2, 3, 4};

// 3 4 5
// 2 9 6
// 1 8 7
// 12345678

int seqBlink[] = {
	B111, B111, B111, B111, B111, B111, B111, B111, B101, 5,
	B000, B000, B000, B000, B000, B000, B000, B000, B010, 1,
	B000, B000, B000, B000, B000, B000, B000, B000, B000, 1,
};
//int maxSeq = 10; // max number of sequence steps allowed
//int seq[maxSeq]; // the current seq being displayed
int seqSize = 3;

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
	Serial.println("Setup");
	delay(2000);// Give reader a chance to see the output.
}

void loop() {
	long start = millis();
	log(start);

	loadLayer();

	while (millis() < start + seqBlink[currentSeq * (PLANESIZE + 1) + 9] * 1000) {
		rotate();
	}

	// move to next layer
	currentSeq = ++currentSeq % seqSize;
}

// LOG methods send to Serial
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

// current layer values
int layer[] = {B00000000, B00000000, B00000000};
int layer9[] = {0,0,0};
// load currentSeq layer
void loadLayer() {
	log("load");
	log(currentSeq);

	int current = 0;

	// iterate the values in the sequence
	// rearrange from rows to layers
	for (int i = 0; i < 8; i++) {
		// get the value of layer0
		int x = seqBlink[currentSeq * (PLANESIZE + 1) +i] & B100;
		// shift into place
		x = x >> 2;
		x = x<< (7 - i);
		// append to values
		current = current | x;
	}
	layer[0] = current;
	{
		int x = seqBlink[currentSeq * (PLANESIZE + 1) + 8] & B100;
		x = x >> 2;
		layer9[0] = x;  
	}

	// iterate the values in the sequence
	// rearrange from rows to layers
	for (int i = 0; i < 8; i++) {
		// get the value of layer0
		int x = seqBlink[currentSeq * (PLANESIZE + 1) +i] & B010;
		// shift into place
		x = x >> 1;
		x = x<< (7 - i);
		// append to values
		current = current | x;
	}
	layer[1] = current;
	{
		int x = seqBlink[currentSeq * (PLANESIZE + 1) + 8] & B010;
		x = x >> 1;
		layer9[1] = x;
	}

	// iterate the values in the sequence
	// rearrange from rows to layers
	for (int i = 0; i < 8; i++) {
		// get the value of layer0
		int x = seqBlink[currentSeq * (PLANESIZE + 1) +i] & B001;
		// shift into place
		x = x<< (7 - i);
		// append to values
		current = current | x;
	}
	layer[2] = current;
	{
		int x = seqBlink[currentSeq * (PLANESIZE + 1) + 8] & B001;
		layer9[2] = x;
	}
}


// rotate next layer in and display
void rotate() {
  // switch layer to next layer
  displayLayer = ++displayLayer % CUBESIZE;
  // load value in
  // take the latchPin low so 
  // the LEDs don't change while you're sending in bits:
  digitalWrite(latchPin, LOW);

  // shift out the bits:
  shiftOut(dataPin, clockPin, LSBFIRST, layer[displayLayer]);
  
  // turn off before changing values to avoid any flicker
  off();
  
  // any work while off will cause dimming!
  // take the latch pin high so the new values get stored in
  digitalWrite(latchPin, HIGH);

  digitalWrite(pin9, layer9[displayLayer]);
  
  // turn back on
  on();
}

