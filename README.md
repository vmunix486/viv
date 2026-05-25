# viv
vmunix's image viewer, using stb

# Features

 - Can run on new and old computers (Tested on at least Windows XP)
 - Supports lots of image formats (jpeg, png, tga, bmp, psd, static gif, hdr, pic, pnm)
 - Double buffering to prevent flashing
 - Very small (~350 lines of code, ~290 SLOC)

# Compilation Guide

Make a new project in Microsoft Visual C++ 2008 (newer versions might work, but I have not tried them), then just drag `main.cpp` and `stb_image.h` into the solution and select to compile. Make sure that the files are actually in the solution folder first before filling out a bug report.

# Usage

If opened, it just shows a default file picker and when the file is picked, it displays it. If you want to load another picture, then just press the L key and choose another picture to load. If you drag a picture to the executable in something like explorer, it will start with that picture loaded already, same with if you set this as your primary image viewer or use the command line itself.
```
viv.exe <path to image>
```

# TODO

 - Fix the name of the Window being garbled garbage
 - Add animated gif support

# Contributing

You can contribute by forking this repository and making your modifications. Your modifications has to be in C++ that is compatible with MSVC++ 2008, and is compatible with at least Windows XP. When you are wanting to merge, just explain all your modifications, and also display a picture or video of the change.
