// SketchOrder.h

#pragma once

enum SketchOrderType
{
	SketchOrderTypeUnknown,
	SketchOrderTypeEmpty,	// an empty sketch
	SketchOrderTypeOpen,	// a well ordered sketch, which doesn't have it's start and end touching
	SketchOrderTypeReverse,	// reverse the sketch, only used as a function, won't be returned by GetSketchOrder()
	SketchOrderTypeBad,	// "badly ordered" ( where the start of one span doesn't line up with the end of the previous span )
	SketchOrderTypeReOrder,	// re-order the sketch, only used as a function, won't be returned by GetSketchOrder()
	SketchOrderTypeCloseCW,	// a well ordered sketch, which is closed and clockwise
	SketchOrderTypeCloseCCW,// a well ordered sketch, which is closed and anti-clockwise
	SketchOrderTypeMultipleCurves,  // each of the separate parts are well ordered
	SketchOrderHasCircles,  // there are circles in the sketch, they will need to be processed seperately and a new sketch made with no circles
	MaxSketchOrderTypes
};

