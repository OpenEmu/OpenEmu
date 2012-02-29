// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009-2011 Philippe Daouadi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "ameteor/audio/dsound.hpp"
#include "../globals.hpp"
#include <cstring>

namespace AMeteor
{
	namespace Audio
	{
		DSound::DSound () :
			m_rpos(0),
			m_wpos(0),
			m_size(0)
		{
			std::memset(m_buffer, 0, sizeof(m_buffer));
		}

		void DSound::FillFifo (int8_t* buffer)
		{
			int8_t* pmbuf = m_buffer + m_wpos;
			// we copy 16 bytes of data
			for (int8_t* pbuf = buffer;
					pbuf < buffer + BUFFER_SIZE/2 && m_size < 32; ++pbuf, ++pmbuf)
			{
				if (pmbuf >= m_buffer + BUFFER_SIZE)
					pmbuf = m_buffer;

				*pmbuf = *pbuf;
				++m_size;
			}

			m_wpos = pmbuf - m_buffer;
		}

		void DSound::FillFifo (int8_t sample)
		{
			if (m_size == 32)
				return;
			if (m_wpos == BUFFER_SIZE)
				m_wpos = 0;
			m_buffer[m_wpos++] = sample;
			++m_size;
		}

		bool DSound::SaveState (std::ostream& stream)
		{
			SS_WRITE_VAR(m_rpos);
			SS_WRITE_VAR(m_wpos);
			SS_WRITE_VAR(m_size);

			SS_WRITE_ARRAY(m_buffer);

			return true;
		}

		bool DSound::LoadState (std::istream& stream)
		{
			SS_READ_VAR(m_rpos);
			SS_READ_VAR(m_wpos);
			SS_READ_VAR(m_size);

			SS_READ_ARRAY(m_buffer);

			return true;
		}
	}
}
