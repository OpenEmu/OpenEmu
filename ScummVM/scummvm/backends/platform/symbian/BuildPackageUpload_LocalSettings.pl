
##################################################################################################################

	@WorkingEngines = qw(
		scumm agos sky queen gob groovie saga drascula
		kyra lure agi touche parallaction cine
		cruise made m4 tinsel tucker sword1 sword2 draci sci teenagent mohawk hugo toon lastexpress tsage
	);

	@WorkingEngines_1st = qw(
		scumm queen groovie saga drascula
		touche parallaction cine
?		cruise made m4 tucker lastexpress
	);

	@WorkingEngines_2nd = qw(
		agos sky gob kyra lure 
		agi tinsel sword1 sword2 
		draci sci teenagent hugo toon
		tsage
	);

	@TestingEngines = qw(

	);

	@EnablableEngines = (@WorkingEngines, @TestingEngines);

	@EnablableSubEngines = qw(
		scumm_7_8
		he
		ihnm
		lol
		agos2
	);

	#disabled subengines lol saga2 personal nightmare

	%UseableFeatures = (
		'zlib'		=> 'zlib.lib',
		'mad'		=> 'libmad.lib',
		'tremor'	=> 'libtremor.lib',
		'flac'		=> 'libflac.lib'
	);

	# these are normally enabled for each variation
	#$DefaultFeatures = qw(zlib,mad);
	$DefaultFeatures = qw(zlib,mad,tremor,flac);

##################################################################################################################
	##
	## General system information, based on $COMPUTERNAME, so this way
	## you can use the same LocalSettings.pl file on multiple machines!
	##
