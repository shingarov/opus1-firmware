#include <WProgram.h>
#include <avr/sleep.h>

void setup();
void loop();

static char s[10];

int getStatus(int out, int in) {
  return (s[out] >> in) & 1;
}

// activate
void turnDown(int out, int in) {
  s[out] &= ~(1<<in);
}

// deactivate
void turnUp(int out, int in) {
  s[out] |= 1<<in;
}

void setup() {
  int l;
  for (l=0; l<10; l++) s[l]=0x3F;

  Serial.begin(9600);
  
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);
  digitalWrite(A5, HIGH);
}


void lineCycle(int outputLine) {
  pinMode(outputLine+2, OUTPUT);
  digitalWrite(outputLine+2, LOW);
  delay(2);

  for (int in=0; in<6; in++) {
    int x = digitalRead(A0+in);
    if (x == LOW) {
      if (getStatus(outputLine, in)) {
        turnDown(outputLine, in);
        int scancode = in<<4 | outputLine;
        Serial.println(scancode, HEX);
    }
      
  }
  if (x==HIGH) {
    if (!getStatus(outputLine,in)) {
      turnUp(outputLine,in);
      Serial.println("RELEASED!!!");
    }
  }
  }

  pinMode(outputLine+2, INPUT);
}

void loop() {
  int outputLine;
  for (outputLine=0; outputLine<10; outputLine++) {
    lineCycle(outputLine);
  }
}


