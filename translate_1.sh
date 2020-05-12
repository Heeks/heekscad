#! /bin/sh

# shell script for creating fresh new .po file

# How to make .po file for new language:
#    work continued after step 0
#    launch translate_1.sh (../../<this_batch_script>)
#    not works? Try to change executable bit (chmod u+x <this_batch_script>)
#
#    then work on fresh .po file as you are used to do (automated translation,
#    by hand with your prefered text editor od some po file editor,... at your will)
#
#    please don't forget to edit .po file header
#    sourcefilelist can be deleted after this step

if [ -f sourcefilelist ] ; then
    xgettext -C -n -k_ -o HeeksCAD.po -f sourcefilelist
else
    echo 'sourcefilelist not found - do translate_0 step first, please'
fi

# EOF translate_1_make_from_source.sh
