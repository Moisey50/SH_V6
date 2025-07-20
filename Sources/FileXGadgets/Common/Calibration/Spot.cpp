#include "stdafx.h"
#include "Spot.h"

Spot::Spot(unsigned int Idx, const double X, const double Y, const double Width, const double Height) :
	idx(Idx), width(Width), height(Height) 
{
	centerGrid.set(X,Y);
	size = width*height;
}
Spot::Spot(const Spot& spot)
{
	Spot(spot.idx, spot.centerGrid.x, spot.centerGrid.y, spot.width, spot.height);
}
void Spot::setArbitrage(double x, double y)
{
	arbitrage.set(x,y);
}
Spot& Spot::operator=(const Spot& other)
{
	idx = other.idx;
	centerGrid = other.centerGrid;
	width = other.width;
	height = other.height;
	size = other.size;
	gridX = other.gridX;
	gridY = other.gridY;
	arbitrage = other.arbitrage;

	return *this;
}






