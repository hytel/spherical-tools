### Spherical Tools for Spherical Cameras

This small repos contains utilities and web related files that I have created for processing and sharing my 360 degree still and video imagery on the web.  Feel free to make comments and suggestions on this repo, but the main reason I'm putting this out there is because others may find this small growing collection useful.  I currently have many ongoing projects and may take some time to get back to you.

The "utility" folder contains the "image_to_cubemap" C++ code which can turn an equirectangular DNG or JPG image from spherical cameras (e.g., an Insta360 X5) and produce a .DDS cubemap image in the same folder (as well as a PNG).  The cubemap files can then be displayed on the web using the files in the "www" subfolder.  Many games can also load DDS cubemaps as skyboxes.  Change the contents of the "cubemaps.txt" file in the 'www' folder to specify what to name each cubemap, and where the converted DDS files are stored so they can be loaded in your browser's page.  This is done mostly via Javascript using three.js and some custom DDS (Direct Draw Surface) loading code as well as tailwind for CSS stylization.

Here's a quick example.  If you have a raw equirectangular image as a cubemap_one.dng from your spherical camera like:

![alt text](docs/source_image.jpg?raw=true "Source Equirectangular Image From Camera")

Then you can build the "image_to_cubemap" utility in Linux or MacOS (using brew) by using CMake.  You must have the Qt6 (image editing and saving) and libraw (to read DNG files) libraries install which are easily available on MacOS (brew) and Linux.  You can compile the utility in the standard way like this:

```
cd utility
mkdir build
cd build
cmake ..
make
```

If you place the source DNG file in the utility/build folder, you can run the newly compiled utility (should be in the utility folder):

./image_to_cubemap ./cubemap_one.dng

This will create a cubemap_one.png and cubemap_one.dds in the same folder as the source DNG file.  The output of the utility may look something like:


```
Extension: 'dng'
DDS: '../cubemap_one.dds'
PNG: '../cubemap_one.png'
Edge length in pixels: 1488
Output image dimensions: 5952x4464
Processing 0 of 5952
Processing 1000 of 5952
Processing 2000 of 5952
Processing 3000 of 5952
Processing 4000 of 5952
Processing 5000 of 5952
Saving Cubemap to PNG: ../cubemap_one.png
Saved Cubemap to PNG: ../cubemap_one.png
Saved Cubemap to DDS: ../cubemap_one.dds
```

You should now have a cubemap_one.png that looks something like this:

![alt text](docs/cubemap_png.jpg?raw=true "Converted Cubemap PNG")

This is the "unfolded" image which looks essentially like you would imagine: six images for each side of an unfolded cube.  The unfolded format can be used by some applications directly, but it also is useful as a format that you can edit before finally converting it to a DDS file for use in a runtime environment.  If you do edit the unfolded cubemap image, image_to_cubemap can also convert the edited unfolded cubemap image directly into a DDS file with the -u option:

./image_to_cubemap -u ./cubemap_one_editted.png

When using the image_to_cubemap utility with the -u flag, the source image is expected to be an unfolded cubemap image, not an equirectangular stitched photo.  The only output is then a DDS runtime cubemap file with the same file path/prefix.  Here's what the edited unfolded image without the photographer might looks like:

![alt text](docs/cubemap_edited.jpg?raw=true "Edited unfolded cubemap image")

The DDS file is harder to see directly (gimp will load it) and is more like a stack of six layers in image editors.  To see a DDS cubemap in the browser, edit the cubemaps.txt file in the "www" folder and add an entry like:

My First Cubemap=cubemaps/cubemap_one.dds

Then put the cubemap_one.dds from the utility/build folder in the www/cubemaps folder.  You can start a test server using the 'start_http_server' script in the 'www' folder.  Then point your browser at:

http://localhost:8080/index.html

You should see your cubemap come up.  it should look something like:

![alt text](docs/cubemap_web.jpg?raw=true "Cubemap DDS Web Viewer")

You can use the mouse to spin the view around.  There is also a symbol in the bottom right of the page which will show the controls so you can pick a different cubemap from your selection in the cubemaps.txt file.  The controls do come up for a split second when the page is first loaded, but I have the HTML automatically select the first cubemap which causes the controls to slowly hide until the button is pressed.

For 360 degree spherical video and still image reframing, I wrote a Qt application called SphericalQt.  OpenCV is also required by the application for image processing.  It is located in the SphericalQt subfolder, and can be built by CMake as follows:

```
cd SphericalQt
mkdir build
cd build
cmake ..
make
```

The application should be built to the SphericalQt folder.  On MacOS, it will be a SphericalQt.app folder (like a normal Mac Application), or just a SphericalQt binary on Linux.  It will load some LUT files during runtime either from the luts sub-folder, or a duplicate copy of the luts folder in the Resources folder for the app on MacOS.   The purpose of this application is to take equirectangular 360 images or mp4 stitched videos and create new reframed images or videos with user input.  It is a quirky app but quite powerful, and uses a non-standard approach.  You can drag-n-drop a 360 still or video onto the viewer panel that comes up.  It will look something like:

![alt text](./docs/spherical_qt_main.jpg?raw=true "Spherical Qt Application")

Using the mouse, you can reframe it by click dragging horizontally or vertically.  If it's a video, it will begin playing back immediately.  Many keys can control the application.  You can press the spacebar to toggle a video's playback.  Pressing the 'q' key will quit the application.  The 'w' key will toggle writting out a reframed video.  By pressing the 's' key for settings, a panel will come up with many sliders that you can use to adjust the video and settings.  It looks like this:

![alt text](./docs/spherical_qt_settings.jpg?raw=true "Spherical Qt Settings Panel")

The final slider allows the user to advance and rewind the playback.  Although no audio is played back, during reframing writes, the audio is written.  Even though the playback isn't realtime, the written file will playback at normal speed, with correct audio as well.  The user can even change the settings and the view reframe while Spherical Qt is writing out the video.  A short summary of some of the keys available in the viewer window are:

```
Spacebar   - Toggle play/pause
1          - Animate heading
2          - Animate pitch
3          - Animate roll
c          - Next color Filter
C          - Previous color Filter
n          - Single frame advance on pause
q          - Quit application
s          - Show settings panel
arrow keys - horizontal/vertical reframing
```

Enjoy!
