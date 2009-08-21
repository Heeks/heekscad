#define UNITTEST_NO_HEEKS

#include "../src/Polygon.h"

template <class T>
void PrintPolygons(T first, T last)
{
    for(T ipoly = first; ipoly != last; ipoly++)
	{
		ipoly->Print();
	}
}

int main(int argc, char *argv[])
{
	std::vector<LineSegment> lines_vector;
	lines_vector.push_back(LineSegment(-2,3,3,4));
	lines_vector.push_back(LineSegment(-4,3,-2,-1));
	lines_vector.push_back(LineSegment(-4,-1,-1,1));
	lines_vector.push_back(LineSegment(-2,-1,2,-1));
	lines_vector.push_back(LineSegment(-4,0.5,-1,-0.5));
	lines_vector.push_back(LineSegment(-3.5,2,1.5,-1));
	lines_vector.push_back(LineSegment(-4,0.5,4,0.5));
	lines_vector.push_back(LineSegment(6,1,8,1));
	lines_vector.push_back(LineSegment(7,4,6,-2));
	lines_vector.push_back(LineSegment(9,1,7,1));
	lines_vector.push_back(LineSegment(1,-4,1,5));
	lines_vector.push_back(LineSegment(6,5,1,5));
	lines_vector.push_back(LineSegment(1,-2,1,-3));
	lines_vector.push_back(LineSegment(4,3,3,2));
	//SweepLine(lines_vector);
	std::list<Polygon> result_list;
	UnionPolygons(lines_vector, result_list);
	return 0;
}

