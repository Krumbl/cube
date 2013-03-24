#define CUBESIZE 3
#define PLANESIZE CUBESIZE*CUBESIZE
// max number of sequence steps allowed - will fail silently and loop atmost 100
#define MAXSEQ 100
// TODO define seq data size (ie CUBESIZE + 1)


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

// TODO alternatively store as one int
// 	value = 1000 ^ layer * layerSeq
// 	max will be 511511511

int speed = 500;
int seqStart[] = {
	0, 0, 0, speed, 
	// columns
	1, 1, 1, speed,
	2, 2, 2, speed, 
	4, 4, 4, speed,
	8, 8, 8, speed, 
	16, 16, 16, speed,
	32, 32, 32, speed,
	64, 64, 64, speed,
	128, 128, 128, speed,
	256, 256, 256, speed,
	// rows
	511, 0, 0, speed,
	0, 511, 0, speed,
	0, 0, 511, speed,
	0, 511, 0, speed,
	511, 0, 0, speed,
	// slices - horizontal (123)
	7, 7, 7, speed,
	56, 56, 56, speed,
	448, 448, 448, speed,
	56, 56, 56, speed,
	7, 7, 7, speed,
	// slices - vertical (147)
	73, 73, 73, speed,
	146, 146, 146, speed,
	292, 292, 292, speed,
	146, 146, 146, speed,
	73, 73, 73, speed,
};


int seqBlink[] = {
	// binary formatter only works on 8 bits must split to get higher
	B00000001 * 256 + B11111111, 256 + B11111111, 256 + B11111111, 2000,
	B00000000, B00000000, B00000000, 1000,
};
int seq[MAXSEQ]; // the current seq being displayed
int seqSize = 25;

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

	chooseSeq(seqStart);
}

void loop() {
	long start = millis();
	log(start);

	loadLayer();

	while (millis() < start + seq[currentSeq * (CUBESIZE+ 1) + CUBESIZE]) {
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
int layer[CUBESIZE];
int layer9[CUBESIZE];
// load currentSeq layer
void loadLayer() {
	log("load");
	log(currentSeq);

	for (int i = 0; i < CUBESIZE; i++) {
		layer[i] = seq[currentSeq * (CUBESIZE + 1) + i] & B11111111;
		layer9[i] = seq[currentSeq * (CUBESIZE + 1) + i] >> 8;
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
	shiftOut(dataPin, clockPin, MSBFIRST, layer[displayLayer]);

	// turn off before changing values to avoid any flicker
	off();

	// any work while off will cause dimming!
	// take the latch pin high so the new values get stored in
	digitalWrite(latchPin, HIGH);

	digitalWrite(pin9, layer9[displayLayer]);

	// turn back on
	on();
}

// TODO use push button/dip switch to select layer
void chooseSeq(int chosen[]) {
	log("Load new seq");
	seqSize = sizeof(chosen) / sizeof(int);
	log(seqSize);
	seqSize = 25;
	for (int i = 0; i < seqSize * (CUBESIZE + 1); i++) {
		seq[i] = chosen[i];
		log(seq[i]);
	}
	log("Done load new seq");
}


// TODO - potentiometer = speed??
