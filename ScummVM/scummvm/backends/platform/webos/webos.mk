# WebOS specific build targets
# ============================================================================
#
# Build instructions:
#
# 1. Install the WebOS SDK and PDK and setup the environment variables
#    WEBOS_SDK and WEBOS_PDK accordingly.
#
# 2. Cross-compile zlib, flac, mad and tremor and install it into the PDK.
#
# 3. Prepare the ScummVM source for a webOS build:
#    $ ./configure --host=webos --enable-plugins --default-dynamic \
#          --enable-release
#
# 4. Create the package:
#    $ make package
#
# The package is now in the "portdist" folder.
#
# See http://wiki.scummvm.org/index.php/Compiling_ScummVM/WebOS for
# more detailed build instructions.
#
#
# Palm App catalog instructions:
#
# VER_PACKAGE must be set to a number which is higher than the currently
# used package version in the app catalog.  So when creating an updated
# package for ScummVM 1.3.9 and the current ScummVM package in the app
# catalog is version 1.3.0902 then you must specify VER_PACKAGE=3 to create
# the ScummVM package with version 1.3.0903.  Yeah, I know that's ugly but
# WebOS package version numbers are restricted to three numeric components.
#
# As long as Palm doesn't support Team-maintained apps the uploaded packages
# MUST NOT be packaged with the default "org.scummvm" base id.  Instead apps
# must be uploaded with a user-specific base id.  A good practice is using
# the github user as base id: com.github.<username>.  It is also necessary
# to use a user-specific app name when submitting the created package to the
# Palm app catalog.  Use "ScummVM (<username>)" instead of "ScummVM" and
# "ScummVM Beta (<username>)" instead of "ScummVM Beta".
#
# The app id is automatically parsed from the installation prefix.  So add a
# configure parameter like this to prepare a build of a package for the Palm
# App Catalog:
#
#   --prefix=/media/cryptofs/apps/usr/palm/applications/com.github.kayahr.scummvm
#
# To build a package for the Palm Beta App Catalog add "-beta" to the prefix:
#
#   --prefix=/media/cryptofs/apps/usr/palm/applications/com.github.kayahr.scummvm-beta
# ============================================================================

# Increment this number when the packaging of the app has been changed while
# ScummVM itself has the same version as before. The number can be reset to 
# 0 when the ScummVM version is increased.
VER_PACKAGE = 0

PATH_DIST = $(srcdir)/dists/webos
PATH_MOJO = $(PATH_DIST)/mojo
APP_ID = $(shell basename $(prefix))
APP_VERSION = $(shell printf "%d.%d.%02d%02d" $(VER_MAJOR) $(VER_MINOR) $(VER_PATCH) $(VER_PACKAGE))
DESTDIR ?= staging
PORTDISTDIR ?= portdist
ifeq ($(HOST_COMPILER),Darwin)
	SED_DASH_I = "-i \"\""
else
	SED_DASH_I = "-i"
endif

install: all
	$(QUIET)$(INSTALL) -d "$(DESTDIR)$(prefix)"
ifeq ($(HOST_COMPILER),Darwin)
	$(QUIET)$(INSTALL) -m 0644 "$(PATH_MOJO)/"* "$(DESTDIR)$(prefix)/"
else
	$(QUIET)$(INSTALL) -m 0644 -t "$(DESTDIR)$(prefix)/" "$(PATH_MOJO)/"*
endif
	$(QUIET)$(INSTALL) -m 0755 "$(PATH_MOJO)/start" "$(DESTDIR)$(prefix)/"
	$(QUIET)$(INSTALL) -d "$(DESTDIR)$(bindir)"
	$(QUIET)$(INSTALL) -c -m 755 "./$(EXECUTABLE)" "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	$(QUIET)$(STRIP) "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	$(QUIET)$(INSTALL) -d "$(DESTDIR)$(docdir)"
	$(QUIET)$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) "$(DESTDIR)$(docdir)"
	$(QUIET)$(INSTALL) -d "$(DESTDIR)$(datadir)"
	$(QUIET)$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) "$(DESTDIR)$(datadir)/"
ifdef DYNAMIC_MODULES
	$(QUIET)$(INSTALL) -d "$(DESTDIR)$(libdir)/"
	$(QUIET)$(INSTALL) -c -m 644 $(PLUGINS) "$(DESTDIR)$(libdir)/"
	$(QUIET)$(STRIP) "$(DESTDIR)$(libdir)/"*
endif
	$(QUIET)sed $(SED_DASH_I) s/'APP_VERSION'/'$(APP_VERSION)'/ "$(DESTDIR)$(prefix)/appinfo.json"
	$(QUIET)sed $(SED_DASH_I) s/'APP_ID'/'$(APP_ID)'/ "$(DESTDIR)$(prefix)/appinfo.json"
ifneq (,$(findstring -beta,$(APP_ID)))
	$(QUIET)sed $(SED_DASH_I) s/'APP_TITLE'/'ScummVM Beta'/ "$(DESTDIR)$(prefix)/appinfo.json"
else
	$(QUIET)sed $(SED_DASH_I) s/'APP_TITLE'/'ScummVM'/ "$(DESTDIR)$(prefix)/appinfo.json"
endif

uninstall:
	$(QUIET)$(RM_REC) "$(DESTDIR)$(prefix)"

package: uninstall install
	$(QUIET)$(RM_REC) "$(PORTDISTDIR)"
	$(QUIET)$(MKDIR) "$(PORTDISTDIR)"
	$(QUIET)$(WEBOS_SDK)/bin/palm-package --use-v1-format "$(DESTDIR)$(prefix)" -o "$(PORTDISTDIR)"

.PHONY: install uninstall package
