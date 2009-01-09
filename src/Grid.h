// Grid.h

#pragma once

class CViewPoint;
class CBox;

extern void RenderGrid(const CViewPoint *view_point);
extern void GetGridBox(const CViewPoint *view_point, CBox &ext);

