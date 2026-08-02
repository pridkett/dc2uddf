dc2uddf
=======
Copyright (c) 2012-2025 [Patrick Wagstrom][pwagstrom]

This is a simple tool that uses [libdivecomputer][libdc] to connect a dive computer
and download all the dive information into a [UDDF][uddf] 3.2.3 file. This information
can then be imported into a variety of different tools (MacDive, Subsurface,
Diving Log, etc.). It can also replay a previously saved binary dump of the
dive data, so you don't need the (slow, IrDA) device connected to iterate.

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

* [libdivecomputer][libdc] 0.9.0 or later - the magical library that interfaces with nearly every dive computer
* [glib][glib] - a utility library with many useful data structures and methods for C programming
* [libxml][libxml] - XML serialization and deserialization library
* [check][check] - unit testing for C programs

On a Mac all of these can be installed using [homebrew][homebrew] with the following command:

    brew install check libxml2 glib libdivecomputer automake autoconf

On Ubuntu you can install most of these with the following command:

    apt-get install check libxml2-dev libglib2.0-dev libdivecomputer-dev irda-utils

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

You can get a list of the backend and device names with `dc2uddf --listbackends` and `dc2uddf --listdevices`. However, as I only have the Uwatec Galileo, it is the only device that is tested.

Working With Dump Files
-----------------------

IrDA transfers run at roughly 1-2 KB/s, so a full download takes 5-10
minutes. To avoid repeating that, dc2uddf can save the raw dive data to disk
during a live download and later regenerate UDDF from that file without any
device attached:

    # live download, also saving the raw dive records
    dc2uddf -b smart -d 0x12345678 --save-dump dump.bin -i -t -o dives.uddf

    # later: regenerate UDDF from the dump, no device needed
    dc2uddf -b smart -d "Uwatec Galileo Sol" --from-dump dump.bin -i -t -o dives.uddf

In `--from-dump` mode there is no device to talk to, so `-d` instead selects
the device *descriptor by product name*. Give the exact product name (see
`--listdevices`): sample decoding is model-specific, and picking the wrong
model produces "Invalid type bits" parse errors. Dump files use the Uwatec
Smart record framing and are compatible with dumps produced by dctool.

Additional Arguments
====================

* `-i`, `--ipf`: Initial pressure fix. When first connecting the Luna and some other devices the pressure will read 0. This goes back and sets the initial pressure to the first valid pressure reading.
* `-t`, `--truncate`: Run an algorithm to truncate dives after surfacing. Basically, this stops a dive after you've surfaced if you don't go down below 1m again. This is handy because the Luna typically records an extra five minutes of data at the end of the dive.
* `-o`, `--output`: Specifies where to save the UDDF data to.
* `-l`, `--limit`: Limit the download to the given number of dives.
* `-s`, `--since`: Only download dives since the given date (YYYY-MM-DD).
* `--from-dump FILE`: Parse dives from a saved dive-data dump instead of a live device.
* `--save-dump FILE`: During a live download, also save the raw dive records to FILE for later replay with `--from-dump`.
* `--dump-memory FILE`: During a live session, save a full device memory image to FILE. This is a backup/debugging artifact in a different layout and can NOT be replayed with `--from-dump`.
* `--invalid`: tells dc2uddf to output &lt;vendor&gt; and &lt;event&gt; tags in violation of the uddf spec, but which are helpful for understanding what your dive computer is actually recording.

My typical usage is something like:

    dc2uddf -b smart -d "Uwatec Galileo" -i -t --save-dump dump.bin -o dives.uddf

[license]: http://www.apache.org/licenses/LICENSE-2.0.html
[libdc]: http://www.divesoftware.org/libdc/
[uddf]: https://www.streit.cc/resources/UDDF/v3.2.3/en/index.html
[pwagstrom]: http://patrick.wagstrom.net/
[glib]: http://developer.gnome.org/glib/
[check]: http://check.sf.net/
[libxml]: http://www.xmlsoft.org/
[homebrew]: http://mxcl.github.com/homebrew/
