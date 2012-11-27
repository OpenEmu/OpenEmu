###############################################
# Common build rules, used by the sub modules and their module.mk files
#
###############################################


# Copy the list of objects to a new variable. The name of the new variable
# contains the module name, a trick we use so we can keep multiple different
# module object lists, one for each module.
MODULE_OBJS-$(MODULE) := $(addprefix $(MODULE)/, $(MODULE_OBJS))

# Add all involved directories to the MODULE_DIRS list
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS-$(MODULE))))



ifdef TOOL_EXECUTABLE
################################################
# Build rule for (tool) executables.
# TODO: Refactor this, so that even our master executable can use this rule?
################################################
TOOL-$(MODULE) := $(MODULE)/$(TOOL_EXECUTABLE)$(EXEEXT)
$(TOOL-$(MODULE)): $(MODULE_OBJS-$(MODULE)) $(TOOL_DEPS)
	$(QUIET_CXX)$(CXX) $(LDFLAGS) $+ -o $@

# Reset TOOL_* vars
TOOL_EXECUTABLE:=
TOOL_DEPS:=

# Add to "devtools" target
devtools: $(TOOL-$(MODULE))

# Pseudo target for comfort, allows for "make devtools/skycpt", etc.
$(MODULE): $(TOOL-$(MODULE))
clean-devtools: clean-$(MODULE)

else
ifdef PLUGIN
################################################
# Build rule for dynamic (loadable) plugins
################################################
PLUGIN-$(MODULE) := plugins/$(PLUGIN_PREFIX)$(notdir $(MODULE))$(PLUGIN_SUFFIX)
$(PLUGIN-$(MODULE)): $(MODULE_OBJS-$(MODULE)) $(PLUGIN_EXTRA_DEPS)
	$(QUIET)$(MKDIR) plugins
	$(QUIET_PLUGIN)$(CXX) $(filter-out $(PLUGIN_EXTRA_DEPS),$+) $(PLUGIN_LDFLAGS) -o $@

# Reset PLUGIN var
PLUGIN:=

# Add to "plugins" target
plugins: $(PLUGIN-$(MODULE))

# Add to the PLUGINS variable
PLUGINS += $(PLUGIN-$(MODULE))

# Pseudo target for comfort, allows for "make common", "make gui" etc.
$(MODULE): $(PLUGIN-$(MODULE))
clean-plugins: clean-$(MODULE)

else
################################################
# Build rule for static modules/plugins
################################################
MODULE_LIB-$(MODULE) := $(MODULE)/lib$(notdir $(MODULE)).a

# If not building as a plugin, add the object files to the main OBJS list
OBJS += $(MODULE_LIB-$(MODULE))

# Convenience library target
$(MODULE_LIB-$(MODULE)): $(MODULE_OBJS-$(MODULE))
	$(QUIET)-$(RM) $@
	$(QUIET_AR)$(AR) $@ $+
	$(QUIET_RANLIB)$(RANLIB) $@

# Pseudo target for comfort, allows for "make common", "make gui" etc.
$(MODULE): $(MODULE_LIB-$(MODULE))

endif # PLUGIN
endif # TOOL_EXECUTABLE

###############################################
# Clean target, removes all object files. This looks a bit hackish, as we have to
# copy the content of MODULE_OBJS to another unique variable (the next module.mk
# will overwrite it after all). The same for the libMODULE.a library file.
###############################################
clean: clean-$(MODULE)
clean-$(MODULE): clean-% :
	-$(RM) $(MODULE_OBJS-$*) $(MODULE_LIB-$*) $(PLUGIN-$*) $(TOOL-$*)

.PHONY: clean-$(MODULE) $(MODULE)
