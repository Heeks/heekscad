#CCFLAGS=-Wall -g -I "./" -I "/usr/include/opencascade" -I/home/dan/Desktop/wxGTK-2.8.9/buildgtk/lib/wx/include/gtk2-ansi-debug-2.8 -I/usr/include/wx-2.8 -D__WXGTK__ -DHEEKSCAD -D__WXDEBUG__ -DWXUSINGDLL
CCFLAGS=-Wall -g -I "./" -I "/usr/include/opencascade" -I/usr/lib/wx/include/gtk2-unicode-release-2.8 -I/usr/include/wx-2.8 -DUNICODE -D__WXGTK__ -DHEEKSCAD -DWXUSINGDLL

#WX_LIB_LETTER=d
WX_LIB_LETTER=u

OBJECTS=advprops.o BezierCurve.o Cone.o ConversionTools.o CoordinateSystem.o Cuboid.o Cylinder.o DigitizedPoint.o DigitizeMode.o DimensionDrawing.o Drawing.o dxf.o Edge.o Face.o Geom.o glfont.o GLList.o GraphicsCanvas.o Grid.o Gripper.o GripperSelTransform.o HArc.o HCircle.o HDimension.o HeeksCAD.o HeeksFrame.o HeeksPrintout.o HILine.o HImage.o History.o HLine.o HPoint.o HText.o HXml.o Images.o InputModeCanvas.o LineArcDrawing.o Loop.o MagDragWindow.o manager.o MarkedList.o ObjPropsCanvas.o odcombo.o OptionsCanvas.o Plugins.o PointDrawing.o PointOrWindow.o PropertiesCanvas.o PropertyTrsf.o PropertyVertex.o propgrid.o props.o RegularShapesDrawing.o RemoveOrAddTool.o RuledSurface.o Ruler.o SelectMode.o Shape.o ShapeData.o Sketch.o Solid.o Sphere.o StlSolid.o StretchTool.o TransformTool.o TransformTools.o TreeCanvas.o ViewPoint.o ViewRotating.o ViewZooming.o Window.o Wire.o wxImageLoader.o HeeksCADInterface.o HeeksColor.o HeeksObj.o LeftAndRight.o MarkedObject.o ObjList.o PropertyCheck.o PropertyChoice.o PropertyColor.o PropertyDouble.o PropertyInt.o PropertyList.o PropertyString.o strconv.o ToolImage.o ToolList.o tinyxml.o tinystr.o tinyxmlerror.o tinyxmlparser.o

EXECUTABLE=../HeeksCAD

${EXECUTABLE}: ${OBJECTS}
	gcc -o ${EXECUTABLE} ${OBJECTS} -lstdc++ -lGLU -lGL -lwx_gtk2${WX_LIB_LETTER}_richtext-2.8 -lwx_gtk2${WX_LIB_LETTER}_aui-2.8 -lwx_gtk2${WX_LIB_LETTER}_xrc-2.8 -lwx_gtk2${WX_LIB_LETTER}_qa-2.8 -lwx_gtk2${WX_LIB_LETTER}_html-2.8 -lwx_gtk2${WX_LIB_LETTER}_adv-2.8 -lwx_gtk2${WX_LIB_LETTER}_core-2.8 -lwx_base${WX_LIB_LETTER}_xml-2.8 -lwx_base${WX_LIB_LETTER}_net-2.8 -lwx_base${WX_LIB_LETTER}-2.8 -lwx_gtk2${WX_LIB_LETTER}_gl-2.8 -lTKVRML -lTKSTL -lTKBRep -lTKIGES -lTKShHealing -lTKSTEP -lTKSTEP209 -lTKSTEPAttr -lTKSTEPBase -lTKXSBase -lTKShapeSchema -lFWOSPlugin -lTKBool -lTKCAF -lTKCDF -lTKernel -lTKFeat -lTKFillet -lTKG2d -lTKG3d -lTKGeomAlgo -lTKGeomBase -lTKHLR -lTKMath -lTKOffset -lTKPrim -lTKPShape -lTKService -lTKTopAlgo -lTKV2d -lTKV3d -lTKMesh -lTKAdvTools -lTKBO -lTKXDESTEP -lTKXCAF -lTKXCAFSchema -lTKLCAF -lTKPLCAF

advprops.o: advprops.cpp
	gcc -c $? ${CCFLAGS} -o $@

BezierCurve.o: BezierCurve.cpp
	gcc -c $? ${CCFLAGS} -o $@

Cone.o: Cone.cpp
	gcc -c $? ${CCFLAGS} -o $@

ConversionTools.o: ConversionTools.cpp
	gcc -c $? ${CCFLAGS} -o $@

CoordinateSystem.o: CoordinateSystem.cpp
	gcc -c $? ${CCFLAGS} -o $@

