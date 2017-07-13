// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "defines.h" // IWYU pragma: keep
#include "CollisionDetection.h"
#include "Rect.h"
#include <cstdlib>

///////////////////////////////////////////////////////////////////////////////

bool IsPointInRect(const Point<int>& pt, const Rect& rect)
{
    return (pt.x >= rect.left && pt.x <= rect.right && pt.y >= rect.top && pt.y <= rect.bottom);
}

bool Coll(const int x, const int y, const Rect& rect)
{
    return IsPointInRect(Point<int>(x, y), rect);
}

bool Coll(const int x, const int y, const int rx, const int ry, const int rwidth, const int rheight)
{
    return Coll(x, y, Rect(rx, ry, rwidth, rheight));
}


bool Coll(const int left1, const int right1, const int left2, const int right2)
{
    return( (left1 >= left2 && left1 <= right2) || (right1 >= left2 && right1 <= right2) || (left2 >= left1 && left2 <= right1) || (right2 >= left1 && right2 <= right1) );
}


bool CollEdges(const Rect& rect1, const Rect& rect2)
{
    return ( Coll(rect1.left, rect1.top, rect2) || Coll(rect1.right, rect1.top, rect2) ||
             Coll(rect1.left, rect1.bottom, rect2) || Coll(rect1.right, rect1.bottom, rect2) ||
             Coll(rect2.left, rect2.top, rect1) || Coll(rect2.right, rect2.top, rect1) ||
             Coll(rect2.left, rect2.bottom, rect1) || Coll(rect2.right, rect2.bottom, rect1) );
}



bool Coll(const Rect& rect1, const Rect& rect2)
{
    // Radius berechnen
    int Radius1X = (rect1.right - rect1.left) / 2;
    int Radius1Y = (rect1.bottom - rect1.top) / 2;

    int Radius2X = (rect2.right - rect2.left) / 2;
    int Radius2Y = (rect2.bottom - rect2.top) / 2;

    // die Mitte der Bounding Box berechnen
    int Center1X = rect1.left + Radius1X;
    int Center1Y = rect1.top + Radius1Y;

    int Center2X = rect2.left + Radius2X;
    int Center2Y = rect2.top + Radius2Y;

    // Abstand berechnen
    int OffsetX = std::abs(Center1X - Center2X);
    int OffsetY = std::abs(Center1Y - Center2Y);

    return CollEdges(rect1, rect2) || (OffsetX < Radius1X + Radius2X && OffsetY < Radius1Y + Radius2Y);
}


