dc2uddf
=======
Copyright (c) 2012 [Patrick Wagstrom][pwagstrom]

This is a simple tool that uses [libdivecomputer][libdc] to connect a dive computer
and download all the dive information into a [UDDF][uddf] file. This information can
then be imported into a variety of different tools.

The impetus for this tool was because I have a Mac and dive with a Uwatec
Luna dive computer. This is a great computer, but it requires IR to sync dives
which doesn't work natively on the Mac. I can hook up a USB&rarr;IR dongle to my
Mac and run this program in a virtual machine and then import the results
into a variety of other diving programs.

The eventual goal is to use this software on a Raspberry Pi to create a
portable device for downloading dive log data that then automatically
syncs the data to a web service or other interface.

License
=======

This software is licensed under the [Apache License, Version 2.0][license]

Compilation Requirements
========================

* [libdivecomputer][libdc] - the magical library that interfaces with nearly every dive computer
* [glib][glib] - a utility library with many useful data structures and methods for C programming
* [libxml][libxml] - XML serialization and deserialization library
* [check][check] - unit testing for C programs

On a Mac all of these except libdivecomputer can be installed using [homebrew][homebrew] with the following command:

    brew install check libxml2 glib

On Ubuntu you can install all of these except libdivecomputer with the following command:

    apt-get install check libxml2-dev libglib2.0-dev
    	
For all platforms you'll probably need to download and install libdivecomputer on your own. This can usually be done with the following commands:

    wget http://www.divesoftware.org/libdc/releases/libdivecomputer-0.2.0.tar.gz
    tar -zxvf libdivecomputer-0.2.0.tar.gz
    cd libdivecomputer
    ./configure
    make
    sudo make install

Running the Program
===================

I only have a single dive computer, the Uwatec Galileo Luna. In order to get everything set up on my Linux virtual machine I first need to plug in the USB&rarr;IR dongle and then run the following commands to setup the IrDA subsystem:

    sudo modprobe ircomm
    sudo modprobe irda_usb
    sudo irattach irda0 -s

Now you should be able to download the data from your computer using the following command:

    dc2uddf -b smart -d "Uwatec Galileo"
    
The general format is:

    dc2uddf -b [backend name] -d [device name] [
    
You can get a list of the backend and device names by running `universal` in the examples directory of libdive computer. At some point in the future I'll update dc2uddf to list all of the possible devices. However, as I only have the Uwatec Galileo Smart, it is the only device that is tested.

[license]: http://www.apache.org/licenses/LICENSE-2.0.html
[libdc]: http://www.divesoftware.org/libdc/
[uddf]: http://www.streit.cc/extern/uddf_v310/en/index.html
[pwagstrom]: http://patrick.wagstrom.net/
[glib]: http://developer.gnome.org/glib/
[check]: http://check.sf.net/
[libxml]: http://www.xmlsoft.org/
[homebrew]: http://mxcl.github.com/homebrew/