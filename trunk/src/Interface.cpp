// defines all the exported functions for HeeksCAD
#include "stdafx.h"

#include "Interface.h"
#include "../interface/HeeksCADInterface.h"

CHeeksCADInterface heekscad_interface;

CHeeksCADInterface* HeeksCADGetInterface(void){
	return &heekscad_interface;
}
