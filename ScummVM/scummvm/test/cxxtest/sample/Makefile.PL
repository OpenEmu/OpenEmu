#!/usr/bin/perl
#
# This isn't a "real" `Makefile.PL'
# It just copies the correct `Makefile.*' to `Makefile'
#
use strict;
use Getopt::Long;
use File::Copy;

sub usage() {
  die "Usage: $0 [--bcc32]\n";
}

my $source;
my $target = 'Makefile';
my $windows = $ENV{'windir'};

GetOptions( 'bcc32' => sub { $source = 'Makefile.bcc32'  } ) or usage();
if ( !defined( $source ) ) {
  $source = $windows ? 'Makefile.msvc' : 'Makefile.unix';
}

unlink($target);
$windows ? copy($source, $target) : symlink($source, $target);

print "`Makefile' is now `$source'.\n";

#
# Local Variables:
# compile-command: "perl Makefile.PL"
# End:
#
