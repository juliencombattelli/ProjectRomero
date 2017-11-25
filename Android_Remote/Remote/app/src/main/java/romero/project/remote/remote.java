package romero.project.remote;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import java.util.List;
import java.util.UUID;

import io.github.controlwear.virtual.joystick.android.JoystickView;

import static android.bluetooth.BluetoothGattCharacteristic.FORMAT_UINT8;

public class remote extends AppCompatActivity {

    private final static String TAG = remote.class.getSimpleName();

    public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";

    boolean started;
    boolean connected;
    boolean turboed;
    boolean autonomous;
    boolean driving;

    String ice = "#94d3e2";
    String blue = "#0097bd";
    String night = "#002e39";
    String purple = "#842f7a";

    public Button connect;
    public Button auto;
    public Button start;
    public Button turbo;
    public JoystickView joystick;
    public ImageView front;
    public ImageView left;
    public ImageView right;
    public ImageView sonar;


    private BluetoothGatt btGatt;
    private BluetoothDevice btDevice;
    BluetoothGattCharacteristic stateCharacteristic;
    BluetoothGattCharacteristic joystickCharacteristic;
    BluetoothGattCharacteristic alertCharacteristic;
    BluetoothGattCharacteristic feedbackCharacteristic;

    Long uuidRemoteService = Long.parseLong("7DB9", 16);
    Long uuidJoystick = Long.parseLong("209D", 16);
    Long uuidState = Long.parseLong("D288", 16);
    Long uuidAlert = Long.parseLong("DCB1", 16);
    Long uuidFeedback = Long.parseLong("C15B", 16);


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_remote);
        Log.i(TAG, "Create\n");

        auto = findViewById(R.id.autonomous);
        connect = findViewById(R.id.connect);
        start = findViewById(R.id.move);
        turbo = findViewById(R.id.turbo);
        joystick = findViewById(R.id.joystick);
        front = findViewById(R.id.front);
        left = findViewById(R.id.left);
        right = findViewById(R.id.right);
        sonar = findViewById(R.id.sonar);

        setBt();

        setConnectButton();
        setStartButton();
        setAutoButton();
        setTurboButton();
        setJoystick();

        started = false;
        connected = false;
        autonomous = false;
        turboed = false;
        driving = false;

    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.i(TAG, "Start\n");
        //Todo
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "Resume\n");
        //Todo
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.i(TAG, "Pause\n");
        //Todo
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.i(TAG, "Stop\n");
        //Todo
        //if (btGatt != null) {
        //    stateCharacteristic.setValue(0, FORMAT_UINT8, 0);
        //    stateCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
        //    btGatt.writeCharacteristic(stateCharacteristic);
        //}
        //started = false;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "Close\n");
        //Todo
        //if (btGatt != null) {
        //    btGatt.disconnect();
        //}
        //connected = false;
    }

    /////////////////////////////////////////////////////////////////////////////
    //setup
    private void setBt() {
        String deviceAddress;
        BluetoothManager btManager;
        BluetoothAdapter btAdapter;
        final Intent intent = getIntent();
        deviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS);
        btManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        btAdapter = btManager.getAdapter();
        btDevice = btAdapter.getRemoteDevice(deviceAddress);
        btGatt = null;
    }

    private void setConnectButton() {
        //Connect button
        connexionChange(false);
        connect.getBackground().setColorFilter(Color.parseColor(night), PorterDuff.Mode.MULTIPLY);
        Log.i(TAG, "Buttons\n");

        connect.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (connected) {
                    if (btGatt != null) {
                        btGatt.disconnect();
                    }
                    Log.i(TAG, "disconnect\n");
                } else {
                    if (btGatt == null) {
                        connect.setText(R.string.connecting);
                        btGatt = btDevice.connectGatt(remote.this, false, gattCallback);
                    }
                    Log.i(TAG, "connect\n");
                }
            }
        });
    }

    private void setStartButton() {
        //start button
        start.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (connected) {
                    if (started) {
                        start.setText(R.string.start);
                        started = false;
                        turboed = false;
                        driving = false;
                        resetSymbols();
                        Log.i(TAG, "stop\n");
                        driving = false;
                        turbo.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                        if (btGatt != null) {
                            sendStateValue(7);
                        }
                    } else {
                        start.setText(R.string.stop);
                        started = true;
                        resetSymbols();
                        Log.i(TAG, "start\n");
                        if (!autonomous) {
                            joystick.setEnabled(true);
                            joystick.invalidate();
                        }
                        if (btGatt != null) {
                            sendStateValue(7);
                        }
                    }
                }
            }
        });
    }

    private void setTurboButton() {
        //turbo button
        turbo.setText(R.string.turbo);
        turbo.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (connected) {
                    if (started) {
                        if (turboed) {
                            turbo.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                            turboed = false;
                            Log.i(TAG, "turbo off\n");
                            if (btGatt != null) {
                                sendStateValue(7);
                            }
                        } else {
                            turbo.getBackground().setColorFilter(Color.parseColor(purple), PorterDuff.Mode.MULTIPLY);
                            turboed = true;
                            Log.i(TAG, "turbo on\n");
                            if (btGatt != null) {
                                sendStateValue(7);
                            }
                        }
                    }
                }
            }
        });
    }

    private void setAutoButton() {
        //auto button
        auto.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (connected) {
                    if (autonomous) {
                        auto.setText(R.string.manual);
                        autonomous = false;
                        started = false;
                        turboed = false;
                        driving = false;
                        resetSymbols();
                        Log.i(TAG, "manual\n");
                        start.setText(R.string.start);
                        turbo.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                        driving = false;
                        if (btGatt != null) {
                            sendStateValue(7);
                        }
                    } else {
                        auto.setText(R.string.autonomous);
                        autonomous = true;
                        started = false;
                        turboed = false;
                        driving = false;
                        resetSymbols();
                        Log.i(TAG, "autonomous\n");
                        start.setText(R.string.start);
                        turbo.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
                        driving = false;
                        if (btGatt != null) {
                            sendStateValue(7);
                        }
                    }
                }
            }
        });
    }

    private void setJoystick() {
        joystick.setOnMoveListener(new JoystickView.OnMoveListener() {

            @Override
            public void onMove(int angle, int strength) {
                if (connected) {
                    if (started) {
                        if (strength > 50) {
                            driving = true;
                            Log.i(TAG, "degrees:" + angle + "\n");
                            sendJoystickValue(angle);
                        }
                        if (strength < 20 & driving) {
                            driving = false;
                            Log.i(TAG, "stopped");
                            if (btGatt != null) {
                                sendStateValue(7);
                            }
                        }
                    }
                }
            }
        }, 200);
    }

    ////////////////////////////////////////////////////////////////////////////////
    //callbacks
    public final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {

        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                resetSymbols();
                connexionChange(true);
                connected = true;
                Log.i(TAG, "Connected to GATT server\n");
                btGatt.requestConnectionPriority(BluetoothGatt.CONNECTION_PRIORITY_HIGH);
                btGatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                resetSymbols();
                connexionChange(false);
                connected = false;
                Log.i(TAG, "Disconnected from GATT server\n");
                btGatt.close();
                btGatt = null;
            }

        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                gatt.disconnect();
                btGatt = null;
                return;
            }
            Log.i(TAG, "service discovered\n");
            List<BluetoothGattService> services = gatt.getServices();
            long uuid = 0;
            // Loop through available GATT Services.
            for (BluetoothGattService gattService : services) {
                uuid = gattService.getUuid().getMostSignificantBits() >> 32;
                Log.i(TAG, uuid + "\n");
                if (uuid == uuidRemoteService) {
                    Log.i(TAG, "found uuid\n");
                    List<BluetoothGattCharacteristic> gattCharacteristics = gattService.getCharacteristics();
                    // Loop through available Characteristics.
                    for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {
                        uuid = gattCharacteristic.getUuid().getMostSignificantBits() >> 32;
                        Log.i(TAG, uuid + "\n");
                        if (uuid == uuidJoystick) {
                            joystickCharacteristic = gattCharacteristic;
                            Log.i(TAG, "found joystick\n");
                        } else if (uuid == uuidState) {
                            stateCharacteristic = gattCharacteristic;
                            Log.i(TAG, "found state\n");
                        } else if (uuid == uuidAlert) {
                            alertCharacteristic = gattCharacteristic;
                            Log.i(TAG, "found alert\n");
                            enableNotification(btGatt, alertCharacteristic);
                        } else if (uuid == uuidFeedback) {
                            feedbackCharacteristic = gattCharacteristic;
                            Log.i(TAG, "found feedback\n");
                            enableNotification(btGatt, feedbackCharacteristic);
                        }
                    }
                }
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic,
                                         int status) {
            int a = characteristic.getIntValue(FORMAT_UINT8, 0);
            Log.i(TAG, characteristic + "" + a + "\n");
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic,
                                          int status) {
            Log.i(TAG, "wrote\n");
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            Log.i(TAG, "notification!");
            final int data = characteristic.getIntValue(FORMAT_UINT8, 0);
            if (characteristic == feedbackCharacteristic) {
                Log.i(TAG, "data: " + data + "\n");
                changeDir(data);
            }
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
    //functions
    private void sendStateValue(int joystick_value) {
        int stateCode;
        int message;
        if (!turboed & !driving & !autonomous & started)
            stateCode = 1;
        else if (!turboed & driving & !autonomous & started)
            stateCode = 2;
        else if (turboed & !driving & !autonomous & started)
            stateCode = 3;
        else if (turboed & driving & !autonomous & started)
            stateCode = 4;
        else if (!turboed & !driving & autonomous & !started)
            stateCode = 5;
        else if (!turboed & !driving & autonomous & started)
            stateCode = 6;
        else
            stateCode = 0;

        Log.i(TAG, "state: " + stateCode + "\n");
        Log.i(TAG, "joystick: " + joystick_value + "\n");
        message = stateCode << 5;
        Log.i(TAG, "new state: " + message + "\n");
        message = message + joystick_value;
        Log.i(TAG, "complete message: " + message + "\n");

        //todo comment
        stateCharacteristic.setValue(message, FORMAT_UINT8, 0);
        stateCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
        btGatt.writeCharacteristic(stateCharacteristic);
    }

    private void sendJoystickValue(int joystick_angle) {
        int joystick_value;
        if (joystick_angle <= 20 | joystick_angle > 340)
            joystick_value = 0;
        else if (joystick_angle <= 65 & joystick_angle > 20)
            joystick_value = 1;
        else if (joystick_angle <= 110 & joystick_angle > 65)
            joystick_value = 2;
        else if (joystick_angle <= 155 & joystick_angle > 110)
            joystick_value = 3;
        else if (joystick_angle <= 200 & joystick_angle > 155)
            joystick_value = 4;
        else
            joystick_value = 7;

        sendStateValue(joystick_value);
    }

    private void enableNotification(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
        gatt.setCharacteristicNotification(characteristic, true);
        Log.i(TAG, "notification\n");
        // 0x2902 org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
        UUID uuid = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(uuid);
        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        gatt.writeDescriptor(descriptor);
    }


    private void changeDir(final int direction_value) {
        runOnUiThread(new Runnable() {
            public void run() {
                switch (direction_value) {
                    case 0:
                        front.setImageResource(R.drawable.ic_arrow_front_ice);
                        right.setImageResource(R.drawable.ic_arrow_right_ice);
                        left.setImageResource(R.drawable.ic_arrow_left_ice);
                        break;
                    case 1:
                        front.setImageResource(R.drawable.ic_arrow_front_ice);
                        right.setImageResource(R.drawable.ic_arrow_right_blue);
                        left.setImageResource(R.drawable.ic_arrow_left_ice);
                        break;
                    case 2:
                        front.setImageResource(R.drawable.ic_arrow_front_blue);
                        right.setImageResource(R.drawable.ic_arrow_right_ice);
                        left.setImageResource(R.drawable.ic_arrow_left_ice);
                        break;
                    case 3:
                        front.setImageResource(R.drawable.ic_arrow_front_ice);
                        right.setImageResource(R.drawable.ic_arrow_right_ice);
                        left.setImageResource(R.drawable.ic_arrow_left_blue);
                        break;
                }
            }
        });
    }

    private void connexionChange(boolean connexion) {
        if (connexion) {
            runOnUiThread(new Runnable() {
                public void run() {
                    auto.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                    start.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                    turbo.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                    start.setText(R.string.start);
                    auto.setText(R.string.manual);
                    connect.setText(R.string.disconnect);
                    started = false;
                    turboed = false;
                    autonomous = false;
                    driving = false;
                }
            });
        } else {
            runOnUiThread(new Runnable() {
                public void run() {
                    auto.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
                    start.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
                    turbo.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
                    start.setText(R.string.start);
                    auto.setText(R.string.manual);
                    connect.setText(R.string.connect);
                }
            });

        }
    }

    public void resetSymbols() {
        runOnUiThread(new Runnable() {
            public void run() {
                joystick.setEnabled(false);
                joystick.resetButtonPosition();
                joystick.invalidate();
                front.setImageResource(R.drawable.ic_arrow_front_ice);
                right.setImageResource(R.drawable.ic_arrow_right_ice);
                left.setImageResource(R.drawable.ic_arrow_left_ice);
                sonar.setImageResource(R.drawable.ic_sonar);
            }
        });
    }

}