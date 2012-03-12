#!/usr/bin/perl -w

use strict;

my $Outfile;
my $Listfile;

my @Filelist;
my %Drivers;
my @Driverlist;

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

	# gamelist.txt
	if ( $ARGV[$i] =~ /^-l/i ) {
		if ( $ARGV[$i] =~ /^-l$/i ) {
			$i++;
			if ( $i < scalar @ARGV ) {
				$Listfile = $ARGV[$i];
			}
		} else {
			$ARGV[$i] =~ /(?<=-l)(.*)/i;
			$Listfile = $1;
		}
		next;
	}

	if ( opendir( INDIR, $ARGV[$i] ) ) {;
		# Argument is a dir
		my @dirlist = readdir( INDIR ) or die "\nError: Couldn't read directory $ARGV[$i] $!";
		foreach my $filename ( @dirlist ) {
			if ( $filename =~ /d\w?_.+\.cpp/ ) {
				push( @Filelist, "$ARGV[$i]/$filename" );
			}
		}
		closedir( INDIR );
		next;
	} else {
		# Argument is a file
		push( @Filelist, $ARGV[$i] );
		next;
	}
}}

unless ( $Outfile and scalar @Filelist ) {
	die "Usage: $0 -o <output file> [-l <gamelist>] <input file|dir> [<input file|dir>...]\n\n";
}

open( OUTFILE, ">$Outfile" ) or die "\nError: Couldn't open OUTPUT file $Outfile $!";

print "Generating gamelist in $Outfile...\n";

