PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
DATADIR=$(PREFIX)/share
HEEKSCADDIR=$(DATADIR)/heekscad
PIXMAPSDIR=$(DATADIR)/pixmaps
APPSDIR=$(DATADIR)/applications
mkdir_p= mkdir -p

WXCFLAGS=$(shell wx-config --cflags)

# dan heeks uncomments the next line to make it work for him on his Inspiron 530
#WXCFLAGS=-I/usr/lib/wx/include/gtk2-unicode-release-2.8 -I/usr/include/wx-2.8

#CCFLAGS=-Wall -g -I "./" -I "/usr/include/opencascade" -I/home/dan/Desktop/wxGTK-2.8.9/buildgtk/lib/wx/include/gtk2-ansi-debug-2.8 -I/usr/include/wx-2.8 -D__WXGTK__ -DHEEKSCAD -DHAVE_LIMITS -D__WXDEBUG__ -DWXUSINGDLL -DHAVE_LIMITS
CCFLAGS=-Wall -g -I "./" -I "/usr/include/opencascade" $(WXCFLAGS) -DUNICODE -D__WXGTK__ -DHEEKSCAD -DHAVE_LIMITS -DWXUSINGDLL -DHAVE_LIMITS -DTIXML_USE_STL

OBJECTS=AboutBox.o advprops.o BezierCurve.o Cone.o ConversionTools.o CoordinateSystem.o Cuboid.o Cylinder.o DigitizedPoint.o DigitizeMode.o DimensionDrawing.o Drawing.o dxf.o Edge.o Face.o Geom.o glfont.o GLList.o GraphicsCanvas.o Grid.o Gripper.o GripperSelTransform.o Group.o HArc.o HCircle.o HDimension.o HeeksCAD.o HeeksFrame.o HeeksPrintout.o HILine.o HEllipse.o HImage.o History.o HLine.o HPoint.o HSpline.o HText.o HXml.o Images.o InputModeCanvas.o LineArcDrawing.o Loop.o MagDragWindow.o manager.o MarkedList.o ObjPropsCanvas.o odcombo.o OptionsCanvas.o Plugins.o PointDrawing.o PointOrWindow.o PropertiesCanvas.o PropertyTrsf.o PropertyVertex.o propgrid.o props.o RegularShapesDrawing.o RemoveOrAddTool.o RuledSurface.o Ruler.o SelectMode.o Shape.o ShapeData.o Sketch.o Solid.o Sphere.o StlSolid.o StretchTool.o TransformTool.o TransformTools.o TreeCanvas.o Vertex.o ViewPoint.o ViewRotating.o ViewZooming.o Window.o Wire.o wxImageLoader.o HeeksCADInterface.o HeeksColor.o HeeksObj.o LeftAndRight.o MarkedObject.o ObjList.o PropertyCheck.o PropertyChoice.o PropertyColor.o PropertyDouble.o PropertyLength.o PropertyInt.o PropertyList.o PropertyString.o strconv.o ToolImage.o ToolList.o tinyxml.o tinyxmlerror.o tinyxmlparser.o

EXECUTABLE=../HeeksCAD

${EXECUTABLE}: ${OBJECTS}
	$(CC) -o ${EXECUTABLE} ${OBJECTS} -lstdc++ -lGLU -lGL `wx-config --libs` `wx-config --gl-libs` -lTKVRML -lTKSTL -lTKBRep -lTKIGES -lTKShHealing -lTKSTEP -lTKSTEP209 -lTKSTEPAttr -lTKSTEPBase -lTKXSBase -lTKShapeSchema -lFWOSPlugin -lTKBool -lTKCAF -lTKCDF -lTKernel -lTKFeat -lTKFillet -lTKG2d -lTKG3d -lTKGeomAlgo -lTKGeomBase -lTKHLR -lTKMath -lTKOffset -lTKPrim -lTKPShape -lTKService -lTKTopAlgo -lTKV2d -lTKV3d -lTKMesh -lTKAdvTools -lTKBO -lTKXDESTEP -lTKXCAF -lTKXCAFSchema -lTKLCAF -lTKPLCAF

AboutBox.o: AboutBox.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

advprops.o: advprops.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

BezierCurve.o: BezierCurve.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Cone.o: Cone.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

ConversionTools.o: ConversionTools.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

CoordinateSystem.o: CoordinateSystem.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Cuboid.o: Cuboid.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Cylinder.o: Cylinder.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

DigitizedPoint.o: DigitizedPoint.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

DigitizeMode.o: DigitizeMode.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

DimensionDrawing.o: DimensionDrawing.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Drawing.o: Drawing.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

dxf.o: dxf.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Edge.o: Edge.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Face.o: Face.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Geom.o: Geom.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

glfont.o: glfont.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

GLList.o: GLList.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

