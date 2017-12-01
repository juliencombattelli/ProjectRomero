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
import android.widget.TextView;

import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

import io.github.controlwear.virtual.joystick.android.JoystickView;

import static android.bluetooth.BluetoothGattCharacteristic.FORMAT_UINT8;

public class remote extends AppCompatActivity {

    private final static String TAG = remote.class.getSimpleName();

    //btGatt variables
    private BluetoothGatt btGatt;
    private BluetoothDevice btDevice;
    BluetoothGattCharacteristic stateCharacteristic;
    BluetoothGattCharacteristic joystickCharacteristic;
    BluetoothGattCharacteristic alertCharacteristic;
    BluetoothGattCharacteristic feedbackCharacteristic;

    //ble gatt constants
    public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";
    public static final Long uuidRemoteService = Long.parseLong("7DB9", 16);
    public static final Long uuidJoystick = Long.parseLong("209D", 16);
    public static final Long uuidState = Long.parseLong("D288", 16);
    public static final Long uuidAlert = Long.parseLong("DCB1", 16);
    public static final Long uuidFeedback = Long.parseLong("C15B", 16);

    //color constants
    public static final String ice = "#94d3e2";
    public static final String blue = "#0097bd";
    public static final String night = "#002e39";
    public static final String purple = "#842f7a";

    //state variables
    boolean started;
    boolean connected;
    boolean turboed;
    boolean autonomous;
    boolean driving;
    int joystick_value;

    //view variables
    public Button connect;
    public Button auto;
    public Button start;
    public Button turbo;
    public JoystickView joystick;
    public ImageView direction;
    public ImageView sonar;
    public ImageView battery;
    public TextView speed;

    /**
     * Finds all the objects in the view, link them to lacal variables
     * call methods to set bluetooth, buttons and joystick
     * initialize state
     *
     * @param savedInstanceState previous sate in case of close or pause
     */
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
        direction = findViewById(R.id.direction);
        sonar = findViewById(R.id.sonar);
        battery = findViewById(R.id.battery);
        speed = findViewById(R.id.speed);

        setBt();

        setConnectButton();
        setStartButton();
        setAutoButton();
        setTurboButton();
        setJoystick();

