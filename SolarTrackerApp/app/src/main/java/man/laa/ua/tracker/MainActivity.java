package man.laa.ua.tracker;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    private static final int REQUEST_ENABLE_BT = 1;

    BluetoothAdapter bluetoothAdapter;

    private UUID myUUID;

    ThreadConnectBTdevice myThreadConnectBTdevice;
    ThreadConnected myThreadConnected;

    public TextView textStatus;
    public TextView textTrackerTime;

    boolean connected = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        final String UUID_STRING_WELL_KNOWN_SPP = "00001101-0000-1000-8000-00805F9B34FB";

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
   00-01-02-03-04-05-06-07-08-09-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25
<<<14-15-10-51-80-12-3B-3B-8F-A6-00-00-8E-A6-00-C2-1C-E3-F4-78-00-28-05-78-00-CD-
   [---HEADER---][H--M--S][PanPos-][F][StopPos][HSPN][OFFS][MRNG][EVNG][PRKG][CS]
*/
        private void decode(byte in[]) {
            String time = String.format("%02d:%02d:%02d", in[5], in[6], in[7]);
            showText(textTrackerTime, time);
        }

        @Override
        public void run() { // Listen bluetooth input
            byte header[] = {0x14, 0x15, 0x10, 0x51, (byte)0x80};
            byte in[] = new byte[32];
            int pnt = 0;
            byte[] inputByte = new byte[1];
            while (true) {
                try {
                    int inputLen = connectedInputStream.read(inputByte);
                } catch (IOException e) {
                    break;
                }
                byte input = inputByte[0];
                in[pnt] = input;
                if ((pnt < header.length) && (header[pnt] != input)) {
                    pnt = 0;
                } else {
                    pnt++;
                }
                if (pnt < 26) continue;
                byte checkSum = 0;
                for (int i = 0; i < 25; i++) checkSum += in[i];
                pnt = 0;
                if (checkSum == in[25]) decode(in);
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

    public void onClickRequest(View v) {
        request();
    }

}
