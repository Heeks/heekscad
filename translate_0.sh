#! /bin/sh

# step 0 for new .po file - creation of file list

# How to creat new file list
#    create new language dir heekscad/translations/<langcode>
#    change dir there (cd <langcode> freshly created)
#    launch filelist creation script (../../<this_batch_script>)
#    not works? Try to change executable bit (chmod u+x <this_batch_script>)
#    resulting file 'sourcefilelist' can be edited by hand if needed
#    then go to step 1 (translate_1_...sh)

ls -1 ../../src/*.cpp > sourcefilelist
ls -1 ../../src/*.h >> sourcefilelist
ls -1 ../../interface/*.cpp >> sourcefilelist
ls -1 ../../interface/*.h >> sourcefilelist
ls -1 ../../PyHeeksCAD/*.cpp >> sourcefilelist
ls -1 ../../PyHeeksCAD/*.h >> sourcefilelist
ls -1 ../../../heekscnc/src/*.cpp >> sourcefilelist
ls -1 ../../../heekscnc/src/*.h >> sourcefilelist

# EOF translate_0_make_sourcefilelist.sh
