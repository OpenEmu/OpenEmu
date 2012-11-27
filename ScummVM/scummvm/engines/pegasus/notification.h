/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_NOTIFICATION_H
#define PEGASUS_NOTIFICATION_H

#include "common/array.h"
#include "common/list.h"

#include "pegasus/types.h"
#include "pegasus/util.h"

namespace Pegasus {

class NotificationManager;
class NotificationReceiver;

struct ReceiverEntry {
	NotificationReceiver *receiver;
	NotificationFlags mask;
};

int operator==(const ReceiverEntry &entry1, const ReceiverEntry &entry2);
int operator!=(const ReceiverEntry &entry1, const ReceiverEntry &entry2);

typedef Common::Array<ReceiverEntry> ReceiverList;

/*
	A notification can have 32 flags associated with it, which can be user-defined.
*/

class Notification : public IDObject {
friend class NotificationManager;

public:
	Notification(const NotificationID id, NotificationManager *owner);
	virtual ~Notification();

	// notifyMe will have this receiver notified when any of the specified notification
	// flags are set.
	// If there is already a notification set for this receiver, notifyMe does a bitwise
	// OR with the receiver's current notification flags.

	// Can selectively set or clear notification bits by using the flags and mask argument.

	void notifyMe(NotificationReceiver*, NotificationFlags flags, NotificationFlags mask);
	void cancelNotification(NotificationReceiver *receiver);

	void setNotificationFlags(NotificationFlags flags,	NotificationFlags mask);
	NotificationFlags getNotificationFlags() { return _currentFlags; }

	void clearNotificationFlags() { setNotificationFlags(0, ~(NotificationFlags)0); }

protected:
	void checkReceivers();

	NotificationManager *_owner;
	ReceiverList _receivers;
	NotificationFlags _currentFlags;
};

class NotificationReceiver {
friend class Notification;

public:
	NotificationReceiver();
	virtual ~NotificationReceiver();

protected:
	// receiveNotification is called automatically whenever a notification that this
	// receiver depends on has its flags set

	virtual void receiveNotification(Notification *, const NotificationFlags);
	virtual void newNotification(Notification *notification);

private:
	Notification *_notification;
};

typedef Common::List<Notification *> NotificationList;

class NotificationManager : public NotificationReceiver {
friend class Notification;

public:
	NotificationManager();
	virtual ~NotificationManager();

	void checkNotifications();

protected:
	void addNotification(Notification *notification);
	void removeNotification(Notification *notification);
	void detachNotifications();

	NotificationList _notifications;
};

} // End of namespace Pegasus

#endif
