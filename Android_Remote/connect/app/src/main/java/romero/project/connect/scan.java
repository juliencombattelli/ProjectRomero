package romero.project.connect;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.graphics.Typeface;
import android.location.LocationManager;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;

import java.lang.annotation.Target;
import java.util.ArrayList;

public class scan extends AppCompatActivity {

    String white = "#ffffff";
    String blue = "#0097bd";
    String night =  "#002e39";

    //variables
    Button scanningButton;
    TableLayout list;
    boolean scanning;

    LocationManager gpsManager;

    BluetoothManager btManager;
    BluetoothAdapter btAdapter;
    BluetoothLeScanner btScanner;

    private ArrayList deviceList = new ArrayList();


    private final static int REQUEST_BLUETOOTH = 1;
    private static final int PERMISSION_REQUEST_COARSE_LOCATION = 1;
    private int i = 0;
    ////////////////////////////////--------------------------////////////////////////


    //Initialisation method
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.scan);

        btManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        btAdapter = btManager.getAdapter();
        btScanner = btAdapter.getBluetoothLeScanner();

        gpsManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);


        // Ensures Bluetooth is available on the device and it is enabled. If not,
        // displays a dialog requesting user permission to enable Bluetooth.
        if (btAdapter == null) {
            final AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Not compatible");
            builder.setMessage("Your phone does not support Bluetooth");
            builder.setPositiveButton("Exit", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    System.exit(0);
                }
            });
            builder.setIcon(android.R.drawable.ic_dialog_alert);
            builder.show();
        } else if (!btAdapter.isEnabled()) {
            Intent enable = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enable, REQUEST_BLUETOOTH);
        }

        // Ensures coarse location permission is granted. If not displays a dialog requesting
        // user to grant permission.
        if (this.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            final AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("This app needs location access");
            builder.setMessage("Please grant location access so this app can detect peripherals.");
            builder.setPositiveButton(android.R.string.ok, null);
            builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
                @Override
                public void onDismiss(DialogInterface dialog) {
                    requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, PERMISSION_REQUEST_COARSE_LOCATION);
                }
            });
            builder.show();
        }

        // Ensures coarse location is enabled on the device. If not displays a dialog requesting
        // user to enable coarse location.
        if (!gpsManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setMessage("Please enable gps so this app can detect peripherals.")
                    .setCancelable(false)
                    .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            Intent intent = new Intent(android.provider.Settings.ACTION_LOCATION_SOURCE_SETTINGS);
                            startActivity(intent);
                        }
                    })
                    .setNegativeButton("No", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            dialog.cancel();
                        }
                    });
            AlertDialog alert = builder.create();
            alert.show();
        }

        ////////////////////////////////--------------------------////////////////////////
        //identifying layout elements
        list = (TableLayout) findViewById(R.id.list);
        scanningButton = (Button) findViewById(R.id.scanButton);

        //initializing buttons
        scanning = false;
        scanningButton.setText(R.string.scan);
        scanningButton.getBackground().setColorFilter(Color.parseColor(night), PorterDuff.Mode.MULTIPLY);

        //scan button
        scanningButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (scanning)
                    stopScanning();
                else
                    startScanning();
            }
        });

    }

    ////////////////////////////////-------location permission------////////////////////////
    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        switch (requestCode) {
            case PERMISSION_REQUEST_COARSE_LOCATION: {
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    final AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("Full functionality");
                    builder.setMessage("coarse location permission granted");
                    builder.setPositiveButton(android.R.string.ok, null);
                    builder.show();
                } else {
                    final AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("Limited functionality");
                    builder.setMessage("Since location access has not been granted, this app will not be able to discover beacons when in the background.");
                    builder.setPositiveButton(android.R.string.ok, null);
                    builder.show();
                }
            }
        }
    }

    ////////////////////////////////-------Scanning------////////////////////////

    //scanning method
    public void startScanning() {
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                btScanner.startScan(leScanCallback);
            }
        });
        scanning = true;
        scanningButton.setText(R.string.stop);
    }

    //stop scanning method
    public void stopScanning() {
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                btScanner.stopScan(leScanCallback);
            }
        });
        scanning = false;
        scanningButton.setText(R.string.scan);
    }

    // Device scan callback.
    private ScanCallback leScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {

            final BluetoothDevice btDevice = result.getDevice();
            final String deviceName = btDevice.getName();
            final String deviceAddress = btDevice.getName();

            if (deviceName != null && deviceName.length() > 0 && !deviceList.contains(deviceAddress)) {
                deviceList.add(deviceAddress);
                AddDevice(btDevice);
            }
        }
    };

    // Device scan callback.
    public void AddDevice(final BluetoothDevice btDevice) {

        final String deviceName = btDevice.getName();
        final String deviceAddress = btDevice.getAddress();

        TableRow tRow = new TableRow(scan.this);
        LinearLayout lView = new LinearLayout(scan.this);
        lView.setOrientation(LinearLayout.VERTICAL);

        TextView Text = new TextView(scan.this);
        Text.setTextColor(Color.BLACK);
        Text.setTypeface(null, Typeface.BOLD);
        Text.setText(deviceName);

        TextView Text2 = new TextView(scan.this);
        Text2.setTextColor(Color.BLACK);
        Text2.setText(deviceAddress);

        final Button button = new Button(scan.this);
        button.setId(i);
        i++;
        button.setText(R.string.connect);
        button.setTextColor(Color.parseColor(white));
        button.getBackground().setColorFilter(Color.parseColor(blue), PorterDuff.Mode.MULTIPLY);
        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                stopScanning();
                //access to the Control page
                final Intent intent = new Intent(scan.this, remote.class);
                intent.putExtra(remote.EXTRAS_DEVICE_ADDRESS, deviceAddress);
                startActivity(intent);
            }
        });

        lView.addView(Text);
        lView.addView(Text2);
        tRow.addView(lView);
        tRow.addView(button);

        list.addView(tRow, new TableLayout.LayoutParams(TableRow.LayoutParams.FILL_PARENT, TableRow.LayoutParams.WRAP_CONTENT));
    }


}
