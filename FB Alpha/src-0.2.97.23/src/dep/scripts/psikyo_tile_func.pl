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

my @TransFun = ( 0, 15, -1 );
my @TransTab = ( "_TRANS0", "_TRANS15", "_SOLID" );

my $RowScroll;
my $Clip;

print OUTFILEFUN "#define ROT 0\n";
print OUTFILEFUN "#define BPP 16\n";
print OUTFILEFUN "#define FLIP 0\n";
print OUTFILEFUN "#define ZOOM 0\n";
print OUTFILEFUN "#define ZBUFFER 0\n";
print OUTFILEFUN "\n";

print OUTFILETAB "// Table with all function addresses.\n";
print OUTFILETAB "static RenderTileFunction RenderTile[] = {\n";

for ( my $Function = 0; $Function < 6; $Function++ ) {
	my $FunctionName = "&RenderTile16" . $TransTab[($Function & 6) / 2] . "_NOFLIP_ROT0";

	if ( ($Function & 1) == 0 ) {
		$RowScroll = "_NOROWSCROLL";
		print OUTFILEFUN "#define TRANS " . $TransFun[($Function & 6) / 2] . "\n\n";
	} else {
		$RowScroll = "_ROWSCROLL";
	}

	print OUTFILEFUN "#define ROWSCROLL " . (($Function & 1) / 1) . "\n";
	print OUTFILETAB "\t";

	print OUTFILEFUN "#define DOCLIP 0\n";
	print OUTFILEFUN "#include \"psikyo_render.h\"\n";
	print OUTFILEFUN "#undef DOCLIP\n";

	print OUTFILETAB $FunctionName . $RowScroll . "_NOZOOM_NOZBUFFER_NOCLIP, ";

	print OUTFILEFUN "#define DOCLIP 1\n";
	print OUTFILEFUN "#include \"psikyo_render.h\"\n";
	print OUTFILEFUN "#undef DOCLIP\n";

	print OUTFILETAB $FunctionName . $RowScroll . "_NOZOOM_NOZBUFFER_CLIP, ";

	print OUTFILEFUN "#undef ROWSCROLL\n";
	if ( ($Function & 1) == 1 ) {
		print OUTFILEFUN "\n#undef TRANS\n";
	}

	print OUTFILETAB "\n";
}

print OUTFILEFUN "#undef ZBUFFER\n";
print OUTFILEFUN "#undef ZOOM\n";
print OUTFILEFUN "#undef FLIP\n";
print OUTFILEFUN "#undef BPP\n";
print OUTFILEFUN "#undef ROT\n";
print OUTFILEFUN "\n";

print OUTFILETAB "};\n\n";

$OutfileTab =~ /(?:.*[\\\/])(.*)/;
print OUTFILEFUN "#include \"$1\"\n";

close( OUTFILETAB );
close( OUTFILEFUN );

