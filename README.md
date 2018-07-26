# Chemistry Augmented

## Purpose

The purpose of this project is to create an Android application that will detect some patterns drawn in some wood cards representing
different chemical elements.

Once identified, the element will be 3D rendered on top of the card.

Next step will be trying to make interactable.

## Setting up the NDK / OpenCV

##### OpenCV   

1. **Download** latest OpenCV sdk for Android from [OpenCV.org](https://opencv.org/releases.html) and decompress the zip file.

2. **Import OpenCV to Android Studio**, From _File -> New -> Import Module_, choose _sdk/java_ folder in the unzipped opencv archive.

3. **Update build.gradle** under imported OpenCV module to update 4 fields to match your project build.gradle a) compileSdkVersion b) buildToolsVersion c) minSdkVersion and d) targetSdkVersion.

4. **Add module dependency** by _Application -> Module Settings_, and select the Dependencies tab. Click + icon at bottom, choose Module Dependency and select the imported OpenCV module.
  + For Android Studio v1.2.2, to access to Module Settings : in the project view, right-click the dependent module -> Open Module Settings

5. **Copy libs folder** under _sdk/native_ to Android Studio under _app/src/main_.

6. In Android Studio, **rename the copied _libs_ directory to _jniLibs_** and we are done.

Step (6) is since Android studio expects native libs in _app/src/main/jniLibs_ instead of older _libs_ folder

##### NDK

To configure the NDK refer to the [official guide](https://developer.android.com/studio/projects/add-native-code?hl=es-419).

##### How to use OpenCV in JNI

You will need to change your **app module's** build gradle to reflect the snippet below.

```
defaultConfig {
    ...
    externalNativeBuild {
        cmake {
            cppFlags "-frtti -fexceptions"
            abiFilters 'x86', 'x86_64', 'armeabi', 'armeabi-v7a', 'arm64-v8a', 'mips', 'mips64'
        }
    }
}
sourceSets {
    main {
        jniLibs.srcDirs = ['src/main/jniLibs']
    }
}
```

Then you want to edit the file **CMakeLists.txt**. In this file you want to include the following code underneath the cmake_minimum_required(VERSION X.X.X) line.

Also, you will need to add **lib_opencv** to the **target_link_libraries** near the bottom of CMakeLists.txt. This will help prevent undefined reference errors.

```
include_directories(your-path-to/OpenCV-android-sdk/sdk/native/jni/include)
add_library( lib_opencv SHARED IMPORTED )
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libopencv_java3.so)

target_link_libraries( # Specifies the target library.
                   native-lib

                   # OpenCV lib
                   lib_opencv

                   # Links the target library to the log library
                   # included in the NDK.
                   ${log-lib} )
```

Make sure to replace your-path-to with your actual path to OpenCV for Android.

Finally, clean your project and refresh your linked C++ projects.

##### OpenCV in Python 3

To import OpenCV you must install `numpy` and, obviously, OpenCV for Python. To do so, just run these commands:

+ `pip install numpy`

+ `pip install opencv-python`

## Calibrate the camera

As every camera creates some distortion, a script has to be run to get the distortion coefficients.

Ten or more photos of the chessboard have to be taken in order to calibrate the camera. Then, the script `calibrate.py` must be run, modifying the path to the pictures and the number of corners that form the chessboard.

The results for my specific camera are:

```
Camera matrix

[[ 517.65350405   0.           319.06418667]
[  0.             518.2757208  238.78380146]
[  0.             0.           1.          ]]
```

```
Distortion coefficients

[ 2.09547937e-01 -1.21926310e+00 -1.29976649e-03 -2.52504602e-03 2.26952234e+00 ]
```

---

###### Links
+ [OpenCV in Android Studio](https://stackoverflow.com/questions/27406303/opencv-in-android-studio/27421494#27421494)
+ [OpenCV and JNI](https://stackoverflow.com/questions/49244078/cannot-find-opencv2-in-android-studio)
+ [NDK Setup](https://developer.android.com/studio/projects/add-native-code?hl=es-419)
+ [OpenCV shape detection](https://www.pyimagesearch.com/2016/02/08/opencv-shape-detection/)
