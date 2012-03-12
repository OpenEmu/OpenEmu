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

print OUTFILEFUN "#define XFLIP 0\n";
print OUTFILEFUN "#define YFLIP 0\n";
print OUTFILEFUN "#define ROT 0\n\n";
print OUTFILEFUN "#define BPP 16\n";

print OUTFILETAB "// Table with all function addresses.\n";
print OUTFILETAB "static RenderTileFunction RenderTileFunctionTable[] = {\n";

for ( my $Size = 320; $Size <= 384; $Size += 64 ) {
	my $RowScroll;
	my $RowSelect;

	print OUTFILEFUN "#define XSIZE $Size\n";
	print OUTFILEFUN "#define EIGHTBIT 1\n";

	my $FunctionName = "&RenderTile16_" . $Size . "_ROT0_NOFLIP";

	for ( my $Function = 0; $Function < 3; $Function++ ) {
		if ( ($Function & 1) == 0 ) {
			$RowScroll = "_NOROWSCROLL";

			if ( ($Function & 2) == 0 ) {
				$RowSelect = "_NOROWSELECT";
			} else {
				$RowSelect = "_ROWSELECT";
			}
		} else {
			$RowScroll = "_ROWSCROLL";
		}

		print OUTFILEFUN "#define ROWSCROLL " . ($Function & 1) . "\n";
		print OUTFILEFUN "#define ROWSELECT " . (($Function & 2) / 2) . "\n";

		print OUTFILETAB "\t";

		print OUTFILEFUN "#define DOCLIP 0\n";
		print OUTFILEFUN "#include \"cave_tile_render.h\"\n";
		print OUTFILEFUN "#undef DOCLIP\n";

		print OUTFILETAB $FunctionName . $RowScroll . $RowSelect . "_NOCLIP_256, ";

		print OUTFILEFUN "#define DOCLIP 1\n";
		print OUTFILEFUN "#include \"cave_tile_render.h\"\n";
		print OUTFILEFUN "#undef DOCLIP\n";

		print OUTFILETAB $FunctionName . $RowScroll . $RowSelect . "_CLIP_256, ";

		print OUTFILEFUN "#undef ROWSELECT\n";
		print OUTFILEFUN "#undef ROWSCROLL\n";

		print OUTFILETAB "\n";
	}
	print OUTFILEFUN "#undef EIGHTBIT\n\n";
	print OUTFILEFUN "#undef XSIZE\n\n";
}

print OUTFILEFUN "#undef BPP\n\n";
print OUTFILEFUN "#undef ROT\n\n";

print OUTFILEFUN "#undef YFLIP\n";
print OUTFILEFUN "#undef XFLIP\n\n";

print OUTFILETAB "};\n\n";

$OutfileTab =~ /(?:.*[\\\/])(.*)/;
print OUTFILEFUN "#include \"$1\"\n";

print OUTFILETAB "static RenderTileFunction* RenderTile_ROT0[2] = {\n\t&RenderTileFunctionTable[0],\n\t&RenderTileFunctionTable[6]\n};\n";

close( OUTFILETAB );
close( OUTFILEFUN );

