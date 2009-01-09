// Images.h

#if !defined Images_HEADER
#define Images_HEADER

#include <wx/imaglist.h>

class Images{
protected:
	std::map<int, int> image_map; // maps object type to image index
	bool InitializeImageList(int width, int height);

public:
	wxImageList *m_image_list;

	Images();
	virtual ~Images() {}

	int GetImage(HeeksObj *object);
};

#endif
