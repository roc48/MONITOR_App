package org.adnlab.adnbox.ui.dashboard;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.formatter.DefaultAxisValueFormatter;
import com.github.mikephil.charting.formatter.ValueFormatter;
import org.adnlab.adnbox.GlobalVariables;
import org.adnlab.adnbox.databinding.FragmentDashboardBinding;

import java.util.Timer;
import java.util.TimerTask;


public class DashboardFragment extends Fragment {

    private FragmentDashboardBinding binding;

    private LineChart fluorescenceChart;

    private TextView heater1TemperatureTV;
    private TextView heater1PowerTV;
    private TextView heater2TemperatureTV;
    private TextView heater2PowerTV;

    LocalBroadcastManager lmb;
    BroadcastReceiver fluo_bc_receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            RefreshFluoData();
        }
    };

    BroadcastReceiver temp_bc_receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            RefreshHeaterData();
        }
    };

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {

        binding = FragmentDashboardBinding.inflate(inflater, container, false);
        View root = binding.getRoot();
        fluorescenceChart = binding.chart;
        heater1TemperatureTV = binding.heater1TempTv;
        heater2TemperatureTV = binding.heater2TempTv;
        heater1PowerTV = binding.heater1PowerTv;
        heater2PowerTV = binding.heater2PowerTv;

        fluorescenceChart.setPinchZoom(true);
        fluorescenceChart.getDescription().setEnabled(false);
        fluorescenceChart.setDrawBorders(true);
        fluorescenceChart.getAxisRight().setEnabled(false);
        fluorescenceChart.getXAxis().setPosition(XAxis.XAxisPosition.BOTTOM);
        fluorescenceChart.getXAxis().setTextSize(8);
        fluorescenceChart.getXAxis().setLabelRotationAngle(-45);
        fluorescenceChart.getAxisLeft().setTextSize(8);
        fluorescenceChart.setDrawBorders(true);
        RefreshFluoData();
        RefreshHeaterData();

        lmb = LocalBroadcastManager.getInstance(binding.getRoot().getContext());
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(GlobalVariables.BC_FLUO_DATA_ARRIVED);
        lmb.registerReceiver(fluo_bc_receiver,intentFilter);


        IntentFilter intentFilter2 = new IntentFilter();
        intentFilter2.addAction(GlobalVariables.BC_TEMP_DATA_ARRIVED);
        lmb.registerReceiver(temp_bc_receiver,intentFilter2);


        return root;
    }

    private void RefreshHeaterData() {
       try {
           String h1temp = "Temperature:";
           String h1pwr = "Power:";
           String h2temp = "Temperature:";
           String h2pwr = "Power:";

           if(GlobalVariables.Heater1Temperature > 0){
               h1temp += GlobalVariables.Heater1Temperature + "°C";
           }else{
               h1temp += "N/A";
           }

           if(GlobalVariables.Heater2Temperature > 0){
               h2temp += GlobalVariables.Heater2Temperature + "°C";
           }else{
               h2temp += "N/A";
           }

           if(GlobalVariables.Heater1Power >= 0){
               h1pwr += String.format("%.2f",((float)GlobalVariables.Heater1Power / 255)*100 )+ "%";
           }else{
               h1pwr += "N/A";
           }

           if(GlobalVariables.Heater2Power >= 0){
               h2pwr += String.format("%.2f",((float)GlobalVariables.Heater2Power / 255)*100 )+ "%";
           }else{
               h2pwr += "N/A";
           }

           heater1TemperatureTV.setText(h1temp);
           heater1PowerTV.setText(h1pwr);
           heater2TemperatureTV.setText(h2temp);
           heater2PowerTV.setText(h2pwr);
       }catch (Exception ignore){};
    }

    private void RefreshFluoData() {
        try {
            LineDataSet dataSet0 = new LineDataSet(GlobalVariables.FluoDataList0, "Fluorescence_0");
            dataSet0.setColor(Color.rgb(85,132,227),128);
            dataSet0.setLineWidth(1.5F);
            dataSet0.setCircleColor(Color.rgb(40,100,220));
            dataSet0.setCircleHoleColor(Color.rgb(40,100,220));
            dataSet0.setCircleRadius(2);
            dataSet0.setDrawValues(false);

            LineDataSet dataSet1 = new LineDataSet(GlobalVariables.FluoDataList1, "Fluorescence_1");
            dataSet1.setColor(Color.rgb(0xfa,0x3f,0x68),128);
            dataSet1.setLineWidth(1.5F);
            dataSet1.setCircleColor(Color.rgb(0xfa,0x3f,0x68));
            dataSet1.setCircleHoleColor(Color.rgb(0xfa,0x3f,0x68));
            dataSet1.setCircleRadius(2);
            dataSet1.setDrawValues(false);

            LineData lineData = new LineData(dataSet0,dataSet1);
            fluorescenceChart.setData(lineData);
            fluorescenceChart.invalidate();
        }catch (Exception ignored){}
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        lmb.unregisterReceiver(fluo_bc_receiver);
        lmb.unregisterReceiver(temp_bc_receiver);
        binding = null;
    }
}