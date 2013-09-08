dc2uddf
=======
Copyright (c) 2012-2013 [Patrick Wagstrom][pwagstrom]

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

Compilation Prerequisites
=========================

* [libdivecomputer][libdc] - the magical library that interfaces with nearly every dive computer
* [glib][glib] - a utility library with many useful data structures and methods for C programming
* [libxml][libxml] - XML serialization and deserialization library
* [check][check] - unit testing for C programs

On a Mac all of these except libdivecomputer can be installed using [homebrew][homebrew] with the following command:

    brew install check libxml2 glib

On Ubuntu you can install all of these except libdivecomputer with the following command:

    apt-get install check libxml2-dev libglib2.0-dev irda-utils
    	
For all platforms you'll probably need to download and install libdivecomputer on your own. This can usually be done with the following commands:

    wget http://www.divesoftware.org/libdc/releases/libdivecomputer-0.2.0.tar.gz
    tar -zxvf libdivecomputer-0.2.0.tar.gz
    cd libdivecomputer
    ./configure
    make
    sudo make install

Compiling the Program
=====================

This step varies a little bit depending on how you got the software. If you obtained dc2uddf as a tarball release compliation is easy and straightforward:

    ./configure
    make
    make install

If you are running off the git version of the software then you'll need to run a few other commands first.

    automake --add-missing
    cp README.md README
    autoreconf
    ./configure
    make
    make install


Running the Program
===================

I only have a single dive computer, the Uwatec Galileo Luna. There are two ways that I can get everything up and running on my Linux Virtual Machine - either by hand when I boot the machine, or by changing some settings so IrDA starts at boot. I'll describe both here:

Starting IrDA by Hand
---------------------

This is a fine way to handle things if you're testing the software. First, make sure to plug in your USB&rarr;IR dongle and then run the following commands to setup the IrDA subsystem:

    sudo modprobe ircomm
    sudo modprobe irda_usb
    sudo modprobe ircomm-tty
    sudo irattach irda0 -s

That should be all that you need to do.

Starting IrDA at Boot
---------------------

There's a two step process that you'll need to get this working at boot. First, you'll need to edit `/etc/modprobe.d/irda-utils.conf` and add the following line to the end of the file. This is really only needed if you're using the mcs7780 series of devices:

    alias irda0 mcs7780

Next, open up `/etc/default/irda-utils` and search for the lines that describe `ENABLE`, `DEVICE`, and `MAX_BAUD_RATE` and change them as follows:

    ENABLE="true"
    DEVICE="irda0"
    MAX_BAUD_RATE="9600"

Then reboot and your IrDA setup should be running with no issues.

Downloading Some Data
----------------------

As the only computer I have is a Uwatec Galileo Luna, it's the only one that I can describe the process for. Just run the following command and you'll get a file called `output.uddf` with all of the data from the computer.

    dc2uddf -b smart -d "Uwatec Galileo"
    
The general format is:

    dc2uddf -b [backend name] -d [device name]
    
You can get a list of the backend and device names by running `universal` in the examples directory of libdive computer. At some point in the future I'll update dc2uddf to list all of the possible devices. However, as I only have the Uwatec Galileo Smart, it is the only device that is tested.

Additional Arguments
====================

* `-i`, `--ipf`: Initial pressure fix. When first connecting the Luna and some other devices the pressure will read 0. This goes back and sets the initial pressure to the first valid pressure reading.
* `-t`, `--truncate`: Run an algorithm to truncate dives after surfacing. Basically, this stops a dive after you've surfaced if you don't go down below 1m again. This is handy because the Luna typically records an extra five minutes of data at the end of the dive.
* `-o`, `--output`: Specifies where to save the UDDF data to.
* `--invalid`: tells dc2uddf to output &lt;vendor&gt; and &lt;event&gt; tags in violation of the uddf spec, but which are helpful for understanding what your dive computer is actually recording.

My typical usage is to somthing like:

    dc2uddf -b smart -d "Uwatec Galileo" -i -t -o dives.uddf

[license]: http://www.apache.org/licenses/LICENSE-2.0.html
[libdc]: http://www.divesoftware.org/libdc/
[uddf]: http://www.streit.cc/extern/uddf_v310/en/index.html
[pwagstrom]: http://patrick.wagstrom.net/
[glib]: http://developer.gnome.org/glib/
[check]: http://check.sf.net/
[libxml]: http://www.xmlsoft.org/
[homebrew]: http://mxcl.github.com/homebrew/
