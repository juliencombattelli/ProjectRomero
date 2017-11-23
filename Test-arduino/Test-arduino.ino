#include <CurieBLE.h>

BLEService remoteService("7DB9"); // create service

// create characteristics
BLEUnsignedIntCharacteristic joystick("209D", BLEWrite);
BLEUnsignedIntCharacteristic state("D288", BLEWrite);
BLEUnsignedIntCharacteristic alert("DCB1", BLERead | BLENotify);
BLEUnsignedIntCharacteristic feedback("C15B", BLERead | BLENotify);

int dir = 0, sonar = 0, new_mode = 0;
int turbo = false, moving = false, mode = false, idle = false;

void setup() {
  Serial.begin(9600);

  // begin initialization
  BLE.begin();

  // set the local name peripheral advertises
  BLE.setLocalName("RemoteReceiver");
  // set the UUID for the service this peripheral advertises
  BLE.setAdvertisedService(remoteService);

  // add the characteristic to the service
  remoteService.addCharacteristic(joystick);
  remoteService.addCharacteristic(state);
  remoteService.addCharacteristic(feedback);
  remoteService.addCharacteristic(alert);

  // add service
  BLE.addService(remoteService);

  // assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // assign event handlers for characteristic
  joystick.setEventHandler(BLEWritten, joystickCharacteristicWritten);
  joystick.setValue(0);

  // assign event handlers for characteristic
  state.setEventHandler(BLEWritten, stateCharacteristicWritten);
  state.setValue(0);

  // start advertising
  BLE.advertise();
  while (!Serial) ;
  Serial.println(("Bluetooth device active, waiting for connections..."));
}

void loop() {
  // poll for BLE events
  BLE.poll();
}

void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}

void joystickCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic
  int current_joystick = joystick.value();
  int new_dir;
  Serial.print("Joystick event, written: ");
  Serial.println(current_joystick);
  if (moving) {
    if ((current_joystick >= 0) & (current_joystick <= 60)){
      new_dir = 1;
    } else if ((current_joystick > 60) & (current_joystick <= 120)){
      new_dir = 2;
    } else if ((current_joystick > 120) & (current_joystick <= 180)){
      new_dir = 3;
    } else {
      new_dir = 0;
    }
    if (new_dir != dir){
      dir = new_dir;
      Serial.print("direction: ");
      Serial.println(dir);
      feedback.setValue(dir);
    }
  }
  
}

void stateCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic
  Serial.print("State event, written: ");
  int current_state = state.value();
  Serial.println(current_state);
  
  switch (current_state) {
    case 0:
      idle = false;
      mode = false;
      moving = false;
      turbo = false;
      break;
    case 1:
      idle = true;
      mode = false;
      moving = false;
      turbo = false;
      break;
    case 2:
      idle = true;
      mode = false;
      moving = true;
      turbo = false;
      break;
    case 3:
      idle = true;
      mode = false;
      moving = false;
      turbo = true;
      break;
    case 4:
      idle = true;
      mode = false;
      moving = true;
      turbo = true;
      break;
    case 5:
      idle = false;
      mode = true;
      moving = false;
      turbo = false;
      break;
    case 6:
      idle = true;
      mode = true;
      moving = false;
      turbo = false;
      break;
  }
   
  /*Serial.print("idle: ");
  Serial.println(idle);
  Serial.print("mode: ");
  Serial.println(mode);
  Serial.print("moving: ");
  Serial.println(moving);
  Serial.print("turbo: ");
  Serial.println(turbo);*/
}
