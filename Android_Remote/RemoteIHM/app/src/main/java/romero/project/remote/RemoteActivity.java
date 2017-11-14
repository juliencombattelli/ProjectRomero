package romero.project.remote;


import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.Button;
import android.graphics.Color;
import android.view.View;
import android.graphics.PorterDuff;

import com.jmedeisis.bugstick.Joystick;
import com.jmedeisis.bugstick.JoystickListener;

public class RemoteActivity extends AppCompatActivity {
    private final static String TAG = RemoteActivity.class.getSimpleName();

    boolean run;
    boolean connected;
    boolean n;
    boolean autonomous;
    public Button connect;
    public Button auto;
    public Button start;
    public Button turbo;
    public Joystick joystick;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.remote_activity);
        Log.i(TAG, "Create\n");

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
                    Log.i(TAG, "disconnect\n");
                } else {
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
                        Log.i(TAG, "stop\n");
                        turbo.getBackground().setColorFilter(Color.parseColor("#6f7fff"), PorterDuff.Mode.MULTIPLY);
                    } else {
                        start.setText(R.string.stop);
                        run = true;
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
                    if (autonomous) {
                        auto.setText(R.string.manual);
                        autonomous = false;
                        Log.i(TAG, "manual\n");
                        turbo.getBackground().setColorFilter(Color.parseColor("#6f7fff"), PorterDuff.Mode.MULTIPLY);
                        joystick.setEnabled(true);
                    } else {
                        auto.setText(R.string.autonomous);
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
                            Log.i(TAG, "degrees:"+ intdegrees +"\n");
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


}