# Build a list of all drivers
foreach my $filename ( @Filelist ) {
	open( INFILE, $filename ) or die "\nError: Couldn't read $filename $!";
	while ( my $line = <INFILE> ) {

		# Strip C++ style // comments
		if ( $line =~ /(.*?)\/\// ) {
			$line = $1;
		}

		# Strip C style /* comments */
		if ( $line =~ /(.*)\/\*/ ) {
			my $temp = $1;
			while ( $line and !($line =~ /\*\/(.*)/) ) {
				$line = <INFILE>;
			}
			$line = $temp;
			substr( $line, length( $line ) ) = $1;
		}

		# get included .cpp.h files in the same directory
		if ( $line =~ /^\s*#include "d_\w*.h"/ ) {
			$line =~ /^\s*#include "(.*)"/;
			my $newfile = $1;
			$filename =~ /(.*[\/\\])[^\/\\]/;
			push( @Filelist, "$1$newfile" );
		}

		if ( $line =~ /struct\s+BurnDriver([D|X]?)\s+(\w+)(.*)/ ) {

			# We're at the start of a BurnDriver declaration

			# Create a hash key with the name of the structure,
			# Fill the first element of the array with the driver status
			my $name = $2;
			$Drivers{$name}[0] = "$1";

			# Read the Burndriver structure into a variable
			my $struct = $3;
			do {
				$line = <INFILE>;

				# Strip C++ style // comments
				if ( $line =~ /(.*?)\/\// ) {
					$line = $1;
				}

				substr( $struct, length( $struct ) ) = $line;
			} until $line =~ /;/;

			# Strip anything after the ;
			$struct =~ /(.*;)/s;
			$struct = $1;

			# Strip C style /* comments */
			while ( $struct =~ /(.*)\/\*.*?\*\/(.*)/s ) {
				$struct = $1;
				substr( $struct, length( $struct ) ) = $2;
			}

			# We now have the complete Burndriver structure without comments

			# Extract the strings from the variable
			$struct =~ /\s*(".*"|null|0)\s*,\s*(".*"|null|0)\s*,\s*(".*"|null|0)\s*,\s*(".*"|null|0)\s*,\s*(".*"|null|0)\s*,\s*(".*"|null|0)\s*,\s*(".*"|null|0)\s*,\s*(".*"|null|0)\s*,\s*(".*"|null|0),\s*(L".*"|null|0)\s*,\s*(L".*"|null|0)\s*,\s*(L".*"|null|0)\s*,\s*(L".*"|null|0)\s*,\s*(\d|[^,]*)/si;

			$Drivers{$name}[1] = $1;						# Name
			$Drivers{$name}[2] = $6;						# Full name
			$Drivers{$name}[3] = $7;						# Remarks
			$Drivers{$name}[4] = $8;						# Company
			$Drivers{$name}[5] = $9;						# Hardware
			$Drivers{$name}[6] = $5;						# Year of release
			$Drivers{$name}[7] = $2;						# Parent

			if ( $14 ne "1" && ($14 =~ /BDF_GAME_WORKING/) == 0 ) {			# Working status
				$Drivers{$name}[8] = "NOT WORKING";
			} else {
				$Drivers{$name}[8] = "";
			}
			$Drivers{$name}[9] = "$filename";								# Filename

			# Convert NULL/null/0 to empty string or remove quotes
			foreach $line ( @{$Drivers{$name}} ) {
				if ( $line =~ /^(null|NULL|0)/ ) {
					$line = "";
				} else {
					$line =~ /(^"?)(.*)\1/;
					$line = $2;
				}
			}

			# We only want the 1st name
			$Drivers{$name}[2] =~ /(.*)\\0.*/;
			$Drivers{$name}[2] = $1;

			$Drivers{$name}[10] = $Drivers{$name}[2];
			if ( $struct =~ /BDF_PROTOTYPE/ || $struct =~ /BDF_BOOTLEG/ || $Drivers{$name}[3] ne "" ) {
				$Drivers{$name}[10] = "$Drivers{$name}[10]" . " [";
			}
			if ( $struct =~ /BDF_PROTOTYPE/ ) {
				$Drivers{$name}[10] = "$Drivers{$name}[10]" . "Prototype";
				if ( $struct =~ /BDF_BOOTLEG/ || $Drivers{$name}[3] ne "" ) {
					$Drivers{$name}[10] = "$Drivers{$name}[10]" . ", ";
				}
			}
			if ( $struct =~ /BDF_BOOTLEG/ ) {
				$Drivers{$name}[10] = "$Drivers{$name}[10]" . "Bootleg";
				if ( $Drivers{$name}[3] ne "" ) {
					$Drivers{$name}[10] = "$Drivers{$name}[10]" . ", ";
				}
			}
			if ( $Drivers{$name}[3] ne "" ) {
				$Drivers{$name}[10] = "$Drivers{$name}[10]" . "$Drivers{$name}[3]";
			}
			if ( $struct =~ /BDF_PROTOTYPE/ || $struct =~ /BDF_BOOTLEG/ || $Drivers{$name}[3] ne "" ) {
				$Drivers{$name}[10] = "$Drivers{$name}[10]" . "]";
			}
		}
	}
	close( INFILE );
}

# Sort the list of drivers using the decorated game name as the key
@Driverlist = sort {
	lc( $Drivers{$a}[10] ) cmp lc( $Drivers{$b}[10] );
} keys %Drivers;

# first define needed constants and include needed files
print OUTFILE << "CPPEND";
// This file was generated by $0 (perl $])

// Declaration of all drivers
#define DRV extern struct BurnDriver
CPPEND

my $Debug = 0;
foreach my $name ( @Driverlist ) {

	if ( $Drivers{$name}[0] eq "X" ) {
		print OUTFILE "//";
	} else {
		if ( $Drivers{$name}[0] eq "D" ) {
			unless ( $Debug == 1) {
				$Debug = 1;
				print OUTFILE "#if defined FBA_DEBUG\n";
			}
		} else {
			if ( $Debug == 1 ) {
				$Debug = 0;
				print OUTFILE "#endif\n";
			}
		}
		print OUTFILE "DRV";
	}
	print OUTFILE "\t\t$name;";
	if ( $Drivers{$name}[0] eq "D" or $Drivers{$name}[0] eq "X" ) {
		my $column = int( (9 + length( $name )) / 4 );
		while ( $column < 8 ) {
			print OUTFILE "\t";
			$column ++;
		}
		print OUTFILE "// ";
		if ( $Drivers{$name}[3] ) {
		print OUTFILE "$Drivers{$name}[3]";
		} else {
			print OUTFILE "no comment";
		}
		if ( $Drivers{$name}[8] ) {
			print OUTFILE " [$Drivers{$name}[8]]";
		}
		print OUTFILE "\n";
	} else {
		print OUTFILE "\n";
	}
}
if ( $Debug == 1 ) {
	print OUTFILE "#endif\n";
}

print OUTFILE << "CPPEND";
#undef DRV

// Structure containing addresses of all drivers
// Needs to be kept sorted (using the full game name as the key) to prevent problems with the gamelist in Kaillera
static struct BurnDriver* pDriver[] = {
CPPEND

$Debug = 0;
foreach my $name ( @Driverlist ) {
	if ( $Drivers{$name}[0] eq "X" ) {
		print OUTFILE "//";
	} else {
		if ( $Drivers{$name}[0] eq "D" ) {
			unless ( $Debug == 1) {
				$Debug = 1;
				print OUTFILE "#if defined FBA_DEBUG\n";
			}
		} else {
			if ( $Debug == 1 ) {
				$Debug = 0;
				print OUTFILE "#endif\n";
			}
		}
	}
	print OUTFILE "\t&$name,";
	my $column = int( (6 + length( $name )) / 4 );
	while ( $column < 8 ) {
		print OUTFILE "\t";
		$column++;
	}
	print OUTFILE "// $Drivers{$name}[2]";
	if ( $Drivers{$name}[0] eq "D" or $Drivers{$name}[0] eq "X" ) {
		print OUTFILE " [";
		if ( $Drivers{$name}[3] ) {
			print OUTFILE "$Drivers{$name}[3]";
		} else {
			print OUTFILE "no comment";
		}
		if ( $Drivers{$name}[8] ) {
			print OUTFILE ", $Drivers{$name}[8]";
		}
		print OUTFILE "]\n";
	} else {
		if ( $Drivers{$name}[8] ) {
			print OUTFILE " [";
			if ( $Drivers{$name}[3] ) {
				print OUTFILE "$Drivers{$name}[3]";
			} else {
				print OUTFILE "no comment";
			}
			print OUTFILE ", $Drivers{$name}[8]]";
		}
		print OUTFILE "\n";
	}
}
if ( $Debug == 1 ) {
	print OUTFILE "#endif\n";
}

print OUTFILE "};\n";

close( OUTFILE );

if ( $Listfile ) {

	open( OUTFILE, ">$Listfile" ) or die "\nError: Couldn't open OUTPUT file $Listfile $!";

	print "Generating gamelist.txt...\n";

	print OUTFILE "This list contains all games supported by FB Alpha.\x0d\x0a\x0d\x0a";

	print OUTFILE scalar @Driverlist . " games supported in total; ";
	print OUTFILE "X = excluded from build; ";
	print OUTFILE "D = included in debug build only; ";
	print OUTFILE "NW = not working\x0d\x0a\x0d\x0a";

	print OUTFILE "  name\t\t  status  full name\t\t\t\t\t\t  parent\t  year\t  company\t  hardware\t  remarks\x0d\x0a";
	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "+-------";
	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "+-------";
	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------+\x0d\x0a";

	my $Text;

	foreach my $name ( @Driverlist ) {

		print OUTFILE "| $Drivers{$name}[1]";
		my $column = int( (2 + length( $Drivers{$name}[1] )) / 8 );
		while ( $column < 2 ) {
			print OUTFILE "\t";
			$column++;
		}

		print OUTFILE "| ";
		if ( $Drivers{$name}[0] ) {
			print OUTFILE "$Drivers{$name}[0]";
		} else {
			print OUTFILE " ";
		}
		if ( $Drivers{$name}[8] ) {
			print OUTFILE " NW";
		}
		print OUTFILE "\t";
		$column++;

		$Text = substr( $Drivers{$name}[2], 0, 54);
		print OUTFILE "| $Text";
		$column += int( (2 + length( $Text )) / 8 );
		while ( $column < 10 ) {
			print OUTFILE "\t";
			$column++;
		}

		print OUTFILE "| $Drivers{$name}[7]";
		$column += int( (2 + length( $Drivers{$name}[7] )) / 8 );
		while ( $column < 12 ) {
			print OUTFILE "\t";
			$column++;
		}
		print OUTFILE "| $Drivers{$name}[6]";
		$column += int( (2 + length( $Drivers{$name}[6] )) / 8 );
		while ( $column < 13 ) {
			print OUTFILE "\t";
			$column++;
		}

		$Text = substr( $Drivers{$name}[4], 0, 14);
		print OUTFILE "| $Text";
		$column += int( (2 + length( $Text )) / 8 );
		while ( $column < 15 ) {
			print OUTFILE "\t";
			$column++;
		}

		$Text = substr( $Drivers{$name}[5], 0, 14);
		print OUTFILE "| $Text";
		$column += int( (2 + length( $Text )) / 8 );
		while ( $column < 17 ) {
			print OUTFILE "\t";
			$column++;
		}

		$Text = substr( $Drivers{$name}[3], 0, 38);
		print OUTFILE "| $Text";
		$column += int( ( 2 + length( $Text )) / 8 );
		while ( $column < 22 ) {
			print OUTFILE "\t";
			$column++;
		}
		print OUTFILE "|\x0d\x0a";
	}

	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "+-------";
	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "+-------";
	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "+-------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------";
	print OUTFILE "--------+\x0d\x0a";

	close( OUTFILE );
}
