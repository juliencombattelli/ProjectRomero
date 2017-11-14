package ProjectRomero.CarRemote;

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
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.Button;
import android.graphics.Color;
import android.view.View;
import android.graphics.PorterDuff;

import com.jmedeisis.bugstick.Joystick;
import com.jmedeisis.bugstick.JoystickListener;

import java.util.List;
import java.util.UUID;

import static android.bluetooth.BluetoothGattCharacteristic.FORMAT_UINT8;

public class RemoteActivity extends AppCompatActivity {
    private final static String TAG = RemoteActivity.class.getSimpleName();

    public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";

    boolean run;
    boolean connected;
    boolean n;
    boolean autonomous;
    public Button connect;
    public Button auto;
    public Button start;
    public Button turbo;
    public Joystick joystick;

    private BluetoothGatt btGatt;
    private BluetoothDevice btDevice;
    BluetoothGattCharacteristic reportCharacteristic;

    Long uuidService = Long.parseLong("1812", 16);
    Long uuidReport = Long.parseLong("2A4D", 16);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.remote_activity);
        Log.i(TAG, "Create\n");

        setBt();

        auto = (Button)findViewById(R.id.autonomous);
        connect = (Button)findViewById(R.id.connect);
        start = (Button)findViewById(R.id.move);
        turbo = (Button)findViewById(R.id.turbo);
        joystick = (Joystick)findViewById(R.id.joystick);
        setConnectButton();
        setStartButton();
        setAutoButton();
        setTurboButton();
        setJoystick();

        run = false;
        connected = false;
        autonomous = false;
        n=false;

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
        /*if (btGatt != null) {
            stateCharacteristic.setValue(0, FORMAT_UINT8, 0);
            stateCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
            btGatt.writeCharacteristic(stateCharacteristic);
        }
        run = false;*/
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "Close\n");
        //Todo
        /*
        if (btGatt != null) {
            btGatt.disconnect();
        }
        connected = false;*/
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
        connect.getBackground().setColorFilter(Color.parseColor("#4853a5"), PorterDuff.Mode.MULTIPLY);
        auto.getBackground().setColorFilter(Color.parseColor("#acc6ff"), PorterDuff.Mode.MULTIPLY);
        start.getBackground().setColorFilter(Color.parseColor("#acc6ff"), PorterDuff.Mode.MULTIPLY);
        turbo.getBackground().setColorFilter(Color.parseColor("#acc6ff"), PorterDuff.Mode.MULTIPLY);
        joystick.setEnabled(false);

        Log.i(TAG, "Buttons\n");

        connect.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (connected) {
                    auto.getBackground().setColorFilter(Color.parseColor("#acc6ff"), PorterDuff.Mode.MULTIPLY);
                    start.getBackground().setColorFilter(Color.parseColor("#acc6ff"), PorterDuff.Mode.MULTIPLY);
                    turbo.getBackground().setColorFilter(Color.parseColor("#acc6ff"), PorterDuff.Mode.MULTIPLY);
                    joystick.setEnabled(false);
                    start.setText(R.string.start);
                    connect.setText(R.string.connect);
                    connected = false;
                    run = false;
                    n = false;
                    autonomous = false;
                    if (btGatt != null) {
                        btGatt.disconnect();
                    }
                    Log.i(TAG, "disconnect\n");
                } else {
                    if (btGatt == null) {
                        btGatt = btDevice.connectGatt(RemoteActivity.this, false, gattCallback);
                    }
                    auto.getBackground().setColorFilter(Color.parseColor("#6f7fff"), PorterDuff.Mode.MULTIPLY);
                    start.getBackground().setColorFilter(Color.parseColor("#6f7fff"), PorterDuff.Mode.MULTIPLY);
                    turbo.getBackground().setColorFilter(Color.parseColor("#6f7fff"), PorterDuff.Mode.MULTIPLY);
                    joystick.setEnabled(true);
                    start.setText(R.string.start);
                    connect.setText(R.string.disconnect);
                    connected = true;
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
                    if (run) {
                        start.setText(R.string.start);
                        run = false;
                        n = false;
                        if (btGatt != null) {
                            reportCharacteristic.setValue(0, FORMAT_UINT8, 0);
                            reportCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                            btGatt.writeCharacteristic(reportCharacteristic);
                            turbo.getBackground().setColorFilter(Color.parseColor("#6f7fff"), PorterDuff.Mode.MULTIPLY);
                        }
                        Log.i(TAG, "stop\n");
                    } else {
                        start.setText(R.string.stop);
                        run = true;
                        if (btGatt != null) {
                            reportCharacteristic.setValue(1, FORMAT_UINT8, 0);
                            reportCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                            btGatt.writeCharacteristic(reportCharacteristic);
                        }
                        Log.i(TAG, "start\n");
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
                    if (run) {
                        if (n) {
                            turbo.getBackground().setColorFilter(Color.parseColor("#6f7fff"), PorterDuff.Mode.MULTIPLY);
                            n = false;
                            Log.i(TAG, "turbo off\n");
                        } else {
                            turbo.getBackground().setColorFilter(Color.parseColor("#a553a5"), PorterDuff.Mode.MULTIPLY);
                            n = true;
                            Log.i(TAG, "turbo on\n");
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
                    Log.i(TAG, "auto?\n");
                    if (autonomous) {
                        auto.setText(R.string.autonomous);
                        autonomous = false;
                        Log.i(TAG, "manual\n");
                        turbo.getBackground().setColorFilter(Color.parseColor("#6f7fff"), PorterDuff.Mode.MULTIPLY);
                        joystick.setEnabled(true);
                    } else {
                        auto.setText(R.string.manual);
                        autonomous = true;
                        Log.i(TAG, "autonomous\n");
                        turbo.getBackground().setColorFilter(Color.parseColor("#acc6ff"), PorterDuff.Mode.MULTIPLY);
                        joystick.setEnabled(false);
                    }
                }
            }
        });
    }

    private void setJoystick() {
        joystick.setJoystickListener(new JoystickListener() {
            @Override
            public void onDown() {
                //Log.i(TAG, "Down\n");
            }

            @Override
            public void onDrag(float degrees, float offset) {
                if (connected) {
                    if (run) {
                        if (offset>0.5){
                            int intdegrees;
                            if (degrees > 0){
                                intdegrees = (int) degrees;
                            } else {
                                intdegrees = (int)(degrees + 360);
                            }
                            reportCharacteristic.setValue(intdegrees, FORMAT_UINT8, 0);
                            reportCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                            btGatt.writeCharacteristic(reportCharacteristic);
                        }
                    }
                }
            }

            @Override
            public void onUp() {
                //Log.i(TAG, "Up\n");
            }
        });
    }

    /*private void enableNotification(boolean enable, BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
        gatt.setCharacteristicNotification(characteristic, enable);
        Log.i(TAG, "notifications\n");
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"));
        descriptor.setValue(enable ? BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE : BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
        gatt.writeDescriptor(descriptor);
    }*/

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
                if (uuid == uuidService) {
                    Log.i(TAG, "found uuid\n");
                    List<BluetoothGattCharacteristic> gattCharacteristics = gattService.getCharacteristics();
                    // Loop through available Characteristics.
                    for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {
                        uuid = gattCharacteristic.getUuid().getMostSignificantBits() >> 32;
                        if (uuid == uuidReport) {
                            reportCharacteristic = gattCharacteristic;
                            Log.i(TAG, "found characteristic\n");
                        }
                    }
                }
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic,
                                         int status) {
            int a = characteristic.getIntValue(FORMAT_UINT8, 0);
            Log.i(TAG, a + "\n");
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic,
                                          int status) {
            //Todo
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            if (run) {
                final int data = characteristic.getIntValue(FORMAT_UINT8, 0);
                Log.i(TAG, "data: " + data + "\n");
            }
        }
    };
}