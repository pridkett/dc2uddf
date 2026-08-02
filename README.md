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

USB IrDA Adapters
-----------------

The authentic ScubaPro IrDA adapter is based on the MosChip **MCS7780**
chipset (USB id `9710:7780`), driven by the `mcs7780` kernel module. Other
chipsets should also work as long as Linux has a driver for them — for
example, the **KS-959** adapter works once you bring in the `ks959_sir`
driver. Check `lsusb` for your adapter's USB id and load the matching
module; the generic `irda_usb` driver does *not* cover either of these.

A few hard-won lessons about USB passthrough into a virtual machine:

* Plug the adapter **directly into your PC** — not through a USB hub or
  multi-port USB-C adapter.
* Configure the VM with a **USB 2.0 (EHCI) controller**. USB 3.0 (xHCI)
  won't work — these are full-speed USB 1.1 devices, and behind an xHCI
  controller the driver's init transfers fail with USB protocol errors.
* Even with all of that right, things are dicey. If the adapter doesn't
  initialize, unload/reload the module and replug before assuming the
  worst.

Starting IrDA by Hand
---------------------

This is a fine way to handle things if you're testing the software. First, make sure to plug in your USB&rarr;IR dongle and then run the following commands to setup the IrDA subsystem (substitute `ks959_sir` or your adapter's driver for `mcs7780` as appropriate):

    sudo modprobe ircomm
    sudo modprobe ircomm-tty
    sudo modprobe mcs7780
    sudo irattach irda0 -s

Note that the driver module must be loaded **before** running `irattach` —
if `irattach` runs first it exits immediately and nothing works.

Then put the dive computer in IR sync mode, hold it close to the adapter
(IR windows facing, ~10-20 cm), and confirm discovery sees it:

    watch -n1 cat /proc/net/irda/discovery

The dive computer should appear within a few seconds with a `daddr:` field.
That 32-bit hex address is what you pass to `-d`.

Debugging the IrDA Link
-----------------------

When the connection doesn't work, check things in this order:

1. **Is the adapter visible in the VM?** `lsusb` should show it (MCS7780 is
   `9710:7780`). If not, fix the hypervisor USB passthrough first.
2. **Did the driver bind?** `dmesg | grep -iE "mcs|ks959|irda"` should show
   the driver registering and `ip link show irda0` should exist.
3. **Is irattach actually running?** `ps aux | grep irattach` — it exits
   silently on failure. Its log lines go to `/var/log/syslog`; look for
   `ioctl(SIOCSIFFLAGS): Protocol error`, which means the driver could not
   initialize the adapter over USB (almost always the USB 3.0 / hub problem
   described above). The `modprobe irda0 ... FATAL` line in syslog is
   harmless noise.
4. **Is the interface up?** `ip link show irda0` should NOT say
   `state DOWN`/`qdisc noop`. You can do irattach's job manually to isolate
   failures: `sudo ip link set irda0 up` (an error here = driver/USB
   problem) then `echo 1 | sudo tee /proc/sys/net/irda/discovery`.
5. **Is discovery seeing the dive computer?** `cat /proc/net/irda/discovery`
   while the computer is in IR mode. Empty log = physical problem (range,
   alignment, device not in sync mode) — `sudo irdadump` shows the raw
   IR frames if you need to tell "no beacons sent" apart from "no reply".
6. **dc2uddf errors decoded:** `Network is unreachable (101)` from
   `dc_socket_connect` means the IrDA interface is down; `No route to host
   (113)` means the stack is up but the address you gave `-d` was not
   discovered.

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

As the only computer I have is a Uwatec Galileo Luna, it's the only one that I can describe the process for. For IrDA devices, `-d` takes the 32-bit device address reported in `/proc/net/irda/discovery` (see above); for serial devices it takes the serial port path.

    dc2uddf -b smart -d 0x5b31b527

The general format is:

    dc2uddf -b [backend name] -d [device address]

You can get a list of the backend and device names with `dc2uddf --listbackends` and `dc2uddf --listdevices`. However, as I only have the Uwatec Galileo, it is the only device that is tested. (Older versions of dc2uddf accepted a product name like `"Uwatec Galileo"` for `-d` because the old libdivecomputer did its own IrDA discovery; with libdivecomputer 0.9 the numeric address is required for live downloads.)

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
