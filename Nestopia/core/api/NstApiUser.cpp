////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#include "NstApiUser.hpp"

namespace Nes
{
	namespace Api
	{
		User::LogCaller      User::logCallback;
		User::EventCaller    User::eventCallback;
		User::QuestionCaller User::questionCallback;
		User::FileIoCaller   User::fileIoCallback;

		const wchar_t* User::File::GetName() const throw()
		{
			return L"";
		}

		uint User::File::GetId() const throw()
		{
			return 0;
		}

		ulong User::File::GetMaxSize() const throw()
		{
			return ULONG_MAX;
		}

		Result User::File::GetContent(const void*&,ulong&) const throw()
		{
			return RESULT_ERR_NOT_READY;
		}

		Result User::File::GetContent(std::ostream&) const throw()
		{
			return RESULT_ERR_NOT_READY;
		}

		Result User::File::GetPatchContent(Patch,std::ostream&) const throw()
		{
			return RESULT_ERR_NOT_READY;
		}

		Result User::File::SetContent(const void*,ulong) throw()
		{
			return RESULT_ERR_NOT_READY;
		}

		Result User::File::SetContent(std::istream&) throw()
		{
			return RESULT_ERR_NOT_READY;
		}

		Result User::File::SetPatchContent(std::istream&) throw()
		{
			return RESULT_ERR_NOT_READY;
		}

		Result User::File::SetSampleContent(const void*,ulong,bool,uint,ulong) throw()
		{
			return RESULT_ERR_NOT_READY;
		}
	}
}
