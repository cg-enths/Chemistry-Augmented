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

extern "C"
{
    JNIEXPORT jboolean JNICALL Java_com_danim_chemistryaugmented_MainActivity_nativeProjectPoints(
          JNIEnv *env
        , jclass
        , jlong rgba
        , jlong objp
        , jlong mtx
        , jlong dist
        , jlong rvecs
        , jlong tvecs
        , jlong output)
    {
        Mat gray;
        Mat * rgbaResized = (Mat*) output;
        vector<Point2f> corners;
        Size patternSize(CHESSBOARD_ROWS, CHESSBOARD_COLUMNS);

        resize(*(Mat*)rgba, *rgbaResized, Size(), .5f, .5f, INTER_NEAREST);

        cvtColor(*rgbaResized, gray, CV_RGBA2GRAY);
        if (findChessboardCorners(gray, patternSize, corners, CALIB_CB_FAST_CHECK))
        {
            /*cornerSubPix( *gray, corners, Size( 11,11 ), Size( -1,-1 )
                    , TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1) );*/

            solvePnPRansac(*(Mat*)objp, corners, *(Mat*)mtx, *(Mat*)dist, *(Mat*)rvecs, *(Mat*)tvecs);

            //projectPoints(*(Mat*)coord, rvecs, tvecs, *(Mat*)mtx, *(Mat*)dist, *(Mat*)imgpts);

            drawChessboardCorners(*rgbaResized, patternSize, Mat(corners), true);

            return static_cast<jboolean>(true);
        }

        return static_cast<jboolean>(false);
    }

    JNIEXPORT void JNICALL Java_com_danim_chemistryaugmented_MainActivity_nativeDrawLineCoord(JNIEnv *env
            , jclass
            , jlong rgba
            , jdouble x1, jdouble y1
            , jdouble x2, jdouble y2 )
    {
        line(*(Mat*)rgba, Point2f(
                static_cast<float>(x1), static_cast<float>(y1)), Point2f(static_cast<float>(x2), static_cast<float>(y2)), Scalar(255, 255, 0), 3);
    }
}