Cuboid.o: Cuboid.cpp
	gcc -c $? ${CCFLAGS} -o $@

Cylinder.o: Cylinder.cpp
	gcc -c $? ${CCFLAGS} -o $@

DigitizedPoint.o: DigitizedPoint.cpp
	gcc -c $? ${CCFLAGS} -o $@

DigitizeMode.o: DigitizeMode.cpp
	gcc -c $? ${CCFLAGS} -o $@

DimensionDrawing.o: DimensionDrawing.cpp
	gcc -c $? ${CCFLAGS} -o $@

Drawing.o: Drawing.cpp
	gcc -c $? ${CCFLAGS} -o $@

dxf.o: dxf.cpp
	gcc -c $? ${CCFLAGS} -o $@

Edge.o: Edge.cpp
	gcc -c $? ${CCFLAGS} -o $@

Face.o: Face.cpp
	gcc -c $? ${CCFLAGS} -o $@

Geom.o: Geom.cpp
	gcc -c $? ${CCFLAGS} -o $@

glfont.o: glfont.cpp
	gcc -c $? ${CCFLAGS} -o $@

GLList.o: GLList.cpp
	gcc -c $? ${CCFLAGS} -o $@

GraphicsCanvas.o: GraphicsCanvas.cpp
	gcc -c $? ${CCFLAGS} -o $@

Grid.o: Grid.cpp
	gcc -c $? ${CCFLAGS} -o $@

Gripper.o: Gripper.cpp
	gcc -c $? ${CCFLAGS} -o $@

GripperSelTransform.o: GripperSelTransform.cpp
	gcc -c $? ${CCFLAGS} -o $@

HArc.o: HArc.cpp
	gcc -c $? ${CCFLAGS} -o $@

HCircle.o: HCircle.cpp
	gcc -c $? ${CCFLAGS} -o $@

HDimension.o: HDimension.cpp
	gcc -c $? ${CCFLAGS} -o $@

HeeksCAD.o: HeeksCAD.cpp
	gcc -c $? ${CCFLAGS} -o $@

HeeksFrame.o: HeeksFrame.cpp
	gcc -c $? ${CCFLAGS} -o $@

HeeksPrintout.o: HeeksPrintout.cpp
	gcc -c $? ${CCFLAGS} -o $@

HILine.o: HILine.cpp
	gcc -c $? ${CCFLAGS} -o $@

HImage.o: HImage.cpp
	gcc -c $? ${CCFLAGS} -o $@

History.o: History.cpp
	gcc -c $? ${CCFLAGS} -o $@

HLine.o: HLine.cpp
	gcc -c $? ${CCFLAGS} -o $@

HPoint.o: HPoint.cpp
	gcc -c $? ${CCFLAGS} -o $@

HText.o: HText.cpp
	gcc -c $? ${CCFLAGS} -o $@

HXml.o: HXml.cpp
	gcc -c $? ${CCFLAGS} -o $@

Images.o: Images.cpp
	gcc -c $? ${CCFLAGS} -o $@

InputModeCanvas.o: InputModeCanvas.cpp
	gcc -c $? ${CCFLAGS} -o $@

LineArcDrawing.o: LineArcDrawing.cpp
	gcc -c $? ${CCFLAGS} -o $@

Loop.o: Loop.cpp
	gcc -c $? ${CCFLAGS} -o $@

MagDragWindow.o: MagDragWindow.cpp
	gcc -c $? ${CCFLAGS} -o $@

manager.o: manager.cpp
	gcc -c $? ${CCFLAGS} -o $@

MarkedList.o: MarkedList.cpp
	gcc -c $? ${CCFLAGS} -o $@

ObjPropsCanvas.o: ObjPropsCanvas.cpp
	gcc -c $? ${CCFLAGS} -o $@

odcombo.o: odcombo.cpp
	gcc -c $? ${CCFLAGS} -o $@

OptionsCanvas.o: OptionsCanvas.cpp
	gcc -c $? ${CCFLAGS} -o $@

Plugins.o: Plugins.cpp
	gcc -c $? ${CCFLAGS} -o $@

PointDrawing.o: PointDrawing.cpp
	gcc -c $? ${CCFLAGS} -o $@

PointOrWindow.o: PointOrWindow.cpp
	gcc -c $? ${CCFLAGS} -o $@

PropertiesCanvas.o: PropertiesCanvas.cpp
	gcc -c $? ${CCFLAGS} -o $@

PropertyTrsf.o: PropertyTrsf.cpp
	gcc -c $? ${CCFLAGS} -o $@

propgrid.o: propgrid.cpp
	gcc -c $? ${CCFLAGS} -o $@

props.o: props.cpp
	gcc -c $? ${CCFLAGS} -o $@

