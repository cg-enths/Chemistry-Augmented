package com.danim.chemistryaugmented;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.SurfaceView;
import android.widget.Toast;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.MatOfDouble;
import org.opencv.core.MatOfPoint2f;
import org.opencv.core.MatOfPoint3f;
import org.opencv.core.Point3;
import org.opencv.core.Size;

@SuppressWarnings("JniMissingFunction")
public class MainActivity extends Activity implements CameraBridgeViewBase.CvCameraViewListener2
{
    /* Constants */
    private static final String TAG = "CHEMISTRY_AUGMENTED";

    private static final int REQUEST_CODE_ASK_PERMISSIONS = 0;

    private static final int CHESSBOARD_COLUMNS = 6;
    private static final int CHESSBOARD_ROWS    = 8;

    /* GUI Elements */
    private CameraBridgeViewBase mOpenCvCameraView;

    /* Data */
    private static boolean mReady;

    private static Size       mPatternSize = null;
    private static MatOfPoint3f      mObjp = null;
    private static MatOfPoint2f    mImgpts = null;
    private static Mat              mTvecs = null;
    private static Mat              mRvecs = null;

    // Camera parameters (intrinsec parameters and distortion coefficients)
    private static Mat      mCamMatrix = null;
    private static MatOfDouble   mDist = null;

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this)
    {
        @Override
        public void onManagerConnected(int status)
        {
            switch (status)
            {
                case LoaderCallbackInterface.SUCCESS:
                    Log.i(TAG, "OpenCV loaded successfully");
                    mOpenCvCameraView.enableView();

                    System.loadLibrary("native-lib");

                    new InitializationTask().execute();
                    break;

                default:
                    super.onManagerConnected(status);
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        requestPermission();

        mOpenCvCameraView = findViewById(R.id.camera_frame_layout);
        mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);

        mOpenCvCameraView.setCvCameraViewListener(this);
    }

    @Override
    public void onPause()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
        {
            mOpenCvCameraView.disableView();
        }
    }

    @Override
    public void onResume()
    {
        super.onResume();
        if (!OpenCVLoader.initDebug())
        {
            Log.i(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, mLoaderCallback);
        }
        else
        {
            Log.i(TAG, "OpenCV library found inside package. Using it!");
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
        if (mOpenCvCameraView != null)
        {
            mOpenCvCameraView.disableView();
        }
    }

    @Override
    public void onCameraViewStarted(int width, int height)
    {
    }

    @Override
    public void onCameraViewStopped()
    {
        mCamMatrix.release();
        mDist.release();
        mImgpts.release();
        mObjp.release();
        mRvecs.release();
        mTvecs.release();
    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame)
    {
        Mat rgba = inputFrame.rgba();

        if (mReady)
        {
            if (nativeDetectCheckerboard(
                  rgba.getNativeObjAddr()
                , mObjp.getNativeObjAddr()
                , mCamMatrix.getNativeObjAddr()
                , mDist.getNativeObjAddr()
                , mRvecs.getNativeObjAddr()
                , mRvecs.getNativeObjAddr()))
            {
                Log.i(TAG, "Corners detected");
            }
        }

        return rgba;
    }

    /**
     * Native function which attempts to detect the checkerboard.
     * @param rgba: input image.
     * @param objp: pattern matrix.
     * @param mtx: camera intrinsec parameters matrix.
     * @param dist: distortion coefficient matrix.
     * @param rvecs: rotation matrix.
     * @param tvecs: translation matrix.
     * @return true if detected.
     */
    private static native boolean nativeDetectCheckerboard(
        long rgba, long objp, long mtx, long dist, long rvecs, long tvecs);

    /**
     * Background process which initializes all the structs and launches the checkerboard
     * detection once finished.
     */
    private static class InitializationTask extends AsyncTask<Void, Void, Integer>
    {
        @Override
        protected void onPreExecute()
        {
            Log.i(TAG, "Initializing");
            mReady = false;
        }

        @Override
        protected Integer doInBackground(Void... voids)
        {
            mPatternSize = new Size(CHESSBOARD_ROWS, CHESSBOARD_COLUMNS);

            mObjp    = new MatOfPoint3f(_initPoint3Vector());
            mImgpts  = new MatOfPoint2f();

            mTvecs = new Mat();
            mRvecs = new Mat();

            mCamMatrix = new Mat ( 3, 3, CvType.CV_32F );

            mCamMatrix.put(0, 0, 517.65350405f);
            mCamMatrix.put(0, 1,          0.0f);
            mCamMatrix.put(0, 2, 319.06418667f);
            mCamMatrix.put(1, 0,          0.0f);
            mCamMatrix.put(1, 1, 518.2757208f);
            mCamMatrix.put(1, 2, 238.78380146f);
            mCamMatrix.put(2, 0,          0.0f);
            mCamMatrix.put(2, 1,          0.0f);
            mCamMatrix.put(2, 2,          1.0f );

            mDist = new MatOfDouble();

            mDist.put(0 , 0,    0.209547937f);
            mDist.put(0 , 1,    -1.21926310f);
            mDist.put(0 , 2, -0.00129976649f);
            mDist.put(0 , 3,  0.00252504602f);
            mDist.put(0 , 4,     2.26952234f);

            return 0;
        }

        @Override
        protected void onPostExecute(Integer unused)
        {
            Log.i(TAG, "Initialization complete");
            mReady = true;
        }
    }

    /**
     * Method that initilizes the pattern matrix
     * @return pattern matrix initialized.
     */
    private static Point3[] _initPoint3Vector( )
    {
        Point3[] vaux = new Point3[(int) (mPatternSize.height * mPatternSize.width)];

        int index = 0;
        for (int i = 0; i < mPatternSize.height; i++)
        {
            for (int j = 0; j < mPatternSize.width; j++)
            {
                vaux[index++] = new Point3(j, i, 0);
            }
        }

        return vaux;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults)
    {
        switch (requestCode)
        {
            case REQUEST_CODE_ASK_PERMISSIONS:
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED)
                {
                    // Permission Granted
                    Toast.makeText(MainActivity.this, "Permission Granted", Toast.LENGTH_SHORT)
                            .show();
                }
                else
                {
                    // Permission Denied
                    Toast.makeText(MainActivity.this, "Permission Denied", Toast.LENGTH_SHORT)
                            .show();
                }
                break;

            default:
                super.onRequestPermissionsResult(requestCode, permissions, grantResults);
                break;
        }
    }

    private void requestPermission()
    {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED )
        {
            ActivityCompat
                    .requestPermissions(MainActivity.this, new String[]{Manifest.permission.CAMERA}, REQUEST_CODE_ASK_PERMISSIONS);
        }
    }
}
