package com.example.lockhome;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.wang.avi.AVLoadingIndicatorView;

import static com.example.lockhome.PairingActivity.btLockSet;
import static com.example.lockhome.PairingActivity.btLockTime;
import static com.example.lockhome.PairingActivity.btPassword;

public class PwdActivity extends AppCompatActivity {
    private FirebaseDatabase firebaseDatabase = FirebaseDatabase.getInstance();
    private DatabaseReference databaseReference = firebaseDatabase.getReference();

    private AVLoadingIndicatorView avi;

    Handler handler = new Handler();
    TextView password;
    Button moveRec;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pwd);

        String indicator=getIntent().getStringExtra("indicator");
        avi = (AVLoadingIndicatorView) findViewById(R.id.avi);
        avi.setIndicator(indicator);

        // 쓰레드 시작
        BackgroundThread thread = new BackgroundThread();
        thread.start();

        password = findViewById(R.id.password);
        password.setText("동기화 중..."); // 패스워드 받아올 때까지 대기

        // 액티비티 전환
        moveRec = findViewById(R.id.moveRec);
        moveRec.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(PwdActivity.this, RecActivity.class);
                startActivity(intent);
            }
        });
    }

    class BackgroundThread extends Thread {
        public void run() {
            while (true) {
                if ((TextUtils.isEmpty(btLockTime)) == false) { // 데이터가 들어오면 데이터베이스에 전송 후 다시 값 초기화
                    databaseReference.child("rec_list").push().setValue(btLockTime + "  " + btLockSet);
                    btLockTime = null;
                    btLockSet = null;
                }
                try {
                    Thread.sleep(1000);
                } catch (Exception e) { }
                handler.post(new Runnable() {
                    public void run() {
                        if ((TextUtils.isEmpty(btPassword)) == false) { // 데이터가 들어오면 비밀번호 표시
                            password.setText(btPassword);
                            btPassword = null;
                        }
                    }
                });
            }
        }
    }

    // 뒤로가기 2번 클릭 시 종료
    private long lastTimeBackPressed;
    @Override
    public void onBackPressed()
    {
        if (System.currentTimeMillis() - lastTimeBackPressed < 2000)
        {
            ActivityCompat.finishAffinity(this);
            return;
        }
        Toast.makeText(this, "'뒤로' 버튼 한번 더 누르시면 종료됩니다.", Toast.LENGTH_SHORT).show();
        lastTimeBackPressed = System.currentTimeMillis();
    }
}
