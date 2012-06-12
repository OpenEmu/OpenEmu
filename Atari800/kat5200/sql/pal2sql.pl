#!/usr/local/bin/perl
#
# Transform a XML Palette file to SQL
#

$numArgs = $#ARGV + 1;

die "Input XML file required" unless $numArgs > 0;

foreach $num (0 .. $#ARGV ) {
	process_xml_file( $ARGV[$num] );
}

sub process_xml_file
{

	open ( INFILE, "$ARGV[$num]" ) or die "Cound not open file: $ARGV[1]: $!\n";
	open ( OUTFILE, ">$ARGV[$num].sql" )  or die "Cound not open file: $ARGV[2]: $!\n";
#	open ( OUTFILE, ">-" )        or die "Cound not STDOUT: $!\n";

	print OUTFILE "BEGIN;\n";

	if ( $ARGV[$num] =~ /([^\/\\]*)\.xml/i ) {
		print OUTFILE "INSERT OR REPLACE INTO Palette VALUES ('$1',NULL\n";
	}
	else {
		print OUTFILE "INSERT OR REPLACE INTO Palette VALUES ('Unknown',NULL\n";
	}

	$string1 = '';
	while ( <INFILE> ) {

		$string1 .= $_;

		if ( $string1 =~ /value>\W*([\w]*)\W*<\/value/si ) {
			$val = hex ( $1 );
			print OUTFILE ",$val\n";

			$string1 = '';
		}
	}
	print OUTFILE ");\nEND;";

	close INFILE;
	close OUTFILE;
}
