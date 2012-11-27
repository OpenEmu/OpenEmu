#!/bin/sh

if [ $# -ne 5 ]; then
	echo "usage: $0 configure plugin template versioncode target"
	exit 1
fi

CONFIGURE=$1
PLUGIN_NAME=$2
TEMPLATE=$3
PLUGIN_VERSION_CODE=$4
TARGET=$5

PLUGIN_DESC=`sed -n "s/add_engine\s$PLUGIN_NAME\s\"\(.\+\)\"\s.*/\1/p" < $CONFIGURE`

sed "s|@PLUGIN_NAME@|$PLUGIN_NAME|;s|@PLUGIN_VERSION_CODE@|$PLUGIN_VERSION_CODE|;s|@PLUGIN_DESC@|$PLUGIN_DESC|" < $TEMPLATE > $TARGET
