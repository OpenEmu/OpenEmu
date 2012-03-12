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

print OUTFILEFUN "#define EIGHTBIT 1\n";
print OUTFILEFUN "#define ROT 0\n\n";
print OUTFILEFUN "#define BPP 16\n";

print OUTFILETAB "// Table with all function addresses.\n";
print OUTFILETAB "static RenderSpriteFunction RenderSpriteFunctionTable[] = {\n";

for ( my $Size = 320; $Size <= 384; $Size += 64 ) {
	my @ZBuffer = ( "_NOZBUFFER", "_RZBUFFER", "_WZBUFFER", "_RWZBUFFER" );
	my $Flip;
	my $Zoom;

	print OUTFILEFUN "#define XSIZE $Size\n";

	my $FunctionName = "&RenderSprite16_" . $Size . "_ROT0";

	# No scaling

	print OUTFILEFUN "#define ZOOM 0\n";

	for ( my $Function = 0; $Function < 8; $Function++ ) {
		if ( ($Function & 3) == 0 ) {
			print OUTFILEFUN "#define XFLIP " . (($Function & 4) / 4) . "\n";

			if ( ($Function & 4) == 0 ) {
				$Flip = "_NOFLIP";
			} else {
				$Flip = "_FLIPX";
			}

			print OUTFILETAB "\t";
		}

		print OUTFILEFUN "#define ZBUFFER " . ($Function & 3) . "\n";

		print OUTFILEFUN "#include \"cave_sprite_render.h\"\n";

		print OUTFILETAB $FunctionName . $Flip . "_NOZOOM_CLIPX" . $ZBuffer[$Function & 3] . "_256, ";

		print OUTFILEFUN "#undef ZBUFFER\n";

		if ( ($Function & 3) == 3 ) {
			print OUTFILEFUN "#undef XFLIP\n";

			print OUTFILETAB "\n";
		}
	}

	print OUTFILEFUN "#undef ZOOM\n\n";

	# Scale up/down

	print OUTFILEFUN "#define XFLIP 0\n";

	for ( my $Function = 0; $Function < 8; $Function++ ) {
		if ( ($Function & 3) == 0 ) {
			print OUTFILEFUN "#define ZOOM " . (1 + ($Function & 4) / 4) . "\n";

			if ( ($Function & 4) == 0 ) {
				$Zoom = "_ZOOMOUT";
			} else {
				$Zoom = "_ZOOMIN";
			}

			print OUTFILETAB "\t";
		}
		print OUTFILEFUN "#define ZBUFFER " . ($Function & 3) . "\n";
		print OUTFILEFUN "#include \"cave_sprite_render_zoom.h\"\n";
		print OUTFILEFUN "#undef ZBUFFER\n";

		print OUTFILETAB $FunctionName . "_NOFLIP" . $Zoom . "_NOCLIP" . $ZBuffer[$Function & 3] . "_256, ";

		if ( ($Function & 3) == 3 ) {
			print OUTFILEFUN "#undef ZOOM\n\n";

			print OUTFILETAB "\n";
		}
	}

	print OUTFILEFUN "#undef XFLIP\n";
	print OUTFILEFUN "#undef XSIZE\n\n";
}

print OUTFILEFUN "#undef BPP\n\n";
print OUTFILEFUN "#undef ROT\n\n";
print OUTFILEFUN "#undef EIGHTBIT\n\n";

print OUTFILETAB "};\n\n";

$OutfileTab =~ /(?:.*[\\\/])(.*)/;
print OUTFILEFUN "#include \"$1\"\n";

print OUTFILETAB "static RenderSpriteFunction* RenderSprite_ROT0[2] = {\n\t&RenderSpriteFunctionTable[0],\n\t&RenderSpriteFunctionTable[16]\n};\n";

close( OUTFILETAB );
close( OUTFILEFUN );
