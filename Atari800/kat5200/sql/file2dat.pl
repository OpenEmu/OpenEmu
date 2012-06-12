#!/usr/local/bin/perl
#
# Take input files, find crc and md5 and put in a dat file
#

use String::CRC32;
use Digest::MD5 qw(md5 md5_hex md5_base64);

$numArgs = $#ARGV + 1;

die "Input ROM Image file required" unless $numArgs > 0;

#
# Create the dat file and write the header
#
open ( OUTFILE, ">Atari-Other-Carts.dat" )  or die "Cound not open file: Atari-Other.dat: $!\n";

print OUTFILE "clrmamepro (\n\tname \"Atari-Other-Carts\"\n\tdescription \"Atari-Other-Carts\"\n\tcategory \"Atari-Other-Carts\"\n\tversion 2008-09-22\n\tauthor Berlin\n)\n";

foreach $num (0 .. $#ARGV ) {
	process_file( $ARGV[$num] );
}

sub process_file
{
	my $data = do{open my $fh, $ARGV[$num] or die "No Dice";binmode($fh);local $/;<$fh>};
	close ( $fh );

	$crc = crc32($data);
	$md5 = md5_hex($data);
	$ARGV[$num] =~ /([^\\\/]*)\..*/; 
	$name = $1;
	$size = -s $ARGV[$num];

	print OUTFILE "game (\n\tname \"$name\"\n\tdescription \"$name\"\n\t";
	printf OUTFILE "rom ( name \"$ARGV[$num]\" size $size crc %x md5 $md5 )\n)\n", $crc;
}

