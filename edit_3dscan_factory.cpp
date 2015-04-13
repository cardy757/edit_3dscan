/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005-2008                                           \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#include "edit_3dscan_factory.h"
#include "edit_3dscan.h"

Edit3DScanFactory::Edit3DScanFactory()
{
	edit3DScan = new QAction(QIcon(":/images/icon_info.png"), "3D Scan", this);
	
	actionList << edit3DScan;
	
	foreach(QAction *editAction, actionList)
		editAction->setCheckable(true);

	plugin = NULL;
}

Edit3DScanFactory::~Edit3DScanFactory()
{
	delete edit3DScan;

	if (plugin)	delete plugin;
}

//gets a list of actions available from this plugin
QList<QAction *> Edit3DScanFactory::actions() const
{
	return actionList;
}

//get the edit tool for the given action
MeshEditInterface* Edit3DScanFactory::getMeshEditInterface(QAction *action)
{
	if (action == edit3DScan)
	{
		plugin = new Edit3DScanPlugin();
		return plugin;
	} else assert(0); //should never be asked for an action that isnt here
}

QString Edit3DScanFactory::getEditToolDescription(QAction *)
{
	return Edit3DScanPlugin::Info();
}

MESHLAB_PLUGIN_NAME_EXPORTER(Edit3DScanFactory)
