package man.laa.ua.tracker;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Calendar;
import java.util.Locale;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    private static final int REQUEST_ENABLE_BT = 1;

    BluetoothAdapter bluetoothAdapter;

    private UUID myUUID;

    ThreadConnectBTdevice myThreadConnectBTdevice;
    ThreadConnected myThreadConnected;

// Visual Components
    public CheckBox checkTime, checkParking, checkPosition, checkAutoTime;
    public TextView textStatus, textTrackerTime, textMorning, textEvening, textParking;
    public TextView textAbsEast, textAbsPos, textAbsWest, textRttEast, textRttPos, textRttWest;
    public TextView textDriveState;
    public Button buttonMoveEast, buttonHold, buttonMoveWest;
    public EditText editTrackerTime, editMorning, editEvening, editParking;

    boolean fillParkingParams = true;

    boolean connected = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

    // Find visual components


        final String UUID_STRING_WELL_KNOWN_SPP = "00001101-0000-1000-8000-00805F9B34FB";

        checkTime = findViewById(R.id.checkTime);
        checkParking = findViewById(R.id.checkParking);
        checkPosition = findViewById(R.id.checkPosition);
        checkAutoTime = findViewById(R.id.checkAutoTime);

        textStatus = findViewById(R.id.textStatus);
        textTrackerTime = findViewById(R.id.textTrackerTime);
        textMorning = findViewById(R.id.textMorning);
        textEvening = findViewById(R.id.textEvening);
        textParking = findViewById(R.id.textParking);

        textDriveState = findViewById(R.id.textDriveState);
        textAbsEast = findViewById(R.id.textAbsEast);
        textAbsPos = findViewById(R.id.textAbsPos);
        textAbsWest = findViewById(R.id.textAbsWest);
        textRttEast = findViewById(R.id.textRttEast);
        textRttPos = findViewById(R.id.textRttPos);
        textRttWest = findViewById(R.id.textRttWest);

        buttonMoveEast = findViewById(R.id.buttonMoveEast);
        buttonHold = findViewById(R.id.buttonHold);
        buttonMoveWest = findViewById(R.id.buttonMoveWest);

        editTrackerTime = findViewById(R.id.editTrackerTime);
        editMorning = findViewById(R.id.editMorning);
        editEvening = findViewById(R.id.editEvening);
        editParking = findViewById(R.id.editParking);

        textStatus = findViewById(R.id.textStatus);
        textTrackerTime = findViewById(R.id.textTrackerTime);

        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH)){
            Toast.makeText(this, "BLUETOOTH NOT support", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        myUUID = UUID.fromString(UUID_STRING_WELL_KNOWN_SPP);

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        if (bluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth is not supported on this hardware platform", Toast.LENGTH_LONG).show();
            finish();
        }
    }


    @Override
    protected void onStart() { // Check for bluetooth enabled, request it, then setup
        super.onStart();
        if (!bluetoothAdapter.isEnabled()) {
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
        }
        setup();
    }

    private void setup() { // Search for paired tracker
        if (connected) return;
        Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
        BluetoothDevice tracker = null;
        for (BluetoothDevice device : pairedDevices) {
            String name = device.getName();
            if (name.length() > 15) {
                if (name.substring(0,15).equals("Svit_Vitru_TSV_")) {
                    tracker = device;
                    break;
                }
            }
        }
        if (tracker != null) {
            textStatus.setText("Connecting " + tracker.getName());
            myThreadConnectBTdevice = new ThreadConnectBTdevice(tracker);
            myThreadConnectBTdevice.start(); // Start connection
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (myThreadConnectBTdevice != null) myThreadConnectBTdevice.cancel();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if(requestCode == REQUEST_ENABLE_BT){ // Restart setup when user enables bluetooth
            if(resultCode == Activity.RESULT_OK) {
                setup();
            }
            else { // Finish app otherwise
                Toast.makeText(this, "Can't continue without bluetooth", Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }


    private class ThreadConnectBTdevice extends Thread { // Поток для коннекта с Bluetooth

        private BluetoothSocket bluetoothSocket = null;
        private String name;

        private ThreadConnectBTdevice(BluetoothDevice device) {
            try {
                name = device.getName();
                bluetoothSocket = device.createRfcommSocketToServiceRecord(myUUID);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void run() { // Connect device here
            if (connected) return;
            try {
                bluetoothSocket.connect();
                connected = true;
            } catch (IOException e) {
                e.printStackTrace();
                runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
                    textStatus.setText("Can't connect " + name);
                    }
                });
                try {
                    bluetoothSocket.close();
                } catch (IOException e1) {
                    e1.printStackTrace();
                }
            }
            if (connected) {  // Start work thread if connected
                runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
                    textStatus.setText("Connected " + name);
                    }
                });
                timerHandler.postDelayed(timerRunnable, 1000);
                myThreadConnected = new ThreadConnected(bluetoothSocket);
                myThreadConnected.start();
            }
        }

        void cancel() {
            Toast.makeText(getApplicationContext(), "Close - BluetoothSocket", Toast.LENGTH_LONG).show();
            try {
                bluetoothSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private class ThreadConnected extends Thread {    // Main work thread

        private final InputStream connectedInputStream;
        private final OutputStream connectedOutputStream;

        private String outputText;

        ThreadConnected(BluetoothSocket socket) {
            InputStream in = null;
            OutputStream out = null;
            try {
                in = socket.getInputStream();
                out = socket.getOutputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }
            connectedInputStream = in;
            connectedOutputStream = out;
        }

        private void showText(final TextView view, String text) {
            outputText = text;
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    view.setText(outputText);
                }
            });
        }
/*
>>>14-02-51-10-00-50-C7-
   00-01-02-03-04-05-06-07-08-09-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26
<<<14-16-10-51-80-12-3B-3B-8F-A6-00-00-8E-A6-00-C2-1C-E3-F4-FF-78-00-28-05-78-00-CD-
   [---HEADER---][H--M--S][PanPos-][F][StopPos][HSPN][OFFSET-][MRNG][EVNG][PRKG][CS]
*/
        final static int FULL_CIRCLE_T = 70693;

        private String posToTime(int pos) {
            while (pos < 0) pos += FULL_CIRCLE_T;
            while (pos >= FULL_CIRCLE_T) pos -= FULL_CIRCLE_T;
            int second = (int)(pos * 1.222186072171219215481);
            int hour = second / 3600;
            second %= 3600;
            int minute = second / 60;
            second %= 60;
            return String.format(Locale.getDefault(), "%02d:%02d:%02d", hour, minute, second);
        }

        private void decode(int in[]) {
            final String time = String.format(Locale.getDefault(), "%02d:%02d:%02d", in[5], in[6], in[7]);
            int morning = in[20] + 256 * in[21];
            final String morningTime = String.format(Locale.getDefault(), "%02d:%02d", morning / 60, morning % 60);
            int evening = in[22] + 256 * in[23];
            final String eveningTime = String.format(Locale.getDefault(), "%02d:%02d", evening / 60, evening % 60);
            int parking = in[24] + 256 * in[25];
            final String parkingTime = String.format(Locale.getDefault(), "%02d:%02d", parking / 60, parking % 60);
            int offset = in[17] + 256 * in[18] + 65536 * in[19];
            if (offset >= 0x800000) offset -= 0x1000000;
            int halfSpan = in[15] + 256 * in[16];
            int flags = in[11];
            final boolean active = (flags & 1) != 0;
            final boolean reverse = (flags & 2) != 0;
            boolean eastLimit =  (flags & 0x10) != 0;
            boolean westLimit =  (flags & 0x20) != 0;
            final int eastLimitColor = eastLimit ? Color.RED : Color.GRAY;
            final int westLimitColor = westLimit ? Color.RED : Color.GRAY;
            final int target = in[12] + 256 * in[13] + 65536 * in[14];
            int panPos = in[8] + 256 * in[9] + 65536 * in[10];
            final String panPosAbs = posToTime(panPos);
            final String eastLimitAbs = "E{" + posToTime(FULL_CIRCLE_T / 2 - halfSpan) + "}";
            final String westLimitAbs = "{" + posToTime(FULL_CIRCLE_T / 2 + halfSpan) + "}W";
            final String panPosRel = posToTime(panPos - offset);
            final String eastLimitRel = "E{" + posToTime(FULL_CIRCLE_T / 2 - halfSpan - offset) + "}";
            final String westLimitRel = "{" + posToTime(FULL_CIRCLE_T / 2 + halfSpan - offset) + "}W";

            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    textTrackerTime.setText(time);
                    textMorning.setText(morningTime);
                    textEvening.setText(eveningTime);
                    textParking.setText(parkingTime);
                    if (fillParkingParams) {
                        fillParkingParams = false;
                        editMorning.setText(morningTime);
                        editEvening.setText(eveningTime);
                        editParking.setText(parkingTime);
                    }
                    textAbsEast.setText(eastLimitAbs);
                    textAbsPos.setText(panPosAbs);
                    textAbsWest.setText(westLimitAbs);
                    textRttEast.setText(eastLimitRel);
                    textRttPos.setText(panPosRel);
                    textRttWest.setText(westLimitRel);
                    if (active) {
                        if (reverse) {
                            textDriveState.setText("{" + posToTime(target) + "}<<P");
                        } else {
                            textDriveState.setText("P>>{" + posToTime(target) + "}");
                        }
                    } else {
                        textDriveState.setText("-------------------");
                    }
                    textAbsEast.setTextColor(eastLimitColor);
                    textRttEast.setTextColor(eastLimitColor);
                    textAbsWest.setTextColor(westLimitColor);
                    textRttWest.setTextColor(westLimitColor);
                }
            });

        }

        private final static int RESPONSE_LENGTH = 27;

        @Override
        public void run() { // Listen bluetooth input
            int header[] = {0x14, 0x16, 0x10, 0x51, 0x80};
            int in[] = new int[RESPONSE_LENGTH];
            int pnt = 0;
            byte[] inputByte = new byte[1];
            while (true) {
                try {
                    int inputLen = connectedInputStream.read(inputByte);
                } catch (IOException e) {
                    break;
                }
                int input = inputByte[0] >= 0 ? inputByte[0] : 0x100 + inputByte[0];
                in[pnt] = input;
                if ((pnt < header.length) && (header[pnt] != input)) {
                    pnt = 0;
                } else {
                    pnt++;
                }
                if (pnt < RESPONSE_LENGTH) continue;
                byte checkSum = 0;
                for (int i = 0; i < RESPONSE_LENGTH - 1; i++) checkSum += in[i];
                pnt = 0;
                if (checkSum == (byte)in[RESPONSE_LENGTH - 1]) {
                    decode(in);
                }
            }
        }

        void write(byte[] buffer) {
            try {
                connectedOutputStream.write(buffer);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void request() {
        if (myThreadConnected==null) return;
        byte[] bytesToSend = {0x14, 0x02, 0x51, 0x10, 0x00, 0x50, (byte)0xC7};
        myThreadConnected.write(bytesToSend );
    }

/////////////////// Timer /////////////////////

    Handler timerHandler = new Handler();
    Runnable timerRunnable = new Runnable() {
        @Override
        public void run() {
            if (myThreadConnected !=null) request();
            if (checkAutoTime.isChecked()) {
                Calendar currentTime = Calendar.getInstance();
                int hour = currentTime.get(Calendar.HOUR_OF_DAY);
                int minute = currentTime.get(Calendar.MINUTE);
                int second = currentTime.get(Calendar.SECOND);
                final String time = String.format(Locale.getDefault(), "%02d:%02d:%02d", hour, minute, second);
                editTrackerTime.setText(time);
            }
            timerHandler.postDelayed(this, 500);
        }
    };

    @Override
    public void onPause() {
        super.onPause();
        timerHandler.removeCallbacks(timerRunnable);
    }

    @Override
    protected void onResume() {
        super.onResume();
        timerHandler.postDelayed(timerRunnable, 1000);
    }

    /////////////////// Buttons /////////////////////

    public void onClickGetParams(View v) {
        editEvening.setText(textEvening.getText());
        editMorning.setText(textMorning.getText());
        editParking.setText(textParking.getText());
    }

}
