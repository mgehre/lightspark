/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009-2011  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "scripting/abc.h"
#include "class.h"
#include "parsing/tags.h"

using namespace lightspark;

ASObject* lightspark::new_asobject()
{
	return Class<ASObject>::getInstanceS();
}

void Class_inherit::finalize()
{
	Class_base::finalize();
	class_scope.clear();
}

ASObject* Class_inherit::getInstance(bool construct, ASObject* const* args, const unsigned int argslen)
{
	ASObject* ret=NULL;
	assert_and_throw(!bindedToRoot);
	if(tag)
	{
		ret=tag->instance();
		assert_and_throw(ret);
	}
	else
	{
		assert_and_throw(super);
		//Our super should not construct, we are going to do it ourselves
		ret=super->getInstance(false,NULL,0);
	}
	//We override the classdef
	ret->setClass(this);
	if(construct)
		handleConstruction(ret,args,argslen,true);
	return ret;
}

void Class_inherit::buildInstanceTraits(ASObject* o) const
{
	assert_and_throw(class_index!=-1);
	//The class is declared in the script and has an index
	LOG(LOG_CALLS,_("Building instance traits"));

	context->buildInstanceTraits(o,class_index);
}

