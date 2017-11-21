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
    String night =  "#002e39";
    String purple =  "#842f7a";

    public Button connect;
    public Button auto;
    public Button start;
    public Button turbo;
    public JoystickView joystick;


    private BluetoothGatt btGatt;
    private BluetoothDevice btDevice;
    BluetoothGattCharacteristic stateCharacteristic;
    BluetoothGattCharacteristic joystickCharacteristic;
    BluetoothGattCharacteristic batteryLevelCharacteristic;
    BluetoothGattCharacteristic alertCharacteristic;

    Long uuidRemoteService = Long.parseLong("7DB9", 16);
    Long uuidJoystick = Long.parseLong("209D", 16);
    Long uuidState = Long.parseLong("D288", 16);
    Long uuidBatteryLevel = Long.parseLong("2A19", 16);
    Long uuidAlert = Long.parseLong("C15B", 16);



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_remote);
        Log.i(TAG, "Create\n");

        auto = (Button)findViewById(R.id.autonomous);
        connect = (Button)findViewById(R.id.connect);
        start = (Button)findViewById(R.id.move);
        turbo = (Button)findViewById(R.id.turbo);
        joystick = (JoystickView)findViewById(R.id.joystick);

        setBt();

        setConnectButton();
        setStartButton();
        setAutoButton();
        setTurboButton();
        setJoystick();

        started = false;
        connected = false;
        autonomous = false;
        turboed =false;
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
        connect.setText(R.string.connect);
        connect.getBackground().setColorFilter(Color.parseColor(night), PorterDuff.Mode.MULTIPLY);
        auto.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
        start.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
        turbo.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
        joystick.setEnabled(false);

        Log.i(TAG, "Buttons\n");

        connect.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (connected) {
                    auto.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
                    start.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
                    turbo.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
                    joystick.setEnabled(false);
                    joystick.resetButtonPosition();
                    joystick.invalidate();
                    start.setText(R.string.start);
                    connect.setText(R.string.connect);
                    connected = false;
                    started = false;
                    turboed = false;
                    autonomous = false;
                    driving = false;
                    if (btGatt != null) {
                        btGatt.disconnect();
                    }
                    Log.i(TAG, "disconnect\n");
                } else {
                    auto.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                    start.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                    turbo.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                    joystick.setEnabled(false);
                    joystick.resetButtonPosition();
                    joystick.invalidate();
                    start.setText(R.string.start);
                    connect.setText(R.string.disconnect);
                    connected = true;
                    if (btGatt == null) {
                        btGatt = btDevice.connectGatt(remote.this, false, gattCallback);
                    }
                    Log.i(TAG, "connect\n");
                }
            }
        });
    }

    private void setStartButton() {
        //start button
        start.setText(R.string.start);
        start.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (connected) {
                    if (started) {
                        start.setText(R.string.start);
                        started = false;
                        turboed = false;
                        driving = false;
                        Log.i(TAG, "stop\n");
                        joystick.setEnabled(false);
                        joystick.resetButtonPosition();
                        joystick.invalidate();
                        driving = false;
                        turbo.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                        if (btGatt != null) {
                            sendState();
                        }
                    } else {
                        start.setText(R.string.stop);
                        started = true;
                        Log.i(TAG, "start\n");
                        if (!autonomous){
                            joystick.setEnabled(true);
                            joystick.invalidate();
                        }
                        if (btGatt != null) {
                            sendState();
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
                                sendState();
                            }
                        } else {
                            turbo.getBackground().setColorFilter(Color.parseColor(purple), PorterDuff.Mode.MULTIPLY);
                            turboed = true;
                            Log.i(TAG, "turbo on\n");
                            if (btGatt != null) {
                                sendState();
                            }
                        }
                    }
                }
            }
        });
    }

    private void setAutoButton() {
        //auto button
        auto.setText(R.string.manual);
        auto.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (connected) {
                    if (autonomous) {
                        auto.setText(R.string.manual);
                        autonomous = false;
                        started = false;
                        turboed = false;
                        driving = false;
                        Log.i(TAG, "manual\n");
                        start.setText(R.string.start);
                        turbo.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                        joystick.setEnabled(false);
                        joystick.resetButtonPosition();
                        joystick.invalidate();
                        driving = false;
                        if (btGatt != null) {
                            sendState();
                        }
                    } else {
                        auto.setText(R.string.autonomous);
                        autonomous = true;
                        started = false;
                        turboed = false;
                        driving = false;
                        Log.i(TAG, "autonomous\n");
                        start.setText(R.string.start);
                        turbo.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
                        joystick.setEnabled(false);
                        joystick.resetButtonPosition();
                        joystick.invalidate();
                        driving = false;
                        if (btGatt != null) {
                            sendState();
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
                        if (strength>0.5){
                            if (!driving){
                                driving = true;
                                Log.i(TAG, "driving");
                                if (btGatt != null) {
                                    sendState();
                                }
                            }
                            Log.i(TAG, "degrees:"+ angle +"\n");
                            joystickCharacteristic.setValue(angle, FORMAT_UINT8, 0);
                            joystickCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                            btGatt.writeCharacteristic(joystickCharacteristic);
                        }
                        if (strength == 0){
                            driving = false;
                            Log.i(TAG, "stopped");
                            if (btGatt != null) {
                                sendState();
                            }
                        }
                    }
                }
            }
        });
    }

    ////////////////////////////////////////////////////////////////////////////////
    //callbacks
    public final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {

        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, "Connected to GATT server\n");
                connected = true;
                btGatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
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
                if (uuid == uuidRemoteService) {
                    Log.i(TAG, "found uuid\n");
                    List<BluetoothGattCharacteristic> gattCharacteristics = gattService.getCharacteristics();
                    // Loop through available Characteristics.
                    for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {
                        uuid = gattCharacteristic.getUuid().getMostSignificantBits() >> 32;
                        if (uuid == uuidJoystick) {
                            joystickCharacteristic = gattCharacteristic;
                            Log.i(TAG, "found joystick\n");
                        } else if (uuid == uuidState) {
                            stateCharacteristic = gattCharacteristic;
                            Log.i(TAG, "found state\n");
                        } else if (uuid == uuidBatteryLevel) {
                            batteryLevelCharacteristic = gattCharacteristic;
                            enableNotification(true, btGatt, batteryLevelCharacteristic);
                            Log.i(TAG, "found battery level\n");
                        } else if (uuid == uuidAlert) {
                            alertCharacteristic = gattCharacteristic;
                            enableNotification(true, btGatt, alertCharacteristic);
                            Log.i(TAG, "found alert\n");
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
            //Log.i(TAG, "setValue\n");
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            if (started) {
                final int data = characteristic.getIntValue(FORMAT_UINT8, 0);
                Log.i(TAG, "data: " + data + "\n");
            }
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
    //functions
    private void sendState(){
        int state;
        if (!turboed & !driving & !autonomous & started)
            state = 1;
        else if (!turboed & driving & !autonomous & started)
            state = 2;
        else if (turboed & !driving & !autonomous & started)
            state = 3;
        else if (turboed & driving & !autonomous & started)
            state = 4;
        else if (!turboed & !driving & autonomous & !started)
            state = 5;
        else if (!turboed & !driving & autonomous & started)
            state = 6;
        else
            state = 0;

        Log.i(TAG, "state: " + state + "\n");
        stateCharacteristic.setValue(state, FORMAT_UINT8, 0);
        stateCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
        btGatt.writeCharacteristic(stateCharacteristic);
    }

    private void enableNotification(boolean enable, BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
        gatt.setCharacteristicNotification(characteristic, enable);
        Log.i(TAG, "notifications\n");
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"));
        descriptor.setValue(enable ? BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE : BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
        gatt.writeDescriptor(descriptor);
    }
}