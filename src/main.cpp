#include <Arduino.h>
#include <EEPROM.h>

uint8_t state = 0; // 0 == show menu, 1 == answerMenu, 2 == start timer, 3 == show lap history
uint8_t BT = 62;
uint8_t lapNum = 0; 

unsigned long timer = 0;
unsigned long now;
unsigned long lastTickTime = 0; // Tracks the last time the stopwatch ticked up
unsigned long lastDebounceTime[3] = {0, 0, 0}; // [2] == lapButton


bool currentState[3] ={HIGH, HIGH, HIGH}; // [2] == lapButton.
bool preState[3] = {HIGH, HIGH, HIGH}; // [2] == lapButton.

bool lapRec = false;
volatile uint8_t buttons[3] = {2, 10, 3}; //[0] == startButton, [1] == lapHistoryButton, [2] == lapButton

void showMenu();
void menuAnswer();
void startTimer();
void showLapHistory();

void lap(){
  if(state == 2){
    if(digitalRead(buttons[2]) == LOW){
      if(millis() - lastDebounceTime[2] > 200) {
        lastDebounceTime[2] = millis();
        lapRec = true;
      }
    }
  }
}


void stop(){
  if(state == 2){
    if(digitalRead(buttons[0]) == LOW){
      if(millis() - lastDebounceTime[0] > 200){
        lastDebounceTime[0] = millis();
        state = 0;
      }
    }
  }
}


void setup() {
  Serial.begin(115200);
  while(!Serial){;}

  for(int i = 0; i <= 2; i++){
    pinMode(buttons[i], INPUT_PULLUP);
  }

  if(EEPROM.read(0) != BT){
    for(int i = 1; i <= 255; i++){
      EEPROM.write(i, 0);
    }
    EEPROM.write(0, BT);
  }

  attachInterrupt(digitalPinToInterrupt(buttons[2]), lap, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttons[0]), stop, CHANGE);

}

void loop() {
  now = millis();
  if(state == 0){
    showMenu();
  }
  else if(state == 1){
    menuAnswer();
  }
  else if(state == 2){
    startTimer();
   
  }
  else if(state == 3){
    showLapHistory();
  }
}

void showMenu(){
  Serial.println("What would you like to do?: ");
  Serial.println("1. Start new timer(press right button).");
  Serial.println("2. See past lap times(press left button).");
  state = 1;
}

void menuAnswer(){
    for(int i = 0; i <= 1; i++){
      currentState[i] = digitalRead(buttons[i]);
      if(currentState[i] != preState[i]){
        if(now - lastDebounceTime[i] >= 70){
          if(currentState[i] == LOW){
            if(i == 0){
              lastTickTime = millis();
              timer = 0;
              state = 2;
            }
            else if(i == 1){
            state = 3;
          }
        }
        lastDebounceTime[i] = now;
      }
      preState[i] = currentState[i];
    }
  }
}

void startTimer(){
  // Check every 10 milliseconds (1 hundredth of a second)
  if(millis() - lastTickTime >= 10){
    timer++;
    lastTickTime = millis();

    // 1 second = 100 ticks. 1 minute = 6000 ticks. 1 hour = 360000 ticks.
    unsigned long hours = timer / 360000;
    unsigned long minutes = (timer % 360000) / 6000;
    unsigned long seconds = (timer % 6000) / 100;
    unsigned long units = timer % 100; // This isolates the 00-99 hundredths

    // Hours
    if(hours < 10){
    Serial.print("0");
    }
    Serial.print(hours);
    Serial.print(" : ");

    // Minutes
    if(minutes < 10){
    Serial.print("0");
  }
    Serial.print(minutes);
    Serial.print(" : ");

    // Seconds
    if(seconds < 10){
      Serial.print("0");
    }
    Serial.print(seconds);
    Serial.print(" . "); // Using a dot for hundredths looks cleaner!

    // Hundredths of a second
    if(units < 10){
      Serial.print("0");
    }
    Serial.print(units);

    Serial.println(); 
  }

  // Lap recording (saves the raw 10ms tick count to EEPROM)
  if(lapRec){
    lapRec = false;

    lapNum = EEPROM.read(1);
    if(lapNum > 62){
      lapNum = 0;
    }
      
    EEPROM.put(2 + (lapNum * sizeof(unsigned long)), timer);
    EEPROM.write(1, lapNum + 1);
  }
}

void showLapHistory(){
  Serial.println("--- Stored Lap History ---");
  
  for(int i = 0; i < 63; i++){ 
    unsigned long lapTime;
    EEPROM.get(2 + (i * sizeof(unsigned long)), lapTime);
    
    if(lapTime != 0){
      // Convert saved 10ms ticks back to HH:MM:SS.hh
      unsigned long h = lapTime / 360000;
      unsigned long m = (lapTime % 360000) / 6000;
      unsigned long s = (lapTime % 6000) / 100;
      unsigned long u = lapTime % 100;

      Serial.print("Lap ");
      Serial.print(i + 1);
      Serial.print(" -> ");

      if(h < 10){
        Serial.print("0");
      }
      Serial.print(h);
      Serial.print(" : ");

      if(m < 10){
        Serial.print("0");
      }
      Serial.print(m);
      Serial.print(" : ");

      if(s < 10){
        Serial.print("0");
      }
      Serial.print(s);
      Serial.print(" . ");

      if(u < 10){
        Serial.print("0");
      }
      Serial.print(u);
      
      Serial.println();
    }
  }
  delay(300); 
  state = 0; 
}