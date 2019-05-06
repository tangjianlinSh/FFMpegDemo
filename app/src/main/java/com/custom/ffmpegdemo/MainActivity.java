package com.custom.ffmpegdemo;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import permissions.dispatcher.NeedsPermission;
import permissions.dispatcher.OnNeverAskAgain;
import permissions.dispatcher.OnPermissionDenied;
import permissions.dispatcher.RuntimePermissions;

import android.Manifest;
import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.custom.ffmpegdemo.listener.OnCompleteListener;
import com.custom.ffmpegdemo.listener.OnInfoListener;
import com.custom.ffmpegdemo.model.TimeBean;
import com.custom.ffmpegdemo.utils.MyLog;
import com.custom.ffmpegdemo.utils.TimeUtils;
import com.custom.ffmpegdemo.widget.VideoView;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Calendar;

@RuntimePermissions
public class MainActivity extends AppCompatActivity {

    TextView textView;

    VideoView videoView;

    FFMpegUtil ffMpegUtil;

    boolean isPause = false;

    SeekBar mSeekBar;

    TextView tvDuration;

    private boolean isSeek = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        textView = findViewById(R.id.text);
        videoView = findViewById(R.id.my_video_view);
        mSeekBar = findViewById(R.id.seek_bar);
        tvDuration = findViewById(R.id.tv_duration);
        initFFmpegUtil();
        initBtnListener();
    }

    private void initFFmpegUtil() {
        ffMpegUtil = new FFMpegUtil();
        ffMpegUtil.setSurface(videoView);
        ffMpegUtil.setOnInfoListener(new OnInfoListener() {
            @Override
            public void onInfo(TimeBean timeBean) {
                MyLog.d("on info1");
                Message msg = new Message();
                msg.what = 1;
                msg.obj = timeBean;
                handler.sendMessage(msg);
            }
        });
        ffMpegUtil.setOnCompleteListener(new OnCompleteListener() {
            @Override
            public void complete() {
                Message msg = new Message();
                msg.what = 2;
                handler.sendMessage(msg);
            }
        });
    }

    private void initBtnListener() {
        findViewById(R.id.btn).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //播放
                MainActivityPermissionsDispatcher.playWithPermissionCheck(MainActivity.this);
                v.setEnabled(false);
            }
        });
        findViewById(R.id.btn1).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //暂停
                ffMpegUtil.stop();
                isPause = !isPause;
                ((Button) v).setText(isPause ? "继续" : "暂停");
            }
        });
        findViewById(R.id.btn3).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //停止
                ffMpegUtil.release();
                findViewById(R.id.btn).setEnabled(true);
            }
        });
        mSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                //进度改变
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                //开始拖动
                isSeek = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                //停止拖动
                isSeek = false;
                ffMpegUtil.seekTo(seekBar.getProgress() * 1000);
            }
        });
    }

    @NeedsPermission({Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE})
    void play() {
        File file = new File(Environment.getExternalStorageDirectory().getAbsolutePath().concat("/Download"), "test1.mp4");
        if (file.exists()) {
            Toast.makeText(this, "文件存在", Toast.LENGTH_SHORT).show();
            ffMpegUtil.play(file.getAbsolutePath());
            if (ffMpegUtil.getTotalTime() != 0) {
                tvDuration.setText("00/" + formatTime(ffMpegUtil.getTotalTime() / 1000));
                MyLog.d("test - " + ffMpegUtil.getTotalTime() / 1000000);
                mSeekBar.setMax(ffMpegUtil.getTotalTime() / 1000000);
                mSeekBar.setProgress(0);
            }
        } else {
            //文件不存在
            Toast.makeText(this, "文件不存在", Toast.LENGTH_SHORT).show();
        }
    }

    void change() {
        MyLog.d("切换视频");
        ffMpegUtil.release();
        ffMpegUtil.play(new File(Environment.getExternalStorageDirectory().getAbsolutePath().concat("/Download"), "test.mp4").getAbsolutePath());
        if (ffMpegUtil.getTotalTime() != 0) {
            tvDuration.setText("00/" + formatTime(ffMpegUtil.getTotalTime() / 1000));
            mSeekBar.setMax(ffMpegUtil.getTotalTime() / 1000000);
            mSeekBar.setProgress(0);
        }
    }

    @OnPermissionDenied(Manifest.permission.CAMERA)
    void playDenied() {
        Toast.makeText(this, "没有读写权限", Toast.LENGTH_SHORT).show();
    }

    @OnNeverAskAgain(Manifest.permission.CAMERA)
    void playNeverAskAgain() {
        Toast.makeText(this, "没有读写权限", Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        MainActivityPermissionsDispatcher.onRequestPermissionsResult(this, requestCode, grantResults);
    }

    /**
     * 将时间戳转换成字符串时间
     *
     * @param time
     * @return
     */
    private String formatTime(long time) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(time);
        String result = "";
        int hour = calendar.get(Calendar.HOUR_OF_DAY);//24小时制
        if (hour < 10) {
            result = result.concat("0" + hour + ":");
        } else {
            result = result.concat(String.valueOf(hour) + ":");
        }
        int minute = calendar.get(Calendar.MINUTE);
        if (minute < 10) {
            result = result.concat("0" + minute + ":");
        } else {
            result = result.concat(String.valueOf(minute) + ":");
        }
        int second = calendar.get(Calendar.SECOND);
        if (second < 10) {
            result = result.concat("0" + second);
        } else {
            result = result.concat(String.valueOf(second));
        }
        return result;
    }

    @SuppressLint("HandlerLeak")
    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case 1:
                    TimeBean timeBean = (TimeBean) msg.obj;
                    if (!isSeek) {
                        mSeekBar.setProgress(timeBean.getCurrentDuration());
                    }
                    tvDuration.setText(TimeUtils.secToTime(timeBean.getCurrentDuration()) + "/" + formatTime(timeBean.getTotalDuration() / 1000));
                    break;
                case 2:
                    change();
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (null != ffMpegUtil) {
            ffMpegUtil.release();
            ffMpegUtil = null;
        }
    }
}
