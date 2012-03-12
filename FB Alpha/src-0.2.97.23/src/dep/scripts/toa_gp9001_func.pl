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
print OUTFILETAB "static RenderTileFunction RenderTileFunctionTable[] = {\n";

for ( my $Rot = 0; $Rot <= 270; $Rot += 270) {
	my @Flip = ( "_NOFLIP", "_FLIPX", "_FLIPY", "_FLIPXY" );
	my $IsOpaque;

	if ( $Rot == 270 ) {
		print OUTFILEFUN "#ifdef DRIVER_ROTATION\n";

		print OUTFILETAB "#ifdef DRIVER_ROTATION\n";
	}

	print OUTFILEFUN "#define ROT $Rot\n\n";
	for ( my $Bitdepth = 16; $Bitdepth <= 32; $Bitdepth += 8 ) {
		print OUTFILEFUN "// " . $Bitdepth . "-bit rendering functions.\n";
		print OUTFILEFUN "#define BPP $Bitdepth\n\n";

		my $FunctionName = "&RenderTile" . $Bitdepth . "_ROT" . "$Rot";

		for ( my $Function = 0; $Function < 8; $Function++ ) {
			if ( ($Function & 3) == 0 ) {
				print OUTFILEFUN "#define ISOPAQUE " . ($Function & 4) / 4 . "\n";

				print OUTFILETAB "\t";

				if ( ($Function & 4) == 0 ) {
					$IsOpaque = "_TRANS";
				} else {
					$IsOpaque = "_OPAQUE";
				}
			}
			print OUTFILEFUN "#define XFLIP " . ($Function & 1) . "\n";
			print OUTFILEFUN "#define YFLIP " . ($Function & 2) / 2 . "\n";

			print OUTFILEFUN "#define DOCLIP 0\n";
			print OUTFILEFUN "#include \"toa_gp9001_render.h\"\n";
			print OUTFILEFUN "#undef DOCLIP\n";

			print OUTFILETAB $FunctionName . $Flip[$Function & 3] . "_NOCLIP" . $IsOpaque . ", ";

			print OUTFILEFUN "#define DOCLIP 1\n";
			print OUTFILEFUN "#include \"toa_gp9001_render.h\"\n";
			print OUTFILEFUN "#undef DOCLIP\n";

			print OUTFILETAB $FunctionName . $Flip[$Function & 3] . "_CLIP" . $IsOpaque . ", ";

			print OUTFILEFUN "#undef YFLIP\n";
			print OUTFILEFUN "#undef XFLIP\n\n";

			if ( ($Function & 3) == 3 ) {
				print OUTFILEFUN "#undef ISOPAQUE\n\n";

				print OUTFILETAB "\n";
			}
		}
		print OUTFILEFUN "#undef BPP\n\n";
	}
	print OUTFILEFUN "#undef ROT\n\n";

	if ( $Rot == 270 ) {
		print OUTFILEFUN "#endif\n";

		print OUTFILETAB "#endif\n";
	}
}

print OUTFILEFUN "\n";

print OUTFILETAB "};\n\n";

$OutfileTab =~ /(?:.*[\\\/])(.*)/;
print OUTFILEFUN "#include \"$1\"\n";

print OUTFILETAB "static RenderTileFunction* RenderTile_ROT0[3] = {\n\t&RenderTileFunctionTable[0],\n\t&RenderTileFunctionTable[16],\n\t&RenderTileFunctionTable[32]\n};\n";
print OUTFILETAB "#ifdef DRIVER_ROTATION\n";
print OUTFILETAB "static RenderTileFunction* RenderTile_ROT270[3] = {\n\t&RenderTileFunctionTable[48],\n\t&RenderTileFunctionTable[64],\n\t&RenderTileFunctionTable[80]\n};\n";
print OUTFILETAB "#endif\n";

close( OUTFILETAB );
close( OUTFILEFUN );

