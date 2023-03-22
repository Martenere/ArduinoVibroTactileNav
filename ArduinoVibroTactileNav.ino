/*
  LED

  This example creates a Bluetooth® Low Energy peripheral with service that contains a
  characteristic to control an LED.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use a generic Bluetooth® Low Energy central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

#define lookingForConnectionLed 3
#define connectionEstablishedLed 2
#define rightVibrationMotor 5
#define leftVibrationMotor 6

BLEService manouverService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLECharacteristic intensityCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, sizeof(uint16_t));

BLEIntCharacteristic actionTypeCharacteristic("1d57b590-bcd5-11ed-afa1-0242ac120002", BLERead | BLEWrite);

BLEIntCharacteristic vibrationProfileCharacteristic("ce2d2513-9515-488b-bbe0-2ebf9b886a37", BLERead | BLEWrite);


bool lostConnection = true;
int vibePin;
int vibeOn;
int vibeOff;
int reps;
int currentActionType = 0;
int vibrationProfile = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // set LED pin to output mode

  pinMode(lookingForConnectionLed, OUTPUT);
  pinMode(connectionEstablishedLed, OUTPUT);

  pinMode(leftVibrationMotor, OUTPUT);
  pinMode(rightVibrationMotor, OUTPUT);



  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("LEDLAMPA");
  BLE.setAdvertisedService(manouverService);

  // add the characteristic to the service
  manouverService.addCharacteristic(intensityCharacteristic);
  manouverService.addCharacteristic(actionTypeCharacteristic);
  manouverService.addCharacteristic(vibrationProfileCharacteristic);

  // add service
  BLE.addService(manouverService);

  // set the initial value for the characeristic:
  intensityCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();
  digitalWrite(lookingForConnectionLed, HIGH);
  digitalWrite(connectionEstablishedLed, LOW);
    // while the central is still connected to peripheral:
    while (central.connected()) {
      lostConnection = true;
      digitalWrite(lookingForConnectionLed, LOW);
      digitalWrite(connectionEstablishedLed, HIGH);
      
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      //update data ie left right
      if (actionTypeCharacteristic.written()){
        Serial.print(F("Recived action-value: "));
        Serial.println(actionTypeCharacteristic.value());
        currentActionType = actionTypeCharacteristic.value();
      }
      //Update intensity
      if (intensityCharacteristic.written()) {
        Serial.print(F("Recived  intensity-value: "));
        
        uint16_t* value_ptr = (uint16_t) intensityCharacteristic.value();
        uint16_t intensity = *value_ptr;

        Serial.println(intensity);
        signal(currentActionType, intensity);

        
      }
      //Update which profile/ of vibrations are sent
      if (vibrationProfileCharacteristic.written()) {
              vibrationProfile = vibrationProfileCharacteristic.value();
              Serial.print(F("Recived vibration-profile-value: "));
              Serial.println(vibrationProfileCharacteristic.value());
              
            }


    }

    // when the central disconnects, print it out:
    if(lostConnection){
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
    lostConnection = false;}
  }

void signal(int dir, int intensity){ //dir: 6/5  = left/right, intensity: 1/2/3
  Serial.println("signal evoked with the variables dir/intensity: " + String(dir)+ "/" + String(intensity) );
  bool both = false;
  int voltage = 182; //150 = 3v, x/5*255
  //set which motor vibrates
  switch(dir){
    case 1: both = false;vibePin = 6;break;//activate left motorpin
    case 2: both = false; vibePin = 5;break;//activate right motor pin
    case 3: both = true;vibePin = 0;break; //activate both pins
  }
  //set what intensity level the motor will vibrate in 
  setIntensity(intensity);

//Start vibration pattern 
  unsigned long currentTime;
  int x;
  both ? vibrateBothMotors(voltage): analogWrite(vibePin, voltage); //start vibration
  currentTime = millis();
  while (millis() - currentTime < vibeOn) {
    // wait for vibeOn milliseconds
  }
  for (x = 0 ; x < reps -1 ; x++){
    both ? vibrateBothMotors(0): analogWrite(vibePin, 0); //stop vibration
    currentTime = millis();
    while (millis() - currentTime < vibeOff) {
      // wait for vibeOff milliseconds
    }

    both ? vibrateBothMotors(voltage): analogWrite(vibePin, voltage); //start vibration
    currentTime = millis();
    while (millis() - currentTime < vibeOn) {
      // wait for vibeOn milliseconds
    }
  }
  both ? vibrateBothMotors(0): analogWrite(vibePin, 0);//stop vibration
  
 
}

void vibrateBothMotors(int amount){
      analogWrite(rightVibrationMotor, amount);
      analogWrite(leftVibrationMotor, amount);

}

void setIntensity(int intensity){
  if(vibrationProfile==0){
    Serial.println("set intensity with profile 0");
      switch(intensity){
      case 1:
        vibeOn = 300;
        vibeOff = 100;
        reps = 3;
        break;
      case 2:
        vibeOn = 300;
        vibeOff = 100;
        reps = 2;
        break;
      case 3:
        vibeOn = 500;
        vibeOff = 0;
        reps = 1;
        break;
    }
  }
  else if(vibrationProfile==1){
    
    Serial.println("set intensity with profile 1");
      switch(intensity){
      case 1:
        vibeOn = 600;
        vibeOff = 600;
        reps = 3;
        break;
      case 2:
        vibeOn = 300;
        vibeOff = 100;
        reps = 2;
        break;
      case 3:
        vibeOn = 100;
        vibeOff = 0;
        reps = 1;
        break;
    }
  }
    else {
      switch(intensity){
      case 1:
        vibeOn = 600;
        vibeOff = 600;
        reps = 3;
        break;
      case 2:
        vibeOn = 300;
        vibeOff = 100;
        reps = 2;
        break;
      case 3:
        vibeOn = 100;
        vibeOff = 0;
        reps = 1;
        break;
    }
  }
}