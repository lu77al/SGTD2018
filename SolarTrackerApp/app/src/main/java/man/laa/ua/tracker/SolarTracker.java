package man.laa.ua.tracker;

import android.graphics.Color;

import java.util.Locale;

import static java.lang.Math.round;

public class SolarTracker {
    private final static int FULL_CIRCLE = 70693;

    private boolean connected = false;
    private boolean active = false;
    private boolean reverse = false;
    private boolean eastLimit = false;
    private boolean westLimit = false;
    private int hour = 12;
    private int minute = 0;
    private int second = 0;
    private int position = FULL_CIRCLE / 2;
    private int target = FULL_CIRCLE / 2;
    private int halfSpan = (int)(FULL_CIRCLE / 24.0 * 5.5);
    private int offset = 0;
    private int evening = (int)(21.5 * 60);
    private int morning = 4 * 60;
    private int parking = 5 * 60;
    private float sunAngle = 180;
    private float panAngle = 180;
    private float midAngle = 180;
    private float hspAngle = 85;
    private float voltage = 0;
    private float current = 0;
    private int power = 0;

/*
>>>14-02-51-10-00-50-C7-
   00-01-02-03-04-05-06-07-08-09-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26
<<<14-16-10-51-80-12-3B-3B-8F-A6-00-00-8E-A6-00-C2-1C-E3-F4-FF-78-00-28-05-78-00-CD-
   [---HEADER---][H--M--S][PanPos-][F][StopPos][HSPN][OFFSET-][MRNG][EVNG][PRKG][CS]
*/
    public void decode(int in[]) {
        int flags = in[11];
        active = (flags & 1) != 0;
        reverse = (flags & 2) != 0;
        eastLimit =  (flags & 0x10) != 0;
        westLimit =  (flags & 0x20) != 0;
        hour = in[5];
        minute = in[6];
        second = in[7];
        position = in[8] + 256 * in[9] + 65536 * in[10];
        target = in[12] + 256 * in[13] + 65536 * in[14];
        halfSpan = in[15] + 256 * in[16];
        offset = in[17] + 256 * in[18] + 65536 * in[19];
        morning = in[20] + 256 * in[21];
        evening = in[22] + 256 * in[23];
        parking = in[24] + 256 * in[25];
        if (in[04] == 0x81) {
            voltage = (float)((in[26] + 256 * in[27]) / 10.0);
            current = (float)(in[28] / 10.0);
        } else {
            voltage = current = 0;
        }
        double koef = 360.0 / 70693.0;
        double offsAngle = offset * koef;
        sunAngle = (float)((hour * 3600 + minute * 60 + second) / 240.0 + offsAngle);
        panAngle = (float)(position * koef);
        midAngle = (float)(180.0 + offsAngle);
        hspAngle = (float)(halfSpan * koef);
        power = round(voltage * current);
        connected = true;
    }

    public String getEveningString() {
        return minutesToTimeString(evening);
    }

    public String getMorningString() {
        return minutesToTimeString(morning);
    }

    public String getParkingString() {
        return minutesToTimeString(parking);
    }

    public String getTimeString() {
        return timeToString(hour, minute, second);
    }

    public String getPowerString() {
        return String.format(Locale.getDefault(), "%dW/%.1fV", power, voltage);
    }

    public String getEastLimitString(boolean absolute) {
        return  "E{" + positionToString(FULL_CIRCLE / 2 - halfSpan, absolute) + "}";
    }

    public String getWestLimitString(boolean absolute) {
        return  "{" + positionToString(FULL_CIRCLE / 2 + halfSpan, absolute) + "}W";
    }

    public String getPositionString(boolean absolute) {
        return  positionToString(position, absolute);
    }

    public String getStateString() {
        if (active) {
            if (reverse) {
                return "{" + positionToString(target, true) + "}<<P";
            } else {
                return "P>>{" + positionToString(target, true) + "}";
            }
        } else {
            return "-------------------";
        }
    }

    public int getEastLimitColor() {
        return eastLimit ? Color.RED : Color.GRAY;
    }

    public int getWestLimitColor() {
        return westLimit ? Color.RED : Color.GRAY;
    }

    public boolean isActive() {
        return active;
    }

    public boolean isConnected() {
        return connected;
    }

    public float getPanAngle() {
        return panAngle;
    }

    public float getSunAngle() {
        return sunAngle;
    }

    public float getMidAngle() {
        return midAngle;
    }

    public float getHspAngle() {
        return hspAngle;
    }

    private String positionToString(int position, boolean absolute) {
        if (!absolute) position = position - offset;
        while (position < 0) position += FULL_CIRCLE;
        while (position >= FULL_CIRCLE) position -= FULL_CIRCLE;
        int second = (int)(position * (24.0 * 3600.0 / FULL_CIRCLE));
        int hour = second / 3600;
        second %= 3600;
        int minute = second / 60;
        second %= 60;
        return String.format(Locale.getDefault(), "%02d:%02d:%02d", hour, minute, second);
    }

    private String timeToString(int hour, int minute, int second) {
        return String.format(Locale.getDefault(), "%02d:%02d:%02d", hour, minute, second);
    }

    private String minutesToTimeString(int minutes) {
        return String.format(Locale.getDefault(), "%02d:%02d", minutes / 60, minutes % 60);
    }



/*
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

*/

/*

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
    driveActive = active;
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
    final String eastButton = driveActive ? "STOP" : "GO\nEAST";
    final String westButton = driveActive ? "STOP" : "GO\nWEST";

    double koef = 360.0 / 70693.0;
    double offsAng = offset * koef;
    sunAng = (float)((in[5] * 3600 + in[6] * 60 + in[7]) / 240.0 + offsAng);
    panAng = (float)(panPos * koef);
    midAng = (float)(180.0 + offsAng);
    hspAng = (float)(halfSpan * koef);

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
            textAbsEast.setTextColor(eastLimitColor);
            textRttEast.setTextColor(eastLimitColor);
            textAbsWest.setTextColor(westLimitColor);
            textRttWest.setTextColor(westLimitColor);
            buttonMoveEast.setText(eastButton);
            buttonMoveWest.setText(westButton);
            if (active) {
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
    });
*/
}
