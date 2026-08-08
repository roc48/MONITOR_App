package org.adnlab.adnbox.ui.home;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.content.*;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.*;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import org.adnlab.adnbox.GlobalVariables;
import org.adnlab.adnbox.R;
import org.adnlab.adnbox.bluetooth.GlobalBLEDevice;
import org.adnlab.adnbox.databinding.FragmentHomeBinding;

import java.io.*;
import java.util.Timer;
import java.util.TimerTask;

public class HomeFragment extends Fragment {

    private FragmentHomeBinding binding;

    String LastStatus = "Device not connected";
    byte[] LastStatusData = null;

    private TextView deviceStatusTV;
    private TextView deviceModelTV;
    private TextView firmwareVersionTV;
    private TextView globalTimeTV;
    private TextView heater1TV;
    private TextView heater1SetTV;
    private TextView heater2TV;
    private TextView heater2SetTV;
    private TextView fridgeTV;
    private TextView laserTV;
    private TextView dynamicTV;
    private TextView bleTV;

    private Button resetBtn;
    private Button heater1SetBtn;
    private Button heater2SetBtn;

    private Switch heater1Switch;
    private Switch heater2Switch;
    private Switch dynamicSwitch;

    private Switch fridgeSwitch;

    BroadcastReceiver status_bc_receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Timer timer = null;
            TimerTask timerTask = new TimerTask() {
                @Override
                public void run() {
                    try {
                        GlobalBLEDevice.bleDevice.ReadDeviceStatus();
                    }catch (Exception ignored){}
                }
            };
            LastStatus = intent.getStringExtra("msg");
            if(intent.getAction().equals(GlobalVariables.BC_ADNBOX_SUCCESS)){
                LastStatusData =  intent.getByteArrayExtra("data");
                timer = new Timer();
                timer.schedule(timerTask,500);
            }else{
                if(timer!=null){
                    timer.cancel();
                    timer = null;
                }
                LastStatusData = null;
                heater1Switch.setChecked(false);
                heater2Switch.setChecked(false);
                dynamicSwitch.setChecked(false);
                fridgeSwitch.setChecked(false);
                heater1Switch.setEnabled(false);
                heater2Switch.setEnabled(false);
                dynamicSwitch.setChecked(false);
                fridgeSwitch.setChecked(false);
                dynamicSwitch.setEnabled(false);
                fridgeSwitch.setEnabled(false);

                resetBtn.setEnabled(false);
                heater1SetBtn.setEnabled(false);
                heater2SetBtn.setEnabled(false);
            }

