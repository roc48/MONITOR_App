package org.adnlab.adnbox;

import android.content.Context;
import android.content.Intent;
import com.github.mikephil.charting.data.Entry;

import java.util.ArrayList;
import java.util.List;

public class GlobalVariables {
    public static final String BC_NOT_ADNBOX = "org.adnlab.adnbox.bluetooth.NOT_ADNBOX";
    public static final String BC_ADNBOX_SUCCESS = "org.adnlab.adnbox.bluetooth.ADNBOX_SUCCESS";
    public static final String BC_BLE_CONNECTED = "org.adnlab.adnbox.bluetooth.BLE_CONNECTED";
    public static final String BC_BLE_DISCONNECTED = "org.adnlab.adnbox.bluetooth.BLE_DISCONNECTED";
    public static final String BC_DEVICE_CONNECTING = "org.adnlab.adnbox.bluetooth.DEVICE_CONNECTING";
    public static final String BC_TEMP_DATA_ARRIVED = "org.adnlab.adnbox.bluetooth.TEMP_DATA_ARRIVED";
    public static final String BC_FLUO_DATA_ARRIVED = "org.adnlab.adnbox.bluetooth.FLUO_DATA_ARRIVED";

    public static final List<Entry> FluoDataList0 = new ArrayList<Entry>();
    public static final List<Entry> FluoDataList1 = new ArrayList<Entry>();

    public static float Heater1Temperature = 0;
    public static int Heater1Power = -1;
    public static float Heater2Temperature = 0;
    public static int Heater2Power = -1;
}
