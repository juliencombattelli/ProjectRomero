#include <CurieBLE.h>

BLEService remoteService("7DB9"); // create service

// create characteristics
BLEUnsignedIntCharacteristic state("D288", BLEWrite);
BLEUnsignedLongCharacteristic feedback("C15B", BLERead | BLENotify);
const int wait = 6;
int dir = 0, sonar = 0, count = wait;
int turbo = false, moving = false, mode = false, idle = false, new_mode = false;

void setup() {
  Serial.begin(9600);

  // begin initialization
  BLE.begin();

  // set the local name peripheral advertises
  BLE.setLocalName("RemoteReceiver");
  // set the UUID for the service this peripheral advertises
  BLE.setAdvertisedService(remoteService);

  // add the characteristic to the service
  remoteService.addCharacteristic(state);
  remoteService.addCharacteristic(feedback);
  // add service
  BLE.addService(remoteService);

  // assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // assign event handlers for characteristic
  state.setEventHandler(BLEWritten, stateCharacteristicWritten);

  // start advertising
  BLE.advertise();
  while (!Serial) ;
  Serial.println(("Bluetooth device active, waiting for connections..."));
}

void loop() {
  // poll for BLE events
  BLE.poll();

  /*while(1){
    if (mode){
      mode = false;
    } else {
      mode = true;
    }
    delay(10000);
  }*/
  
  if (new_mode != mode){
    Serial.print(mode);
    Serial.print(" --- ");
    Serial.println(new_mode);
    if (count){
      Serial.println(count);
      count--;
    } else {
      count = wait;
      new_mode = mode;
    }
  } else {
    count = wait;
  }
  
  int ret = 0;
  if (new_mode)
    ret = 1;
  ret = (ret + (dir << 1) + ((dir*4) << 3) + ((int(dir/2)) << 8) + (dir << 10) +  (dir << 13));
  feedback.setValue(ret);
  delay(500);
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

void stateCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic
  int current_state = state.value() >> 5;
  Serial.print("State: ");
  Serial.println(current_state);
  int current_direction = state.value() & 0x7;

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

  if (current_direction != dir) {
    dir = current_direction;
    Serial.print("Direction: ");
    Serial.println(dir);

    if (!moving) {
      if (dir != 7) {
        dir = 7;
      }
    }
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
