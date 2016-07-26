#! /usr/bin/python
#import system libraries
import time
import sys
import os
import subprocess
import pygame
import pygame.camera
from pygame.locals import *

pygame.init()
pygame.camera.init()

#cameras = pygame.camera.list_cameras()
#print cameras

# set up a camera object
cam = pygame.camera.Camera('/dev/video0',(720,480))
# start the camera
cam.start()
image = cam.get_image()

pygame.image.save(image,'image.png')

#run image processing bash command on images in the same folder
subprocess.call('convert -rotate 180 -crop 560x115+25+95 -brightness-contrast 10x100 -set option:modulate:colorspace hsb -modulate 100,0 -threshold 40 image.png imagefix.png', shell=True)

#get the name of the last image processed (should be just one hopefully moving forward)
#filelist = os.listdir(os.getcwd())

x = 0 

for file in os.listdir(os.getcwd()):
    if file.endswith("fix.png"):
    #create string of bash command
        namefile = str(x) 
        
        stringtess = "/usr/bin/sudo tesseract " + file + " " + namefile + " -psm 8 digits"
        
        #print stringtess
    #run bash command tesseract on the image, creating a txt file
        subprocess.call(stringtess, shell=True)

    #load the text file
        namefilet = namefile + '.txt'
        
        textfile =  open(namefilet, "r+")
 
    #parse the text file into a string and remove the . character
        numberArray = textfile.read().split('.')

        #rework this to be more universal
        if len(numberArray) > 1:
            num1 = numberArray[0].strip()
            num2 = numberArray[1].strip()

    #either convert into a number and multiply by 10, or add a 0 to the end and then make a number
            numTotal = num1 + num2 + "0"

        else: 
            numTotal = numberArray[0].strip()
#send this number to the console
        print numTotal
            
        target = open("final.txt", "r+")
        target.write(numTotal) 

target.close()
#try:

#finally:

print "success"
