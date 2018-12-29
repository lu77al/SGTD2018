package man.laa.ua.tracker;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageView;
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
    public Button buttonMoveEast, buttonMoveWest, buttonSetParking;
    public Button buttonSetTime, buttonSetOffset, buttonAlawaysParking, buttonManual;
    public EditText editTrackerTime, editMorning, editEvening, editParking;
    public ImageView imagePanel;

    final static int CMD_NONE = 0;
    final static int CMD_SET_TIME = 1;
    final static int CMD_SET_OFFSET = 2;
    final static int CMD_SET_PARKING = 3;
    final static int CMD_GO_EAST = 4;
    final static int CMD_GO_WEST = 5;
    final static int CMD_HOLD = 6;
    final static int CMD_STOP = 7;
    final static int CMD_FAKE_LIMIT = 8;

    int userCommand = CMD_NONE;

    boolean fillParkingParams = true;

    boolean connected = false;

    SolarTracker tracker = new SolarTracker();

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
        buttonMoveWest = findViewById(R.id.buttonMoveWest);
        buttonSetParking = findViewById(R.id.buttonSetParking);
        buttonAlawaysParking = findViewById(R.id.buttonAlways);
        buttonManual = findViewById(R.id.buttonManual);
        buttonSetTime = findViewById(R.id.buttonSetTime);
        buttonSetOffset = findViewById(R.id.buttonSetOffset);

        editTrackerTime = findViewById(R.id.editTrackerTime);
        editMorning = findViewById(R.id.editMorning);
        editEvening = findViewById(R.id.editEvening);
        editParking = findViewById(R.id.editParking);

        textStatus = findViewById(R.id.textStatus);
        textTrackerTime = findViewById(R.id.textTrackerTime);

        imagePanel = findViewById(R.id.imagePanel);

        buttonMoveEast.setEnabled(false);
        buttonMoveWest.setEnabled(false);
        buttonSetParking.setEnabled(false);
        buttonAlawaysParking.setEnabled(false);
        buttonManual.setEnabled(false);
        buttonSetTime.setEnabled(false);
        buttonSetOffset.setEnabled(false);

        checkBoxChangeEvents();
        setupImageOnClick();

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
                    tracker.decode(in);
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

    private int parseParkingPoint(String time) {
        String timeChunks[] = time.split(":");
        if (timeChunks.length != 2) return -1;
        int hour, minute;
        try {
            hour = Integer.parseInt(timeChunks[0]);
            minute = Integer.parseInt(timeChunks[1]);
        } catch ( NumberFormatException e) {
            return -1;
        }
        if ((hour < 0) || (hour > 23) ||
            (minute < 0) || (minute > 59)) return -1;
        return hour * 60 + minute;
    }

    private int[] formSetParking() {
        int evening = parseParkingPoint(editEvening.getText().toString());
        int morming = parseParkingPoint(editMorning.getText().toString());
        int parking = parseParkingPoint(editParking.getText().toString());
        if ((evening == -1) || (morming == -1) || (parking == -1)) return null;
        return new int[]{0x57, morming % 256, morming / 256, evening % 256, evening / 256, parking % 256, parking / 256};
    }

    public int[] formSetTimeRequest(boolean offset) {
        String time = editTrackerTime.getText().toString();
        if (time.length() != 8) return null;
        String timeChunks[] = time.split(":");
        if (timeChunks.length != 3) return null;
        int hour, minute, second;
        try {
            hour = Integer.parseInt(timeChunks[0]);
            minute = Integer.parseInt(timeChunks[1]);
            second = Integer.parseInt(timeChunks[2]);
        } catch ( NumberFormatException e) {
            return null;
        }
        if ((hour < 0) || (hour > 23) ||
                (minute < 0) || (minute > 59) ||
                (second < 0) || (second > 59)) return null;
        return new int[]{offset ? 0x56 : 0x55, hour, minute, second};
    }

    private void request() {
        if (myThreadConnected==null) return;
        int[] request = null;
        switch (userCommand) {
            case CMD_GO_EAST:
                request = new int[]{0x51};
                break;
            case CMD_GO_WEST:
                request = new int[]{0x52};
                break;
            case CMD_STOP:
                request = new int[]{0x53};
                break;
            case CMD_HOLD:
                request = new int[]{0x54};
                break;
            case CMD_FAKE_LIMIT:
                request = new int[]{0x58};
                break;
            case CMD_SET_TIME:
                request = formSetTimeRequest(false);
                break;
            case CMD_SET_OFFSET:
                request = formSetTimeRequest(true);
                break;
            case CMD_SET_PARKING:
                request = formSetParking();
                break;
        }
        if (request == null) {
            request = new int[]{0x50};
        }

        byte[] bytesToSend = new byte[request.length + 6];
        bytesToSend[0] = 0x14;
        bytesToSend[1] = (byte)(request.length + 1);
        bytesToSend[2] = 0x51;
        bytesToSend[3] = 0x10;
        bytesToSend[4] = 0x00;
        for (int i = 0; i < request.length; i++) {
            bytesToSend[5 + i] = (byte)request[i];
        }
        byte checkSum = 0;
        for (int i = 0; i < bytesToSend.length - 1; i++) checkSum += bytesToSend[i];
        bytesToSend[bytesToSend.length - 1] = checkSum;
        myThreadConnected.write(bytesToSend);
        userCommand = CMD_NONE;
    }