##################################################################################################################

	if ($ENV{'COMPUTERNAME'} eq "PC-21") #########################################################################
	{
		# might use this string for file/dir naming in the future :)
		$Producer = "SumthinWicked";
		$RedirectSTDERR = 0;
		$HaltOnError = 0;
		$SkipExistingPackages = 0;
		$ReallyQuiet = 0;
		$DevBase = "C:\\S";

		# specify an optional FTP server to upload to after each Build+Package (can leave empty)
		#$FTP_Host = "host.com";
		$FTP_User = "something";
		$FTP_Pass = "password";
		$FTP_Dir  = "cvsbuilds";

		# What Platform SDKs are installed on this machine?
		# possible SDKs: ("UIQ2", UIQ3", "S60v1", "S60v2", "S60v3", "S80", "S90")
		# Note1: the \epoc32 directory needs to be in these rootdirs
		# Note2: these paths do NOT end in a backslash!
	#	$SDK_RootDirs{'UIQ2'}	= "$DevBase\\UIQ_21";
		$SDK_RootDirs{'UIQ3'}	= "$DevBase\\UIQ3";
	#	$SDK_RootDirs{'S60v1'}	= "$DevBase\\S60v1";
	#	$SDK_RootDirs{'S60v2'}	= "$DevBase\\S60v2";
		$SDK_RootDirs{'S60v3'}	= "$DevBase\\S60v3";
	#	$SDK_RootDirs{'S80'}	= "$DevBase\\S80";
	#	$SDK_RootDirs{'S90'}	= "$DevBase\\S90";

		$SDK_ToolchainDirs{'S60v3'} = "$DevBase\\arm-symbianelf\\bin";
		$SDK_ToolchainDirs{'UIQ2'}	= "$DevBase\\ECompXL\\bin"; # only needed for UIQ2/UIQ3
		$SDK_ToolchainDirs{'UIQ3'}	= "$DevBase\\ECompXL\\bin"; # only needed for UIQ2/UIQ3

		# these supporting libraries get built first, then all the Variations
		# Note: the string {'xxx.lib'} is used in checking in build success: so needs to be accurate!
		if (0) # so we can turn them on/off easily
		{
			## Standard libraries
			$SDK_LibraryDirs{'ALL'}{'zlib.lib'}		= "$DevBase\\zlib-1.2.2\\epoc";
			#$SDK_LibraryDirs{'ALL'}{'libmad.lib'}	= "$DevBase\\libmad-0.15.1b\\group";
			$SDK_LibraryDirs{'ALL'}{'libtremor.lib'}= "$DevBase\\tremor\\epoc";

			## SDL 1.2.12 / AnotherGuest / Symbian version
			my $SdlBase = "$DevBase\\SDL-1.2.12-ag\\Symbian";
			#$SDK_LibraryDirs{'S60v1'}{'esdl.lib'}	= "$SdlBase\\S60"; // unsupported?
			#$SDK_LibraryDirs{'S60v2'}{'esdl.lib'}	= "$SdlBase\\S60v2";
			$SDK_LibraryDirs{'S60v3'}{'esdl.lib'}	= "$SdlBase\\S60v3";
			#$SDK_LibraryDirs{'S80'}{'esdl.lib'}	= "$SdlBase\\S80";
			#$SDK_LibraryDirs{'S90'}{'esdl.lib'}	= "$SdlBase\\S90";
			#$SDK_LibraryDirs{'UIQ2'}{'esdl.lib'}	= "$SdlBase\\UIQ2"
			#$SDK_LibraryDirs{'UIQ3'}{'esdl.lib'}	= "$SdlBase\\UIQ3";
		}

		# now you can add $VariationSets only built on this PC below this line :)

		#$VariationSets{'ALL'}{'scumm'} = "$DefaultFeatures scumm scumm_7_8 he";
		#$VariationSets{'ALL'}{'all'} = "$DefaultFeatures @WorkingEngines @EnablableSubEngines";

	}
	elsif ($ENV{'COMPUTERNAME'} eq "TSSLND0106") #################################################################
	{
		$Producer = "AnotherGuest";
		$RedirectSTDERR = 1;
		$HaltOnError = 0;
		$SkipExistingPackages = 0;
		$ReallyQuiet = 0;

		#$FTP_Host = "host.com";
		#$FTP_User = "ag@host.com";
		#$FTP_Pass = "password";
		#$FTP_Dir  = "cvsbuilds";

		#$SDK_RootDirs{'UIQ2'}= "C:\\UIQ2";
		$SDK_RootDirs{'UIQ3'}= "C:\\UIQ3";
		#$SDK_RootDirs{'S60v1'}= "C:\\S60v1";
		$SDK_RootDirs{'S60v2'}= "C:\\S60v2";
		$SDK_RootDirs{'S60v3'}= "C:\\S60v3";
		#$SDK_RootDirs{'S80'}= "C:\\S80";
		#$SDK_RootDirs{'S90'}= "C:\\S90";
		$ECompXL_BinDir= "C:\\ECompXL\\";
		if (0) # so we can turn them on/off easily
		{
#			$SDK_LibraryDirs{'ALL'}{'zlib.lib'}		= "C:\\S\\zlib-1.2.2\\epoc";
#			$SDK_LibraryDirs{'ALL'}{'libmad.lib'}	= "C:\\S\\libmad-0.15.1b\\group";
			$SDK_LibraryDirs{'ALL'}{'libtremor.lib'}= "C:\\tremor\\epoc";
#			$SDK_LibraryDirs{'UIQ2'}{'esdl.lib'}	= $SDK_LibraryDirs{'UIQ3'}{'esdl.lib'} = "C:\\S\\ESDL\\epoc\\UIQ";
#			$SDK_LibraryDirs{'S60v1'}{'esdl.lib'}	= $SDK_LibraryDirs{'S60v2'}{'esdl.lib'} = $SDK_LibraryDirs{'S60v3'}{'esdl.lib'} = "C:\\S\\ESDL\\epoc\\S60";
#			$SDK_LibraryDirs{'S80'}{'esdl.lib'}		= "C:\\S\\ESDL\\epoc\\S80";
#			$SDK_LibraryDirs{'S90'}{'esdl.lib'}		= "C:\\S\\ESDL\\epoc\\S90";
		}

		# now you can add $VariationSets only built on this PC below this line :)

	}
	elsif ($ENV{'COMPUTERNAME'} eq "BIGMACHINE") #################################################################
	{
		$Producer = "AnotherGuest";
		$RedirectSTDERR = 1;
		$HaltOnError = 0;
		$SkipExistingPackages = 1;
		$ReallyQuiet = 1;

		#$FTP_Host = "host.com";
		#$FTP_User = "ag@host.com";
		#$FTP_Pass = "password";
		#$FTP_Dir  = "cvsbuilds";

		#$SDK_RootDirs{'UIQ2'}= "D:\\UIQ2";
		$SDK_RootDirs{'UIQ3'}= "D:\\UIQ3";
		#$SDK_RootDirs{'S60v1'}= "D:\\S60v1";
		#$SDK_RootDirs{'S60v2'}= "D:\\S60v2";
		$SDK_RootDirs{'S60v3'}= "D:\\S60v3";
		#$SDK_RootDirs{'S80'}= "D:\\S80";
		#$SDK_RootDirs{'S90'}= "D:\\S90";
		$ECompXL_BinDir= "D:\\ECompXL\\";
		if (0) # so we can turn them on/off easily
		{
#			$SDK_LibraryDirs{'ALL'}{'zlib.lib'}		= "C:\\S\\zlib-1.2.2\\epoc";
#			$SDK_LibraryDirs{'ALL'}{'libmad.lib'}	= "C:\\S\\libmad-0.15.1b\\group";
#			$SDK_LibraryDirs{'ALL'}{'libtremor.lib'}= "C:\\tremor\\epoc";
			$SDK_LibraryDirs{'UIQ2'}{'esdl.lib'} = "E:\\WICKED\\ESDL\\epoc\\UIQ";
			$SDK_LibraryDirs{'S60v1'}{'esdl.lib'}	= $SDK_LibraryDirs{'S60v2'}{'esdl.lib'} = "E:\\WICKED\\ESDL\\epoc\\S60";
			$SDK_LibraryDirs{'S80'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S80";
			$SDK_LibraryDirs{'S90'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S90";
			$SDK_LibraryDirs{'S60v3'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S60\\S60V3";
			$SDK_LibraryDirs{'UIQ3'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\UIQ\\UIQ3";
		}

		# now you can add $VariationSets only built on this PC below this line :)

	}
	elsif ($ENV{'COMPUTERNAME'} eq "EMBEDDEV-LAPE") #################################################################
	{
		$Producer = "AnotherGuest";
		$RedirectSTDERR = 1;
		$HaltOnError = 0;
		$SkipExistingPackages = 1;
		$ReallyQuiet = 1;

		#$FTP_Host = "host.com";
		#$FTP_User = "ag@host.com";
		#$FTP_Pass = "password";
		#$FTP_Dir  = "cvsbuilds";

		#$SDK_RootDirs{'UIQ2'}= "D:\\UIQ2";
		$SDK_RootDirs{'UIQ3'}= "G:\\UIQ3";
		#$SDK_RootDirs{'S60v1'}= "D:\\S60v1";
		#$SDK_RootDirs{'S60v2'}= "D:\\S60v2";
		$SDK_RootDirs{'S60v3'}= "G:\\S60v3";
		#$SDK_RootDirs{'S80'}= "D:\\S80";
		#$SDK_RootDirs{'S90'}= "D:\\S90";
		$ECompXL_BinDir= "D:\\ECompXL\\";
		if (0) # so we can turn them on/off easily
		{
#			$SDK_LibraryDirs{'ALL'}{'zlib.lib'}		= "C:\\S\\zlib-1.2.2\\epoc";
#			$SDK_LibraryDirs{'ALL'}{'libmad.lib'}	= "C:\\S\\libmad-0.15.1b\\group";
#			$SDK_LibraryDirs{'ALL'}{'libtremor.lib'}= "C:\\tremor\\epoc";
			$SDK_LibraryDirs{'UIQ2'}{'esdl.lib'} = "E:\\WICKED\\ESDL\\epoc\\UIQ";
			$SDK_LibraryDirs{'S60v1'}{'esdl.lib'}	= $SDK_LibraryDirs{'S60v2'}{'esdl.lib'} = "E:\\WICKED\\ESDL\\epoc\\S60";
			$SDK_LibraryDirs{'S80'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S80";
			$SDK_LibraryDirs{'S90'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S90";
			$SDK_LibraryDirs{'S60v3'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S60\\S60V3";
			$SDK_LibraryDirs{'UIQ3'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\UIQ\\UIQ3";
		}

		# now you can add $VariationSets only built on this PC below this line :)

	}
	elsif ($ENV{'COMPUTERNAME'} eq "EMBEDDEV_VAIO1") #################################################################
	{
		$Producer = "AnotherGuest";
		$RedirectSTDERR = 1;
		$HaltOnError = 0;
		$SkipExistingPackages = 1;
		$ReallyQuiet = 1;

		#$FTP_Host = "host.com";
		#$FTP_User = "ag@host.com";
		#$FTP_Pass = "password";
		#$FTP_Dir  = "cvsbuilds";

		#$SDK_RootDirs{'UIQ2'}= "D:\\UIQ2";
		$SDK_RootDirs{'UIQ3'}= "G:\\UIQ3";
		#$SDK_RootDirs{'S60v1'}= "D:\\S60v1";
		#$SDK_RootDirs{'S60v2'}= "D:\\S60v2";
		$SDK_RootDirs{'S60v3'}= "G:\\S60v3";
		#$SDK_RootDirs{'S80'}= "D:\\S80";
		#$SDK_RootDirs{'S90'}= "D:\\S90";
		#$ECompXL_BinDir= "D:\\ECompXL\\";
		if (0) # so we can turn them on/off easily
		{
#			$SDK_LibraryDirs{'ALL'}{'zlib.lib'}		= "C:\\S\\zlib-1.2.2\\epoc";
#			$SDK_LibraryDirs{'ALL'}{'libmad.lib'}	= "C:\\S\\libmad-0.15.1b\\group";
#			$SDK_LibraryDirs{'ALL'}{'libtremor.lib'}= "C:\\tremor\\epoc";
			$SDK_LibraryDirs{'UIQ2'}{'esdl.lib'} = "E:\\WICKED\\ESDL\\epoc\\UIQ";
			$SDK_LibraryDirs{'S60v1'}{'esdl.lib'}	= $SDK_LibraryDirs{'S60v2'}{'esdl.lib'} = "E:\\WICKED\\ESDL\\epoc\\S60";
			$SDK_LibraryDirs{'S80'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S80";
			$SDK_LibraryDirs{'S90'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S90";
			$SDK_LibraryDirs{'S60v3'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S60\\S60V3";
			$SDK_LibraryDirs{'UIQ3'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\UIQ\\UIQ3";
		}

		# now you can add $VariationSets only built on this PC below this line :)

	}

	else #########################################################################################################
	{
		print "ERROR: Computer name ".$ENV{'COMPUTERNAME'}." not recognized! Plz edit _LocalSettings.pl!";
		exit 1;
	}

##################################################################################################################
	##
	## Variation defines:
	##
##################################################################################################################

	# second hash index = literal string used in .sis file created.
	# empty string also removes the trailing '_'. Some 051101 examples:

	# $VariationSets{'UIQ2'}{''} would produce:
	#   scummvm-051101-SymbianUIQ2.sis

	# $VariationSets{'S60v2'}{'agos'} would produce:
	#   scummvm-051101-SymbianS60v2_agos.sis

	# $VariationSets{'ALL'}{'queen'} with all $SDK_RootDirs defined would produce:
	#   scummvm-051101-SymbianUIQ2_queen.sis
	#   scummvm-051101-SymbianUIQ3_queen.sis
	#   scummvm-051101-SymbianS60v1_queen.sis
	#   scummvm-051101-SymbianS60v2_queen.sis
	#   scummvm-051101-SymbianS60v3_queen.sis
	#   scummvm-051101-SymbianS80_queen.sis
	#   scummvm-051101-SymbianS90_queen.sis

	# NOTE: empty $VariationSets{''} string instead of 'ALL' = easy way to disable pkg!

	if (1) # all regular combo's
	{
		# the first one includes all SDKs & release-ready engines

			$VariationSets{'ALL'}{'all'} = "$DefaultFeatures @WorkingEngines @EnablableSubEngines";
#			$VariationSets{'ALL'}{'1St'} = "$DefaultFeatures @WorkingEngines_1st @EnablableSubEngines";
#			$VariationSets{'ALL'}{'2nd'} = "$DefaultFeatures @WorkingEngines_2nd @EnablableSubEngines";
		# now one for each ready-for-release engine
		if (0)
		{
			foreach (@WorkingEngines)
			{
				$VariationSets{'ALL'}{$_} = "$DefaultFeatures $_";
			}
			# for scumm, we need to add 2 features:
			#$VariationSets{'ALL'}{'scumm'} .= " scumm_7_8 he";
		}

		# now one for each not-ready-for-release-or-testing engine
		if (0)
		{
			foreach (@TestingEngines)
			{
				$VariationSets{'ALL'}{"test_$_"} = "$DefaultFeatures $_";
			}
		}
		# below here you could specify weird & experimental combinations, non-ready engines

			# Separate version for the broken sword engines (1&2)
			#$VariationSets{'ALL'}{'brokensword'} = "$DefaultFeatures sword1 sword2";

			# Separate version for Scumm games (COMI) since memory usage might be high
			#$VariationSets{'ALL'}{'scumm'} = "$DefaultFeatures scumm scumm_7_8 he";

			# for mega-fast-testing only plz! Warning: contains to engines!
			#$VariationSets{'ALL'}{'fast_empty'} = "";

	} # end quick-n-fast if (1|0)


##################################################################################################################

1;