GraphicsCanvas.o: GraphicsCanvas.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Grid.o: Grid.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Gripper.o: Gripper.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

GripperSelTransform.o: GripperSelTransform.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Group.o: Group.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HArc.o: HArc.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HCircle.o: HCircle.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HDimension.o: HDimension.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HeeksCAD.o: HeeksCAD.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HeeksFrame.o: HeeksFrame.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HeeksPrintout.o: HeeksPrintout.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HILine.o: HILine.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HImage.o: HImage.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

History.o: History.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HLine.o: HLine.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HPoint.o: HPoint.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HEllipse.o: HEllipse.cpp
	$(CC) -c $? ${CCFLAGS} -o $@
	
HSpline.o: HSpline.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HText.o: HText.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HXml.o: HXml.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Images.o: Images.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

InputModeCanvas.o: InputModeCanvas.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

LineArcDrawing.o: LineArcDrawing.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Loop.o: Loop.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

MagDragWindow.o: MagDragWindow.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

manager.o: manager.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

MarkedList.o: MarkedList.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

ObjPropsCanvas.o: ObjPropsCanvas.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

odcombo.o: odcombo.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

OptionsCanvas.o: OptionsCanvas.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Plugins.o: Plugins.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PointDrawing.o: PointDrawing.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PointOrWindow.o: PointOrWindow.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PropertiesCanvas.o: PropertiesCanvas.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PropertyTrsf.o: PropertyTrsf.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

propgrid.o: propgrid.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

props.o: props.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

RegularShapesDrawing.o: RegularShapesDrawing.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

RemoveOrAddTool.o: RemoveOrAddTool.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

RuledSurface.o: RuledSurface.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Ruler.o: Ruler.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

SelectMode.o: SelectMode.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Shape.o: Shape.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

ShapeData.o: ShapeData.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Sketch.o: Sketch.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Solid.o: Solid.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Sphere.o: Sphere.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

StlSolid.o: StlSolid.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

StretchTool.o: StretchTool.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

TransformTool.o: TransformTool.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

TransformTools.o: TransformTools.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

TreeCanvas.o: TreeCanvas.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Vertex.o: Vertex.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

ViewPoint.o: ViewPoint.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

ViewRotating.o: ViewRotating.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

ViewZooming.o: ViewZooming.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Window.o: Window.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

Wire.o: Wire.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

wxImageLoader.o: wxImageLoader.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HeeksCADInterface.o: ../interface/HeeksCADInterface.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HeeksColor.o: ../interface/HeeksColor.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

HeeksObj.o: ../interface/HeeksObj.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

LeftAndRight.o: ../interface/LeftAndRight.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

MarkedObject.o: ../interface/MarkedObject.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

ObjList.o: ../interface/ObjList.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PropertyCheck.o: ../interface/PropertyCheck.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PropertyChoice.o: ../interface/PropertyChoice.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PropertyColor.o: ../interface/PropertyColor.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PropertyDouble.o: ../interface/PropertyDouble.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PropertyLength.o: ../interface/PropertyLength.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PropertyVertex.o: ../interface/PropertyVertex.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PropertyInt.o: ../interface/PropertyInt.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PropertyList.o: ../interface/PropertyList.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

PropertyString.o: ../interface/PropertyString.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

strconv.o: ../interface/strconv.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

ToolImage.o: ../interface/ToolImage.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

ToolList.o: ../interface/ToolList.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

tinyxml.o: ../tinyxml/tinyxml.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

#tinystr.o: ../tinyxml/tinystr.cpp
#	$(CC) -c $? ${CCFLAGS} -o $@

tinyxmlerror.o: ../tinyxml/tinyxmlerror.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

tinyxmlparser.o: ../tinyxml/tinyxmlparser.cpp
	$(CC) -c $? ${CCFLAGS} -o $@

clean:
	-rm ${EXECUTABLE} ${OBJECTS}

install:
	test -z "$(DESTDIR)$(HEEKSCADDIR)" || $(mkdir_p) "$(DESTDIR)$(BINDIR)"
	test -z "$(DESTDIR)$(HEEKSCADDIR)/bitmaps" || $(mkdir_p) "$(DESTDIR)$(HEEKSCADDIR)/bitmaps"
	test -z "$(DESTDIR)$(HEEKSCADDIR)/icons" || $(mkdir_p) "$(DESTDIR)$(HEEKSCADDIR)/icons"
	test -z "$(DESTDIR)$(PIXMAPSDIR)" || $(mkdir_p) "$(DESTDIR)$(PIXMAPSDIR)"
	test -z "$(DESTDIR)$(APPSDIR)" || $(mkdir_p) "$(DESTDIR)$(APPSDIR)"
	strip $(EXECUTABLE)
	cp $(EXECUTABLE) $(DESTDIR)$(BINDIR)/
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
