/* MapShipyardPanel.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "MapShipyardPanel.h"

#include "Format.h"
#include "GameData.h"
#include "Planet.h"
#include "PlayerInfo.h"
#include "Point.h"
#include "Screen.h"
#include "Ship.h"
#include "Sprite.h"
#include "StellarObject.h"
#include "System.h"

#include <algorithm>
#include <set>

using namespace std;



MapShipyardPanel::MapShipyardPanel(PlayerInfo &player)
	: MapSalesPanel(player, false)
{
	Init();
}



MapShipyardPanel::MapShipyardPanel(const MapPanel &panel)
	: MapSalesPanel(panel, false)
{
	Init();
}



const Sprite *MapShipyardPanel::SelectedSprite() const
{
	return selected ? selected->GetSprite().GetSprite() : nullptr;
}



const Sprite *MapShipyardPanel::CompareSprite() const
{
	return compare ? compare->GetSprite().GetSprite() : nullptr;
}



const ItemInfoDisplay &MapShipyardPanel::SelectedInfo() const
{
	return selectedInfo;
}



const ItemInfoDisplay &MapShipyardPanel::CompareInfo() const
{
	return compareInfo;
}



void MapShipyardPanel::Select(int index)
{
	if(index < 0 || index >= static_cast<int>(list.size()))
		selected = nullptr;
	else
	{
		selected = list[index];
		selectedInfo.Update(*selected);
	}
}



void MapShipyardPanel::Compare(int index)
{
	if(index < 0 || index >= static_cast<int>(list.size()))
		compare = nullptr;
	else
	{
		compare = list[index];
		compareInfo.Update(*compare);
	}
}



bool MapShipyardPanel::HasAny(const Planet *planet) const
{
	return !planet->Shipyard().empty();
}



bool MapShipyardPanel::HasThis(const Planet *planet) const
{
	return planet->Shipyard().Has(selected);
}



void MapShipyardPanel::DrawItems() const
{
	list.clear();
	Point corner = Screen::TopLeft() + Point(0, scroll);
	for(const string &category : categories)
	{
		auto it = catalog.find(category);
		if(it == catalog.end())
			continue;
		
		// Draw the header. If this category is collapsed, skip drawing the items.
		if(DrawHeader(corner, category))
			continue;
		
		for(const Ship *ship : it->second)
		{
			string price = Format::Number(ship->Cost()) + " credits";
			
			string info = Format::Number(ship->Attributes().Get("shields")) + " shields / ";
			info += Format::Number(ship->Attributes().Get("hull")) + " hull";
			
			bool isForSale = true;
			if(selectedSystem)
			{
				isForSale = false;
				for(const StellarObject &object : selectedSystem->Objects())
					if(object.GetPlanet() && object.GetPlanet()->Shipyard().Has(ship))
					{
						isForSale = true;
						break;
					}
			}
			
			Draw(corner, ship->GetSprite().GetSprite(), isForSale, ship == selected,
				ship->ModelName(), price, info);
			list.push_back(ship);
		}
	}
	maxScroll = corner.Y() - scroll - .5 * Screen::Height();
}



void MapShipyardPanel::Init()
{
	catalog.clear();
	set<const Ship *> seen;
	for(const auto &it : GameData::Planets())
		if(player.HasVisited(it.second.GetSystem()))
			for(const Ship *ship : it.second.Shipyard())
				if(seen.find(ship) == seen.end())
				{
					catalog[ship->Attributes().Category()].push_back(ship);
					seen.insert(ship);
				}
	
	for(auto &it : catalog)
		sort(it.second.begin(), it.second.end(),
			[](const Ship *a, const Ship *b) {return a->ModelName() < b->ModelName();});
}
