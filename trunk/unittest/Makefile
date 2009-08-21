CC = g++
PYTHONCFLAGS=$(shell python-config --includes)

FLAGS_64BIT=$(shell if [ x`getconf LONG_BIT` = x64 ] ; then echo "-D_OCC64 -m64" ; fi )

CCFLAGS=$(CFLAGS) -Wall -g -I "./" -I "/usr/include/opencascade" $(FLAGS_64BIT) -DUNICODE -DHEEKSCAD -DHAVE_LIMITS -DWXUSINGDLL -DHAVE_LIMITS -DTIXML_USE_STL ${RIP} $(PYTHONCFLAGS)  `wx-config  --cflags` -DUNITTEST

#if user doesn't have OCC debs, they will have casroot set - look for and use it
ifneq ($(strip $(CASROOT)),)
    CASLIBPATH=-L$(CASROOT)/lib
else
    CASLIBPATH=
endif

OCCLIBS=-lTKVRML -lTKSTL -lTKBRep -lTKIGES -lTKShHealing -lTKSTEP -lTKSTEP209 -lTKSTEPAttr -lTKSTEPBase -lTKXSBase -lTKShapeSchema -lFWOSPlugin -lTKBool -lTKCAF -lTKCDF -lTKernel -lTKFeat -lTKFillet -lTKG2d -lTKG3d -lTKGeomAlgo -lTKGeomBase -lTKHLR -lTKMath -lTKOffset -lTKPrim -lTKPShape -lTKService -lTKTopAlgo -lTKV2d -lTKV3d -lTKMesh -lTKAdvTools -lTKBO -lTKXDESTEP -lTKXCAF -lTKXCAFSchema -lTKLCAF -lTKPLCAF ${CASLIBPATH}

all: Polygontest

Polygontest: Polygontest.cpp Polygon.o ../src/Polygon.h
	$(CC) Polygontest.cpp Polygon.o $(CCFLAGS) $(OCCLIBS) -o Polygontest

Polygon.o: ../src/Polygon.cpp ../src/Polygon.h
	$(CC) ../src/Polygon.cpp -c $(CCFLAGS) -o Polygon.o -DUNITTEST_NO_HEEKS

clean:
	-rm -rf Polygontest Polygon.o









