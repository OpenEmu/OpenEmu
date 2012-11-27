
use Cwd;
use Switch;

system("cls");
require "BuildPackageUpload_LocalSettings.pl";

##################################################################################################################
# prep some vars

# the dir containing the build files: '.\backends\platforms\symbian\$SDK_BuildDir\'
$SDK_BuildDirs{'UIQ2'}	= "UIQ2";
$SDK_BuildDirs{'UIQ3'}	= "UIQ3";
$SDK_BuildDirs{'S60v1'}	= "S60";
$SDK_BuildDirs{'S60v2'}	= "S60";
$SDK_BuildDirs{'S60v3'}	= "S60v3";
$SDK_BuildDirs{'S80'}	= "S80";
$SDK_BuildDirs{'S90'}	= "S90";

# the target name inserted here: 'abld BUILD $SDK_TargetName UREL'
$SDK_TargetName{'UIQ2'}	= "armi";
$SDK_TargetName{'UIQ3'}	= "gcce";
$SDK_TargetName{'S60v1'}= "armi";
$SDK_TargetName{'S60v2'}= "armi";
$SDK_TargetName{'S60v3'}= "gcce";
$SDK_TargetName{'S80'}	= "armi";
$SDK_TargetName{'S90'}	= "armi";

# Binaries are installed here: '$SDK_RootDirs\epoc32\release\$SDK_TargetDir\urel\'
$SDK_TargetDir{'UIQ2'}	= "armi";
$SDK_TargetDir{'UIQ3'}	= "armv5";
$SDK_TargetDir{'S60v1'}	= "armi";
$SDK_TargetDir{'S60v2'}	= "armi";
$SDK_TargetDir{'S60v3'}	= "armv5";
$SDK_TargetDir{'S80'}	= "armi";
$SDK_TargetDir{'S90'}	= "armi";

$build_dir = getcwd();
$output_dir = "$build_dir/Packages";
chdir("../../../");
$base_dir = getcwd();
chdir($build_dir);
$build_log_out = "$build_dir/out.build.out.log"; # don't start these files with "Build"
$build_log_err = "$build_dir/out.build.err.log"; # so "B"+TAB completion works in 1 go :P

$initial_path = $ENV{'PATH'}; # so we can start with a fresh PATH for each Build

