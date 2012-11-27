Readme file for keyboard
-------------------------
This keyboard started out as the Danzeff keyboard, but was modified by Bluddy for efficiency and reduced memory consumption.

Image Modification
------------------
The .xcf files are GIMP 2.0 files which were used to build the images. From these files, 32-bit PNG files were created (GIMP doesn't support 8 or 4 bit files properly i.e. letter anti-aliasing becomes messed up).
The program pngquant.exe which is part of libpng was then used to reduce the image from 32-bits to 4-bits using the command "pngquant 256 pngname.png".
Finally, for the selection images only (*_s.png), Irfanview (www.irfanview.com) was used to save the images and select the transparent background color. GIMP was not used for this purpose as its default transparent color is black. (Irfanview also doesn't fully support alpha bits for 8 or 4-bit PNG images, but it was useful in setting the background to transparent in this particular case.)
