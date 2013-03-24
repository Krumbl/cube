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

// 1 2 3
// 4 5 6
// 7 8 9
// 987654321

int seqBlink[] = {
	// binary formatter only works on 8 bits must split to get higher
	B00000001 * 256 + B11111111, 256 + B11111111, 256 + B11111111, 2,
	B00000000, B00000000, B00000000, 1,
};
//int maxSeq = 10; // max number of sequence steps allowed
//int seq[maxSeq]; // the current seq being displayed
int seqSize = 1;

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

	while (millis() < start + seqBlink[currentSeq * (CUBESIZE+ 1) + CUBESIZE] * 1000) {
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
int layer[CUBESIZE];// = {B00000000, B00000000, B00000000};
int layer9[CUBESIZE];// = {0,0,0};
// load currentSeq layer
void loadLayer() {
	log("load");
	log(currentSeq);

	for (int i = 0; i < CUBESIZE; i++) {
		layer[i] = seqBlink[currentSeq * (CUBESIZE + 1) + i] & B11111111;
		layer9[i] = seqBlink[currentSeq * (CUBESIZE + 1) + i] >> 8;
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

