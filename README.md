HeeksCAD is an open source, CAD application based on OpenCASCADE

Best known as the CAD in HeeksCNC ( CAD/CAM software )
see http://www.heeks.net

This code is forked from the original HeeksCAD [source](https://github.com/Heeks/heekscad)

See [here](https://github.com/Heeks/heekscad/wiki/UsingHeeksCAD) for instruction how to use HeeksCAD.

# MacOS #
This is an update to allow HeeksCAD to compile on Mac and install using [brew](https://brew.sh). When all is said and done in a couple of days you should be able to simply use the following to install HeeksCAD on your Mac.

Tap my brew formulas

	brew tap mtott/homebrew-multiverse
	
Then install what you want
	
	brew install heekscad

or

	brew install heekscnc
	
**NOTE** - OCE (OpenCASCADE) has to compile from source and it takes a _Long_ time (60 minutes on my 2016 MacBook and 15 minutes on my 2015 MacBook Pro). So if you want to do things in stages try the following first

	brew tap science
	brew install oce	
	brew install heekscad
	brew install heekscnc

Finally to run you will have to launch on a command line (sorry). I will work on a brew cask at some point to get a launcher in the Launchpad.

# What works, What doesn't #
Most of the core functionality works.

* Mac keybindings have been added for the Magic Touch Pads (On Screen Help was updated)
* Remember on the Mac, Command Key = Windows Ctrl Key
* The toolbars which were flyouts on Windows don't work on the Mac. I will change these shortly to something that will work on the Mac

# Technologies #
The solid modeling is provided by [Open CASCADE](http://www.opencascade.org).
The graphical user interface is made using [wxWidgets](http://www.wxwidgets.org) updated in this fork to version 3.X.

# Platforms #
This version has been built on _MacOS 10.13 High Sierra_

This fork has NOT been tested for _Windows_, _Ubuntu_, _Debian_, or _OpenSUSE_.

# License #
All of my work here follows Dan's New BSD License.

This means you can take all my work and use it for your own commercial application. Do what you want with it. It uses wxWidgets and Open CASCADE. Any changes to wxWidgets or Open CASCADE are subject to their licenses.

# Contact #

Send messages here on GitHub

Peace

_M. T. Lott_
