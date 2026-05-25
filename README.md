# viv
vmunix's image viewer, using stb

# Why

Becuase all the image viewers that were on my Windows XP laptop sucked, and I didn't feel like going out and getting some other bloated image viewer that fried my Pentium M with all these fancy stuff.

# How

Microsoft Visual C++ 2008.

Wait, if you mean how do you compile it, then just make a new project in MSVC++ 2008 and drag in both the `main.cpp` and `stb_image.h` files and compile it.

# Usage

If opened, it just shows a default file picker and when the file is picked, it displays it. If you want to load another picture, then just press the L key and choose another picture to load. If you drag a picture to the executable in something like explorer, it will start with that picture loaded already, same with if you set this as your primary image viewer or use the command line itself.

**WARNING: RESIZING THE WINDOW CAUSES THE IMAGE TO FLASH ON SLOW COMPUTERS. THIS CAN BE DANGEROUS TO PEOPLE WITH EPILEPSY. YOU HAVE BEEN WARNED!!!**

# TODO

 - Add double buffering support to fix flashing on resize
 - Fix the name of the Window being garbled garbage

# Contributing

You can contribute by forking this repository and making your modifications. Your modifications has to be in C++ that is compatible with MSVC++ 2008, and is compatible with at least Windows XP. When you are wanting to merge, just explain all your modifications, and also display a picture or video of the change.
