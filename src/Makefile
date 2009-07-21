PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
DATADIR=$(PREFIX)/share
HEEKSCADDIR=$(DATADIR)/heekscad
PIXMAPSDIR=$(DATADIR)/pixmaps
APPSDIR=$(DATADIR)/applications
mkdir_p= mkdir -p

WXCFLAGS=$(shell wx-config --cflags)
PYTHONCFLAGS=$(shell python-config --cflags)

#run-in-place, for people on linux who don't want to sudo make install:
ifeq ($(strip $(RUNINPLACE)),yes)
  RIP=-DRUNINPLACE
else
  RIP= 
endif

CCFLAGS=$(CFLAGS) -x c++ -Wall -g -I "./" -I "/usr/include/opencascade" $(WXCFLAGS)  -DUNICODE -D__WXGTK__ -DHEEKSCAD -DHAVE_LIMITS -DWXUSINGDLL -DHAVE_LIMITS -DTIXML_USE_STL ${RIP} $(PYTHONCFLAGS)

OBJECTS=AboutBox.o advprops.o BezierCurve.o Cone.o ConversionTools.o CoordinateSystem.o Cuboid.o Cylinder.o DigitizedPoint.o DigitizeMode.o DimensionDrawing.o Drawing.o dxf.o svg.o Edge.o Face.o Geom.o glfont2.o GLList.o GraphicsCanvas.o Grid.o Gripper.o GripperSelTransform.o Group.o HArc.o HCircle.o HDimension.o HeeksCAD.o HeeksFrame.o HeeksPrintout.o HILine.o HEllipse.o HImage.o History.o HLine.o HPoint.o HSpline.o HText.o HXml.o Images.o InputModeCanvas.o LineArcDrawing.o Loop.o MagDragWindow.o manager.o MarkedList.o ObjPropsCanvas.o odcombo.o OptionsCanvas.o Plugins.o PointDrawing.o PointOrWindow.o PropertiesCanvas.o PropertyTrsf.o PropertyVertex.o propgrid.o props.o RegularShapesDrawing.o RemoveOrAddTool.o RuledSurface.o Ruler.o SelectMode.o Shape.o ShapeData.o Sketch.o Solid.o Sphere.o StlSolid.o StretchTool.o TransformTool.o TransformTools.o TreeCanvas.o Vertex.o ViewPoint.o ViewRotating.o ViewZooming.o Window.o Wire.o wxImageLoader.o HeeksCADInterface.o HeeksColor.o HeeksObj.o LeftAndRight.o MarkedObject.o ObjList.o PropertyCheck.o PropertyChoice.o PropertyColor.o PropertyDouble.o PropertyLength.o PropertyInt.o PropertyList.o PropertyString.o strconv.o ToolImage.o ToolList.o tinyxml.o tinyxmlerror.o tinyxmlparser.o Constraint.o ConstrainedObject.o ConstraintTools.o EndedObject.o SolveSketch.o solve.o CorrelationTool.o sketchinterface.o solveimpl.o errorfuncs.o OCCSolver.o MultiPoly.o BentleyOttmann.o

EXECUTABLE=../HeeksCAD

WXLIBS=$(shell wx-config --libs) $(shell wx-config --gl-libs)
PYTHONLIBS=$(shell python-config --ldflags)

#if user doesn't have OCC debs, they will have casroot set - look for and use it
ifneq ($(strip $(CASROOT)),)
    CASLIBPATH=-L$(CASROOT)/lib
else
    CASLIBPATH=
endif

OCCLIBS=-lTKVRML -lTKSTL -lTKBRep -lTKIGES -lTKShHealing -lTKSTEP -lTKSTEP209 -lTKSTEPAttr -lTKSTEPBase -lTKXSBase -lTKShapeSchema -lFWOSPlugin -lTKBool -lTKCAF -lTKCDF -lTKernel -lTKFeat -lTKFillet -lTKG2d -lTKG3d -lTKGeomAlgo -lTKGeomBase -lTKHLR -lTKMath -lTKOffset -lTKPrim -lTKPShape -lTKService -lTKTopAlgo -lTKV2d -lTKV3d -lTKMesh -lTKAdvTools -lTKBO -lTKXDESTEP -lTKXCAF -lTKXCAFSchema -lTKLCAF -lTKPLCAF ${CASLIBPATH}

.PHONY: executable
executable: ${OBJECTS}
	$(CC) -o ${EXECUTABLE} ${OBJECTS} -lstdc++ -lGLU ${PYTHONLIBS} -lGL -g ${WXLIBS} ${OCCLIBS}

stdafx.h.gch: stdafx.h
	$(CC) ${CCFLAGS} -c $? -o $@
 
AboutBox.o: AboutBox.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

advprops.o: advprops.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

BezierCurve.o: BezierCurve.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Cone.o: Cone.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

ConversionTools.o: ConversionTools.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

