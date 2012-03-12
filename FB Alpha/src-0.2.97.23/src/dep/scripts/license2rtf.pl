#!/usr/bin/perl -w

use strict;

my $Outfile;
my $Infile;

my $Font = "Tahoma";
my $FontSize = 16;

my $escape = 0;

my %FontFamilies = ("Tahoma",			"swiss",
					"Times New Roman",	"roman"
);

# Process command line arguments
for ( my $i = 0; $i < scalar @ARGV; $i++ ) {{

	# Output file
	if ( $ARGV[$i] =~ /^-o/i ) {
		if ( $ARGV[$i] =~ /^-o$/i ) {
			$i++;
			if ( $i < scalar @ARGV ) {
				$Outfile = $ARGV[$i];
			}
		} else {
			$ARGV[$i] =~ /(?<=-o)(.*)/i;
			$Outfile = $1;
		}
		next;
	}
	# Font
	if ( $ARGV[$i] =~ /^-f/i ) {
		if ( $ARGV[$i] =~ /^-f$/i ) {
			$i++;
			if ( $i < scalar @ARGV ) {
				$Font = $ARGV[$i];
			}
		} else {
			$ARGV[$i] =~ /(?<=-f)(.*)/i;
			$Font = $1;
		}
		next;
	}
	# Font size
	if ( $ARGV[$i] =~ /^-s/i ) {
		if ( $ARGV[$i] =~ /^-s$/i ) {
			$i++;
			if ( $i < scalar @ARGV ) {
				$FontSize = $ARGV[$i];
			}
		} else {
			$ARGV[$i] =~ /(?<=-s)(.*)/i;
			$FontSize = $1;
		}
		next;
	}
	if ( $ARGV[$i] =~ /^-esc/i ) {
		$escape = 1;
	}

	$Infile  = $ARGV[$i];
	next;
}}

unless ( $Outfile and $Font and $FontSize and $Infile ) {
	die "Usage: $0 -o <output file> <input file> [-f <font> -s <font-size> -escape]\n\n";
}

open( INFILE, $Infile ) or die "\nError: Couldn't open INPUT file $Infile $!";
open( OUTFILE, ">$Outfile" ) or die "\nError: Couldn't open OUTPUT file $Outfile $!";

print "Generating $Outfile...\n";

print OUTFILE "{\\rtf1\\ansi\\ansicpg1252\\deff0{\\fonttbl(\\f0\\f$FontFamilies{$Font}\\fprq2 $Font;)}{\\colortbl\\red0\\green0\\blue0;\\red255\\green0\\blue0;\\red0\\green0\\blue191;}\\deflang1033\\horzdoc\\fromtext\\pard\\plain\\f0\\fs$FontSize";

while ( my $line = <INFILE> ) {

	# kill line endings (Cygin perl has problems with chomp)
	$line =~ s/[\r\n]//gm;

	$line = "$line\\par";

	# if a paragraph starts with " - " treat it as a bulleted paragraph
	if ($line =~ /^\s-\s/) {
		$line =~ s/^\s-\s//;
		$line = "\\pard{\\*\\pn\\pnlvlblt\\pnindent0{\\pntxtb\\\'B7}}\\fi-144\\li144 $line\\pard";
	}

	# highlight URLs
	$line =~ s/(http:\/\/[\w~\/\.\-]*)/\\cf2 $1\\cf0/g;

	# if a paragraph starts with "DISCLAIMER:" make the text bold and red
	if ($line =~ /^DISCLAIMER/) {
		$line = "\\cf1\\b $line\\b0\\cf0";
	}

	# if a paragraph starts with a quote make the text italic
	if ($line =~ /^\"/) {
		$line = "\\i $line\\i0";
	}

	if ($escape) {
		# Escape quotes and backslashes
		$line =~ s/([\"\\])/\$1/g;
	}

	print OUTFILE "$line ";
}

print OUTFILE "}";

close( OUTFILE );
close( INFILE );