RegularShapesDrawing.o: RegularShapesDrawing.cpp
	gcc -c $? ${CCFLAGS} -o $@

RemoveOrAddTool.o: RemoveOrAddTool.cpp
	gcc -c $? ${CCFLAGS} -o $@

RuledSurface.o: RuledSurface.cpp
	gcc -c $? ${CCFLAGS} -o $@

Ruler.o: Ruler.cpp
	gcc -c $? ${CCFLAGS} -o $@

SelectMode.o: SelectMode.cpp
	gcc -c $? ${CCFLAGS} -o $@

Shape.o: Shape.cpp
	gcc -c $? ${CCFLAGS} -o $@

ShapeData.o: ShapeData.cpp
	gcc -c $? ${CCFLAGS} -o $@

Sketch.o: Sketch.cpp
	gcc -c $? ${CCFLAGS} -o $@

Solid.o: Solid.cpp
	gcc -c $? ${CCFLAGS} -o $@

Sphere.o: Sphere.cpp
	gcc -c $? ${CCFLAGS} -o $@

StlSolid.o: StlSolid.cpp
	gcc -c $? ${CCFLAGS} -o $@

StretchTool.o: StretchTool.cpp
	gcc -c $? ${CCFLAGS} -o $@

TransformTool.o: TransformTool.cpp
	gcc -c $? ${CCFLAGS} -o $@

TransformTools.o: TransformTools.cpp
	gcc -c $? ${CCFLAGS} -o $@

TreeCanvas.o: TreeCanvas.cpp
	gcc -c $? ${CCFLAGS} -o $@

ViewPoint.o: ViewPoint.cpp
	gcc -c $? ${CCFLAGS} -o $@

ViewRotating.o: ViewRotating.cpp
	gcc -c $? ${CCFLAGS} -o $@

ViewZooming.o: ViewZooming.cpp
	gcc -c $? ${CCFLAGS} -o $@

Window.o: Window.cpp
	gcc -c $? ${CCFLAGS} -o $@

Wire.o: Wire.cpp
	gcc -c $? ${CCFLAGS} -o $@

wxImageLoader.o: wxImageLoader.cpp
	gcc -c $? ${CCFLAGS} -o $@

HeeksCADInterface.o: ../interface/HeeksCADInterface.cpp
	gcc -c $? ${CCFLAGS} -o $@

HeeksColor.o: ../interface/HeeksColor.cpp
	gcc -c $? ${CCFLAGS} -o $@

HeeksObj.o: ../interface/HeeksObj.cpp
	gcc -c $? ${CCFLAGS} -o $@

LeftAndRight.o: ../interface/LeftAndRight.cpp
	gcc -c $? ${CCFLAGS} -o $@

MarkedObject.o: ../interface/MarkedObject.cpp
	gcc -c $? ${CCFLAGS} -o $@

ObjList.o: ../interface/ObjList.cpp
	gcc -c $? ${CCFLAGS} -o $@

PropertyCheck.o: ../interface/PropertyCheck.cpp
	gcc -c $? ${CCFLAGS} -o $@

PropertyChoice.o: ../interface/PropertyChoice.cpp
	gcc -c $? ${CCFLAGS} -o $@

PropertyColor.o: ../interface/PropertyColor.cpp
	gcc -c $? ${CCFLAGS} -o $@

PropertyDouble.o: ../interface/PropertyDouble.cpp
	gcc -c $? ${CCFLAGS} -o $@

PropertyVertex.o: ../interface/PropertyVertex.cpp
	gcc -c $? ${CCFLAGS} -o $@

PropertyInt.o: ../interface/PropertyInt.cpp
	gcc -c $? ${CCFLAGS} -o $@

PropertyList.o: ../interface/PropertyList.cpp
	gcc -c $? ${CCFLAGS} -o $@

PropertyString.o: ../interface/PropertyString.cpp
	gcc -c $? ${CCFLAGS} -o $@

strconv.o: ../interface/strconv.cpp
	gcc -c $? ${CCFLAGS} -o $@

ToolImage.o: ../interface/ToolImage.cpp
	gcc -c $? ${CCFLAGS} -o $@

ToolList.o: ../interface/ToolList.cpp
	gcc -c $? ${CCFLAGS} -o $@

tinyxml.o: ../tinyxml/tinyxml.cpp
	gcc -c $? ${CCFLAGS} -o $@

tinystr.o: ../tinyxml/tinystr.cpp
	gcc -c $? ${CCFLAGS} -o $@

tinyxmlerror.o: ../tinyxml/tinyxmlerror.cpp
	gcc -c $? ${CCFLAGS} -o $@

tinyxmlparser.o: ../tinyxml/tinyxmlparser.cpp
	gcc -c $? ${CCFLAGS} -o $@

clean:
	-rm ${EXECUTABLE} ${OBJECTS}


