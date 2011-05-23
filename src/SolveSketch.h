// SolveSketch.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#ifdef MULTIPLE_OWNERS
void SolveSketch(CSketch* sketch);
void SolveSketch(CSketch* sketch,HeeksObj* dragged, void* whichpoint);
#endif
