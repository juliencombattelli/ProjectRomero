package romero.project.ihmremote;


import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.Button;
import android.graphics.Color;
import android.view.View;
import android.graphics.PorterDuff;

import io.github.controlwear.virtual.joystick.android.JoystickView;

public class RemoteActivity extends AppCompatActivity {
    private final static String TAG = RemoteActivity.class.getSimpleName();

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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.remote_activity);
        Log.i(TAG, "Create\n");

        auto = (Button)findViewById(R.id.autonomous);
        connect = (Button)findViewById(R.id.connect);
        start = (Button)findViewById(R.id.move);
        turbo = (Button)findViewById(R.id.turbo);
        joystick = (JoystickView)findViewById(R.id.joystick);
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
                    } else {
                        start.setText(R.string.stop);
                        started = true;
                        Log.i(TAG, "start\n");
                        if (!autonomous){
                            joystick.setEnabled(true);
                            joystick.invalidate();
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
                            }
                            Log.i(TAG, "degrees:"+ angle +"\n");
                        }
                        if (strength == 0){
                            driving = false;
                        }
                    }
                }
            }
        });
    }


}