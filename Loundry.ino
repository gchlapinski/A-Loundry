#include <avr/wdt.h>
#include <avr/sleep.h>

//Disabling ADC saves ~230uAF. Needs to be re-enable for the internal voltage check
#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC

// pins
const byte relPowerPin = 2;
const byte relSignalPin = 4;
const byte dhtPowerPin = 1;
const byte dhtSignalPin = 3;
const byte ledSignalPin = 0;

#include <dht.h>
dht DHT22;
int hm = 0;
const int hmWarn = 650;
boolean hmOnOff = false;
boolean relOnOff = false;
boolean relSignal = false;
unsigned long relOnTime = 0;

// led blinking variable
unsigned long ledOnTime = 0;
const unsigned long ledBlinkFreq = 200;
unsigned long ledBlinkFreqRel = 0;
boolean ledOn = false;
byte blinkNo = 0;

// humidity reading frequency 
unsigned long lastHm = 0;
unsigned long lastHmOn = 0;
const unsigned long freqHm = 30000; // 900000
const unsigned long durHm = 10000; 

unsigned long oneMillis = 0;
const unsigned long freq = 1500;

/////////////////////////////////////// setup

void setup() {
  // watchdog
  //wdt_disable();
  // turn off adc
  adc_disable(); 

  // signalLed
  digitalWrite(dhtPowerPin, LOW);
  digitalWrite(relPowerPin, LOW);
  digitalWrite(relSignalPin, LOW);
  digitalWrite(ledSignalPin, LOW);
  pinMode(dhtPowerPin, OUTPUT);
  pinMode(relPowerPin, OUTPUT);
  pinMode(relSignalPin, OUTPUT);
  pinMode(ledSignalPin, OUTPUT);

  // blink three times 
  for (byte nr=0; nr<3; nr++) {
    digitalWrite(ledSignalPin, HIGH);
    delay(900);
    digitalWrite(ledSignalPin, LOW);
    delay(300);
  }

  delay(1000);
}

/////////////////////////////////////// loop
 
void loop() {
  oneMillis = millis(); 

  // turn on DHT
  if (hmOnOff == false) {
    hmOnOff = true;
    lastHmOn = oneMillis;
    blinkNo = 0;
    digitalWrite(dhtPowerPin, HIGH);
  } else if (oneMillis - lastHmOn > durHm 
             && hmOnOff == true 
             && relOnOff == false) {
    hmOnOff = false;
    digitalWrite(dhtPowerPin, LOW);
    // go sleep
    powerdownDelay(freqHm);
  }

  // humidity measure
  if (oneMillis - lastHm > freq && hmOnOff == true) {
    lastHm = oneMillis;
    readHumidity();
  }

  // turn on relay
  if (hm > hmWarn && relOnOff == false) {
    relOnTime = oneMillis;
    relOnOff = true;
    digitalWrite(relPowerPin, HIGH);
  } else if (hm <= hmWarn && relOnOff == true 
             && oneMillis - relOnTime > 10000
             && ledOn == false) {
    relOnOff = false; 
    relSignal = false;
    digitalWrite(relSignalPin, LOW);
    digitalWrite(relPowerPin, LOW);
  }

  if (oneMillis - relOnTime > 2000 && relOnOff == true && relSignal == false) {
    digitalWrite(relSignalPin, HIGH);
    relSignal = true;
  }

  // led indicator
  ledIndicator();
}

/////////////////////////////////////// function/procedures
void ledIndicator() {
  if (hmOnOff == true && relOnOff == false) {
    if (blinkNo < 2) {
      if (ledOn == false && oneMillis - ledOnTime > ledBlinkFreq) {
        ledOn = true;
        ledOnTime = oneMillis;
        digitalWrite(ledSignalPin, HIGH);    
      }

      if (ledOn == true && oneMillis - ledOnTime > ledBlinkFreq) {
        ledOn = false;
        ledOnTime = oneMillis;
        digitalWrite(ledSignalPin, LOW);
        blinkNo++;    
      }
    } else {
      if (oneMillis - ledOnTime > 5*ledBlinkFreq) {
        blinkNo = 0;
      }
    }
  }

  if (relOnOff == true) {
    if (hm > 900) {    
      ledBlinkFreqRel = 150;
    } else if (hm > 800) {
      ledBlinkFreqRel = 300;
    } else if (hm > 700) {
      ledBlinkFreqRel = 600;
    } else if (hm > 600) {
      ledBlinkFreqRel = 900;
    }
    
    if (ledOn == false && oneMillis - ledOnTime > ledBlinkFreqRel) {
      ledOn = true;
      ledOnTime = oneMillis;
      digitalWrite(ledSignalPin, HIGH);    
    }

    if (ledOn == true && oneMillis - ledOnTime > ledBlinkFreqRel) {
      ledOn = false;
      ledOnTime = oneMillis;
      digitalWrite(ledSignalPin, LOW);    
    }
  }

  if (hmOnOff == false && ledOn == true) {
    ledOn = false;
    digitalWrite(ledSignalPin, LOW);  
  }
}

void readHumidity() {
  int chk = DHT22.read(dhtSignalPin); 

  if (chk == DHTLIB_OK) {
    hm = floor(10*DHT22.humidity);  
  }
}

SIGNAL(WDT_vect) {
  wdt_disable();
  wdt_reset();

  WDTCR &= ~_BV(WDIE);
}

void powerdown(uint8_t wdt_period) {
  wdt_enable(wdt_period);
  wdt_reset();

  WDTCR |= _BV(WDIE);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // Set sleep enable (SE) bit:
  sleep_enable();

  //Going to sleep
  sleep_mode();

  //Woke up
  sleep_disable();
  
  wdt_disable();

  WDTCR &= ~_BV(WDIE);
}

void powerdownDelay(unsigned long milliseconds) {
  while (milliseconds >= 8000) { powerdown(WDTO_8S); milliseconds -= 8000; }
  if (milliseconds >= 4000) { powerdown(WDTO_4S); milliseconds -= 4000; }
  if (milliseconds >= 2000) { powerdown(WDTO_2S); milliseconds -= 2000; }
  if (milliseconds >= 1000) { powerdown(WDTO_1S); milliseconds -= 1000; }
  if (milliseconds >= 500) { powerdown(WDTO_500MS); milliseconds -= 500; }
  if (milliseconds >= 250) { powerdown(WDTO_250MS); milliseconds -= 250; }
  if (milliseconds >= 125) { powerdown(WDTO_120MS); milliseconds -= 120; }
  if (milliseconds >= 64) { powerdown(WDTO_60MS); milliseconds -= 60; }
  if (milliseconds >= 32) { powerdown(WDTO_30MS); milliseconds -= 30; }
  if (milliseconds >= 16) { powerdown(WDTO_15MS); milliseconds -= 15; }
}
