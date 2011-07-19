This is a skeleton android app that has bare-bones android/qualcomm augmented reality setup and ready to run. Think of it as scaffolding with:

Android API Level 3 (2.1 update 1 Eclair)
Android NDK
OpenGL ES 2.0
Qualcomm Augmented Reality 2.0 (w/ built in 3D model)
Ant build script
[Screen shot here]

To install:

* clone to your local dir
* Choose some image and go to ar.qualcomm.com and create a target 
* Get the xml and .dat file from previous step and put in assets dir

To Build the app you need apache ant or it may build in Eclipse too:

* ndk-build
* ant install (with phone plugged in on USB)

http://eggie5.com/