package org.adnlab.adnbox.bluetooth;

import android.annotation.SuppressLint;
import android.bluetooth.*;
import android.content.Intent;
import android.util.Log;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import com.github.mikephil.charting.data.Entry;
import kotlin.UShort;
import org.adnlab.adnbox.MainActivity;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.security.PrivateKey;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

import  org.adnlab.adnbox.GlobalVariables;

import static org.adnlab.adnbox.GlobalVariables.*;

public class BLEDevice {

    public String getName() {
        return Name;
    }

    public void setName(String name) {
        Name = name;
    }

    public String getAddress() {
        return Address;
    }

    public void setAddress(String address) {
        Address = address;
    }


    public int getdBm() {
        return dBm;
    }

    public void setdBm(int dBm) {
        this.dBm = dBm;
    }

    public BLEDevice(String name, String address, int dBm,BluetoothDevice bluetoothDevice) {
        Name = name;
        Address = address;
        this.dBm = dBm;
        this.bluetoothDevice = bluetoothDevice;
    }

    public BluetoothDevice bluetoothDevice;
    private String Name;
    private String Address;
    private int dBm;

    public BluetoothGatt mBluetoothGatt = null;
    BluetoothGattCharacteristic deviceCtlChar;
    BluetoothGattCharacteristic deviceNotifyChar;
    Timer timer = new Timer();
    void SendSimpleBroadcast(String action,String msg){
        LocalBroadcastManager lbm = LocalBroadcastManager.getInstance(GlobalBLEDevice.bleManager.ContextActivity);
        Intent i = new Intent(action);
        i.putExtra("msg",msg);
        lbm.sendBroadcast(i);
    }

   @SuppressLint("MissingPermission")
   public void ReadDeviceStatus(){
       mBluetoothGatt.readCharacteristic(deviceCtlChar);
   }

    @SuppressLint({"MissingPermission"})
    public void SendDeviceCtl(byte[] ctldata){
        deviceCtlChar.setValue(ctldata);
        deviceCtlChar.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
        boolean x =  mBluetoothGatt.writeCharacteristic(deviceCtlChar);

    }
    private BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        @SuppressLint("MissingPermission")
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            BluetoothGattService deviceCtlService = gatt.getService(UUID.fromString("000000ee-0000-1000-8000-00805f9b34fb"));
            BluetoothGattService deviceNotifyService = gatt.getService(UUID.fromString("000000ff-0000-1000-8000-00805f9b34fb"));
            if(deviceCtlService == null  || deviceNotifyService == null ){
                SendSimpleBroadcast(GlobalVariables.BC_NOT_ADNBOX,"'" + getName()+ "' is not an ADNBox device");
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
                mBluetoothGatt.disconnect();
                return;
            }
            deviceCtlChar= deviceCtlService.getCharacteristic(UUID.fromString("0000ee01-0000-1000-8000-00805f9b34fb"));
            deviceNotifyChar = deviceNotifyService.getCharacteristic(UUID.fromString("0000ff01-0000-1000-8000-00805f9b34fb"));
            if(deviceCtlChar == null || deviceNotifyChar == null){
                SendSimpleBroadcast(GlobalVariables.BC_NOT_ADNBOX,"'" + getName()+ "' is not an ADNBox device");
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
                mBluetoothGatt.disconnect();
                return;
            }



            //

