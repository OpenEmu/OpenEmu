#!/usr/bin/perl -w

use strict;

my $OutfileFun;
my $OutfileTab;

# Process command line arguments
if ( $ARGV[0] =~ /^-o/i ) {
   if ( $ARGV[0] =~ /^-o$/i ) {
      $OutfileFun = $ARGV[1];
   } else {
      $ARGV[0] =~ /(?<=-o)(.*)/i;
      $OutfileFun = $1;
   }
}

unless ( $OutfileFun ) {
   die "Usage: $0 -o <output file>\n\n";
}

$OutfileTab = $OutfileFun;
$OutfileTab =~ s/(\.\w+)/_table$1/;

open( OUTFILEFUN, ">$OutfileFun" ) or die "\nError: Couldn't open OUTPUT file $OutfileFun: $!";
open( OUTFILETAB, ">$OutfileTab" ) or die "\nError: Couldn't open OUTPUT file $OutfileTab: $!";

print "Generating $OutfileFun & $OutfileTab...\n";

# Generate optimized functions and a table with the function names

print OUTFILETAB "// Table with all function addresses.\n";
print OUTFILETAB "static RenderBankFunction RenderBankFunctionTable[] = {\n";

print OUTFILEFUN "#define ISOPAQUE 0\n\n";

for ( my $Bitdepth = 16; $Bitdepth <= 32; $Bitdepth += 8 ) {
	print OUTFILEFUN "// " . $Bitdepth . "-bit rendering functions.\n";
	print OUTFILEFUN "#define BPP $Bitdepth\n\n";

	my $FunctionName = "&RenderBank" . $Bitdepth;
	my $DoClip;

	for ( my $Function = 0; $Function < 32; $Function++ ) {
		if ( ($Function & 15) == 0 ) {
			print OUTFILEFUN "#define DOCLIP " . ($Function & 16) / 16 . "\n";

			if ( ($Function & 16) == 0 ) {
				$DoClip = "_NOCLIP";
			} else {
				$DoClip = "_CLIP";
			}
			print OUTFILETAB "\t";
		}

		print OUTFILEFUN "#define XZOOM " . ($Function & 15) . "\n";
		print OUTFILEFUN "#include \"neo_sprite_render.h\"\n";
		print OUTFILEFUN "#undef XZOOM\n";

		print OUTFILETAB "$FunctionName" . "_ZOOM" . ($Function & 15) . $DoClip . "_TRANS, ";

		if ( ($Function & 15) == 15 ) {
			print OUTFILEFUN "#undef DOCLIP\n";

			print OUTFILETAB "\n";
		}

	}
	print OUTFILEFUN "#undef BPP\n\n";
}
print OUTFILEFUN "#undef ISOPAQUE\n\n";

$OutfileTab =~ /(?:.*[\\\/])(.*)/;
print OUTFILEFUN "#include \"$1\"\n";

print OUTFILETAB "};\n\n";
print OUTFILETAB "static RenderBankFunction* RenderBankNormal[3] = {\n\t&RenderBankFunctionTable[0],\n\t&RenderBankFunctionTable[32],\n\t&RenderBankFunctionTable[64]\n};\n";

close( OUTFILETAB );
close( OUTFILEFUN );

