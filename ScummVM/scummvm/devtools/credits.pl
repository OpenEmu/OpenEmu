#!/usr/bin/perl
#
# This tools is kind of a hack to be able to maintain the credits list of
# ScummVM in a single central location. We then generate the various versions
# of the credits in other places from this source. In particular:
# - The AUTHORS file
# - The gui/credits.h header file
# - The Credits.rtf file used by the Mac OS X port
# - The credits.xml file, part of the DocBook manual
# - Finally, credits.xml, for use on the website (different format than the DocBook one)
#
# Initial version written by Fingolfin in December 2004.
#


use strict;
use Text::Wrap;

if ($Text::Wrap::VERSION < 2001.0929) {
	die "Text::Wrap version >= 2001.0929 is required. You have $Text::Wrap::VERSION\n";
}

my $mode = "";
my $max_name_width;

# Count the level in the section hierarchy, i.e. how deep we are currently nested
# in terms of 'sections'.
my $section_level = 0;

# Count how many sections there have been on this level already
my @section_count = ( 0, 0, 0 );

if ($#ARGV >= 0) {
	$mode = "TEXT" if ($ARGV[0] eq "--text");	# AUTHORS file
	$mode = "XML-WEB" if ($ARGV[0] eq "--xml-website");	# credits.xml (for use on the website)
	$mode = "CPP" if ($ARGV[0] eq "--cpp");		# credits.h (for use by about.cpp)
	$mode = "XML-DOC" if ($ARGV[0] eq "--xml-docbook");		# credits.xml (DocBook)
	$mode = "RTF" if ($ARGV[0] eq "--rtf");		# Credits.rtf (Mac OS X About box)
}

if ($mode eq "") {
	print STDERR "Usage: $0 [--text | --xml-website | --cpp | --xml-docbook | --rtf]\n";
	print STDERR " Just pass --text / --xml-website / --cpp / --xml-docbook / --rtf as parameter, and credits.pl\n";
	print STDERR " will print out the corresponding version of the credits to stdout.\n";
	exit 1;
}

$Text::Wrap::unexpand = 0;
if ($mode eq "TEXT") {
	$Text::Wrap::columns = 78;
	$max_name_width = 23; # The maximal width of a name.
} elsif ($mode eq "CPP") {
	$Text::Wrap::columns = 48;	# Approx.
}

# Convert HTML entities to ASCII for the plain text mode
sub html_entities_to_ascii {
	my $text = shift;

	# For now we hardcode these mappings
	# &aacute;  -> a
	# &eacute;  -> e
	# &igrave;  -> i
	# &oacute;  -> o
	# &oslash;  -> o
	# &ouml;    -> o / oe
	# &auml;    -> a
	# &uuml;    -> ue
	# &aring;   -> aa
	# &amp;     -> &
	# &#322;    -> l
	# &#347;    -> s
	# &Scaron;  -> S
	$text =~ s/&aacute;/a/g;
	$text =~ s/&eacute;/e/g;
	$text =~ s/&igrave;/i/g;
	$text =~ s/&oacute;/o/g;
	$text =~ s/&oslash;/o/g;
	$text =~ s/&#322;/l/g;
	$text =~ s/&#347;/s/g;
	$text =~ s/&Scaron;/S/g;
	$text =~ s/&aring;/aa/g;

	$text =~ s/&auml;/a/g;
	$text =~ s/&uuml;/ue/g;
	# HACK: Torbj*o*rn but G*oe*ffringmann and R*oe*ver and J*oe*rg
	$text =~ s/Torbj&ouml;rn/Torbjorn/g;
	$text =~ s/&ouml;/oe/g;

	$text =~ s/&amp;/&/g;

	return $text;
}

# Convert HTML entities to C++ characters
sub html_entities_to_cpp {
	my $text = shift;

	# The numerical values are octal!
	$text =~ s/&aacute;/\\341/g;
	$text =~ s/&eacute;/\\351/g;
	$text =~ s/&igrave;/\\354/g;
	$text =~ s/&oacute;/\\363/g;
	$text =~ s/&oslash;/\\370/g;
	$text =~ s/&#322;/l/g;
	$text =~ s/&#347;/s/g;
	$text =~ s/&Scaron;/S/g;
	$text =~ s/&aring;/\\345/g;

	$text =~ s/&auml;/\\344/g;
	$text =~ s/&ouml;/\\366/g;
	$text =~ s/&uuml;/\\374/g;

	$text =~ s/&amp;/&/g;

	return $text;
}

# Convert HTML entities to RTF codes
# This is using the Mac OS Roman encoding
sub html_entities_to_rtf {
	my $text = shift;

	$text =~ s/&aacute;/\\'87/g;
	$text =~ s/&eacute;/\\'8e/g;
	$text =~ s/&igrave;/\\'93/g;
	$text =~ s/&oacute;/\\'97/g;
	$text =~ s/&oslash;/\\'bf/g;
	$text =~ s/&aring;/\\'8c/g;
	# The following numerical values are octal!
	$text =~ s/&#322;/\\uc0\\u322 /g;
	$text =~ s/&Scaron;/\\uc0\\u540 /g;

	# Back to hex numbers
	$text =~ s/&auml;/\\'8a/g;
	$text =~ s/&ouml;/\\'9a/g;
	$text =~ s/&uuml;/\\'9f/g;

	$text =~ s/&amp;/&/g;

	return $text;
}

