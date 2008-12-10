// declares all the exported functions for HeeksCAD

class CHeeksCADInterface;

extern "C"{
#ifdef WIN32
__declspec( dllexport ) CHeeksCADInterface* __cdecl HeeksCADGetInterface(void);
#else
CHeeksCADInterface* HeeksCADGetInterface(void);
#endif
}
