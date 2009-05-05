// MarkedObject.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#if !defined MarkedObject_HEADER
#define MarkedObject_HEADER

class HeeksObj;

enum EnumStackedType{
	EverythingStackedType,
	TopOnlyStackedType,
	BottomOnlyStackedType,
	OneFromLevelStackedType
};

class MarkedObject{
private:
	unsigned long m_depth;
	std::map<HeeksObj*, MarkedObject*>::iterator CurrentIt;
	std::map<int, std::map<int, MarkedObject*> > m_types;
	int m_window_size;
	bool m_processed;
	EnumStackedType m_stacked_type;
	HeeksObj* m_object;

	virtual bool single_type() = 0;

public:
	std::map<HeeksObj*, MarkedObject*> m_map;

	MarkedObject();
	MarkedObject(unsigned long depth, HeeksObj* object, int window_size);
	MarkedObject(const MarkedObject &f);
	virtual ~MarkedObject();

	const MarkedObject &operator=(const MarkedObject &rhs);
	unsigned long GetDepth();
	int GetWindowSize(){return m_window_size;}
	HeeksObj* GetObject(){return m_object;}
	void Clear();
	MarkedObject* Add(HeeksObj* object, unsigned long z_depth, int window_size);
	virtual void SetFirst(EnumStackedType stacked_type);
	virtual HeeksObj* GetFirstOfEverything();
	virtual HeeksObj* GetFirstOfTopOnly();
	virtual HeeksObj* GetFirstOfBottomOnly();
	virtual HeeksObj* GetFirstOfOneFromLevel();
	virtual HeeksObj* Increment();
};

class MarkedObjectOneOfEach:public MarkedObject{
	bool single_type(){return true;}

public:
	MarkedObjectOneOfEach():MarkedObject(){}
	MarkedObjectOneOfEach(unsigned long z, HeeksObj* object, int window_size):MarkedObject(z, object, window_size){}
};


class MarkedObjectManyOfSame: public MarkedObject{
	bool single_type(){return false;}

public:
	MarkedObjectManyOfSame():MarkedObject(){}
	MarkedObjectManyOfSame(unsigned long z, HeeksObj* object, int window_size):MarkedObject(z, object, window_size){}
};

#endif
