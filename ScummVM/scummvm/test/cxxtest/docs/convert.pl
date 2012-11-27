#!/usr/bin/perl

die "Usage: $0 <text file> <html file> <TexInfo file>\n"
  unless scalar @ARGV == 3;

my ($text, $html, $texi) = @ARGV;

open TEXT, "<$text" or die "Cannot open text file \"$text\"\n";
open HTML, ">$html" or die "Cannot create html file \"$html\"\n";
open TEXI, ">$texi" or die "Cannot create TexInfo file \"$texi\"\n";

print HTML "<html>";

sub analyze($) {
  my ($line) = @_;
  my ($htmlLine, $texiLine) = ($line, $line);

  # command line options
  $texiLine =~ s/ (--?[a-z-]*)/ \@option{$1}/g;
  $htmlLine =~ s/ (--?[a-z-]*)/ <tt>$1<\/tt>/g;

  # [Class::]function()
  $texiLine =~ s/([^A-Za-z])(([A-Z][A-Za-z0-9]*::)?[A-Za-z0-9]+\(\))/$1\@code{$2}/g;
  $htmlLine =~ s/([^A-Za-z])(([A-Z][A-Za-z0-9]*::)?[A-Za-z0-9]+\(\))/$1<code>$2<\/code>/g;

  # `file'
  $texiLine =~ s/`([A-Za-z.\/]*)'/\@file{$1}/g;
  $htmlLine =~ s/`([A-Za-z.\/]*)'/<tt>`$1'<\/tt>/g;

  # TS...
  $texiLine =~ s/(^|[^A-Z])(TS[A-Za-z_*()]*)/$1\@code{$2}/g;
  $htmlLine =~ s/(^|[^A-Z])(TS[A-Za-z_*()]*)/$1<code>$2<\/code>/g;

  # CXXTEST_
  $texiLine =~ s/(CXXTEST_[A-Z_]*)/\@code{$1}/g;
  $htmlLine =~ s/(CXXTEST_[A-Z_]*)/<tt>$1<\/tt>/g;

  return ($htmlLine, $texiLine);
}

my $line;
my $inRelease = 0;
while ( defined( $line = <TEXT> ) ) {
  chomp $line;
  if ( $line =~ m/^CxxTest Releases/ ) {
    print HTML "<title>CxxTest Releases</title>\n";
    print HTML "<h1>CxxTest Releases</h1>\n\n";

    print TEXI "\@appendix Version history\n";
    print TEXI "\@itemize \@bullet\n";
  }
  elsif ( $line =~ m/^(.*):$/ ) {
    if ( $inRelease ) {
      print HTML "</ul>\n\n";
      print TEXI "\@end itemize\n";
    }

    print HTML "<h2>$1</h2>\n";
    print HTML "<ul>\n";

    print TEXI "\@item\n\@strong{$1}\n";
    print TEXI "\@itemize \@minus\n";

    $inRelease = 1;
  }
  elsif ( $line =~ m/^ - (.*)$/ ) {
    my ($htmlLine, $texiLine) = analyze($1);
    print HTML "<li>$htmlLine</li>\n";
    print TEXI "\@item\n$texiLine\n";
  }
}

if ( $inRelease ) {
  print HTML "</ul>\n\n";
  print TEXI "\@end itemize\n\n";
}

print HTML "</html>\n";
print TEXI "\@end itemize\n";

close TEXT or die "Error closing text file \"$text\"\n";
close HTML or die "Error closing html file \"$html\"\n";
close TEXI or die "Error closing TexInfo file \"$texi\"\n";
