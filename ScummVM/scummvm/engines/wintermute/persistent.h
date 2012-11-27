/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_PERSISTENT_H
#define WINTERMUTE_PERSISTENT_H

namespace Wintermute {

class BasePersistenceManager;

// persistence support
typedef void *(*PERSISTBUILD)(void);
typedef bool(*PERSISTLOAD)(void *, BasePersistenceManager *);
typedef void (*SYS_INSTANCE_CALLBACK)(void *Instance, void *Data);
} // end of namespace Wintermute

#include "engines/wintermute/system/sys_class_registry.h"
namespace Wintermute {


#define DECLARE_PERSISTENT(class_name, parent_class)\
	static const char _className[];\
	static void* persistBuild(void);\
	virtual const char* getClassName();\
	static bool persistLoad(void* Instance, BasePersistenceManager* PersistMgr);\
	class_name(TDynamicConstructor p1, TDynamicConstructor p2) :parent_class(p1, p2){ /*memset(this, 0, sizeof(class_name));*/ };\
	virtual bool persist(BasePersistenceManager* PersistMgr);\
	void* operator new (size_t size);\
	void operator delete(void* p);\


#define IMPLEMENT_PERSISTENT(class_name, persistent_class)\
	const char class_name::_className[] = #class_name;\
	void* class_name::persistBuild(){\
		return ::new class_name(DYNAMIC_CONSTRUCTOR, DYNAMIC_CONSTRUCTOR);\
	}\
	\
	bool class_name::persistLoad(void* Instance, BasePersistenceManager* PersistMgr){\
		return ((class_name*)Instance)->persist(PersistMgr);\
	}\
	\
	const char* class_name::getClassName(){\
		return #class_name;\
	}\
	\
	/*SystemClass Register##class_name(class_name::_className, class_name::PersistBuild, class_name::PersistLoad, persistent_class);*/\
	\
	void* class_name::operator new (size_t size){\
		void* ret = ::operator new(size);\
		SystemClassRegistry::getInstance()->registerInstance(#class_name, ret);\
		return ret;\
	}\
	\
	void class_name::operator delete (void* p){\
		SystemClassRegistry::getInstance()->unregisterInstance(#class_name, p);\
		::operator delete(p);\
	}\

#define TMEMBER(member_name) #member_name, &member_name
#define TMEMBER_INT(member_name) #member_name, (int*)&member_name

} // end of namespace Wintermute

#endif
