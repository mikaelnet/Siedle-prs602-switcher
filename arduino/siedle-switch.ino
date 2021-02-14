/*
 * Controller of Seidle programmer
 */

// HIGH drives the relays, LOW releases them
#define RELAY1  2
#define RELAY2  3
#define RELAY3  4

// LOW turns on leds, HIGH turns them off
#define LED1    5
#define LED2    6
#define LED3    7

#define SWITCH1 A0
#define SWITCH2 A1
#define SWITCH3 A2

#define RELAY_TIMEOUT   (10*60*1000L)
#define RELAY_TIMEOUT_WARN1 (8*60*1000L)
#define RELAY_TIMEOUT_WARN2 (9*60*1000L)

String inputString = "";
bool commandRecieved = false;

unsigned long relayTimer;

int relayPins[3] = { RELAY1, RELAY2, RELAY3 };
int ledPins[3] = { LED1, LED2, LED3 };
int switchPins[3] = { SWITCH1, SWITCH2, SWITCH3 };
int activeRelay = 0;

void resetTimer ()
{
  relayTimer = millis();
}

void setRelay (int relay)
{
  relay &= 0x03;    // Ensure not out of bounds
  resetTimer();
  // Do nothing if expected relay is already active
  if (relay == activeRelay) {
    return;
  }

  // Release all relays
  for (int i=0 ; i < 3 ; i ++) {
    digitalWrite (relayPins[i], LOW);
  }
  delay(100); // Wait a short while to ensure the relay is released

  // Enable relay, if any selected
  if (relay > 0) {
    digitalWrite (relayPins[relay-1], HIGH);
    delay(50);  // Wait a short while to ensure the relay is active
  }
  
  activeRelay = relay;
  Serial.println (relay);
}

void setLed (int led, bool on)
{
  led &= 0x03;
  if (led > 0) {
    digitalWrite (ledPins[led-1], on ? LOW : HIGH);
  }
}

void setAllLedOff ()
{
  for (int led=1 ; led <= 3 ; led ++) {
    setLed (led, false);
  }
}

int getButton ()
{
  for (int i=0 ; i < 3 ; i ++) {
    int btn = digitalRead(switchPins[i]);
    if (btn == LOW)
      return i+1;
  }
  return 0;
}

void updateInterface ()
{
  if (activeRelay == 0) 
    return;

  unsigned long timeLapsed = millis() - relayTimer;

  // Turn off led at timeout
  if (timeLapsed > RELAY_TIMEOUT) {
    setLed (activeRelay, false);
    setRelay (0);
    return;
  }

  // Fast blink close to timeout
  if (timeLapsed > RELAY_TIMEOUT_WARN2) {
    setLed (activeRelay, timeLapsed % 200 > 100);
    return;
  }

  // Slow blink before timeout
  if (timeLapsed > RELAY_TIMEOUT_WARN1) {
    setLed (activeRelay, timeLapsed % 1000 > 500);
    return;
  }
}

void invokeCommand (int relay) 
{
  setRelay (relay);
  setAllLedOff();
  setLed (relay, true);
}

void sendInfo() {
  // TODO: Add more content here
  Serial.println ("# Siedle output switcher");
  Serial.println ("# Send relay ID followed by a newline to activate port");
  Serial.println ("# Relay ID is 1, 2 or 3. ");
  Serial.println ("# Send 0 (zero) followed by newline to disable all relays.");
  Serial.println ("# All relays are automatically released after 5 minutes.");
  Serial.println ("# Empty newline repeats this message.");
}

void setup() {
  // Set the relay driver pins to output and 
  // ensure all drivers are disabled
  for (int i=0 ; i < 3 ; i ++) {
    pinMode (relayPins[i], OUTPUT);
    digitalWrite (relayPins[i], LOW); // Relays enables at HIGH

    pinMode (ledPins[i], OUTPUT);
    digitalWrite (ledPins[i], HIGH);  // Leds lit at LOW

    pinMode (switchPins[i], INPUT);   // Switches are pressed at LOW
  }
  
  inputString.reserve(80);

  Serial.begin(9600);
  while (!Serial) {
    // cycle leds here as boot
  }

  sendInfo();
  Serial.println ("0");
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\b') {   // Backspace. Remove last character
      int length = inputString.length();
      if (length > 0) {
        inputString.remove(length - 1);
      }
    }
    else if (inChar == '\r' || inChar == '\n') {
      commandRecieved = true;
    }
    else {
      inputString += inChar;
    }
  }
}

void loop() {
  updateInterface();
  int button = getButton();
  if (button > 0) {
    invokeCommand (button);
  }
  
  if (commandRecieved) {
    if (inputString.length() != 1) {
      sendInfo();
    }
    else {
      char ch = inputString[0];
      if (ch >= '0' && ch <= '3') {
        invokeCommand (ch - '0');        
      }
    }
    inputString = "";
    commandRecieved = false;
  }
}
