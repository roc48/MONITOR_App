package org.adnlab.adnbox;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.*;
import android.content.pm.PackageManager;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.Toast;
import androidx.annotation.ColorInt;
import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.coordinatorlayout.widget.CoordinatorLayout;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import com.google.android.material.bottomnavigation.BottomNavigationView;
import androidx.appcompat.app.AppCompatActivity;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
import org.adnlab.adnbox.bluetooth.BLEDevice;
import org.adnlab.adnbox.bluetooth.BLEDeviceListAdapter;
import org.adnlab.adnbox.bluetooth.GlobalBLEDevice;
import org.adnlab.adnbox.databinding.ActivityMainBinding;
import org.jetbrains.annotations.NotNull;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

import static android.Manifest.permission.*;

public class MainActivity extends AppCompatActivity {

    private ActivityMainBinding binding;

    String[] permissions = new String[]{BLUETOOTH_CONNECT,BLUETOOTH_SCAN,ACCESS_FINE_LOCATION};

@SuppressLint("MissingPermission")
void scanDevice(DialogInterface dialogInterface, View dialogView){
    if(GlobalBLEDevice.bleDevice!= null && GlobalBLEDevice.bleDevice.mBluetoothGatt != null){
        GlobalBLEDevice.bleDevice.mBluetoothGatt.disconnect();
    }
    if(! GlobalBLEDevice.bleManager.mScanning) {
        GlobalBLEDevice.bleManager.checkBle(MainActivity.this);
        GlobalBLEDevice.bleManager.openBluetooth();
        List<BLEDevice> DeviceList = new ArrayList<>();
        ListView listView = dialogView.findViewById(R.id.DeviceList);
        BLEDeviceListAdapter DeviceListAdapter = new BLEDeviceListAdapter(binding.getRoot().getContext(), R.layout.device_selection_dialog, DeviceList);
        listView.setAdapter(DeviceListAdapter);
        GlobalBLEDevice.bleManager.startDiscoveryDevice(DeviceListAdapter);

        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {

                GlobalBLEDevice.bleManager.stopDiscoveryDevice();

                GlobalBLEDevice.bleDevice= (BLEDevice) adapterView.getItemAtPosition(i);
                GlobalBLEDevice.bleDevice.Connect();

                LocalBroadcastManager lbm = LocalBroadcastManager.getInstance(GlobalBLEDevice.bleManager.ContextActivity);
                Intent intent = new Intent("org.adnlab.adnbox.bluetooth.DEVICE_CONNECTING");
                intent.putExtra("msg","Connecting...");
                lbm.sendBroadcast(intent);

                Field field = null;
                try {
                    field = dialogInterface.getClass().getSuperclass().getDeclaredField("mShowing");
                    field.setAccessible(true);
                    field.set(dialogInterface, true);
                    dialogInterface.dismiss();
                } catch (NoSuchFieldException | IllegalAccessException e) {
                    throw new RuntimeException(e);
                }
            }
        });
    }else{
        Toast.makeText(this, "It is scanning now!", Toast.LENGTH_SHORT).show();
    }
}


    void showScanDialog(){
        AlertDialog.Builder customizeDialog =new AlertDialog.Builder(binding.getRoot().getContext());
        final View dialogView = LayoutInflater.from(binding.getRoot().getContext()).inflate(R.layout.device_selection_dialog,null);

        customizeDialog.setTitle("Select an ADNBox Device");
        customizeDialog.setView(dialogView);
        customizeDialog.setCancelable(true);
        customizeDialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialogInterface) {

                GlobalBLEDevice.bleManager.stopDiscoveryDevice();
            }
        });
        customizeDialog.setNegativeButton("Close", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                GlobalBLEDevice.bleManager.stopDiscoveryDevice();
                Field field = null;
                try {
                    field = dialogInterface.getClass().getSuperclass().getDeclaredField("mShowing");
                    field.setAccessible(true);
                    field.set(dialogInterface, true);
                    dialogInterface.dismiss();
                } catch (NoSuchFieldException | IllegalAccessException e) {
                    throw new RuntimeException(e);
                }
            }
        });
        customizeDialog.setPositiveButton("Scan", new DialogInterface.OnClickListener() {
            @SuppressLint("MissingPermission")
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                Field field = null;
                try {
                    field = dialogInterface.getClass().getSuperclass().getDeclaredField("mShowing");
                    field.setAccessible(true);
                    field.set(dialogInterface, false);
                    dialogInterface.dismiss();
                } catch (NoSuchFieldException | IllegalAccessException e) {
                    throw new RuntimeException(e);
                }

                scanDevice(dialogInterface,dialogView);

            }
        });

        AlertDialog dialog = customizeDialog.create();
        dialog.setOnShowListener(new DialogInterface.OnShowListener() {
            @Override
            public void onShow(DialogInterface dialogInterface) {
                scanDevice(dialogInterface,dialogView);
            }
        });
        dialog.show();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        binding.fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showScanDialog();
            }
        });
        BottomNavigationView navView = findViewById(R.id.nav_view);
        // Passing each menu ID as a set of Ids because each
        // menu should be considered as top level destinations.
        AppBarConfiguration appBarConfiguration = new AppBarConfiguration.Builder(
                R.id.navigation_home, R.id.navigation_dashboard, R.id.navigation_notifications)
                .build();
        NavController navController = Navigation.findNavController(this, R.id.nav_host_fragment_activity_main);
        NavigationUI.setupActionBarWithNavController(this, navController, appBarConfiguration);
        NavigationUI.setupWithNavController(binding.navView, navController);

        FloatingActionButton fab = findViewById(R.id.fab);
        LocalBroadcastManager lmb = LocalBroadcastManager.getInstance(binding.getRoot().getContext());

        IntentFilter DEVICE_CONNECTING_IntentFilter = new IntentFilter();
        DEVICE_CONNECTING_IntentFilter.addAction(GlobalVariables.BC_DEVICE_CONNECTING);
        DEVICE_CONNECTING_IntentFilter.addAction(GlobalVariables.BC_ADNBOX_SUCCESS);
        DEVICE_CONNECTING_IntentFilter.addAction(GlobalVariables.BC_BLE_DISCONNECTED);

        lmb.registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                switch (intent.getAction()){
                    case GlobalVariables.BC_DEVICE_CONNECTING:
                        fab.setBackgroundTintList(ColorStateList.valueOf(Color.rgb(252,182,95)));
                        break;
                    case GlobalVariables.BC_ADNBOX_SUCCESS:
                        fab.setBackgroundTintList(ColorStateList.valueOf(Color.rgb(71,201,127)));
                        break;
                    default:
                        fab.setBackgroundTintList(ColorStateList.valueOf(Color.rgb(250,63,104)));
                        break;
                }
            }
        },DEVICE_CONNECTING_IntentFilter);


        boolean permissionFlag = true;
        for(int i=0;i< permissions.length;i++) {
            if(checkSelfPermission(permissions[i]) != PackageManager.PERMISSION_GRANTED) {
                permissionFlag = false;
                break;
            }
        }
        if(!permissionFlag) {
            requestPermissions(permissions,1);
            Toast.makeText(this, "Please grant permissions to ADNBox!", Toast.LENGTH_SHORT).show();
        }

        ActionBar actionBar = this.getSupportActionBar();
        actionBar.setCustomView(R.layout.title);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM);

    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull @NotNull String[] permissions, @NonNull @NotNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        boolean permissionFlag = true;
        for(int i=0;i< permissions.length;i++) {
            if(checkSelfPermission(permissions[i]) != PackageManager.PERMISSION_GRANTED) {
                permissionFlag = false;
                break;
            }
        }
        if(!permissionFlag){
            Toast.makeText(this, "Permissions NOT granted!", Toast.LENGTH_SHORT).show();
            android.os.Process.killProcess(android.os.Process.myPid());
            System.exit(0);
        }

    }
}