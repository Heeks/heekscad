// declares all the exported functions for HeeksCAD

class CHeeksCADInterface;

extern "C"{
__declspec( dllexport ) CHeeksCADInterface* __cdecl HeeksCADGetInterface(void);
}