CoordinateSystem.o: CoordinateSystem.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Cuboid.o: Cuboid.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Cylinder.o: Cylinder.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

DigitizedPoint.o: DigitizedPoint.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

DigitizeMode.o: DigitizeMode.cpp	\
		SelectMode.h
	$(CC) -c DigitizeMode.cpp ${CCFLAGS} -o $@

DimensionDrawing.o: DimensionDrawing.cpp
	$(CC) -c DimensionDrawing.cpp ${CCFLAGS} -o $@

Drawing.o: Drawing.cpp	\
		SelectMode.h
	$(CC) -c Drawing.cpp ${CCFLAGS} -o $@

dxf.o: dxf.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

svg.o: svg.cpp
	$(CC) ${CCFLAGS} -c $? -o $@
	
Edge.o: Edge.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Face.o: Face.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Geom.o: Geom.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

glfont2.o: glfont2.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

GLList.o: GLList.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

GraphicsCanvas.o: GraphicsCanvas.cpp	 \
		SelectMode.h
	$(CC) -c GraphicsCanvas.cpp ${CCFLAGS} -o $@

Grid.o: Grid.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Gripper.o: Gripper.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

GripperSelTransform.o: GripperSelTransform.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Group.o: Group.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HArc.o: HArc.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HCircle.o: HCircle.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HDimension.o: HDimension.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HeeksCAD.o: HeeksCAD.cpp	\
		SelectMode.h
	$(CC) -c HeeksCAD.cpp ${CCFLAGS} -o $@

HeeksFrame.o: HeeksFrame.cpp \
		SelectMode.h
	$(CC) -c HeeksFrame.cpp ${CCFLAGS} -o $@

HeeksPrintout.o: HeeksPrintout.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HILine.o: HILine.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HImage.o: HImage.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

History.o: History.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HLine.o: HLine.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HPoint.o: HPoint.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HEllipse.o: HEllipse.cpp
	$(CC) ${CCFLAGS} -c $? -o $@
	
HSpline.o: HSpline.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HText.o: HText.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HXml.o: HXml.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Images.o: Images.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

InputModeCanvas.o: InputModeCanvas.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

LineArcDrawing.o: LineArcDrawing.cpp	\
		SelectMode.h
	$(CC) -c LineArcDrawing.cpp ${CCFLAGS} -o $@

Loop.o: Loop.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

MagDragWindow.o: MagDragWindow.cpp	\
		SelectMode.h
	$(CC) -c MagDragWindow.cpp ${CCFLAGS} -o $@

manager.o: manager.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

MarkedList.o: MarkedList.cpp	\
		SelectMode.h
	$(CC) -c MarkedList.cpp ${CCFLAGS} -o $@

ObjPropsCanvas.o: ObjPropsCanvas.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

odcombo.o: odcombo.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

OptionsCanvas.o: OptionsCanvas.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Plugins.o: Plugins.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PointDrawing.o: PointDrawing.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PointOrWindow.o: PointOrWindow.cpp	\
		SelectMode.h
	$(CC) -c PointOrWindow.cpp ${CCFLAGS} -o $@

PropertiesCanvas.o: PropertiesCanvas.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PropertyTrsf.o: PropertyTrsf.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

propgrid.o: propgrid.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

props.o: props.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

RegularShapesDrawing.o: RegularShapesDrawing.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

RemoveOrAddTool.o: RemoveOrAddTool.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

RuledSurface.o: RuledSurface.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Ruler.o: Ruler.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

SelectMode.o: SelectMode.cpp	\
		SelectMode.h
	$(CC) -c SelectMode.cpp ${CCFLAGS} -o $@

Shape.o: Shape.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

ShapeData.o: ShapeData.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Sketch.o: Sketch.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Solid.o: Solid.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Sphere.o: Sphere.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

StlSolid.o: StlSolid.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

StretchTool.o: StretchTool.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

TransformTool.o: TransformTool.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

TransformTools.o: TransformTools.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

TreeCanvas.o: TreeCanvas.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Vertex.o: Vertex.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

ViewPoint.o: ViewPoint.cpp	\
		SelectMode.h
	$(CC) -c ViewPoint.cpp ${CCFLAGS} -o $@

ViewRotating.o: ViewRotating.cpp	\
		SelectMode.h
	$(CC) -c ViewRotating.cpp ${CCFLAGS} -o $@

ViewZooming.o: ViewZooming.cpp	\
		SelectMode.h
	$(CC) -c ViewZooming.cpp ${CCFLAGS} -o $@

Window.o: Window.cpp	\
		SelectMode.h
	$(CC) -c Window.cpp ${CCFLAGS} -o $@

Wire.o: Wire.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

wxImageLoader.o: wxImageLoader.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HeeksCADInterface.o: ../interface/HeeksCADInterface.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HeeksColor.o: ../interface/HeeksColor.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

