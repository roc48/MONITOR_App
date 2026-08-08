package org.adnlab.adnbox.bluetooth;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;

public class BLEManager {


    public BluetoothManager bluetoothManager;
    public BluetoothAdapter bluetoothAdapter;

    public Context ContextActivity;
    public boolean mScanning;

    public boolean checkBle(Activity context) {
        ContextActivity = context;
        bluetoothManager = (BluetoothManager) ContextActivity.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager == null) {
            return false;
        }
        bluetoothAdapter = bluetoothManager.getAdapter();
        return bluetoothAdapter != null;
    }

    public boolean isEnable() {
        if (bluetoothAdapter == null) {
            return false;
        }
        return bluetoothAdapter.isEnabled();
    }

    @SuppressLint("MissingPermission")
    public void openBluetooth() {
        if (!isEnable()) {
            Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            ContextActivity.startActivity(intent);
        }
    }
    BLEDeviceListAdapter deviceListAdapter;
    private BluetoothAdapter.LeScanCallback leScanCallback = new BluetoothAdapter.LeScanCallback() {
        @SuppressLint("MissingPermission")
        @Override
        public void onLeScan(BluetoothDevice bluetoothDevice, int i, byte[] bytes) {
                if (bluetoothDevice.getName() == null) {
                    return;
                }
                if(deviceListAdapter == null){
                    return;
                }


                deviceListAdapter.add(new BLEDevice(bluetoothDevice.getName(),bluetoothDevice.getAddress(),i,bluetoothDevice));
                deviceListAdapter.notifyDataSetChanged();

        }
    };

    @SuppressLint("MissingPermission")
    public  void stopDiscoveryDevice(){
        mScanning = false;
        bluetoothAdapter.stopLeScan(leScanCallback);
        deviceListAdapter = null   ;
    }
    @SuppressLint("MissingPermission")
    public void startDiscoveryDevice(BLEDeviceListAdapter thisDeviceListAdapter) {

        if (isEnable() && !mScanning) {
            // Stops scanning after a pre-defined scan period.

            new Handler().postDelayed(new Runnable() {
                @SuppressLint("MissingPermission")
                @Override
                public void run() {
                    mScanning = false;
                    bluetoothAdapter.stopLeScan(leScanCallback);
                    deviceListAdapter = null   ;
                }
            },1000*3);

            mScanning = true;
            deviceListAdapter = thisDeviceListAdapter   ;
            bluetoothAdapter.startLeScan(leScanCallback);
        } else {
            mScanning = false;
            bluetoothAdapter.stopLeScan(leScanCallback);
            deviceListAdapter = null   ;
        }


    }

}
