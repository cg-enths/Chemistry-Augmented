/*
 * native-lib.cpp
 *
 *  Created on: 19/07/2018
 *      Author: Daniel Mancebo
 */
#include <jni.h>

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <android/log.h>

using namespace std;
using namespace cv;

/* Constants */
const int CHESSBOARD_COLUMNS = 6;
const int CHESSBOARD_ROWS    = 8;

const float RESIZING_RATIO   = 2.0f;

const char * TAG = "CHEMISTRY_AUGMENTED";

/* Data */
Mat rgbaResized;
Mat grayResized;

vector< Vec4i > lines;

extern "C"
{
    JNIEXPORT jboolean JNICALL Java_com_danim_chemistryaugmented_MainActivity_nativeDetectCheckerboard(
          JNIEnv * env
        , jclass
        , jlong rgba
        , jlong objp
        , jlong mtx
        , jlong dist
        , jlong rvecs
        , jlong tvecs )
    {
        Mat rgbaClone = ( *( Mat * )rgba ).clone();
        Mat gray;
        Mat grayResized;
        Mat rgbaResized;

        vector< Point2f > corners;
        Size patternSize( CHESSBOARD_ROWS, CHESSBOARD_COLUMNS );

        // Without rescaling, performance is very poor. So, the image is going to get
        // resized 75% -> 10FPS
        resize( *( Mat * )rgba, rgbaResized, Size( ), ( 1 / RESIZING_RATIO ), ( 1 / RESIZING_RATIO ), INTER_NEAREST );

        // Convert them to gray scale.
        cvtColor( rgbaResized, grayResized, CV_RGBA2GRAY );
        cvtColor( rgbaClone, gray, CV_RGBA2GRAY );

        // Find the checkerboard using the low resolution image.
        if ( findChessboardCorners( grayResized, patternSize, corners, CALIB_CB_FAST_CHECK ) )
        {
            // Scale up the corners, as they're going to get refined using the high resolution image.
            for ( vector< Point2f >::iterator it = corners.begin( ); it != corners.end( ); it++ )
            {
                *it *= RESIZING_RATIO;
            }

            // Refine the corners.
            cornerSubPix( gray, corners, Size( 11, 11 ), Size( -1, -1 )
                    , TermCriteria( CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1 ) );

            // Calculate the rotation and translation of the card.
            solvePnPRansac( *( Mat * )objp, corners, *( Mat * )mtx, *( Mat * )dist, *( Mat * )rvecs, * ( Mat * )tvecs );

            drawChessboardCorners( *( Mat * )rgba, patternSize, Mat( corners ), true );

            return static_cast< jboolean >( true );
        }

        return static_cast< jboolean >( false );
    }

    JNIEXPORT jboolean JNICALL Java_com_danim_chemistryaugmented_MainActivity_nativeDetectContour(
          JNIEnv * env
        , jclass
        , jlong rgba)
    {
        Mat cannied;
        Mat thresh;

        // Resize it to a smaller factor so that the shapes can be approximated better
        resize( *( Mat * )rgba, rgbaResized, Size( ), ( 1 / RESIZING_RATIO ), ( 1 / RESIZING_RATIO ), INTER_NEAREST );

        __android_log_write( ANDROID_LOG_DEBUG, TAG, "Resizing complete" );

        Canny( rgbaResized, cannied, 50, 200, 3 );

        __android_log_write( ANDROID_LOG_DEBUG, TAG, "Canny complete" );

        cvtColor( cannied, grayResized, CV_GRAY2BGR );

        __android_log_write( ANDROID_LOG_DEBUG, TAG, "BW complete" );

        HoughLinesP( cannied, lines, 1, CV_PI / 180, 50, 50, 10 );

        for ( vector< Vec4i >::iterator it = lines.begin(); it != lines.end(); it++ )
        {
            *it *= RESIZING_RATIO;
        }

        __android_log_write( ANDROID_LOG_DEBUG, TAG, "Hough lines complete" );

        for ( vector< Vec4i >::iterator it = lines.begin( ); it != lines.end( ); it++ )
        {
            line( *( Mat * )rgba, Point( ( *it )[ 0 ], ( *it )[ 1 ]), Point( ( *it )[ 2 ], ( *it )[ 3 ] ), Scalar( 0, 0, 255 ), 3, CV_AA );
        }

        return static_cast< jboolean >( true );
    }
}