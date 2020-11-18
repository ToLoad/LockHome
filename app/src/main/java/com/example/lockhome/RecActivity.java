package com.example.lockhome;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;

import com.google.firebase.database.ChildEventListener;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

import java.util.ArrayList;
import java.util.List;

public class RecActivity extends AppCompatActivity {
    private FirebaseDatabase mDatabase;
    private DatabaseReference mReference;
    private ChildEventListener mChild;

    private ListView doorRecord;
    private ArrayAdapter<String> adapter;
    List<Object> Array = new ArrayList<Object>();
    Button movePwd;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_rec);
        doorRecord = findViewById(R.id.doorRecord);
        initDatabase();

        adapter = new ArrayAdapter<String>(this, R.layout.custom_list, new ArrayList<String>());
        doorRecord.setAdapter(adapter);

        mReference = mDatabase.getReference("rec_list"); // 변경값을 확인할 child 이름
        mReference.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                adapter.clear();
                for (DataSnapshot messageData : dataSnapshot.getChildren()) {
                    String msg2 = messageData.getValue().toString();
                    Array.add(msg2);
                    adapter.add(msg2);
                }
                adapter.notifyDataSetChanged();
                doorRecord.setSelection(adapter.getCount() - 1);
            }

            @Override
            public void onCancelled(DatabaseError databaseError) {

            }
        });

        // 뒤로가기 관련
        movePwd = findViewById(R.id.movePwd);
        movePwd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) { onBackPressed(); }
        });
    }

    private void initDatabase() {

        mDatabase = FirebaseDatabase.getInstance();

        mReference = mDatabase.getReference("log");
        mReference.child("log").setValue("check");

        mChild = new ChildEventListener() {

            @Override
            public void onChildAdded(DataSnapshot dataSnapshot, String s) {
            }

            @Override
            public void onChildChanged(DataSnapshot dataSnapshot, String s) {
            }

            @Override
            public void onChildRemoved(DataSnapshot dataSnapshot) {
            }

            @Override
            public void onChildMoved(DataSnapshot dataSnapshot, String s) {
            }

            @Override
            public void onCancelled(DatabaseError databaseError) {
            }
        };
        mReference.addChildEventListener(mChild);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mReference.removeEventListener(mChild);
    }
}