            RefreshView();
        }
    };

    LocalBroadcastManager lbm;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {


        binding = FragmentHomeBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        dynamicSwitch = binding.switchDynamic;
        heater1Switch = binding.switch1;
        heater2Switch = binding.switch2;
        fridgeSwitch = binding.switchFridge;
        heater1Switch.setChecked(false);
        heater2Switch.setChecked(false);
        heater1Switch.setEnabled(false);
        heater2Switch.setEnabled(false);
        dynamicSwitch.setEnabled(false);
        dynamicSwitch.setChecked(false);
        fridgeSwitch.setEnabled(false);
        fridgeSwitch.setChecked(false);


        fridgeSwitch.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(fridgeSwitch.isChecked()){
                     SetFridgeValueDialog();
                }else{
                    fridgeSwitch.setChecked(true);
                }
            }
        });


        dynamicSwitch.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(dynamicSwitch.isChecked()){
                    GlobalBLEDevice.bleDevice.SendDeviceCtl(new byte[]{0x06,0x00,0x00,0x00,0x01,0x00,0x00,0x00});
                }else{
                    GlobalBLEDevice.bleDevice.SendDeviceCtl(new byte[]{0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00});
                }
            }
        });


        heater1Switch.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(heater1Switch.isChecked()){
                    GlobalBLEDevice.bleDevice.SendDeviceCtl(new byte[]{0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00});
                }else{
                    GlobalBLEDevice.bleDevice.SendDeviceCtl(new byte[]{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00});
                }
            }
        });

        heater2Switch.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(heater2Switch.isChecked()){
                    GlobalBLEDevice.bleDevice.SendDeviceCtl(new byte[]{0x03,0x00,0x00,0x00,0x01,0x00,0x00,0x00});
                }else{
                    GlobalBLEDevice.bleDevice.SendDeviceCtl(new byte[]{0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00});
                }
            }
        });

        resetBtn = binding.btnRestart;
        resetBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GlobalBLEDevice.bleDevice.SendDeviceCtl(new byte[]{0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00});
            }
        });

        heater1SetBtn = binding.btnHeater1TempSet;
        heater1SetBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SetTemperatureValueDialog(2);

            }
        });
        heater2SetBtn = binding.btnHeater2TempSet;
        heater2SetBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SetTemperatureValueDialog(4);
            }
        });

        resetBtn.setEnabled(false);
        heater1SetBtn.setEnabled(false);
        heater2SetBtn.setEnabled(false);
        deviceStatusTV = binding.textHome;
        deviceModelTV = binding.deviceModel;
        firmwareVersionTV= binding.firmwareVersion;
        globalTimeTV= binding.globalTime;
        heater1TV= binding.heater1;
        heater1SetTV= binding.heater1TempSet;
        heater2TV= binding.heater2;
        heater2SetTV= binding.heater2TempSet;
        fridgeTV= binding.fridge;
        laserTV= binding.laser;
        dynamicTV= binding.dynamic;
        bleTV= binding.ble;

        RefreshView();
        if(lbm == null){
            lbm = LocalBroadcastManager.getInstance(binding.getRoot().getContext());
        }
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(GlobalVariables.BC_NOT_ADNBOX);
        intentFilter.addAction(GlobalVariables.BC_ADNBOX_SUCCESS);
        intentFilter.addAction(GlobalVariables.BC_BLE_CONNECTED);
        intentFilter.addAction(GlobalVariables.BC_BLE_DISCONNECTED);
        intentFilter.addAction(GlobalVariables.BC_DEVICE_CONNECTING);

        lbm.unregisterReceiver(status_bc_receiver);
        lbm.registerReceiver(status_bc_receiver,intentFilter);

        return root;
    }

    private void SetTemperatureValueDialog(int heater_id) {
        if (heater_id!=2 && heater_id!=4){
            return;
        }
        AlertDialog.Builder customizeDialog =new AlertDialog.Builder(binding.getRoot().getContext());
        final View dialogView = LayoutInflater.from(binding.getRoot().getContext()).inflate(R.layout.temp_input_dialog,null);
        customizeDialog.setTitle("Set temperature");
        customizeDialog.setView(dialogView);
        customizeDialog.setCancelable(true);

        EditText editText =  dialogView.findViewById(R.id.val);
        if(heater_id == 2)
        { editText.setText("100");}
        else{ editText.setText("37");}

        editText.setHint("Input temperature");

        customizeDialog.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                int value = Integer.parseInt(editText.getText().toString());
                if(value <= 14 || value > 121){
                    Toast.makeText(dialogView.getContext(), "value must greater than 15 and less than 120!", Toast.LENGTH_SHORT).show();
                }else {
                    byte[] data = new byte[]{(byte) heater_id,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
                    data[4] = (byte)(value);
                    data[5] = (byte)(value >> 8);
                    data[6] = (byte)(value >> 16);
                    data[7] = (byte)(value >> 24);
                    GlobalBLEDevice.bleDevice.SendDeviceCtl(data);
                }

            }
        });
        customizeDialog.show();

    }

    private void SetFridgeValueDialog() {
        AlertDialog.Builder customizeDialog =new AlertDialog.Builder(binding.getRoot().getContext());
        final View dialogView = LayoutInflater.from(binding.getRoot().getContext()).inflate(R.layout.time_input_dialog,null);
        customizeDialog.setTitle("Set fridge up time");
        customizeDialog.setView(dialogView);
        customizeDialog.setCancelable(true);

        EditText editText =  dialogView.findViewById(R.id.val);
        editText.setText("180000");
        editText.setHint("Input fridge time");

        customizeDialog.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                int value = Integer.parseInt(editText.getText().toString());
                if(value <= 0 || value > 600001){
                    Toast.makeText(dialogView.getContext(), "value must greater than 0 and less than 600000!", Toast.LENGTH_SHORT).show();
                }else {
                    byte[] data = new byte[]{0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
                    data[4] = (byte)(value);
                    data[5] = (byte)(value >> 8);
                    data[6] = (byte)(value >> 16);
                    data[7] = (byte)(value >> 24);
                    GlobalBLEDevice.bleDevice.SendDeviceCtl(data);
                }

            }
        });
        customizeDialog.show();


    };

    private void RefreshView() {
        deviceStatusTV.setText( LastStatus  );

        short device_model = -1;
        long global_time = -1;
        int firmware_version = -1;
        int status = -1;
        int heater_1 = -1;
        int heater_1_temp_set = -1;
        int heater_2 = -1;
        int heater_2_temp_set = -1;
        int fridge = -1;
        int laser = -1;
        int dynamic = -1;
        int ble = -1;


        if(LastStatusData != null){

            resetBtn.setEnabled(true);
            heater1SetBtn.setEnabled(true);
            heater2SetBtn.setEnabled(true);
            heater1Switch.setEnabled(true);
            heater2Switch.setEnabled(true);
            dynamicSwitch.setEnabled(true);
            fridgeSwitch.setEnabled(true);

            DataInputStream inputStream = new DataInputStream(new ByteArrayInputStream(LastStatusData));
            try {
                device_model = Short.reverseBytes(inputStream.readShort()) ;
                global_time =   Long.reverseBytes(inputStream.readLong());
                firmware_version =inputStream.readUnsignedByte();
                status = inputStream.readUnsignedByte();
                heater_1 = inputStream.readUnsignedByte();
                heater_1_temp_set = inputStream.readUnsignedByte();
                heater_2 = inputStream.readUnsignedByte();
                heater_2_temp_set = inputStream.readUnsignedByte();
                fridge = inputStream.readUnsignedByte();
                laser = inputStream.readUnsignedByte();
                dynamic = inputStream.readUnsignedByte();
                ble = inputStream.readUnsignedByte();
            } catch (IOException e) {
                device_model = -1;
                global_time = -1;
                firmware_version = -1;
                status = -1;
                heater_1 = -1;
                heater_1_temp_set = -1;
                heater_2 = -1;
                heater_2_temp_set = -1;
                fridge = -1;
                laser = -1;
                dynamic = -1;
                ble = -1;
            }
        }


        if(device_model != -1){
            deviceModelTV.setText("Device model: ADNBox " + device_model + " Series");
        }else{
            deviceModelTV.setText("Device model: N/A" );
        }

        if(firmware_version   != -1){
            firmwareVersionTV.setText("Firmware version: " + firmware_version);
        }else{
            firmwareVersionTV.setText("Firmware version: N/A" );
        }

        if(global_time  >= 0){
            globalTimeTV.setText("Up time: " + global_time/1000 + "s");
        }else{
            globalTimeTV.setText("Up time: N/A" );
        }

        if(heater_1 == 0){
            heater1TV.setText("Heater 1: Disabled");
            heater1Switch.setChecked(false);
        }else if (heater_1 == 1) {
            heater1TV.setText("Heater 1: Enabled");
            heater1Switch.setChecked(true);
        }else{
            heater1TV.setText("Heater 1: N/A");
            heater1Switch.setChecked(false);
        }

        if(heater_1_temp_set  != -1){
            heater1SetTV.setText("Heater 1 set-value: " + heater_1_temp_set + "°C");
        }else{
            heater1SetTV.setText("Heater 1 set-value: N/A" );
        }

        if(heater_2 == 0){
            heater2TV.setText("Heater 2: Disabled");
            heater2Switch.setChecked(false);
        }else if (heater_2 == 1) {
            heater2TV.setText("Heater 2: Enabled");
            heater2Switch.setChecked(true);
        }else{
            heater2TV.setText("Heater 2: N/A");
            heater2Switch.setChecked(false);
        }

        if(heater_2_temp_set  != -1){
            heater2SetTV.setText("Heater 2 set-value: " + heater_2_temp_set + "°C");
        }else{
            heater2SetTV.setText("Heater 2 set-value: N/A" );
        }

        if(fridge == 0){
            fridgeTV.setText("Fridge: Disabled");
            fridgeSwitch.setChecked(false);
        }else if (fridge == 1) {
            fridgeTV.setText("Fridge: Enabled");
            fridgeSwitch.setChecked(true);
        }else{
            fridgeTV.setText("Fridge: N/A");
            fridgeSwitch.setChecked(false);
        }

        if(laser == 0){
            laserTV.setText("Laser: Disabled");
        }else if (laser == 1) {
            laserTV.setText("Laser: Enabled");
        }else{
            laserTV.setText("Laser: N/A");
        }

        if(dynamic == 0){
            dynamicTV.setText("Dynamic monitoring: Disabled");
            dynamicSwitch.setChecked(false);
        }else if (dynamic == 1) {
            dynamicTV.setText("Dynamic monitoring: Enabled");
            dynamicSwitch.setChecked(true);
        }else{
            dynamicTV.setText("Dynamic monitoring: N/A");
            dynamicSwitch.setChecked(false);
        }

        if(ble == 0){
            bleTV.setText("Bluetooth: Disconnected");
        }else if (ble == 1) {
            bleTV.setText("Bluetooth: Connected");
        }else{
            bleTV.setText("Bluetooth: N/A");
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
        //lbm.unregisterReceiver(status_bc_receiver);
    }
}