        speed.setText("0.0");

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
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "Close\n");
        //Todo
    }

    /////////////////////////////////////////////////////////////////////////////
    //setup

    /**
     * gets the address of the device to connect to
     * creates the bluetooth manager service from the system and adapter
     * initializes the bluetooth device, and btGatt to null
     */
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

    /**
     * Initializes the Connect button
     * adds an on click listener for the button
     * sets the actions executed by the button:
     * connect and disconnect
     * change state variables
     * change buttons text and color depending on the state
     * resets the dashbord and joystick
     */
    private void setConnectButton() {
        //Connect button
        connected = false;
        connexionChange();
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

    /**
     * Initializes the Autonomous button
     * adds an on click listener for the button
     * sets the actions executed by the button:
     * change state variables
     * change buttons texts and colors depending on the state
     * resets the dashbord and joystick
     */
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
                    }
                }
            }
        });
    }

    /**
     * Initializes the Start button
     * adds an on click listener for the button
     * sets the actions executed by the button:
     * change state variables
     * change button text depending on the state
     * resets the dashbord and joystick
     */
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
                        turbo.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                    } else {
                        start.setText(R.string.stop);
                        started = true;
                        resetSymbols();
                        Log.i(TAG, "start\n");
                        if (!autonomous) {
                            joystick.setEnabled(true);
                            joystick.invalidate();
                        }
                    }
                }
            }
        });
    }

    /**
     * Initializes the Turbo button
     * adds an on click listener for the button
     * sets the actions executed by the button:
     * change state variables
     * change button text and color depending on the state
     */
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
                        } else {
                            turbo.getBackground().setColorFilter(Color.parseColor(purple), PorterDuff.Mode.MULTIPLY);
                            turboed = true;
                            Log.i(TAG, "turbo on\n");
                        }
                    }
                }
            }
        });
    }

    /**
     * Initializes the Joystick button
     * adds an on move listener for the joystick every 200ms
     * sets the actions executed by the joystick:
     * change state variable driving
     * changes the angle value
     */
    private void setJoystick() {
        joystick.setOnMoveListener(new JoystickView.OnMoveListener() {

            @Override
            public void onMove(int angle, int strength) {
                if (connected) {
                    if (started) {
                        if (strength > 50) {
                            driving = true;
                        }
                        if (strength < 50 & driving) {
                            driving = false;
                            Log.i(TAG, "stopped");
                        }
                        computeJoystickValue(angle);
                    }
                }
            }
        }, 200);
    }

    ////////////////////////////////////////////////////////////////////////////////
    //callbacks
    public final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {

        /**
         * Callback indicating when GATT client has connected/disconnected to/from a remote GATT server.
         *
         * @param  gatt   GATT client
         * @param  status Status of the connect or disconnect operation. GATT_SUCCESS if the operation succeeds
         * @param  newState Returns the new connection state. Can be one of STATE_DISCONNECTED or STATE_CONNECTED
         */
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                resetSymbols();
                connected = true;
                connexionChange();
                messageSend();
                Log.i(TAG, "Connected to GATT server\n");
                btGatt.requestConnectionPriority(BluetoothGatt.CONNECTION_PRIORITY_HIGH);
                btGatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                resetSymbols();
                connected = false;
                connexionChange();
                Log.i(TAG, "Disconnected from GATT server\n");
                btGatt.close();
                btGatt = null;
            }

        }

        /**
         * Callback invoked when the list of remote services, characteristics and descriptors for the remote device
         * have been updated, ie new services have been discovered.
         * searches for the awaited service and characteristics
         * initializes the bluetooth variables associates to service and characteristics
         *
         * @param  gatt   GATT client
         * @param  status Status of the connect or disconnect operation. GATT_SUCCESS if the operation succeeds
         */
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                gatt.disconnect();
                btGatt = null;
                return;
            }
            Log.i(TAG, "service discovered\n");
            List<BluetoothGattService> services = gatt.getServices();
            long uuid;
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

        /**
         * Callback reporting the result of a characteristic read operation.
         *
         * @param  gatt   GATT client invoked
         * @param  characteristic  Characteristic that was read from the associated remote device.
         * @param  status GATT_SUCCESS if the read operation was completed successfully.
         */
        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic,
                                         int status) {
            int a = characteristic.getIntValue(FORMAT_UINT8, 0);
            Log.i(TAG, characteristic + "" + a + "\n");
        }

        /**
         * Callback indicating the result of a characteristic write operation.
         *
         * @param  gatt   GATT client invoked
         * @param  characteristic  Characteristic that was written to the associated remote device.
         * @param  status The result of the write operation GATT_SUCCESS if the operation succeeds.
         */
        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic,
                                          int status) {
            //Log.i(TAG, "wrote\n");
        }

        /**
         * Callback triggered as a result of a remote characteristic notification.
         * calls functions associated with dashboard element that need to be updated
         *
         * @param  gatt   GATT client the characteristic is associated with
         * @param  characteristic Characteristic that has been updated as a result of a remote notification event.
         */
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            //Log.i(TAG, "notification!");
            final int data = characteristic.getIntValue(FORMAT_UINT8, 0);
            if (characteristic == feedbackCharacteristic) {
                int mode = data & 0x01;
                //changeMode(mode);
                Log.i(TAG, "mode: " + mode + "\n");
                int dir = (data >> 1) & 0x03;
                changeDir(dir);
                Log.i(TAG, "dir: " + dir + "\n");
                int speed = (data >> 3) & 0x1F;
                changeSpeed(speed);
            } else if (characteristic == alertCharacteristic) {
                int battValue = data & 0x03;
                changeBatt(battValue);
                int radar = (data >> 2) & 0x07;
                changeSonar(radar);
                int route = (data >> 5) &  0x07;
            }
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
    //functions

    /**
     * Function changing the UI and changing state variables according to the connection state
     */
    private void connexionChange() {
        if (connected) {
            started = false;
            turboed = false;
            autonomous = false;
            driving = false;
            runOnUiThread(new Runnable() {
                public void run() {
                    auto.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                    start.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                    turbo.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                    start.setText(R.string.start);
                    auto.setText(R.string.manual);
                    connect.setText(R.string.disconnect);
                }
            });
        } else {
            started = false;
            turboed = false;
            autonomous = false;
            driving = false;
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

    /**
     * Function enabling Notification for a characteristic and creating the associated descriptor.
     *
     * @param gatt           GATT client the characteristic is associated with
     * @param characteristic Characteristic that has been updated as a result of a remote notification event.
     */
    private void enableNotification(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
        gatt.setCharacteristicNotification(characteristic, true);
        Log.i(TAG, "notification enabled\n");
        // 0x2902 org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
        UUID uuid = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(uuid);
        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        gatt.writeDescriptor(descriptor);
    }

    /**
     * Function resetting all symbols to their rest appearance
     */
    public void resetSymbols() {
        runOnUiThread(new Runnable() {
            public void run() {
                joystick.setEnabled(false);
                joystick.resetButtonPosition();
                joystick.invalidate();
                direction.setImageResource(R.drawable.ic_direction_ice);
                sonar.setImageResource(R.drawable.ic_sonar);
                battery.setImageResource(R.drawable.ic_batt_full);
            }
        });
    }
    /**
     * Function updating mode
     *
     * @param modeValue mode: true = autonomous, false = manual
     */
    private void changeMode(final int modeValue) {
        runOnUiThread(new Runnable() {
            public void run() {
                if (modeValue == 1 & autonomous == false){
                    auto.setText(R.string.autonomous);
                    autonomous = true;
                    started = false;
                    turboed = false;
                    driving = false;
                    resetSymbols();
                    Log.i(TAG, "autonomous\n");
                    start.setText(R.string.start);
                    turbo.getBackground().setColorFilter(Color.parseColor(ice), PorterDuff.Mode.MULTIPLY);
                } else if (modeValue == 0 & autonomous == true){
                    auto.setText(R.string.manual);
                    autonomous = false;
                    started = false;
                    turboed = false;
                    driving = false;
                    resetSymbols();
                    Log.i(TAG, "manual\n");
                    start.setText(R.string.start);
                    turbo.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
                }
            }
        });
    }

    /**
     * Function updating direction indicators according to wheel direction
     *
     * @param directionValue wheel direction
     */
    private void changeDir(final int directionValue) {

        Log.i(TAG, "change direction\n");
        runOnUiThread(new Runnable() {
            public void run() {
                switch (directionValue) {
                    case 1:
                        direction.setImageResource(R.drawable.ic_direction_right);
                        break;
                    case 2:
                        direction.setImageResource(R.drawable.ic_direction_front);
                        break;
                    case 3:
                        direction.setImageResource(R.drawable.ic_direction_left);
                        break;
                    case 0:
                    default:
                        direction.setImageResource(R.drawable.ic_direction_ice);
                        break;
                }
            }
        });
    }

    /**
     * Function updating speed indicator in hm/h according to vehicle speed
     *
     * @param speedValue vehicle speed in m/s
     */
    private void changeSpeed(int speedValue) {
        final double convSpeedValue = speedValue * 3.6;
        runOnUiThread(new Runnable() {
            public void run() {
                //todo
            }
        });
    }


    /**
     * Function updating battery indicators according to battery level
     *
     * @param batteryValue battery level
     */
    private void changeBatt(final int batteryValue) {
        runOnUiThread(new Runnable() {
            public void run() {
                switch (batteryValue) {
                    case 1:
                        battery.setImageResource(R.drawable.ic_batt_mid);
                        break;
                    case 2:
                        battery.setImageResource(R.drawable.ic_batt_low);
                        break;
                    case 3:
                        battery.setImageResource(R.drawable.ic_batt_critical);
                        break;
                    case 0:
                    default:
                        battery.setImageResource(R.drawable.ic_batt_full);
                        break;
                }
            }
        });
    }

    /**
     * Function updating obstacle presence indicators according to sonar value
     *
     * @param sonarValue presence localization
     */
    private void changeSonar(final int sonarValue) {
        runOnUiThread(new Runnable() {
            public void run() {
                switch (sonarValue) {
                    case 1:
                        //presence right
                        break;
                    case 2:
                        //presence front
                        break;
                    case 3:
                        //presence front and right
                        break;
                    case 4:
                        //presence left
                        break;
                    case 5:
                        //presence left and right
                        break;
                    case 6:
                        //presence left and front
                        break;
                    case 7:
                        //presence left front and right;
                        break;
                    case 0:
                    default:
                        sonar.setImageResource(R.drawable.ic_sonar);
                        break;
                }
            }
        });
    }


    ////////////////////////////////////////////////////////////////////////////////
    //message sending

    /**
     * Function calculating joystick direction from joystick angle
     *
     * @param joystick_angle joystick direction
     */
    private void computeJoystickValue(int joystick_angle) {
        if (driving) {
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
        } else {
            joystick_value = 7;
        }
    }


    /**
     * Function creating messages to send with bluetooth depending on state and joystick direction
     */
    private int createMessage() {
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

        //Log.i(TAG, "state: " + stateCode + "\n");
        //Log.i(TAG, "joystick: " + joystick_value + "\n");
        message = stateCode << 5;
        message = message + joystick_value;
        return message;
    }

    private void messageSend() {
        int delay = 2000; // delay for 2 s.
        int period = 200; // repeat every 1 s.
        Timer timer = new Timer();

        timer.scheduleAtFixedRate(new TimerTask() {
            int previousMessage = 7;
            int message;
            public void run() {
                if (connected) {
                    message = createMessage();
                    if (message != previousMessage){
                        previousMessage = message;
                        Log.i(TAG, "to send: " + message + "\n");
                        stateCharacteristic.setValue(message, FORMAT_UINT8, 0);
                        stateCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                        btGatt.writeCharacteristic(stateCharacteristic);
                    }
                } else {
                    Log.i(TAG, "delete timer\n");
                    cancel();
                }
            }

        }, delay, period);
    }

}
