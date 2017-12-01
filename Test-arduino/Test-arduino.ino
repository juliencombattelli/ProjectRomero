#include <CurieBLE.h>

BLEService remoteService("7DB9"); // create service

// create characteristics
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
  remoteService.addCharacteristic(state);
  remoteService.addCharacteristic(feedback);
  remoteService.addCharacteristic(alert);

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
    int ret;
    switch (dir) {
      case 0:
      case 1:
        ret = 1;
        break;
      case 2:
        ret = 2;
        break;
      case 3:
      case 4:
        ret = 3;
        break;
      case 7:
      default:
        ret = 0;
        break;
    }
    ret = ret << 1;
    Serial.print("Retour: ");
    Serial.println(ret);
    feedback.setValue(ret);

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
