#!/usr/bin/perl

#
# This script is a hack to update the ScummVM version in all (?) files that
# contain it. Obviously, it should be used before a release.

use strict;

if ($#ARGV+1 < 3 or $#ARGV+1 > 4) {
	# TODO: Allow the user to specify the version as "1.2.3svn"
	# and automatically split that into 1, 2, 3, svn
	print STDERR "Usage: $0 MAJOR MINOR PATCH [EXTRA]\n";
	print STDERR "  TODO\n";
	exit 1;
}

# TODO: Verify that major/minor/patch are actually numbers
my $VER_MAJOR = $ARGV[0];
my $VER_MINOR = $ARGV[1];
my $VER_PATCH = $ARGV[2];
my $VER_EXTRA = $ARGV[3];
my $VERSION = "$VER_MAJOR.$VER_MINOR.$VER_PATCH$VER_EXTRA";

die "MAJOR must be a natural number\n" unless ($VER_MAJOR =~ /^\d+$/);
die "MINOR must be a natural number\n" unless ($VER_MINOR =~ /^\d+$/);
die "PATCH must be a natural number\n" unless ($VER_PATCH =~ /^\d+$/);


print "Setting version to '$VERSION'\n";


# List of the files in which we need to perform substitution.
my @subs_files = qw(
	base/internal_version.h
	dists/redhat/scummvm.spec
	dists/redhat/scummvm-tools.spec
	dists/scummvm.rc
	dists/slackware/scummvm.SlackBuild
	dists/macosx/Info.plist
	dists/iphone/Info.plist
	dists/irix/scummvm.spec
	dists/win32/scummvm.nsi
	dists/wii/meta.xml
	dists/android/AndroidManifest.xml
	dists/android/plugin-manifest.xml
	dists/openpandora/PXML.xml
	dists/openpandora/README-OPENPANDORA
	dists/openpandora/README-PND.txt
	dists/openpandora/index.html
	dists/gph/README-GPH
	dists/gph/scummvm.ini
	backends/platform/psp/README.PSP
	);

my %subs = (
	VER_MAJOR	=>	$VER_MAJOR,
	VER_MINOR	=>	$VER_MINOR,
	VER_PATCH	=>	$VER_PATCH,
	VER_EXTRA	=>	$VER_EXTRA,
	VERSION		=>	$VERSION
	);

foreach my $file (@subs_files) {
	print "Processing $file...\n";
	open(INPUT, "< $file.in") or die "Can't open '$file.in' for reading: $!\n";
	open(OUTPUT, "> $file") or die "Can't open '$file' for writing: $!\n";

	while (<INPUT>) {
		while (my ($key, $value) = each(%subs)) {
			s/\@$key\@/$value/;
		}
		print OUTPUT;
	}

	close(INPUT);
	close(OUTPUT);
}