HeeksObj.o: ../interface/HeeksObj.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

LeftAndRight.o: ../interface/LeftAndRight.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

MarkedObject.o: ../interface/MarkedObject.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

ObjList.o: ../interface/ObjList.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PropertyCheck.o: ../interface/PropertyCheck.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PropertyChoice.o: ../interface/PropertyChoice.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PropertyColor.o: ../interface/PropertyColor.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PropertyDouble.o: ../interface/PropertyDouble.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PropertyLength.o: ../interface/PropertyLength.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PropertyVertex.o: ../interface/PropertyVertex.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PropertyInt.o: ../interface/PropertyInt.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PropertyList.o: ../interface/PropertyList.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

PropertyString.o: ../interface/PropertyString.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

strconv.o: ../interface/strconv.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

ToolImage.o: ../interface/ToolImage.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

ToolList.o: ../interface/ToolList.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

tinyxml.o: ../tinyxml/tinyxml.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

#tinystr.o: ../tinyxml/tinystr.cpp
#	$(CC) ${CCFLAGS} -c $? -o $@

tinyxmlerror.o: ../tinyxml/tinyxmlerror.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

tinyxmlparser.o: ../tinyxml/tinyxmlparser.cpp
	$(CC) ${CCFLAGS} -c $? -o $@
	
SolveSketch.o: SolveSketch.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

solve.o: ../sketchsolve/src/solve.cpp
	$(CC) ${CCFLAGS} -c $? -o $@
	
solveimpl.o: ../sketchsolve/src/solveimpl.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

errorfuncs.o: ../sketchsolve/src/errorfuncs.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

Constraint.o: Constraint.cpp
	$(CC) ${CCFLAGS} -c $? -o $@
	
ConstrainedObject.o: ConstrainedObject.cpp
	$(CC) ${CCFLAGS} -c $? -o $@
	
ConstraintTools.o: ConstraintTools.cpp
	$(CC) ${CCFLAGS} -c $? -o $@
	
EndedObject.o: EndedObject.cpp
	$(CC) ${CCFLAGS} -c $? -o $@

CorrelationTool.o: CorrelationTool.cpp	\
		CorrelationTool.h
	$(CC) -c CorrelationTool.cpp ${CCFLAGS} -o $@
	
sketchinterface.o: sketchinterface.cpp	
	$(CC) -c sketchinterface.cpp ${CCFLAGS} -o $@

OCCSolver.o: OCCSolver.cpp	
	$(CC) -c OCCSolver.cpp ${CCFLAGS} -o $@

MultiPoly.o: MultiPoly.cpp	
	$(CC) -c MultiPoly.cpp ${CCFLAGS} -o $@

BentleyOttmann.o: BentleyOttmann.cpp	
	$(CC) -c BentleyOttmann.cpp ${CCFLAGS} -o $@


clean:
	-rm ${EXECUTABLE} ${OBJECTS}

install:
	test -z "$(DESTDIR)$(HEEKSCADDIR)" || $(mkdir_p) "$(DESTDIR)$(BINDIR)"
	test -z "$(DESTDIR)$(HEEKSCADDIR)/bitmaps" || $(mkdir_p) "$(DESTDIR)$(HEEKSCADDIR)/bitmaps"
	test -z "$(DESTDIR)$(HEEKSCADDIR)/icons" || $(mkdir_p) "$(DESTDIR)$(HEEKSCADDIR)/icons"
	test -z "$(DESTDIR)$(PIXMAPSDIR)" || $(mkdir_p) "$(DESTDIR)$(PIXMAPSDIR)"
	test -z "$(DESTDIR)$(APPSDIR)" || $(mkdir_p) "$(DESTDIR)$(APPSDIR)"
	install -s $(EXECUTABLE) $(DESTDIR)$(BINDIR)/
	cp ../bitmaps/*.png $(DESTDIR)$(HEEKSCADDIR)/bitmaps/
	cp ../bitmaps/*.svg $(DESTDIR)$(HEEKSCADDIR)/bitmaps/
	cp ../bitmaps/*.glf $(DESTDIR)$(HEEKSCADDIR)/bitmaps/
	cp ../icons/*.png $(DESTDIR)$(HEEKSCADDIR)/icons/
	cp ../bitmaps/cyl.png $(DESTDIR)$(PIXMAPSDIR)/HeeksCAD.png
	cp ../HeeksCAD.desktop $(DESTDIR)$(APPSDIR)/

uninstall:
	-rm -rf $(DESTDIR)$(HEEKSCADDIR)
	-rm $(DESTDIR)$(BINDIR)/HeeksCAD
	-rm $(DESTDIR)$(PIXMAPSDIR)/HeeksCAD.png
	-rm $(DESTDIR)$(APPSDIR)/HeeksCAD.desktop

.PHONY: all-before
all-before: stdafx.h.gch

.PHONY: all
all: all-before	executable