($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime();
$date = sprintf("%02d%02d%02d", $year-100, $mon+=1, $mday);
$file_tpl_pkg = "scummvm-CVS-Symbian%s.pkg";
$file_tpl_sis = "scummvm-%s-Symbian%s%s.sis";
$version_tpl_sis = "$date"; # "CVS$date"

$PackagesQueued = 0;
$PackagesCreated = 0;
$PackagesUploaded = 0;
@ErrorMessages = ();
$ftp_url = "FTP://$FTP_User\@$FTP_Host/$FTP_Dir/";

# these macros are always defined:
$ExtraMacros =  "MACRO		NONSTANDARD_PORT\n";
$ExtraMacros .=  "MACRO		ENABLE_VKEYBD\n";
$ExtraMacros .= "MACRO		DISABLE_FANCY_THEMES\n";
$ExtraMacros .= "MACRO		USE_TRANSLATION\n";

# prep nice list of SDKs
#while( ($SDK, $RootDir) = each(%SDK_RootDirs) )
foreach $SDK (sort keys(%SDK_RootDirs))
{
	# see if it exists!
	if (-d $SDK_RootDirs{$SDK})
	{
		$SDKs .= "$SDK\t$SDK_RootDirs{$SDK}\n\t\t\t";
	}
	else # missing?
	{
		$SDKs .= "$SDK\t$SDK_RootDirs{$SDK}\t[MISSING: Skipping!]\n\t\t\t";
		# remove it from array, to prevent building!
		delete $SDK_RootDirs{$SDK};
	}
}

# prep nice list of Libraries
while( ($SDK, $Value) = each(%SDK_LibraryDirs) )
{
	while( ($Library, $Path) = each(%{$SDK_LibraryDirs{$SDK}}) )
	{
		# maybe it's already been built?
		if (-e $SDK_RootDirs{$SDK}."\\epoc32\\release\\$SDK_TargetDir{$SDK}\\urel\\$Library")
		{
			$PresentLibs{$Library} = "$Path  [EXISTS: Skipping!]";
			delete $SDK_LibraryDirs{$SDK}{$Library};
		}
		else # make it!
		{
			$PresentLibs{$Library} = "$Path";
		}
	}
}
foreach $Library (sort keys(%PresentLibs))
{
	$LIBs .= "$Library\t$PresentLibs{$Library}\n\t\t\t";
}

# prep nice list of Variations
#while( ($SDK, $Value) = each(%SDK_Variations) )
#{
#	while( ($Variation, $Value2) = each(%{$SDK_Variations{$SDK}}) )
#	{
#		$Extra = ($Variation ne '' ? "_$Variation" : "");
#		if ($SDK eq "ALL")
#		{
#			while( ($SDK2, $RootDir) = each(%SDK_RootDirs) )
#			{
#				if ($SDK_RootDirs{$SDK2} ne '') # is this SDK listed as installed? (fails silently)
#				{
#					push @Packages, sprintf($file_tpl_sis, $version_tpl_sis, $SDK2, $Extra);
#					$PackagesQueued++;
#				}
#			}
#		}
#		else
#		{
#			if ($SDK_RootDirs{$SDK} ne '') # is this SDK listed as installed? (fails silently)
#			{
#				push @Packages, sprintf($file_tpl_sis, $version_tpl_sis, $SDK, $Extra);
#				$PackagesQueued++;
#			}
#		}
#	}
#}
while( ($SDK, $Value) = each(%VariationSets) )
{
	while( ($Variation, $FeaturesBlock) = each(%{$VariationSets{$SDK}}) )
	{
#my $MacroBlock = &MakeMppMacroDefs($FeaturesBlock);
		$Extra = ($Variation ne '' ? "_$Variation" : "");
		if ($SDK eq "ALL")
		{
			while( ($SDK2, $RootDir) = each(%SDK_RootDirs) )
			{
				if ($SDK_RootDirs{$SDK2} ne '') # is this SDK listed as installed? (fails silently)
				{
					push @Packages, sprintf($file_tpl_sis, $version_tpl_sis, $SDK2, $Extra);
					$PackagesQueued++;
				}
			}
		}
		else
		{
			if ($SDK_RootDirs{$SDK} ne '') # is this SDK listed as installed? (fails silently)
			{
				push @Packages, sprintf($file_tpl_sis, $version_tpl_sis, $SDK, $Extra);
				$PackagesQueued++;
			}
		}
	}
}
foreach $Package (sort @Packages)
{
	$PackagesStr .= "$Package\n\t\t\t";
}

print "
=======================================================================================
Preparing to Build, Package & Upload $PackagesQueued SymbianOS ScummVM variations
=======================================================================================

	Producer     \t$Producer (RedirE:$RedirectSTDERR HaltE:$HaltOnError Skip:$SkipExistingPackages Quiet:$ReallyQuiet)

	SDKs inst'd  \t$SDKs	".(	%SDK_LibraryDirs ? "
	LIBs inst'd  \t$LIBs	" : "" )."
	$PackagesQueued Variations \t$PackagesStr
	DIR base     \t$base_dir
	    build    \t$build_dir
	    output   \t$output_dir
".(	$FTP_Host ne '' ? "
	FTP host     \t$FTP_Host
	    user     \t$FTP_User
	    pass     \t"."*" x length($FTP_Pass)."
	    dir      \t$FTP_Dir
" : "" )."
=======================================================================================
Press Ctrl-C to abort or enter to continue Build, Package & Upload $PackagesQueued Variations...
=======================================================================================
";

$line = <STDIN>;

# make sure the output dir exists!
mkdir($output_dir, 0755) if (! -d $output_dir);

unlink($build_log_out);
unlink($build_log_err);

# init _base.mmp now, so we can start changing it without affecting the CVS version _base.mmp.in!
my $name = "mmp/scummvm_base.mmp";
my $file  = "$build_dir/$name";
open FILE, "$file.in";	@lines = <FILE>; close FILE;
my $onestr = join("",@lines);
open FILE, ">$file"; print FILE $onestr; close FILE;

##################################################################################################################
# do the actual deeds for all present libs

while( ($SDK, $Value) = each(%SDK_LibraryDirs) )
{
	while( ($Library, $Path) = each(%{$SDK_LibraryDirs{$SDK}}) )
	{
		if ($SDK eq "ALL")
		{
			while( ($SDK2, $RootDir) = each(%SDK_RootDirs) )
			{
				if ($SDK_RootDirs{$SDK2} ne '') # is this SDK listed as installed? (fails silently)
				{
					# do we already have this one?
					next if (-e $SDK_RootDirs{$SDK2}."\\epoc32\\release\\$SDK_TargetDir{$SDK2}\\urel\\$Library");

					$LibrariesQueued++;
					DoLibrary($SDK2, $Library, $Path);
				}
			}
		}
		else
		{
			if ($SDK_RootDirs{$SDK} ne '') # is this SDK listed as installed? (fails silently)
			{
				# do we already have this one?
				next if (-e $SDK_RootDirs{$SDK}."\\epoc32\\release\\$SDK_TargetDir{$SDK}\\urel\\$Library");

				$LibrariesQueued++;
				DoLibrary($SDK, $Library, $Path);
			}
		}
	}
}

#system('pause');

##################################################################################################################
# do the actual deeds for all configured variations

while( ($SDK, $VariationsHash) = each(%SDK_Variations) )
{
	while( ($Variation, $MacroBlock) = each(%{$SDK_Variations{$SDK}}) )
	{
		if ($SDK eq "ALL")
		{
			while( ($SDK2, $RootDir) = each(%SDK_RootDirs) )
			{
				if ($SDK_RootDirs{$SDK2} ne '')
				{
					DoVariation($SDK2, $Variation, $MacroBlock);
				}
			}
		}
		else
		{
			if ($SDK_RootDirs{$SDK} ne '')
			{
				DoVariation($SDK, $Variation, $MacroBlock);
			}
		}
	}
}

while( ($SDK, $VariationsHash) = each(%VariationSets) )
{
	while( ($Variation, $FeaturesBlock) = each(%{$VariationSets{$SDK}}) )
	{
		my $MacroBlock = &MakeMppMacroDefs($FeaturesBlock);
		if ($SDK eq "ALL")
		{
			while( ($SDK2, $RootDir) = each(%SDK_RootDirs) )
			{
				if ($SDK_RootDirs{$SDK2} ne '')
				{
					DoVariation($SDK2, $Variation, $MacroBlock);
				}
			}
		}
		else
		{
			if ($SDK_RootDirs{$SDK} ne '')
			{
				DoVariation($SDK, $Variation, $MacroBlock);
			}
		}
	}
}

##################################################################################################################
# give report

chdir($build_dir);

print "
=======================================================================================".(	%SDK_LibraryDirs ? "
Libraries selected: \t$LibrariesQueued
          built:    \t$LibrariesSucceeded				" : "" )."
Packages  selected: \t$PackagesQueued	$base_dir                 ".( $PackagesExisted ? "
          existed:  \t$PackagesExisted	$output_dir     " : "" )."
          created:  \t$PackagesCreated	$output_dir		          ".( $FTP_Host ne '' ? "
          uploaded: \t$PackagesUploaded	$ftp_url		" : "" )."
=======================================================================================
";

my $count = @ErrorMessages;
if ($count)
{
	print "Hmm, unfortunately some ERRORs have occurred during the process:\n";
	foreach $Error (@ErrorMessages)
	{
		print "> $Error\n";
	}
	print "=======================================================================================\n";
	print "\007\007";
}

# first clean up 'initial path' by removing possible old entries (in case of aborted builds)
#$initial_path_system_cleaned = CleanupPath($initial_path_system);

# show them we cleaned up?
#if ($initial_path_system_cleaned ne $initial_path_system)
#{
#	PrintMessage("PATH cleaned up from:\n$initial_path_system\n\nto:\n$initial_path_system_cleaned");
#}

print "       SumthinWicked wishes you a ridiculously good and optimally happy day :P\n=======================================================================================";

##################################################################################################################
##################################################################################################################

# create a set of "MACRO xxx" definitions for use in the scummvm_base.mpp file
sub MakeMppMacroDefs
{
	my ($features) = @_;

	my %EnabledFeatures = ();
	foreach (split(/\W|\r|\n/, $features))
	{
		if ($_ ne "")
		{
			#print "FEATURE: $_\n";
			$EnabledFeatures{$_} = 1;
		}
	}

	my $MacroDefs = "";

	$MacroDefs .= "	// Features //\n";
	foreach my $e (sort keys %UseableFeatures)
	{
		my $E = uc($e);
		if ($EnabledFeatures{$e})
		{
			$MacroDefs .= "MACRO		USE_$E			// LIB:$UseableFeatures{$e}\n";
			# this one is used: remove it now
			delete $EnabledFeatures{$e};
			# this will leave us with a list of unparsed options!
		}
		else
		{
			$MacroDefs .= "//MACRO		USE_$E\n";
		}
	}

	$MacroDefs .= "	// Engines //\n";
	foreach my $e (sort @EnablableEngines)
	{
		my $E = uc($e);
		if ($EnabledFeatures{$e})
		{
			$MacroDefs .= "MACRO		ENABLE_$E		// LIB:scummvm_$e.lib\n";
			# this one is used: remove it now
			delete $EnabledFeatures{$e};
			# this will leave us with a list of unparsed options!
		}
		else
		{
			$MacroDefs .= "//MACRO		ENABLE_$E\n";
		}
	}

	$MacroDefs .= "	// SubEngines //\n";
	foreach my $e (sort @EnablableSubEngines)
	{
		my $E = uc($e);
		if ($EnabledFeatures{$e})
		{
			$MacroDefs .= "MACRO		ENABLE_$E\n";
			# this one is used: remove it now
			delete $EnabledFeatures{$e};
			# this will leave us with a list of unparsed options!
		}
		else
		{
			$MacroDefs .= "//MACRO		ENABLE_$E\n";
		}
	}

#print "\n\n'$features' ==> $MacroDefs\n\n\n";
	return $MacroDefs;
}

##################################################################################################################

# Build, Package & Upload a single Variation
sub DoLibrary
{
	my ($SDK, $Library, $Path) = @_;
	my $TargetName = $SDK_TargetName{$SDK};
	my $TargetDir  = $SDK_TargetDir{$SDK};
	my $Target = "$SDK - $Library";
	my $TargetFilePath = $SDK_RootDirs{$SDK}."\\epoc32\\release\\$TargetDir\\urel\\$Library";
	#my $TargetIntermediatePath = uc($SDK_RootDirs{$SDK}."\\EPOC32\\BUILD\\".substr($Path, 3));
	# does this remove too much?
	my $TargetIntermediatePath = uc($SDK_RootDirs{$SDK}."\\EPOC32\\BUILD\\");

my $header = "
=======================================================================================
=======================================================================================
=======================================================================================
		Preparing to build library $Target
=======================================================================================
=======================================================================================
=======================================================================================
";
	print $header if (!$ReallyQuiet);
	open FILE, ">>$build_log_out"; print FILE $header; close FILE;
	open FILE, ">>$build_log_err"; print FILE $header; close FILE;

	# easy for error-handling:
	$CurrentTarget = $Target;
	my $OK = 1;

	PrepSdkPaths($SDK);

	chdir($Path) or $OK=0;
	PrintErrorMessage("Changing to $Path failed!") if (!$OK);
	return 0 if (!$OK);

	PrintMessage("Cleaning for $Target") if (!$ReallyQuiet);
	system("bldmake bldfiles > NUL 2> NUL");
	PrintErrorMessage("'bldmake bldfiles' exited with value " . ($? >> 8)) if ($? >> 8);

	system("abld MAKEFILE $TargetName > NUL 2> NUL");
	PrintErrorMessage("'abld MAKEFILE $TargetName' exited with value " . ($? >> 8)) if ($? >> 8);

	system("abld CLEAN $TargetName UREL > NUL 2> NUL");
	PrintErrorMessage("'abld CLEAN $TargetName urel' exited with value " . ($? >> 8)) if ($? >> 8);
	# remove file so we are sure that after .lib generation we have a fresh copy!
	if (-e $TargetFilePath) { unlink($TargetFilePath) or PrintErrorMessage("Removing $TargetFilePath"); }

	my $Redirection = "OUT:file, ERR:".($RedirectSTDERR ? "file" : "screen");
	my $Message = "Building $Target ($Redirection)";
	PrintMessage($Message) if (!$ReallyQuiet);
	print("  $Message\n")  if ($ReallyQuiet);

	my $OldSize = (-s $build_log_err);
	$Redirection = ($RedirectSTDERR ? "2>> $build_log_err" : "");
	system("abld TARGET $TargetName UREL $Redirection >> $build_log_out");
	$OK = 0 if ($? >> 8);
#	print "  STDERR: ".((-s $build_log_err)-$OldSize)." bytes output written to $build_log_err\n+--------------------------------------------------------------------------------------\n" if ($OldSize != (-s $build_log_err));
	PrintErrorMessage("'abld TARGET $TargetName UREL' exited with value " . ($? >> 8)) if ($? >> 8);
	return 0 if (!$OK); # ABLD always returns ok :( grr
	PrintMessage("Done.") if (!$ReallyQuiet);

	# did it work? :)
	if (-e $TargetFilePath)
	{
		$LibrariesSucceeded++;

		if ($TargetIntermediatePath ne '' && $TargetIntermediatePath =~ /\\EPOC32\\BUILD\\/i) # make really sure it's a valid path!
		{
			system("del /S /Q $TargetIntermediatePath > NUL");
		}
		return 1;
	}
	else
	{
		PrintErrorMessage("'abld TARGET $TargetName UREL' apparently failed.");
		if ($HaltOnError)
 		{
			PrintErrorMessage("Halting on error as requested!");
			exit 1;
		}
		return 0;
	}
}

##################################################################################################################

# Build, Package & Upload a single Variation
sub DoVariation
{
	my ($SDK, $Variation, $MacroBlock) = @_;
	my $Extra = ($Variation ne '' ? "_$Variation" : "");
	my $Package = sprintf($file_tpl_sis, $version_tpl_sis, $SDK, $Extra);

	if ($SkipExistingPackages && -f "$output_dir/$Package")
	{
		PrintMessage("Skipping $Package (already exists!)");
		$PackagesExisted++;
		return;
	}

my $header = "
=======================================================================================
=======================================================================================
=======================================================================================
		Preparing to build $Package
=======================================================================================
=======================================================================================
=======================================================================================
";
	print $header if (!$ReallyQuiet);
	open FILE, ">>$build_log_out"; print FILE $header; close FILE;
	open FILE, ">>$build_log_err"; print FILE $header; close FILE;

	# easy for error-handling:
	$CurrentTarget = $Package;
	my $OK;

	$OK = PrepVariation($SDK, $Variation, $Package, $MacroBlock);

	if ($OK)
	{
		$OK = BuildVariation($SDK, $Variation, $Package, $MacroBlock);

		if ($OK && $FTP_Host ne '')
		{
			UploadVariation($SDK, $Variation, $Package);
		}
	}
}

##################################################################################################################

sub PrepVariation()
{
	my ($SDK, $Variation, $Package, $MacroBlock) = @_;
	my $OK = 1;

	PrepSdkPaths($SDK);

	chdir($build_dir) or $OK=0;
	PrintErrorMessage("Changing to $build_dir failed!") if (!$OK);
	return 0 if (!$OK);

	# insert $MacroBlock into AUTO_MACRO_MASTER in scummvm_base.mmp
	PrintMessage("Setting new AUTO_MACROS_MASTER in scummvm_base.mmp for '$Variation'") if (!$ReallyQuiet);
	my $n = "AUTO_MACROS_MASTER";
	my $a = "\/\/START_$n\/\/";
	my $b = "\/\/STOP_$n\/\/";
	my $name = "scummvm_base.mmp";
	my $file = "$build_dir/mmp/$name";
	my $updated = " Updated @ ".localtime();

	open FILE, "$file" or $OK=0;
	PrintErrorMessage("Reading file '$file'") if (!$OK);
	return 0 if (!$OK);
	my @lines = <FILE>;
	close FILE;

	my $onestr = join("",@lines);
	$MacroBlock =~ s/^\s*//gm;
	$onestr =~ s/$a(.*)$b/$a$updated\n$ExtraMacros$MacroBlock$b/s;

	open FILE, ">$file" or $OK=0;
	PrintErrorMessage("Writing file '$file'") if (!$OK);
	return 0 if (!$OK);
	print FILE $onestr;
	close FILE;

	# running AdaptAllMMPs.pl to propagate changes
	PrintMessage("Running AdaptAllMMPs.pl to propagate MACRO changes") if (!$ReallyQuiet);
	system("perl AdaptAllMMPs.pl > NUL");
	$OK = 0 if ($? >> 8);
	PrintErrorMessage("'AdaptAllMMPs.pl' exited with value " . ($? >> 8)) if ($? >> 8);
	return 0 if (!$OK);

	# we are here: so all is ok :)
	return 1;
}

##################################################################################################################

sub BuildVariation()
{
	my ($SDK, $Variation, $Package, $MacroBlock) = @_;
	my $TargetName = $SDK_TargetName{$SDK};
	my $TargetDir  = $SDK_TargetDir{$SDK};
	my $OK = 1;

	my $dir = $build_dir."/".$SDK_BuildDirs{$SDK};
	$dir =~ s#/#\\#g;
	chdir($dir);

	#my $TargetIntermediatePath = uc($SDK_RootDirs{$SDK}."\\EPOC32\\BUILD\\".substr($dir, 3));
	# does this remove too much?
	my $TargetIntermediatePath = uc($SDK_RootDirs{$SDK}."\\EPOC32\\BUILD\\");

	PrintMessage("Cleaning for $Package") if (!$ReallyQuiet);

	# remove some files so we are sure that after .sis package generation we have a fresh copy!
	my $UnlinkFile = "$output_dir/$Package";
	if (-e $UnlinkFile) { unlink($UnlinkFile) or PrintErrorMessage("Removing $UnlinkFile"); }
	$UnlinkFile = $SDK_RootDirs{$SDK}."/epoc32/release/$TargetDir/urel/ScummVM.app";
	if (-e $UnlinkFile) { unlink($UnlinkFile) or PrintErrorMessage("Removing $UnlinkFile"); }
	$UnlinkFile = $SDK_RootDirs{$SDK}."/epoc32/release/$TargetDir/urel/ScummVM.exe";
	if (-e $UnlinkFile) { unlink($UnlinkFile) or PrintErrorMessage("Removing $UnlinkFile"); }
	# remove all libs here, note they are in another dir!
	system("del ".$SDK_RootDirs{$SDK}."/epoc32/release/$TargetName/urel/scummvm_*.lib");

	system("bldmake bldfiles 2> NUL > NUL");
	PrintErrorMessage("'bldmake bldfiles' exited with value " . ($? >> 8)) if ($? >> 8);

	system("abld CLEAN $TargetName UREL 2> NUL > NUL");
	PrintErrorMessage("'abld CLEAN $TargetName UREL' exited with value " . ($? >> 8)) if ($? >> 8);

	my $Redirection = "OUT:file, ERR:".($RedirectSTDERR ? "file" : "screen");
	my $Message = "Building $Package ($Redirection)";
	PrintMessage($Message) if (!$ReallyQuiet);
	print("  $Message\n")  if ($ReallyQuiet);

	my $OldSize = (-s $build_log_err);
	$Redirection = ($RedirectSTDERR ? "2>> $build_log_err" : "");
	system("abld BUILD $TargetName UREL $Redirection >> $build_log_out");
	$OK = 0 if ($? >> 8);
	print "  STDERR: ".((-s $build_log_err)-$OldSize)." bytes output written to $build_log_err\n+--------------------------------------------------------------------------------------\n" if ($OldSize != (-s $build_log_err) && !$ReallyQuiet);
	PrintErrorMessage("'abld BUILD $TargetName UREL' exited with value " . ($? >> 8)) if ($? >> 8);
	return 0 if (!$OK); # ABLD always returns ok :( grr
	PrintMessage("Done.") if (!$ReallyQuiet);

	# do we have an override suffix for the package name?
	$MacroBlock =~ /^\s*\/\/\s*PKG_SUFFIX:\s*(\w+)\s*/gm; # using '@' as delimiter here instead of '/' for clarity
	my $PkgSuffix = $1; # can be ""
	my $PkgFile = sprintf($file_tpl_pkg, $SDK.$PkgSuffix);

	PrintMessage("Creating package $Package") if (!$ReallyQuiet);
## fix  if (!$ReallyQuiet) for next:
	system("makesis -d\"".$SDK_RootDirs{$SDK}."\" $PkgFile \"$output_dir/$Package\" $Redirection >> $build_log_out");
	$OK = 0 if ($? >> 8);
	PrintErrorMessage("'makesis' $PkgFile exited with value " . ($? >> 8)) if (!$OK);
	if ($HaltOnError && !$OK)
	{
		PrintErrorMessage("Halting on error as requested!");
		exit 1;
	}
	return 0 if (!$OK);

	# did it work? :)
	if (-e "$output_dir/$Package")
	{
		$PackagesCreated++;

		if ($TargetIntermediatePath ne '' && $TargetIntermediatePath =~ /\\EPOC32\\BUILD\\/i) # make really sure it's a valid path!
		{
			#PrintMessage("Cleaning $TargetIntermediatePath");
			system("del /S /Q $TargetIntermediatePath > NUL");
		}
		return 1;
	}
	else
	{
		PrintErrorMessage("'makesis' apparently failed. (?)");
		return 0;
	}
}

##################################################################################################################

sub UploadVariation()
{
	my ($SDK, $Variation, $Package) = @_;

	use Net::FTP;
	my $newerr;

	PrintMessage("Connecting to FTP $FTP_Host") if (!$ReallyQuiet);

	$ftp = Net::FTP->new($FTP_Host,Timeout=>240) or $newerr=1;
	PrintErrorMessage("Connecting to FTP $FTP_Host! Aborting!") if $newerr;
	if (!$newerr)
	{
		$ftp->login($FTP_User, $FTP_Pass) or $newerr=1;
		PrintErrorMessage("Logging in with $FTP_User to $FTP_Host! Aborting!") if $newerr;
		if (!$newerr)
		{
			if ($FTP_Dir ne '') # do we need to change dir?
			{
				PrintMessage("Changing to dir $FTP_Dir");
				$ftp->cwd($FTP_Dir) or $newerr=1;

				if ($newerr)
				{
					PrintErrorMessage("Changing to dir $FTP_Dir! Aborting!");
					$ftp->quit;
					return;
				}
			}

# leave this for possible auto-deletion of old files?
#			@files = $ftp->dir or $newerr=1;
#			  push @ERRORS, "Can't get file list $!\n" if $newerr;
#			print "Got  file list\n";
#			foreach(@files) {
#			  print "$_\n";
#			}

			PrintMessage("Uploading $Package (".(-s "$output_dir/$Package")." bytes)");

			$ftp->binary;
			$ftp->put("$output_dir/$Package") or $newerr=1;
			PrintErrorMessage("Uploading package! Aborting!") if $newerr;
			$PackagesUploaded++ if (!$newerr);
		}

		$ftp->quit;
	}
}

##################################################################################################################

sub PrepSdkPaths()
{
	my ($SDK) = @_;
	my $EPOCROOT = $SDK_RootDirs{$SDK};
	my $EPOC32RT = "$EPOCROOT\\epoc32";
	my $AdditionalPathEntries = "";
	my $OK = 1;

	# do the directories exist?
	if (! -d $EPOCROOT) { PrintErrorMessage("$SDK Directory does not exist: '$EPOCROOT'"); return 0; }
	if (! -d $EPOC32RT) { PrintErrorMessage("$SDK Directory does not exist: '$EPOC32RT'"); return 0; }

	# set env stuff
	PrintMessage("Prepending $SDK specific paths to %PATH%") if (!$ReallyQuiet);
	$AdditionalPathEntries .= "$SDK_ToolchainDirs{$SDK};" if ($SDK_ToolchainDirs{$SDK} ne '');
	$AdditionalPathEntries .= "$ECompXL_BinDir;" if ($ECompXL_BinDir ne '' && $SDK eq 'UIQ2');
	$AdditionalPathEntries .= "$EPOC32RT\\include;";
	$AdditionalPathEntries .= "$EPOC32RT\\tools;";
	$AdditionalPathEntries .= "$EPOC32RT\\gcc\\bin;";
	$ENV{'EPOCROOT'} = substr($EPOCROOT,2)."\\"; # strips drive letter, needs to end with backslash!
	$ENV{'PATH'} = "$AdditionalPathEntries$initial_path";

	return 1;
}

##################################################################################################################

sub CleanupPath()
{
	my ($path) = @_;

	if ($ECompXL_BinDir ne '')
	{
		$path =~ s/\"\Q$ECompXL_BinDir\E\";//g;
	}

	while( ($SDK, $RootDir) = each(%SDK_RootDirs) )
	{
		if ($SDK_RootDirs{$SDK} ne '')
		{
			my $path_component = "\"".$SDK_RootDirs{$SDK}."\\epoc32\\";
			$path =~ s/\Q$path_component\E.*?\";//g;
		}
	}

	return $path;
}

##################################################################################################################

sub PrintErrorMessage()
{
	my ($msg) = @_;

	# add to array, so we can print it @ the end!
	push @ErrorMessages, "$CurrentTarget: $msg";

	print "+--------------------------------------------------------------------------------------\n";
	PrintMessage("ERROR: $CurrentTarget: $msg");
	print "\007" if (!$HaltOnError);
	print "\007\007" if ($HaltOnError); # make more noise if halt-on-error
}

sub PrintMessage()
{
	my ($msg) = @_;

	print "| $msg\n";
	print "+--------------------------------------------------------------------------------------\n";
#$line = <STDIN>;

	#print "\e[1,31m> $msg\e[0m\n";
}

##################################################################################################################
