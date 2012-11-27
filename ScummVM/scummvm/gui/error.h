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

#ifndef GUI_ERROR_H
#define GUI_ERROR_H

#include "common/error.h"

namespace GUI {

/**
 * Displays an error dialog for some error code.
 *
 * @param error error code
 * @param extraText extra text to be displayed in addition to default string description(optional)
 */
void displayErrorDialog(const Common::Error &error, const char *extraText = "");

/**
 * Displays an error dialog for a given message.
 *
 * @param text message to be displayed
 */
void displayErrorDialog(const char *text);

} // End of namespace GUI

#endif //GUI_ERROR_H
