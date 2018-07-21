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

using namespace std;
using namespace cv;

const int CHESSBOARD_COLUMNS = 6;
const int CHESSBOARD_ROWS    = 8;

const float RESIZING_RATIO   = 4.0f;

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
        Mat rgba_clone = ( *( Mat * )rgba ).clone();
        Mat gray;
        Mat grayResized;
        Mat rgbaResized;

        vector< Point2f > corners;
        Size patternSize( CHESSBOARD_ROWS, CHESSBOARD_COLUMNS );

        // Without rescaling, performance is very poor. So, the image is going to get
        // resized 75% -> 10FPS
        resize( *( Mat * )rgba, rgbaResized, Size( ), (1 / RESIZING_RATIO), (1 / RESIZING_RATIO), INTER_NEAREST );

        // Convert them to gray scale.
        cvtColor( rgbaResized, grayResized, CV_RGBA2GRAY );
        cvtColor( rgba_clone, gray, CV_RGBA2GRAY );

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

            drawChessboardCorners(*( Mat * )rgba, patternSize, Mat( corners ), true );

            return static_cast< jboolean >( true );
        }

        return static_cast< jboolean >( false );
    }
}