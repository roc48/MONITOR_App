package org.adnlab.adnbox.bluetooth;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import org.adnlab.adnbox.R;

import java.util.List;

public class BLEDeviceListAdapter extends ArrayAdapter<BLEDevice> {

    public BLEDeviceListAdapter(@NonNull Context context, int resource, @NonNull List<BLEDevice> objects) {
        super(context, resource, objects);
    }

    @Override
    public synchronized void add(@Nullable BLEDevice object) {
        boolean addFlag = true;
        for (int j = 0; j < this.getCount(); j++) {
            if(this.getItem(j).getAddress().equals(object.getAddress())){
                this.getItem(j).setdBm(object.getdBm());
                this.getItem(j).setName(object.getName());
                addFlag = false;
            }
        }
        if(addFlag){
            super.add(object);
        }
    }

    @Override
    public View getView(int i, View view, ViewGroup viewGroup) {
        BLEDevice device=getItem(i);
        View v= LayoutInflater.from(getContext()).inflate(R.layout.device_list,viewGroup,false);
        TextView name =v.findViewById(R.id.device_name);
        TextView addr=v.findViewById(R.id.device_addr);
        TextView dbm=v.findViewById(R.id.device_dbm);
        name.setText(device.getName());
        addr.setText(device.getAddress());
        dbm.setText(device.getdBm()+"dBm");
        return v;
    }
}
