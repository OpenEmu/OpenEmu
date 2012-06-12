#!/usr/local/bin/perl
#
# Transform a MAME DAT file to SQL
#
my $tosec_type;
my $media_type;

#
# Obtained from cart.h
#
my $CART_MIN = 0;
my $CART_32K;
my $CART_16K_TWO_CHIP;
my $CART_16K_ONE_CHIP;
my $CART_40K;
my $CART_64K;
my $CART_8K;
my $CART_4K;
my $CART_PC_8K;
my $CART_PC_16K;
my $CART_PC_32K;
my $CART_PC_40K;
my $CART_PC_RIGHT;
my $CART_XEGS;
my $CART_SWXEGS;
my $CART_MEGA;
my $CART_OSS;
my $CART_OSS2;
my $CART_ATMAX;
my $CART_ATRAX;
my $CART_WILL;
my $CART_SDX;
my $CART_EXP;
my $CART_DIAMOND;
my $CART_PHOENIX;
my $CART_BLIZZARD;
my $CART_CART = 26;
my $CART_FLOPPY = 27;
my $CART_CASSETTE = 28;
my $CART_EXE = 29;

$numArgs = $#ARGV + 1;

die "Input DAT file required" unless $numArgs > 0;

foreach $num (0 .. $#ARGV ) {
	process_dat_file( $ARGV[$num] );
}

sub process_dat_file
{

	open ( INFILE, "$ARGV[$num]" ) or die "Cound not open file: $ARGV[1]: $!\n";
	open ( OUTFILE, ">$ARGV[$num].sql" )  or die "Cound not open file: $ARGV[2]: $!\n";
#	open ( OUTFILE, ">-" )        or die "Cound not STDOUT: $!\n";

	print OUTFILE "BEGIN;\n";

	$string1 = '';
	while ( <INFILE> ) {

		$string1 .= $_;

		if ( $string1 =~ /clrmamepro.*\(.*name\W+"?([^"\n]*)"?\W+description.*category.*version\W+([^\n]*)\W+author.*\)/si ) {

			# So after matching we get name and version
			$tosec_name = $1;
			$tosec_version = $2;

			if ( $string1 =~ /Atari 8bit - Applications - \[ATR\]/ ) {
				$tosec_type = 1;
				$media_type = $CART_FLOPPY;
			} 
			elsif ( $string1 =~ /Atari 8bit - Applications - \[BIN\]/ ) {
				$tosec_type = 2;
				$media_type = $CART_CART;
			} 
			elsif ( $string1 =~ /Atari 8bit - Applications - \[CAS\]/ ) {
				$tosec_type = 3;
				$media_type = $CART_CASSETTE;
			} 
			elsif ( $string1 =~ /Atari 8bit - Applications - \[COM\]/ ) {
				$tosec_type = 4;
				$media_type = $CART_EXE;
			} 
			elsif ( $string1 =~ /Atari 8bit - Demos - \[ATR\]/ ) {
				$tosec_type = 5;
				$media_type = $CART_FLOPPY;
			} 
			elsif ( $string1 =~ /Atari 8bit - Demos - \[COM\]/ ) {
				$tosec_type = 6;
				$media_type = $CART_EXE;
			} 
			elsif ( $string1 =~ /Atari 8bit - Games - \[ATR\]/ ) {
				$tosec_type = 7;
				$media_type = $CART_FLOPPY;
			} 
			elsif ( $string1 =~ /Atari 8bit - Games - \[BIN\]/ ) {
				$tosec_type = 8;
				$media_type = $CART_CART;
			} 
			elsif ( $string1 =~ /Atari 8bit - Games - \[CAS\]/ ) {
				$tosec_type = 9;
				$media_type = $CART_CASSETTE;
			} 
			elsif ( $string1 =~ /Atari 8bit - Games - \[XEX\]/ ) {
				$tosec_type = 10;
				$media_type = $CART_EXE;
			} 
			elsif ( $string1 =~ /Atari 8bit - Magazines - \[ATR\]/ ) {
				$tosec_type = 11;
				$media_type = $CART_FLOPPY;
			} 
			elsif ( $string1 =~ /Atari 8bit - Magazines - \[COM\]/ ) {
				$tosec_type = 12;
				$media_type = $CART_EXE;
			} 
			elsif ( $string1 =~ /Atari 8bit - Operating Systems/ ) {
				$tosec_type = 13;
				$media_type = 0;
			} 
			elsif ( $string1 =~ /Atari 5200 - Applications/ ) {
				$tosec_type = 14;
				$media_type = $CART_CART;
			} 
			elsif ( $string1 =~ /Atari 5200 - BIOS/ ) {
				$tosec_type = 15;
				$media_type = 0;
			} 
			elsif ( $string1 =~ /Atari 5200 - Demos/ ) {
				$tosec_type = 16;
				$media_type = $CART_CART;
			} 
			elsif ( $string1 =~ /Atari 5200 - Games/ ) {
				$tosec_type = 17;
				$media_type = $CART_CART;
			} 
			elsif ( $string1 =~ /Carts/ ) {
				$tosec_type = 0;
				$media_type = $CART_CART;
			}
			else {
				$tosec_type = 0;
				$media_type = 0;
			}

			print OUTFILE "INSERT OR REPLACE INTO TosecTypes VALUES ($tosec_type,'$tosec_name','$tosec_version');\n";

			$string1 = '';
		}

		if ( $string1 =~ /game.*\(.*name\W+"?([^"\n]*)"?\W+description.*rom.*\(.*name.*size\W+([\w]*)\W+crc\W+([\w]*)\W+md5\W+([\w]*)\W+\)\W+\)/si ) {

			$crc = hex ( $3 );
			$name = $1;
			$size = $2;
			$md5 = $4;
			$ram = "0";
			if ( ($tosec_type > 0 && $tosec_type < 14) || $tosec_name =~ 800  ) {
				$machine_type = 0;
			}
			else {
				$machine_type = 1;
			}
			$name =~ s/'/''/g;
			if ( $name =~ /\[BASIC\]/i ) {
				$basic = "1";
			}
			else {
				$basic = "2";
			}
			if ( $name =~ /\[Req OSb\]/i ) {
				$machine_type = 2;
			}
			if ( $name =~ /\[Req 128k\]/i ) {
				$machine_type = 3;
				$ram = "128";
			}
			if ( $name =~ /\[Req 64k\]/i ) {
				$machine_type = 3;
				$ram = "64";
			}
			if ( $name =~ /\[Req 16k\]/i ) {
				$ram = "16";
			}
			print OUTFILE "INSERT OR REPLACE INTO Media VALUES ('$name',NULL,$size,$crc,'$md5',NULL,$machine_type,0,$media_type,NULL,NULL,NULL,1,$basic,2,0,$ram,$tosec_type);\n";

			$string1 = '';
		}
	}
	print OUTFILE "END;";

	close INFILE;
	close OUTFILE;
}