/////////////////// Timer /////////////////////

    Handler timerHandler = new Handler();
    Runnable timerRunnable = new Runnable() {
        @Override
        public void run() {
            showState();
            if (checkAutoTime.isChecked()) showSystemTime();
            if (tracker.isConnected() && fillParkingParams) {
                fillParkingParams = false;
                editEvening.setText(textEvening.getText());
                editMorning.setText(textMorning.getText());
                editParking.setText(textParking.getText());
            }
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
        timerHandler.postDelayed(timerRunnable, 100);
    }

    /////////////////// Show state ///////////////

    private void showSystemTime() {
        Calendar currentTime = Calendar.getInstance();
        int hour = currentTime.get(Calendar.HOUR_OF_DAY);
        int minute = currentTime.get(Calendar.MINUTE);
        int second = currentTime.get(Calendar.SECOND);
        final String time = String.format(Locale.getDefault(), "%02d:%02d:%02d", hour, minute, second);
        editTrackerTime.setText(time);
    }

    private void showState() {
        showTime();
        showParking();
        showPosition();
        drawPanel();
        adjustHandlePermission();
    }

    private void showTime() {
        textTrackerTime.setText(tracker.getTimeString());
        if (checkAutoTime.isChecked()) {
            Calendar currentTime = Calendar.getInstance();
            int hour = currentTime.get(Calendar.HOUR_OF_DAY);
            int minute = currentTime.get(Calendar.MINUTE);
            int second = currentTime.get(Calendar.SECOND);
            final String time = String.format(Locale.getDefault(), "%02d:%02d:%02d", hour, minute, second);
            editTrackerTime.setText(time);
        }
    }

    private void showParking() {
        textEvening.setText(tracker.getEveningString());
        textMorning.setText(tracker.getMorningString());
        textParking.setText(tracker.getParkingString());
    }

    private void showPosition() {
        textAbsEast.setText(tracker.getEastLimitString(true));
        textAbsPos.setText(tracker.getPositionString(true));
        textAbsWest.setText(tracker.getWestLimitString(true));
        textRttEast.setText(tracker.getEastLimitString(false));
        textRttPos.setText(tracker.getPositionString(false));
        textRttWest.setText(tracker.getWestLimitString(false));
        textDriveState.setText(tracker.getStateString());
        textAbsEast.setTextColor(tracker.getEastLimitColor());
        textRttEast.setTextColor(tracker.getEastLimitColor());
        textAbsWest.setTextColor(tracker.getWestLimitColor());
        textRttWest.setTextColor(tracker.getWestLimitColor());
        buttonMoveEast.setText(tracker.isActive() ? "STOP" : "GO\nEAST");
        buttonMoveWest.setText(tracker.isActive() ? "STOP" : "GO\nWEST");
    }

    private void adjustHandlePermission() {
        if (tracker.isActive()) {
            if (checkTime.isEnabled()) {
                checkTime.setChecked(false);
                checkTime.setEnabled(false);
            }
            if (checkParking.isEnabled()) {
                checkParking.setChecked(false);
                checkParking.setEnabled(false);;
            }
        } else {
            if (!checkTime.isEnabled()) {
                checkTime.setEnabled(true);
            }
            if (!checkParking.isEnabled()) {
                checkParking.setEnabled(true);
            }
        }
    }

    /////////////////// Draw animated icon/////////////////////

    private Canvas canvas;
    private Paint paint = new Paint();
    private Bitmap bitmap = null;
    private static final int SUN_SIZE = 32;

    public void drawPanel() {
        int size = imagePanel.getWidth();
        if (bitmap == null) { // Init bitmap and canvas
            bitmap = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
            imagePanel.setImageBitmap(bitmap);
            canvas = new Canvas(bitmap);
        }

        float sunAng = tracker.getSunAngle();
        float panAng = tracker.getPanAngle();
        float midAng = tracker.getMidAngle();
        float hspAng = tracker.getHspAngle();

        canvas.drawColor(0xffbbffbb);

        float center = size / 2;
        float radius = center - SUN_SIZE - 5;
        RectF rect = new RectF(center - radius, center - radius,  center + radius, center + radius);
        // --- Shady halfcircle ---
        paint.setColor(0xffa0e0e0);
        paint.setStyle(Paint.Style.FILL);
        canvas.drawArc(rect, panAng - 185, 190, true, paint);
        // --- Sunny halfcircle ---
        paint.setColor(0xffffffaa);
        canvas.drawArc(rect, panAng,  180, true, paint);
        float pdx = radius * (float)Math.cos(Math.toRadians(panAng));
        float pdy = radius * (float)Math.sin(Math.toRadians(panAng));
        // --- Panel line ---
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(12);
        paint.setColor(0xff0000aa);
        canvas.drawLine(center + pdx, center + pdy, center - pdx, center - pdy, paint);
        // --- Perpendicular to panel ---
        paint.setStrokeWidth(2);
        canvas.drawLine(center, center, center - pdy, center + pdx, paint);
        // --- Unreachable arc (outside hall limits) ---
        paint.setColor(0xffaaaaaa);
        paint.setStrokeWidth(7);
        canvas.drawArc(rect, hspAng - 90, 360 - 2*hspAng, false, paint);
        // --- Active arc (inside hall limits) ---
        paint.setColor(0xff00ff00);
        canvas.drawArc(rect, 270 - hspAng, 2*hspAng, false, paint);

        radius = center - SUN_SIZE / 2;
        // --- Sun orbit ---
        paint.setColor(0xffffaa33);
        paint.setStrokeWidth(4);
        canvas.drawCircle(center, center, radius, paint);
        float x = center - radius * (float)Math.sin(Math.toRadians(sunAng));
        float y = center + radius * (float)Math.cos(Math.toRadians(sunAng));
        // --- Sun icon ---
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(0xffff8800);
        canvas.drawCircle(x, y, SUN_SIZE / 2, paint);

        float x1 = - (center - SUN_SIZE) * (float)Math.sin(Math.toRadians(midAng));
        float y1 = (center - SUN_SIZE) * (float)Math.cos(Math.toRadians(midAng));
        float x2 = - center * (float)Math.sin(Math.toRadians(midAng));
        float y2 = center * (float)Math.cos(Math.toRadians(midAng));
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(4);
        // --- Midday (12:00) mark ---
        paint.setColor(0xffff0000);
        canvas.drawLine(center + x1, center + y1, center + x2, center + y2, paint);
        // --- Midnight (00:00) mark ---
        paint.setColor(0xff000000);
        canvas.drawLine(center - x1, center - y1, center - x2, center - y2, paint);
        // --- Evening (18:00) mark ---
        paint.setColor(0xffcccccc);
        canvas.drawLine(center + y1, center - x1, center + y2, center - x2, paint);
        // --- Morning (06:00) mark ---
        canvas.drawLine(center - y1, center + x1, center - y2, center + x2, paint);
        imagePanel.invalidate();
    }

    /////////////////// Buttons /////////////////////

    private boolean doubleClick = false;
    Handler doubleHandler = new Handler();
    Runnable doubleRunnable = new Runnable() {
        @Override
        public void run() {
            doubleClick = false;
        }
    };

    private void setupImageOnClick() {
        imagePanel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (doubleClick) {
                    doubleClick = false;
                    if (!tracker.isActive()) {
                        userCommand = CMD_HOLD;
                    } else {
                        userCommand = CMD_FAKE_LIMIT;
                    }
                } else {
                    doubleClick = true;
                    doubleHandler.postDelayed(doubleRunnable, 300);
                }
            }
        });
    }

    public void onClickSetTime(View v) {
        if (!checkTime.isChecked()) return;
        if (tracker.isActive()) return;
        userCommand = CMD_SET_TIME;
    }

    public void onClickSetOffset(View v) {
        if (!checkTime.isChecked()) return;
        if (tracker.isActive()) return;
        userCommand = CMD_SET_OFFSET;
    }

    public void onClickSetParking(View v) {
        if (!checkParking.isChecked()) return;
        if (tracker.isActive()) return;
        userCommand = CMD_SET_PARKING;
    }

    public void onClickGoEast(View v) {
        if (!checkPosition.isChecked()) return;
        if (!tracker.isActive()) {
            userCommand = CMD_GO_EAST;
        } else {
            userCommand = CMD_STOP;
        }
    }

    public void onClickGoWest(View v) {
        if (!checkPosition.isChecked()) return;
        if (!tracker.isActive()) {
            userCommand = CMD_GO_WEST;
        } else {
            userCommand = CMD_STOP;
        }
    }

    public void onClickAlwaysParking(View v) {
        if (!checkParking.isChecked()) return;
        if (tracker.isActive()) return;
        int morning = parseParkingPoint(editMorning.getText().toString());
        int parking = parseParkingPoint(editParking.getText().toString());
        if ((morning == -1) || (parking == -1)) return;
        int evening = (morning + 1) % (24 * 60);
        String eveningTime = String.format(Locale.getDefault(), "%02d:%02d", evening / 60, evening % 60);
        editEvening.setText(eveningTime);
        userCommand = CMD_SET_PARKING;
    }

    public void onClickManual(View v) {
        if (!checkParking.isChecked()) return;
        if (tracker.isActive()) return;
        if (parseParkingPoint(editParking.getText().toString()) == -1) return;
        String time = "12:00";
        editEvening.setText(time);
        editMorning.setText(time);
        userCommand = CMD_SET_PARKING;
    }

    private void checkBoxChangeEvents() {
        checkPosition.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                buttonMoveEast.setEnabled(isChecked);
                buttonMoveWest.setEnabled(isChecked);
            }
        });
        checkParking.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                buttonSetParking.setEnabled(isChecked);
                buttonAlawaysParking.setEnabled(isChecked);
                buttonManual.setEnabled(isChecked);
            }
        });
        checkTime.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                buttonSetTime.setEnabled(isChecked);
                buttonSetOffset.setEnabled(isChecked);
            }
        });
    }
}
