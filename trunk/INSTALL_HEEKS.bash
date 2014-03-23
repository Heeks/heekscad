# cd to the folder this file is in.
# Make it executable with chmod +x INSTALL_HEEKS.bash
# run it with ./INSTALL_HEEKS.bash. DO NOT USE SUDO!.
# You will be asked a few things and prompted for your sudo password. After that you will see a lot of lines and you should wait about an hour. It will prompt you a second time for your sudo password
# Why, because it takes about 30 to 45 minutes to build the oce libraries and after that we need sudo a few times more.


#!/bin/bash
# This will install heekscad and heekscnc on Ubuntu 10.04.04LTS, 12.04.02LTS and 13.04. These Versions are tested.
# This installation will also solve the issue #18 on https://github.com/Heeks/heekscnc/issues/18
# If your installation fails, just delete the directory and compile again!
# After a successful installation, you will have build your own *.deb packages and see HeeksCAD running

BUILDPATH=~/         # Buildapath in our home-directory
BUILDDIR=HEEKSCAD   # Directory to build for installation

clear
read -p "
I WILL TAKE NO RESPONSIBILITY IF THIS IS NOT WORKING OR DAMAGING YOUR SYSTEM!!!

Welcome to elrippos install script for the installation of HeeksCAD and HeeksCNC http://elrippoisland.net .
This script has a dialog basis and will check the dependencies of the needed libraries, and ask you to install them if they are not met.
Further it will check, if there is already a possible HeeksCAD and/or HeeksCNC installation present on your system.
After this installation, you should find the *.deb packages in your /home folder and a running HeeksCAD and HeeksCNC installation.
If you want to proceed, please answer yes. If you don't, please answer no.

        
        Please answer with yes|no.:" answer

case "$answer" in
Yes|yes|Y|y|"") echo "
Ok, we will start now....
"
    ;;
No|no|N|n) echo "ABORTED from the beginning."
    exit 1
    ;;
*) echo "Unknown parameter. ABORTED from the beginning. Please type yes or no." 
    exit 255
    ;;
esac

if [ -f /usr/bin/heekscad ]; then
    read -p "
We found an old heekscad executable. For a clean new install we should purge former installs. Do you want to do this?

        Please answer with yes|no.:" answer0

case "$answer0" in
Yes|yes|Y|y|"") echo "
Ok, we will purge former installs.
"
    sudo apt-get purge heekscad heekscnc
    sudo rm -fR /usr/bin/heekscad
    ;;
No|no|N|n) echo "
ABORTED deleting heekscad. We will try to do a clean install. This may work or not.
"
    ;;
*) echo "
Unknown parameter. ABORTED deleteing heekscad. Please type yes or no.
" 
    exit 255
    ;;
esac
    else
    echo "Did not find any heeks executable, CONTINUING."
fi

cd ~/

if [ -f /usr/lib/libboost_python-py26.so.1.40.0 ] || [ -f /usr/lib/libboost_python-py26.so.1.42.0 ] || [ -f /usr/lib/libboost_python-py27.so.1.40.0 ] || [ -f /usr/lib/libboost_python-py27.so.1.42.0 ] ; then
        echo "Excellent!!! Found 

"$(find /usr/lib/libboost_python-py26.so.1.40.0 /usr/lib/libboost_python-py26.so.1.42.0 /usr/lib/libboost_python-py27.so.1.40.0 /usr/lib/libboost_python-py27.so.1.42.0)" 

We already have the libboost libraries installed which we need!!!"
        else 
        read -p "We do not have, or can not find the libboost libraries needed to build HeeksCNC. Do you want to download and install the libraries from http://pkgs.org?
        
        Please answer with yes|no.:" answer1

case "$answer1" in
Yes|yes|Y|y|"") echo "
Ok, firstly we will check on which system you are running. Either 32bit or 64bit and then install the corresponding libraries from pkgs.org.
"
readkernelversion="$(uname -a | cut -d" " -f12)"
    if [ -d /lib64 ] && [ $readkernelversion = x86_64 ]; then
    echo "Found 64bit kernel. Trying to download and install libboost-python1.40.0_1.40.0-4ubuntu4_amd64.deb."
    wget -t 3 http://archive.ubuntu.com/ubuntu/pool/main/b/boost1.40/libboost-python1.40.0_1.40.0-4ubuntu4_amd64.deb
    sudo dpkg -i libboost-python1.40.0_1.40.0-4ubuntu4_amd64.deb
        else
        echo "Found 32bit kernel. Trying to download and install libboost-python1.40.0_1.40.0-4ubuntu4_i386.deb."
        wget -t 3 http://archive.ubuntu.com/ubuntu/pool/main/b/boost1.40/libboost-python1.40.0_1.40.0-4ubuntu4_i386.deb
        sudo dpkg -i libboost-python1.40.0_1.40.0-4ubuntu4_i386.deb
    fi
       if [ -f /usr/lib/libboost_python-py26.so.1.40.0 ] || [ -f /usr/lib/libboost_python-py26.so.1.42.0 ] || [ -f /usr/lib/libboost_python-py27.so.1.40.0 ] || [ -f /usr/lib/libboost_python-py27.so.1.42.0 ] ; then
       echo "Installing of the libbost-libraries was succesful, CONTINUING!"
           else
           echo "FAILED to install the libboost libraries. EXITING!"
           exit 1
       fi
    ;;
