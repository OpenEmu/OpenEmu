/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Version Macros.                          *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef __MDP_VERSION_H
#define __MDP_VERSION_H

/**
 * MDP_VERSION(): Create an MDP version number.
 * @param major Major version number.
 * @param minor Minor version number.
 * @param revision Revision number.
 * @return MDP version number. (uint32_t)
 */
#define MDP_VERSION(major, minor, revision) \
	((((major) & 0xFF) << 24) | (((minor) & 0xFF) << 16) | ((revision) & 0xFFFF))

/**
 * MDP_VERSION_MAJOR(): Extract the major version number from an MDP version number.
 * @param mdp_version MDP version number.
 * @return Major version number. (uint8_t)
 */
#define MDP_VERSION_MAJOR(mdp_version)		(((mdp_version) >> 24) & 0xFF)

/**
 * MDP_VERSION_MINOR(): Extract the minor version number from an MDP version number.
 * @param mdp_version MDP version number.
 * @return Minor version number. (uint8_t)
 */
#define MDP_VERSION_MINOR(mdp_version)		(((mdp_version) >> 16) & 0xFF)

/**
 * MDP_VERSION_REVISION(): Extract the revision number from an MDP version number.
 * @param mdp_version MDP version number.
 * @return Revision number. (uint16_t)
 */
#define MDP_VERSION_REVISION(mdp_version)	((mdp_version) & 0xFFFF)

#endif /* __MDP_VERSION_H */
