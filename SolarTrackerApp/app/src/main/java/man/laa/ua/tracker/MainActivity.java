package man.laa.ua.tracker;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
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
    public TextView textCounter;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        final String UUID_STRING_WELL_KNOWN_SPP = "00001101-0000-1000-8000-00805F9B34FB";

        textStatus = findViewById(R.id.textStatus);
        textCounter = findViewById(R.id.textCounter);

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
            return;
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
            boolean connected = false;
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
            if(connected) {  // Start work thread if connected
                runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
                    textStatus.setText("Connected " + name);
                    }
                });
                myThreadConnected = new ThreadConnected(bluetoothSocket);
                myThreadConnected.start();
            }
        }

        public void cancel() {
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

        public ThreadConnected(BluetoothSocket socket) {
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

        @Override
        public void run() { // Listen bluetooth input
            int count = 0;
            while (true) {
                try {
                    byte[] buffer = new byte[1];
                    int bytes = connectedInputStream.read(buffer);
                    count++;
                    outputText = Integer.toString(count);
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            textCounter.setText(outputText);
                        }
                    });
                } catch (IOException e) {
                    break;
                }
            }
        }

        public void write(byte[] buffer) {
            try {
                connectedOutputStream.write(buffer);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

/////////////////// Buttons /////////////////////

    public void onClickRequest(View v) {
        if (myThreadConnected==null) return;
        byte[] bytesToSend = {0x14, 0x02, 0x51, 0x10, 0x00, 0x50, (byte)0xC7};
        myThreadConnected.write(bytesToSend );
    }

}
