Python prototype for handling Optical character recognition

This file is setup to run on Raspberry pi as it uses Pygame for the image capture.

It also uses Image Magick (batch image processing) and Tesseract (OCR library maintained by Google)

Both Image Magick: http://www.imagemagick.org/Magick++/tutorial/Magick++_tutorial.pdf 
and Tesseract: https://code.google.com/p/tesseract-ocr/wiki/APIExample
have C++ wrappers available

Image magick command:
-rotate (degree) 
-crop widthXheight+x_offset+y_offset 
-brightness-contrast brightnessPercentXcontrastPercent
-set option:modulate:colorspace hsb -modulate 100,0    (change to HSB, remove all saturation)
-threshold threshnumber       (this one changes everything to black or white, I found it best to change the contrast and reduce the saturation first for better results

Tesseract command:
Tesseract nameofimage toSaveFilename(no extension needed) -psm 8 digits

psm sets the mode, in this case 8 indicates one line of text (since Tesseract is used for full pages
digits simplifies the library (instead of looking for english etc)

Linux system calls are made using the python subprocess (image magick, tesseract)

OCR.php and the accompanying style1.css place the initial webcam image, the modified image, and the 
result of some string processing into a single webpage to make it easier to view remote from the Pi

There is also a live camera feed option on raspberry pi installed. This is started by sudo service motion start, but must be stopped since only one thing can control the camera at a time. The live feed for this camera can been seen on the local network at localip:8081
