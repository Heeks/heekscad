// Images.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#if !defined Images_HEADER
#define Images_HEADER


class Images{
protected:
	std::map<wxString, int> image_map; // maps icon string to image index
	bool InitializeImageList(int width, int height);

public:
	wxImageList *m_image_list;

	Images();
	virtual ~Images() {}

	int GetImage(HeeksObj *object);
};

#endif