            mBluetoothGatt.setCharacteristicNotification(deviceNotifyChar,true);
            List<BluetoothGattDescriptor> descriptorList = deviceNotifyChar.getDescriptors();
            if(descriptorList != null && descriptorList.size() > 0) {
                for(BluetoothGattDescriptor descriptor : descriptorList) {
                    descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                    boolean notify_success = false;
                    while(!notify_success){
                        notify_success =  mBluetoothGatt.writeDescriptor(descriptor);
                        try {
                            Thread.sleep(133);
                        } catch (InterruptedException e) {
                            throw new RuntimeException(e);
                        }
                    }

                }
            }


        }

        @SuppressLint("MissingPermission")
        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            boolean read_success = false;
            while(!read_success){
                read_success =  mBluetoothGatt.readCharacteristic(deviceCtlChar);
                try {
                    Thread.sleep(133);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            byte[] data = characteristic.getValue();
            //
            //
            DataInputStream inputStream = new DataInputStream(new ByteArrayInputStream(data));
            try {
                short type = Short.reverseBytes(inputStream.readShort());
                long time = Long.reverseBytes(inputStream.readLong());
                if(type == 0xFFFFBABE){
                    //dynamic
                    int fluo_value0 = Short.toUnsignedInt(Short.reverseBytes(inputStream.readShort()));
                    int fluo_value1 = Short.toUnsignedInt(Short.reverseBytes(inputStream.readShort()));
                    boolean send_flag = false;
                    if(fluo_value0 != 65535 ){
                        FluoDataList0.add(new Entry(time,fluo_value0));
                        send_flag = true;
                    }
                    if(fluo_value1 != 65535 ){
                        FluoDataList1.add(new Entry(time,fluo_value1));
                        send_flag = true;
                    }
                    if(send_flag){
                        SendSimpleBroadcast(BC_FLUO_DATA_ARRIVED,"");
                    }
                }else if(type == 0xFFFFA1AB){
                    //temperature
                    byte controller_id = inputStream.readByte();
                    byte pwm_value = inputStream.readByte();
                    float temperature = Float.intBitsToFloat(Integer.reverseBytes(inputStream.readInt())) ;
                    if(controller_id == 0){
                        Heater1Power = Byte.toUnsignedInt(pwm_value);
                        Heater1Temperature = temperature;
                    }else if (controller_id == 1){
                        Heater2Power = Byte.toUnsignedInt(pwm_value);
                        Heater2Temperature = temperature;
                    }
                    SendSimpleBroadcast(BC_TEMP_DATA_ARRIVED,"");
                }
            } catch (IOException e) {
                throw new RuntimeException(e);
            }

        }

        @SuppressLint("MissingPermission")
        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            if(status == BluetoothGatt.GATT_SUCCESS) {
                if(characteristic ==  deviceCtlChar){
                    byte[] data=characteristic.getValue();
                    if(data == null){
                        SendSimpleBroadcast(GlobalVariables.BC_NOT_ADNBOX,"'" + getName()+ "' is not an ADNBox device");
                        try {
                            Thread.sleep(1000);
                        } catch (InterruptedException e) {
                            throw new RuntimeException(e);
                        }
                        mBluetoothGatt.disconnect();
                        return;
                    }

                    LocalBroadcastManager lbm = LocalBroadcastManager.getInstance(GlobalBLEDevice.bleManager.ContextActivity);
                    Intent i = new Intent(GlobalVariables.BC_ADNBOX_SUCCESS);
                    i.putExtra("msg","ADNBox connected");
                    i.putExtra("data",data);
                    lbm.sendBroadcast(i);

                    return;
                }


            }
            mBluetoothGatt.disconnect();
        }

        @SuppressLint("MissingPermission")
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);

            timer.cancel();
            if (newState == BluetoothProfile.STATE_CONNECTED) {  
                Log.d("", "STATE_CONNECTED");
                mBluetoothGatt = gatt;
                SendSimpleBroadcast(GlobalVariables.BC_BLE_CONNECTED,"Device '" + getName() + "' connected");
                mBluetoothGatt.discoverServices();  
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {       
                FluoDataList0.clear();
                FluoDataList1.clear();
                Heater1Power = -1;
                Heater1Temperature = 0;
                Heater2Power = -1;
                Heater2Temperature = 0;
                SendSimpleBroadcast(GlobalVariables.BC_BLE_DISCONNECTED,"Device '" + getName() + "' disconnected");
            }

        }
    };
    @SuppressLint("MissingPermission")
    public void Connect() {
        bluetoothDevice.connectGatt(GlobalBLEDevice.bleManager.ContextActivity,false,mGattCallback);
    }
    @SuppressLint("MissingPermission")
    public void disConnect() {
        if(mBluetoothGatt != null){
            mBluetoothGatt.disconnect();
        }
    }
}
