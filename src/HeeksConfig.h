// HeeksConfig.h
#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>

class HeeksConfig: public wxConfig
{
public:
	HeeksConfig():wxConfig(_T("HeeksCAD")){}
	~HeeksConfig(){}
};
