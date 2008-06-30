// Images.h

#if !defined Images_HEADER
#define Images_HEADER

#include <wx/imaglist.h>

class Images{
protected:
	std::map<wxIcon*, int> image_map;
	bool InitializeImageList(int width, int height);
	int Add(wxIcon* hicon);

public:
	wxImageList *m_image_list;

	Images();
	virtual ~Images() {}

	int GetImage(HeeksObj *object);
};

#endif