# Convert HTML entities to TeX codes
sub html_entities_to_tex {
	my $text = shift;

	$text =~ s/&aacute;/\\'a/g;
	$text =~ s/&eacute;/\\'e/g;
	$text =~ s/&igrave;/\\`\\i/g;
	$text =~ s/&oacute;/\\'o/g;
	$text =~ s/&oslash;/{\\o}/g;
	$text =~ s/&aring;/\\aa /g;
	$text =~ s/&#322;/{\\l}/g;
	$text =~ s/&Scaron;/{\\v S}/g;

	$text =~ s/&auml;/\\"a/g;
	$text =~ s/&ouml;/\\"o/g;
	$text =~ s/&uuml;/\\"u/g;

	$text =~ s/&amp;/\\&/g;

	return $text;
}

#
# Small reference of the RTF commands used here:
#
#  \fs28   switches to 14 point font (28 = 2 * 14)
#  \pard   reset to default paragraph properties
#
#  \ql     left-aligned text
#  \qr     right-aligned text
#  \qc     centered text
#  \qj     justified text
#
#  \b      turn on bold
#  \b0     turn off bold
#
# For more information: <http://latex2rtf.sourceforge.net/rtfspec.html>
#

sub begin_credits {
	my $title = shift;

	if ($mode eq "TEXT") {
		#print html_entities_to_ascii($title)."\n";
	} elsif ($mode eq "RTF") {
		print '{\rtf1\mac\ansicpg10000' . "\n";
		print '{\fonttbl\f0\fswiss\fcharset77 Helvetica-Bold;\f1\fswiss\fcharset77 Helvetica;}' . "\n";
		print '{\colortbl;\red255\green255\blue255;\red0\green128\blue0;\red128\green128\blue128;}' . "\n";
		print '\vieww6920\viewh15480\viewkind0' . "\n";
		print "\n";
	} elsif ($mode eq "CPP") {
		print "// This file was generated by credits.pl. Do not edit by hand!\n";
		print "static const char *credits[] = {\n";
	} elsif ($mode eq "XML-DOC") {
		print "<?xml version='1.0'?>\n";
		print "<!-- This file was generated by credits.pl. Do not edit by hand! -->\n";
		print "<!DOCTYPE appendix PUBLIC '-//OASIS//DTD DocBook XML V4.2//EN'\n";
		print "       'http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd'>\n";
		print "<appendix id='credits'>\n";
		print "  <title>" . $title . "</title>\n";
		print "  <informaltable frame='none'>\n";
		print "  <tgroup cols='3' align='left' colsep='0' rowsep='0'>\n";
		print "  <colspec colname='start' colwidth='0.5cm'/>\n";
		print "  <colspec colname='name' colwidth='4cm'/>\n";
		print "  <colspec colname='job'/>\n";
		print "  <tbody>\n";
	} elsif ($mode eq "XML-WEB") {
		print "<?xml version='1.0'?>\n";
		print "<!-- This file was generated by credits.pl. Do not edit by hand! -->\n";
		print "<credits>\n";
	}
}

sub end_credits {
	if ($mode eq "TEXT") {
	} elsif ($mode eq "RTF") {
		print "}\n";
	} elsif ($mode eq "CPP") {
		print "};\n";
	} elsif ($mode eq "XML-DOC") {
		print "  </tbody>\n";
		print "  </tgroup>\n";
		print "  </informaltable>\n";
		print "</appendix>\n";
	} elsif ($mode eq "XML-WEB") {
		print "</credits>\n";
	}
}

sub begin_section {
	my $title = shift;

	if ($mode eq "TEXT") {
		$title = html_entities_to_ascii($title);

		if ($section_level >= 2) {
			$title .= ":"
		}

		print "  " x $section_level . $title."\n";
		if ($section_level eq 0) {
			print "  " x $section_level . "*" x (length $title)."\n";
		} elsif ($section_level eq 1) {
			print "  " x $section_level . "-" x (length $title)."\n";
		}
	} elsif ($mode eq "RTF") {
		$title = html_entities_to_rtf($title);

		# Center text
		print '\pard\qc' . "\n";
		print '\f0\b';
		if ($section_level eq 0) {
			print '\fs40 ';
		} elsif ($section_level eq 1) {
			print '\fs32 ';
		}

		# Insert an empty line before this section header, *unless*
		# this is the very first section header in the file.
		if ($section_level > 0 || @section_count[0] > 0) {
			print "\\\n";
		}
		print '\cf2 ' . $title . "\n";
		print '\f1\b0\fs24 \cf0 \\' . "\n";
	} elsif ($mode eq "CPP") {
		if ($section_level eq 0) {
		  # TODO: Would be nice to have a 'fat' or 'large' mode for
		  # headlines...
		  $title = html_entities_to_cpp($title);
		  print '"C1""'.$title.'",' . "\n";
		  print '"",' . "\n";
		} else {
		  $title = html_entities_to_cpp($title);
		  print '"C1""'.$title.'",' . "\n";
		}
	} elsif ($mode eq "XML-DOC") {
		print "  <row><entry namest='start' nameend='job'>";
		print "<emphasis role='bold'>" . $title . ":</emphasis>";
		print "</entry></row>\n";
	} elsif ($mode eq "XML-WEB") {
		if ($section_level eq 0) {
			print "\t<section>\n";
			print "\t\t<title>" . $title . "</title>\n";
		} elsif ($section_level eq 1) {
			print "\t\t<subsection>\n";
			print "\t\t\t<title>" . $title . "</title>\n";
		} else {
			#print "\t\t\t<group>" . $title . "</group>\n";
			#print "\t\t\t\t<name>" . $title . "</name>\n";
		}
	}

	# Implicit start of person list on section level 2
	if ($section_level >= 2) {
		begin_persons($title);
	}
	@section_count[$section_level]++;
	$section_level++;
	@section_count[$section_level] = 0;
}

sub end_section {
	$section_level--;

	# Implicit end of person list on section level 2
	if ($section_level >= 2) {
		end_persons();
	}

	if ($mode eq "TEXT") {
		# nothing
	} elsif ($mode eq "RTF") {
		# nothing
	} elsif ($mode eq "CPP") {
		print '"",' . "\n";
	} elsif ($mode eq "XML-DOC") {
		print "  <row><entry namest='start' nameend='job'> </entry></row>\n\n";
	} elsif ($mode eq "XML-WEB") {
		if ($section_level eq 0) {
			print "\t</section>\n";
		} elsif ($section_level eq 1) {
			print "\t\t</subsection>\n";
		} else {
			#print "\t\t\t</group>\n";
		}
	}
}

sub begin_persons {
	my $title = shift;
	if ($mode eq "XML-WEB") {
		print "\t\t\t<group>\n";
		print "\t\t\t\t<name>" . $title . "</name>\n";
		#print "\t\t\t\t<persons>\n";
	}
}

sub end_persons {
	if ($mode eq "TEXT") {
		print "\n";
	} elsif ($mode eq "RTF") {
		# nothing
	} elsif ($mode eq "XML-WEB") {
		#print "\t\t\t\t</persons>\n";
		print "\t\t\t</group>\n";
	}
}

sub add_person {
	my $name = shift;
	my $nick = shift;
	my $desc = shift;
	my $tab;

	if ($mode eq "TEXT") {
		my $min_name_width = length $desc > 0 ? $max_name_width : 0;
		$name = $nick if $name eq "";
		$name = html_entities_to_ascii($name);
		if (length $name > $max_name_width) {
			print STDERR "Warning: max_name_width is too small (" . $max_name_width . " < " . (length $name) . " for \"" . $name. "\")\n";
		}
		$desc = html_entities_to_ascii($desc);

		$tab = " " x ($section_level * 2 + 1);
		printf $tab."%-".$min_name_width.".".$max_name_width."s", $name;

		# Print desc wrapped
		if (length $desc > 0) {
		  my $inner_indent = ($section_level * 2 + 1) + $max_name_width + 3;
		  my $multitab = " " x $inner_indent;
		  print " - " . substr(wrap($multitab, $multitab, $desc), $inner_indent);
		}
		print "\n";
	} elsif ($mode eq "RTF") {
		$name = $nick if $name eq "";
		$name = html_entities_to_rtf($name);

		# Center text
		print '\pard\qc' . "\n";
		# Activate 1.5 line spacing mode
		print '\sl360\slmult1';
		# The name
		print $name . "\\\n";
		# Description using italics
		if (length $desc > 0) {
			$desc = html_entities_to_rtf($desc);
			print '\pard\qc' . "\n";
			print "\\cf3\\i " . $desc . "\\i0\\cf0\\\n";
		}
	} elsif ($mode eq "CPP") {
		$name = $nick if $name eq "";
		$name = html_entities_to_cpp($name);

		print '"C0""'.$name.'",' . "\n";

		# Print desc wrapped
		if (length $desc > 0) {
			$desc = html_entities_to_cpp($desc);
			print '"C2""'.$desc.'",' . "\n";
		}
	} elsif ($mode eq "XML-DOC") {
		$name = $nick if $name eq "";
		print "  <row><entry namest='name'>" . $name . "</entry>";
		print "<entry>" . $desc . "</entry></row>\n";
	} elsif ($mode eq "XML-WEB") {
		$name = "???" if $name eq "";
		print "\t\t\t\t<person>\n";
		print "\t\t\t\t\t<name>" . $name . "</name>\n";
		print "\t\t\t\t\t<alias>" . $nick . "</alias>\n";
		print "\t\t\t\t\t<description>" . $desc . "</description>\n";
		print "\t\t\t\t</person>\n";
	}
}

sub add_paragraph {
	my $text = shift;
	my $tab;

	if ($mode eq "TEXT") {
		$tab = " " x ($section_level * 2 + 1);
		print wrap($tab, $tab, html_entities_to_ascii($text))."\n";
		print "\n";
	} elsif ($mode eq "RTF") {
		$text = html_entities_to_rtf($text);
		# Center text
		print '\pard\qc' . "\n";
		print "\\\n";
		print $text . "\\\n";
	} elsif ($mode eq "CPP") {
		$text = html_entities_to_ascii($text);
		my $line_start = '"C0""';
		my $line_end = '",';
		print $line_start . $text . $line_end . "\n";
		print $line_start . $line_end . "\n";
	} elsif ($mode eq "XML-DOC") {
		print "  <row><entry namest='start' nameend='job'>" . $text . "</entry></row>\n";
		print "  <row><entry namest='start' nameend='job'> </entry></row>\n\n";
	} elsif ($mode eq "XML-WEB") {
		print "\t\t<paragraph>" . $text . "</paragraph>\n";
	}
}

#
# Now follows the actual credits data! The format should be clear, I hope.
# Note that people are sorted by their last name in most cases; in the
# 'Team' section, they are first grouped by category (Engine; porter; misc).
#

begin_credits("Credits");
	begin_section("ScummVM Team");
		begin_section("Project Leader");
			begin_persons();
				add_person("Eugene Sandulenko", "sev", "");
			end_persons();
		end_section();

		begin_section("PR Office");
			begin_persons();
				add_person("Arnaud Boutonn&eacute;", "Strangerke", "Public Relations Officer, Project Administrator");
				add_person("Eugene Sandulenko", "sev", "Project Leader");
			end_persons();
		end_section();

		begin_section("Core Team");
			begin_persons();
				add_person("Willem Jan Palenstijn", "wjp", "");
				add_person("Eugene Sandulenko", "sev", "");
				add_person("Johannes Schickel", "LordHoto", "");
			end_persons();
		end_section();

		begin_section("Retired Project Leaders");
			begin_persons();
				add_person("James Brown", "ender", "");
				add_person("Vincent Hamm", "yaz0r", "ScummVM co-founder, Original Cruise/CinE author");
				add_person("Max Horn", "Fingolfin", "");
				add_person("Ludvig Strigeus", "ludde", "Original ScummVM and SimonVM author");
			end_persons();
		end_section();

		begin_section("Engine Teams");
			begin_section("SCUMM");
				add_person("Torbj&ouml;rn Andersson", "eriktorbjorn", "");
				add_person("James Brown", "ender", "(retired)");
				add_person("Jonathan Gray", "khalek", "(retired)");
				add_person("Vincent Hamm", "yaz0r", "(retired)");
				add_person("Max Horn", "Fingolfin", "(retired)");
				add_person("Travis Howell", "Kirben", "");
				add_person("Pawe&#322; Ko&#322;odziejski", "aquadran", "Codecs, iMUSE, Smush, etc.");
				add_person("Gregory Montoir", "cyx", "(retired)");
				add_person("Eugene Sandulenko", "sev", "FT INSANE, MM NES, MM C64, game detection, Herc/CGA");
				add_person("Ludvig Strigeus", "ludde", "(retired)");
			end_section();

			begin_section("HE");
				add_person("Jonathan Gray", "khalek", "(retired)");
				add_person("Travis Howell", "Kirben", "");
				add_person("Gregory Montoir", "cyx", "(retired)");
				add_person("Eugene Sandulenko", "sev", "");
			end_section();

			begin_section("AGI");
				add_person("Stuart George", "darkfiber", "");
				add_person("Matthew Hoops", "clone2727", "");
				add_person("Filippos Karapetis", "[md5]", "");
				add_person("Pawe&#322; Ko&#322;odziejski", "aquadran", "");
				add_person("Walter van Niftrik", "waltervn", "(retired)");
				add_person("Kari Salminen", "Buddha^", "");
				add_person("Eugene Sandulenko", "sev", "");
				add_person("David Symonds", "dsymonds", "(retired)");
			end_section();

			begin_section("AGOS");
				add_person("Torbj&ouml;rn Andersson", "eriktorbjorn", "");
				add_person("Paul Gilbert", "dreammaster", "");
				add_person("Travis Howell", "Kirben", "");
				add_person("Oliver Kiehl", "olki", "(retired)");
				add_person("Ludvig Strigeus", "ludde", "(retired)");
			end_section();

			begin_section("CGE");
				add_person("Arnaud Boutonn&eacute;", "Strangerke", "");
				add_person("Paul Gilbert", "dreammaster", "");
			end_section();

			begin_section("Cine");
				add_person("Vincent Hamm", "yaz0r", "(retired)");
				add_person("Pawe&#322; Ko&#322;odziejski", "aquadran", "");
				add_person("Gregory Montoir", "cyx", "(retired)");
				add_person("Kari Salminen", "Buddha^", "");
				add_person("Eugene Sandulenko", "sev", "");
			end_section();

			begin_section("Composer");
				add_person("Alyssa Milburn", "fuzzie", "");
			end_section();

			begin_section("CruisE");
				add_person("Paul Gilbert", "dreammaster", "");
				add_person("Vincent Hamm", "yaz0r", "(retired)");
			end_section();

			begin_section("Draci");
				add_person("Denis Kasak", "dkasak13", "");
				add_person("Robert &Scaron;palek", "spalek", "");
			end_section();

			begin_section("Drascula");
				add_person("Filippos Karapetis", "[md5]", "");
				add_person("Pawe&#322; Ko&#322;odziejski", "aquadran", "");
			end_section();

			begin_section("DreamWeb");
				add_person("Torbj&ouml;rn Andersson", "eriktorbjorn", "");
				add_person("Bertrand Augereau", "Tramb", "");
				add_person("Filippos Karapetis", "[md5]", "");
				add_person("Vladimir Menshakov", "whoozle", "(retired)");
				add_person("Willem Jan Palenstijn", "wjp", "");
			end_section();

			begin_section("Gob");
				add_person("Torbj&ouml;rn Andersson", "eriktorbjorn", "");
				add_person("Arnaud Boutonn&eacute;", "Strangerke", "");
				add_person("Sven Hesse", "DrMcCoy", "");
				add_person("Eugene Sandulenko", "sev", "");
			end_section();

			begin_section("Groovie");
				add_person("Henry Bush", "spookypeanut", "");
				add_person("Scott Thomas", "ST", "");
				add_person("Jordi Vilalta Prat", "jvprat", "");
			end_section();

			begin_section("Hugo");
				add_person("Arnaud Boutonn&eacute;", "Strangerke", "");
				add_person("Oystein Eftevaag", "vinterstum", "");
				add_person("Eugene Sandulenko", "sev", "");
			end_section();

			begin_section("Kyra");
				add_person("Torbj&ouml;rn Andersson", "eriktorbjorn", "VQA Player");
				add_person("Oystein Eftevaag", "vinterstum", "");
				add_person("Florian Kagerer", "athrxx", "");
				add_person("Gregory Montoir", "cyx", "(retired)");
				add_person("Johannes Schickel", "LordHoto", "");
			end_section();

			begin_section("Lastexpress");
				add_person("Matthew Hoops", "clone2727", "");
				add_person("Jordi Vilalta Prat", "jvprat", "");
				add_person("Julien Templier", "littleboy", "");
			end_section();

			begin_section("Lure");
				add_person("Paul Gilbert", "dreammaster", "");
			end_section();

			begin_section("MADE");
				add_person("Benjamin Haisch", "john_doe", "");
				add_person("Filippos Karapetis", "[md5]", "");
			end_section();

			begin_section("Mohawk");
				add_person("Bastien Bouclet", "bgk", "");
				add_person("Matthew Hoops", "clone2727", "");
				add_person("Filippos Karapetis", "[md5]", "");
				add_person("Alyssa Milburn", "fuzzie", "");
				add_person("Eugene Sandulenko", "sev", "");
				add_person("David Turner", "digitall", "");
			end_section();

			begin_section("Parallaction");
				add_person("", "peres", "");
			end_section();

			begin_section("Pegasus");
				add_person("Matthew Hoops", "clone2727", "");
			end_section();

			begin_section("Queen");
				add_person("David Eriksson", "twogood", "(retired)");
				add_person("Gregory Montoir", "cyx", "(retired)");
				add_person("Joost Peters", "joostp", "");
			end_section();

			begin_section("SAGA");
				add_person("Torbj&ouml;rn Andersson", "eriktorbjorn", "");
				add_person("Daniel Balsom", "DanielFox", "Original engine reimplementation author (retired)");
				add_person("Filippos Karapetis", "[md5]", "");
				add_person("Andrew Kurushin", "ajax16384", "");
				add_person("Eugene Sandulenko", "sev", "");
			end_section();

			begin_section("SCI");
				add_person("Greg Frieger", "_FRG_", "");
				add_person("Paul Gilbert", "dreammaster", "");
				add_person("Max Horn", "Fingolfin", "(retired)");
				add_person("Filippos Karapetis", "[md5]", "");
				add_person("Martin Kiewitz", "m_kiewitz", "");
				add_person("Walter van Niftrik", "waltervn", "(retired)");
				add_person("Willem Jan Palenstijn", "wjp", "");
				add_person("Jordi Vilalta Prat", "jvprat", "");
				add_person("Lars Skovlund", "lskovlun", "");
			end_section();

			begin_section("Sky");
				add_person("Robert G&ouml;ffringmann", "lavosspawn", "(retired)");
				add_person("Oliver Kiehl", "olki", "(retired)");
				add_person("Joost Peters", "joostp", "");
			end_section();

			begin_section("Sword1");
				add_person("Fabio Battaglia", "Hkz", "PSX version support");
				add_person("Thierry Crozat", "criezy", "Mac version support");
				add_person("Robert G&ouml;ffringmann", "lavosspawn", "(retired)");
			end_section();

			begin_section("Sword2");
				add_person("Torbj&ouml;rn Andersson", "eriktorbjorn", "");
				add_person("Fabio Battaglia", "Hkz", "PSX version support");
				add_person("Jonathan Gray", "khalek", "(retired)");
			end_section();

			begin_section("Sword25");
				add_person("Torbj&ouml;rn Andersson", "eriktorbjorn", "");
				add_person("Paul Gilbert", "dreammaster", "");
				add_person("Max Horn", "Fingolfin", "(retired)");
				add_person("Filippos Karapetis", "[md5]", "");
				add_person("Eugene Sandulenko", "sev", "");
			end_section();

			begin_section("TeenAgent");
				add_person("Robert Megone", "sanguine", "Help with callback rewriting");
				add_person("Vladimir Menshakov", "whoozle", "(retired)");
			end_section();

			begin_section("Tinsel");
				add_person("Torbj&ouml;rn Andersson", "eriktorbjorn", "");
				add_person("Fabio Battaglia", "Hkz", "PSX version support");
				add_person("Paul Gilbert", "dreammaster", "");
				add_person("Sven Hesse", "DrMcCoy", "");
				add_person("Max Horn", "Fingolfin", "(retired)");
				add_person("Filippos Karapetis", "[md5]", "");
				add_person("Joost Peters", "joostp", "");
			end_section();

			begin_section("Tony");
				add_person("Arnaud Boutonn&eacute;", "Strangerke", "");
				add_person("Paul Gilbert", "dreammaster", "");
				add_person("Alyssa Milburn", "fuzzie", "");
			end_section();

			begin_section("Toon");
				add_person("Sylvain Dupont", "SylvainTV", "");
			end_section();

			begin_section("Touch&eacute;");
				add_person("Gregory Montoir", "cyx", "(retired)");
			end_section();

			begin_section("TsAGE");
				add_person("Arnaud Boutonn&eacute;", "Strangerke", "");
				add_person("Paul Gilbert", "dreammaster", "");
			end_section();

			begin_section("Tucker");
				add_person("Gregory Montoir", "cyx", "(retired)");
			end_section();

			begin_section("Wintermute");
				add_person("Einar Johan T. S&oslash;m&aring;en", "somaen", "");
			end_section();

		end_section();


		begin_section("Backend Teams");
			begin_section("Android");
				add_person("Andre Heider", "dhewg", "");
				add_person("Angus Lees", "Gus", "");
			end_section();

			begin_section("BADA");
				add_person("Chris Warren-Smith", "", "");
			end_section();

			begin_section("Dreamcast");
				add_person("Marcus Comstedt", "", "");
			end_section();

			begin_section("GPH Devices (GP2X, GP2XWiz &amp; Caanoo)");
				add_person("John Willis", "DJWillis", "");
			end_section();

			begin_section("iPhone");
				add_person("Oystein Eftevaag", "vinterstum", "");
			end_section();

			begin_section("LinuxMoto");
				add_person("Lubomyr Lisen", "", "");
			end_section();

			begin_section("Maemo");
				add_person("Frantisek Dufka", "fanoush", "(retired)");
				add_person("Tarek Soliman", "tsoliman", "");
			end_section();

			begin_section("Nintendo 64");
				add_person("Fabio Battaglia", "Hkz", "");
			end_section();

			begin_section("Nintendo DS");
				add_person("Bertrand Augereau", "Tramb", "HQ software scaler");
				add_person("Neil Millstone", "agent-q", "");
			end_section();

			begin_section("OpenPandora");
				add_person("John Willis", "DJWillis", "");
			end_section();

			begin_section("PocketPC / WinCE");
				add_person("Nicolas Bacca", "arisme", "(retired)");
				add_person("Ismail Khatib", "CeRiAl", "");
				add_person("Kostas Nakos", "Jubanka", "(retired)");
			end_section();

			begin_section("PlayStation 2");
				add_person("Robert G&ouml;ffringmann", "lavosspawn", "(retired)");
				add_person("Max Lingua", "sunmax", "");
			end_section();

			begin_section("PSP (PlayStation Portable)");
				add_person("Yotam Barnoy", "bluddy", "");
				add_person("Joost Peters", "joostp", "");
			end_section();

			begin_section("SDL (Win/Linux/OS X/etc.)");
				add_person("Max Horn", "Fingolfin", "(retired)");
				add_person("Eugene Sandulenko", "sev", "Asm routines, GFX layers");
			end_section();

			begin_section("SymbianOS");
				add_person("Jurgen Braam", "SumthinWicked", "");
				add_person("Lars Persson", "AnotherGuest", "");
			end_section();

			begin_section("WebOS");
				add_person("Klaus Reimer", "kayahr", "");
			end_section();

			begin_section("Wii");
				add_person("Andre Heider", "dhewg", "");
			end_section();

		end_section();

		begin_section("Other subsystems");
			begin_section("Infrastructure");
				add_person("Max Horn", "Fingolfin", "Backend &amp; Engine APIs, file API, sound mixer, audiostreams, data structures, etc. (retired)");
				add_person("Eugene Sandulenko", "sev", "");
				add_person("Johannes Schickel", "LordHoto", "");
			end_section();

			begin_section("GUI");
				add_person("Vicent Marti", "tanoku", "");
				add_person("Eugene Sandulenko", "sev", "");
				add_person("Johannes Schickel", "LordHoto", "");
			end_section();

			begin_section("Miscellaneous");
				add_person("David Corrales-Lopez", "david_corrales", "Filesystem access improvements (GSoC 2007 task) (retired)");
				add_person("Jerome Fisher", "KingGuppy", "MT-32 emulator");
				add_person("Benjamin Haisch", "john_doe", "Heavily improved de-/encoder for DXA videos");
				add_person("Jochen Hoenicke", "hoenicke", "Speaker &amp; PCjr sound support, AdLib work (retired)");
				add_person("Chris Page", "cp88", "Return to launcher, savestate improvements, leak fixes, ... (GSoC 2008 task) (retired)");
				add_person("Robin Watts", "robinwatts", "ARM assembly routines for nice speedups on several ports; improvements to the sound mixer");
			end_section();
		end_section();

		begin_section("Website (code)");
			begin_persons();
				add_person("Fredrik Wendel", "", "(retired)");
			end_persons();
		end_section();

		begin_section("Website (maintenance)");
			begin_persons();
				add_person("James Brown", "Ender", "IRC Logs maintainer");
				add_person("Thierry Crozat", "criezy", "Wiki maintainer");
				add_person("Andre Heider", "dhewg", "Buildbot maintainer");
				add_person("Joost Peters", "JoostP", "Doxygen Project Documentation maintainer");
				add_person("Jordi Vilalta Prat", "jvprat", "Wiki maintainer");
				add_person("Eugene Sandulenko", "sev", "Forum, IRC channel, Screen Shots and Mailing list maintainer");
				add_person("John Willis", "DJWillis", "");
			end_persons();
		end_section();

		begin_section("Website (content)");
			add_paragraph("All active team members");
		end_section();

		begin_section("Documentation");
			begin_persons();
				add_person("Thierry Crozat", "criezy", "Numerous contributions to documentation");
				add_person("Joachim Eberhard", "joachimeberhard", "Numerous contributions to documentation (retired)");
				add_person("Matthew Hoops", "clone2727", "Wiki editor");
			end_persons();
		end_section();

		begin_section("Retired Team Members");
			begin_persons();
				add_person("Chris Apers", "chrilith ", "Former PalmOS porter");
				add_person("Ralph Brorsen", "painelf", "Help with GUI implementation");
				add_person("Jamieson Christian", "jamieson630", "iMUSE, MIDI, all things musical");
				add_person("Felix Jakschitsch", "yot", "Zak256 reverse engineering");
				add_person("Mutwin Kraus", "mutle", "Original MacOS porter");
				add_person("Peter Moraliyski", "ph0x", "Port: GP32");
				add_person("Jeremy Newman", "laxdragon", "Former webmaster");
				add_person("Lionel Ulmer", "bbrox", "Port: X11");
				add_person("Won Star", "wonst719", "Former GP32 porter");
			end_persons();
		end_section();
	end_section();


	begin_section("Other contributions");

		begin_section("Packages");
			begin_section("AmigaOS 4");
				add_person("Hans-J&ouml;rg Frieden", "", "(retired)");
				add_person("Hubert Maier", "Raziel_AOne", "");
				add_person("Juha Niemim&auml;ki", "", "(retired)");
			end_section();

			begin_section("Atari/FreeMiNT");
				add_person("Keith Scroggins", "KeithS", "");
			end_section();

			begin_section("BeOS");
				add_person("Stefan Parviainen", "", "");
				add_person("Luc Schrijvers", "Begasus", "");
			end_section();

			begin_section("Debian GNU/Linux");
				add_person("Tore Anderson", "tore", "(retired)");
				add_person("David Weinehall", "tao", "");
			end_section();

			begin_section("Fedora / RedHat");
				add_person("Willem Jan Palenstijn", "wjp", "");
			end_section();

			begin_section("Mac OS X");
				add_person("Max Horn", "Fingolfin", "(retired)");
				add_person("Oystein Eftevaag", "vinterstum", "");
			end_section();

			begin_section("Mandriva");
				add_person("Dominik Scherer", "", "(retired)");
			end_section();

			begin_section("MorphOS");
				add_person("Fabien Coeurjoly", "fab1", "");
				add_person("R&uuml;diger Hanke", "", "(retired)");
			end_section();

			begin_section("OS/2");
				add_person("Paul Smedley", "Creeping", "");
			end_section();

			begin_section("SlackWare");
				add_person("Robert Kelsen", "", "");
			end_section();

			begin_section("Solaris x86");
				add_person("Laurent Blume", "laurent", "");
			end_section();

			begin_section("Solaris SPARC");
				add_person("Markus Strangl", "WooShell", "");
			end_section();

			begin_section("Win32");
				add_person("Travis Howell", "Kirben", "");
			end_section();

			begin_section("Win64");
				add_person("Chris Gray", "Psychoid", "(retired)");
				add_person("Johannes Schickel", "LordHoto", "");
			end_section();
		end_section();

		begin_section("Translations");
				begin_persons();
					add_person("Thierry Crozat", "criezy", "Translation Lead");
				end_persons();
				begin_section("Basque");
					add_person("Mikel Iturbe Urretxa", "", "");
				end_section();
				begin_section("Catalan");
					add_person("Jordi Vilalta Prat", "jvprat", "");
				end_section();
				begin_section("Czech");
					add_person("Zbyn&igrave;k Schwarz", "", "");
				end_section();
				begin_section("Danish");
					add_person("Steffen Nyeland", "", "");
				end_section();
				begin_section("French");
					add_person("Thierry Crozat", "criezy", "");
				end_section();
				begin_section("Galician");
					add_person("Santiago G. Sanz", "sgsanz", "");
				end_section();
				begin_section("German");
					add_person("Simon Sawatzki", "SimSaw", "");
					add_person("Lothar Serra Mari", "Lothar93", "(retired)");
				end_section();
				begin_section("Hungarian");
					add_person("Alex Bevilacqua", "", "");
					add_person("George Kormendi", "GoodOldGeorg", "");
				end_section();
				begin_section("Italian");
					add_person("Matteo Angelino", "Maff", "");
				end_section();
				begin_section("Norwegian (Bokm&aring;l)");
					add_person("Einar Johan S&oslash;m&aring;en", "somaen", "");
				end_section();
				begin_section("Norwegian (Nynorsk)");
					add_person("Einar Johan S&oslash;m&aring;en", "somaen", "");
				end_section();
				begin_section("Polish");
					add_person("GrajPoPolsku.pl Team", "", "");
				end_section();
				begin_section("Brazilian Portuguese");
					add_person("ScummBR Team", "", "");
				end_section();
				begin_section("Russian");
					add_person("Eugene Sandulenko", "sev", "");
				end_section();
				begin_section("Spanish");
					add_person("Tom&aacute;s Maidagan", "", "");
					add_person("Jordi Vilalta Prat", "jvprat", "");
				end_section();
				begin_section("Swedish");
					add_person("Hampus Flink", "", "");
				end_section();
				begin_section("Ukrainian");
					add_person("Lubomyr Lisen", "", "");
				end_section();
		end_section();

		begin_section("Websites (design)");
			begin_persons();
				add_person("Dob&oacute; Bal&aacute;zs", "draven", "Website design");
				add_person("William Claydon", "billwashere", "Skins for doxygen, buildbot and wiki");
				add_person("Yaroslav Fedevych", "jafd", "HTML/CSS for the website");
				add_person("Jean Marc Gimenez", "", "ScummVM logo");
				add_person("David Jensen", "Tyst", "SVG logo conversion");
				add_person("", "Raina", "ScummVM forum buttons");
			end_persons();
		end_section();

		begin_section("Code contributions");
			begin_persons();
				add_person("Ori Avtalion", "salty-horse", "Subtitle control options in the GUI; BASS GUI fixes");
				add_person("Stuart Caie", "", "Decoders for Amiga and AtariST data files (AGOS engine)");
				add_person("Paolo Costabel", "", "PSP port contributions");
				add_person("Martin Doucha", "next_ghost", "CinE engine objectification");
				add_person("Thomas Fach-Pedersen", "madmoose", "ProTracker module player, Smacker video decoder");
				add_person("Tobias Gunkel", "hennymcc", "Sound support for C64 version of MM/Zak, Loom PCE support");
				add_person("Janne Huttunen", "", "V3 actor mask support, Dig/FT SMUSH audio");
				add_person("Kov&aacute;cs Endre J&aacute;nos", "", "Several fixes for Simon1");
				add_person("Jeroen Janssen", "japj", "Numerous readability and bugfix patches");
				add_person("Andreas Karlsson", "Sprawl", "Initial port for SymbianOS");
				add_person("Claudio Matsuoka", "", "Daily Linux builds");
				add_person("Thomas Mayer", "", "PSP port contributions");
				add_person("Sean Murray", "lightcast", "ScummVM tools GUI application (GSoC 2007 task)");
				add_person("", "n0p", "Windows CE port aspect ratio correction scaler and right click input method");
				add_person("Mikesch Nepomuk", "mnepomuk", "MI1 VGA floppy patches");
				add_person("Nicolas Noble", "pixels", "Config file and ALSA support");
				add_person("Tim Phillips", "realmz", "Initial MI1 CD music support");
				add_person("", "Quietust", "Sound support for Amiga SCUMM V2/V3 games, MM NES support");
				add_person("Robert Crossfield", "segra", "Improved support for Apple II/C64 versions of MM");
				add_person("Andreas R&ouml;ver", "", "Broken Sword I &amp; II MPEG2 cutscene support");
				add_person("Edward Rudd", "urkle", "Fixes for playing MP3 versions of MI1/Loom audio");
				add_person("Daniel Schepler", "dschepler", "Final MI1 CD music support, initial Ogg Vorbis support");
				add_person("Andr&eacute; Souza", "luke_br", "SDL-based OpenGL renderer");
				add_person("Tom Frost", "TomFrost", "WebOS port contributions");
			end_persons();
		end_section();

		begin_section("FreeSCI Contributors");
			begin_persons();
				add_person("Francois-R Boyer", "", "MT-32 information and mapping code");
				add_person("Rainer Canavan", "", "IRIX MIDI driver and bug fixes");
				add_person("Xiaojun Chen", "", "");
				add_person("Paul David Doherty", "", "Game version information");
				add_person("Vyacheslav Dikonov", "", "Config script improvements");
				add_person("Ruediger Hanke", "", "Port to the MorphOS platform");
				add_person("Matt Hargett", "", "Clean-ups, bugfixes, Hardcore QA, Win32");
				add_person("Max Horn", "", "SetJump implementation");
				add_person("Ravi I.", "", "SCI0 sound resource specification");
				add_person("Emmanuel Jeandel", "", "Bugfixes and bug reports");
				add_person("Dmitry Jemerov", "", "Port to the Win32 platform, numerous bugfixes");
				add_person("Chris Kehler", "", "Makefile enhancements");
				add_person("Christopher T. Lansdown", "", "Original CVS maintainer, Alpha compatibility fixes");
				add_person("Sergey Lapin", "", "Port of Carl's type 2 decompression code");
				add_person("Rickard Lind", "", "MT-32->GM MIDI mapping magic, sound research");
				add_person("Hubert Maier", "", "AmigaOS 4 port");
				add_person("Johannes Manhave", "", "Document format translation");
				add_person("Claudio Matsuoka", "", "CVS snapshots, daily builds, BeOS and cygwin ports");
				add_person("Dark Minister", "", "SCI research (bytecode and parser)");
				add_person("Carl Muckenhoupt", "", "Sources to the SCI resource viewer tools that started it all");
				add_person("Anders Baden Nielsen", "", "PPC testing");
				add_person("Walter van Niftrik", "", "Ports to the Dreamcast and GP32 platforms");
				add_person("Rune Orsval", "", "Configuration file editor");
				add_person("Solomon Peachy", "", "SDL ports and much of the sound subsystem");
				add_person("Robey Pointer", "", "Bug tracking system hosting");
				add_person("Magnus Reftel", "", "Heap implementation, Python class viewer, bugfixes");
				add_person("Christoph Reichenbach", "", "UN*X code, VM/Graphics/Sound/other infrastructure");
				add_person("George Reid", "", "FreeBSD package management");
				add_person("Lars Skovlund", "", "Project maintenance, most documentation, bugfixes, SCI1 support");
				add_person("Rink Springer", "", "Port to the DOS platform, several bug fixes");
				add_person("Rainer De Temple", "", "SCI research");
				add_person("Sean Terrell", "", "");
				add_person("Hugues Valois", "", "Game selection menu");
				add_person("Jordi Vilalta", "", "Numerous code and website clean-up patches");
				add_person("Petr Vyhnak", "", "The DCL-INFLATE algorithm, many Win32 improvements");
				add_person("Bas Zoetekouw", "", "Man pages, debian package management, CVS maintenance");
			end_persons();
			add_paragraph("Special thanks to Prof. Dr. Gary Nutt ".
                    "for allowing the FreeSCI VM extension as a ".
                    "course project in his Advanced OS course.");
			add_paragraph("Special thanks to Bob Heitman and Corey Cole for their support of FreeSCI.");
		end_section();

		add_paragraph("And to all the contributors, users, and beta testers we've missed. Thanks!");

	end_section();


	# HACK!
	$max_name_width = 17;

	begin_section("Special thanks to");
		begin_persons();
			add_person("Daniel Balsom", "DanielFox", "For the original Reinherit (SAGA) code");
			add_person("Sander Buskens", "", "For his work on the initial reversing of Monkey2");
			add_person("", "Canadacow", "For the original MT-32 emulator");
			add_person("Kevin Carnes", "", "For Scumm16, the basis of ScummVM's older gfx codecs");
			add_person("Curt Coder", "", "For the original TrollVM (preAGI) code");
			add_person("Patrick Combet", "Dorian Gray", "For the original Gobliiins ADL player");
			add_person("Ivan Dubrov", "", "For contributing the initial version of the Gobliiins engine");
			add_person("Henrik Engqvist", "qvist", "For generously providing hosting for our buildbot, SVN repository, planet and doxygen sites as well as tons of HD space");
			add_person("DOSBox Team", "", "For their awesome OPL2 and OPL3 emulator");
			add_person("Yusuke Kamiyamane", "", "For contributing some GUI icons ");
			add_person("Till Kresslein", "Krest", "For design of modern ScummVM GUI");
			add_person("", "Jezar", "For his freeverb filter implementation");
			add_person("Jim Leiterman", "", "Various info on his FM-TOWNS/Marty SCUMM ports");
			add_person("", "lloyd", "For deep tech details about C64 Zak &amp; MM");
			add_person("Sarien Team", "", "Original AGI engine code");
			add_person("Jimmi Th&oslash;gersen", "", "For ScummRev, and much obscure code/documentation");
			add_person("", "Tristan", "For additional work on the original MT-32 emulator");
			add_person("James Woodcock", "", "Soundtrack enhancements");
		end_persons();

	add_paragraph(
    "Tony Warriner and everyone at Revolution Software Ltd. for sharing ".
    "with us the source of some of their brilliant games, allowing us to ".
    "release Beneath a Steel Sky as freeware... and generally being ".
    "supportive above and beyond the call of duty.");

	add_paragraph(
    "John Passfield and Steve Stamatiadis for sharing the source of their ".
    "classic title, Flight of the Amazon Queen and also being incredibly ".
    "supportive.");

	add_paragraph(
    "Joe Pearce from The Wyrmkeep Entertainment Co. for sharing the source ".
    "of their famous title Inherit the Earth and always prompt replies to ".
    "our questions.");

	add_paragraph(
    "Aric Wilmunder, Ron Gilbert, David Fox, Vince Lee, and all those at ".
    "LucasFilm/LucasArts who made SCUMM the insane mess to reimplement ".
    "that it is today. Feel free to drop us a line and tell us what you ".
    "think, guys!");

	add_paragraph(
    "Alan Bridgman, Simon Woodroffe and everyone at Adventure Soft for ".
    "sharing the source code of some of their games with us.");

	add_paragraph(
    "John Young, Colin Smythe and especially Terry Pratchett himself for ".
    "sharing the source code of Discworld I &amp; II with us.");

	add_paragraph(
    "Emilio de Paz Arag&oacute;n from Alcachofa Soft for sharing the source code ".
    "of Drascula: The Vampire Strikes Back with us and his generosity with ".
    "freewaring the game.");

	add_paragraph(
    "David P. Gray from Gray Design Associates for sharing the source code ".
    "of the Hugo trilogy.");

	add_paragraph(
    "Broken Sword 2.5 team for providing sources of their engine and their great ".
    "support.");

	add_paragraph(
    "Neil Dodwell and David Dew from Creative Reality for providing the source ".
    "of Dreamweb and for their tremendous support.");

	add_paragraph(
    "Janusz Wi&#347;niewski and Miroslaw Liminowicz from Laboratorium Komputerowe Avalon ".
    "for providing full source code for So&#322;tys and letting us redistribute the game.");

	add_paragraph(
    "Jan Nedoma for providing the sources to the Wintermute-engine, and for his ".
    "support while porting the engine to ScummVM.");

	add_paragraph(
    "Bob Bell, Michel Kripalani, Tommy Yune, from Presto Studios for ".
    "providing the source code of The Journeyman Project: Pegasus Prime.");

	end_section();

end_credits();
