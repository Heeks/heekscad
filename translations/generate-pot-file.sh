if [ "$#" -ne 1 ]; then
    echo "Usage $0 output.pot"
    exit 1
fi
xgettext -C -k_ -o $1 src/*.cpp src/*.h interface/*.cpp interface/*.h ../heekscnc/src/*.cpp ../heekscnc/src/*.h