No|no|N|n) echo "ABORTED libboost install."
    exit 1
    ;;
*) echo "Unknown parameter. ABORTED libboost install. Please type yes or no." 
    exit 255
    ;;
esac

fi

# Install the dependencies
read -p "
Now we will install the other librabries needed. Do you want to continue?

Please answer with yes|no.:" answer2

case "$answer2" in
Yes|yes|Y|y|"") echo "
Installing the libraries.
"
    sudo apt-get install git-core subversion libwxbase2.8-dev cmake build-essential libopencascade-dev libwxgtk2.8-dev libgtkglext1-dev python-dev cmake libboost-python-dev python-wxgtk2.8 python-wxversion libftgl-dev
    ;;
No|no|N|n) echo "ABORTED installling needed libraries."
    exit 1
    ;;
*) echo "Unknown parameter. ABORTED installing needed librabries. Please type yes or no." 
    exit 255
    ;;
esac

read -p "
BUILDING the HEEKSCAD directory in your /home folder and downloading the source. Do you want to continue?

Please answer with yes|no.:" answer3

case "$answer3" in
Yes|yes|Y|y|"") echo ""
cd $BUILDPATH       # Make the directory
mkdir $BUILDDIR	
cd $BUILDPATH/$BUILDDIR
svn checkout http://heekscad.googlecode.com/svn/trunk/ heekscad
cd $BUILDPATH/$BUILDDIR/heekscad
svn checkout http://heekscnc.googlecode.com/svn/trunk/ heekscnc
svn checkout http://libarea.googlecode.com/svn/trunk/ libarea
git clone --recursive git://github.com/sliptonic/opencamlib.git
git clone --recursive git://github.com/aewallin/opencamlib.git
git clone --recursive git://github.com/tpaviot/oce.git
sudo ldconfig

# OCE -installation:
cd $BUILDPATH/$BUILDDIR/heekscad/oce/
mkdir build
cd $BUILDPATH/$BUILDDIR/heekscad/oce/build
cmake ..
make
sudo make install
#make clean

# HeeksCNC uses a number of libraries to perform various operations. Install all of them or
# only the ones you plan to use.
# Install libarea
#
# area.so is required for pocket operations.

cd $BUILDPATH/$BUILDDIR/heekscad/libarea/
#make clean # OPTIONAL: only needed if you have previous make -attempts, otherwise, not needed
cmake .
make
sudo make install
#make clean

# Install opencamlib
# opencamlib is the replacement for pycam. It's required for zigzag operations.

cd $BUILDPATH/$BUILDDIR/heekscad/opencamlib/src
#make clean # OPTIONAL: only needed if you have previous make -attempts, otherwise, not needed
cmake .
make
sudo make install
#make clean

# Install heekscnc:
cd $BUILDPATH/$BUILDDIR/heekscad/heekscnc
cmake .
make package
sudo dpkg -i heekscnc_*.deb # only one .deb to install, but the filename may change
#make clean
cp heekscnc_*.deb ~/

# Install heekscad:
cd $BUILDPATH/$BUILDDIR/heekscad
cmake .
make package
sudo dpkg -i heekscad_*.deb #only one .deb to install, but the filename may change
#make clean
cp heekscad_*.deb ~/

# Give command:
sudo ldconfig

# Delete the files in the builddirectory. These won't be needed
#sudo rm -fR $BUILDPATH/$BUILDDIR
    ;;
No|no|N|n) echo "ABORTED building from source."
    exit 1
    ;;
*) echo "Unknown parameter. ABORTED building from source. Please type yes or no." 
    exit 255
    ;;
esac

# Let's start our newly installed heekscad / heekscnc:
echo "Let's start Heekscad :D"
heekscad

