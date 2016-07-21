#
# Description:
#  Module de fonctions generiques
#

package ogutil;

use strict;
use XML::DOM;
use XML::XQL;
use XML::XQL::DOM;
use Path::Class;
use Digest::MD5;

# System utility functions
sub OgCheckPlatform ();
sub OgParsePlatform ($);
sub OgOsString ($;$);
sub OgOsVersion ($);
sub OgOsTagString ($);
sub OgArchString ($;$);
sub OgPathToList ($);
sub OgCygpath ($$);
sub OgPathIsAbsolute($);

# XML utility functions
sub OgXmlGetNodeText ($);
sub OgCheckDTD ($$);

# SVN utility functions
sub OgGetSvnRevision ($);
sub OgIsPathInGitRepo ($) ;

# File utility functions
sub OgCksum ($);
sub OgMD5Sign ($);

# String utility functions
sub OgStringProtect ($);
sub OgStringSqueeze ($);
sub OgStringTrim ($);
sub OgStringParagTrim ($);
sub OgStringParagIndent ($$);
sub OgStringParagComment ($);
sub OgStringParagUncomment ($);
sub OgStringListParagTrim (@);
sub OgStringListParagIndent ($@);
sub OgStringListParagComment (@);
sub OgStringListParagUncomment (@);

# File utility fuctions
sub OgFileSlurp ($);

# Miscellaneous utility functions
sub OgDebug (@);
sub OgVerbose (@);

# System utility function
sub OgCheckProcess($);
sub OgKillProcess($);
sub OgDetachProcess($);

# Pertimm specific utility functions
sub OgPlatformPath ($);


use Exporter;
use vars qw( @ISA @EXPORT );
@ISA    = qw( Exporter );
@EXPORT = qw(
  OgCheckPlatform OgParsePlatform
  OgOsString OgOsVersion OgOsTagString OgArchString
  OgPathToList OgCygpath OgPathIsAbsolute
  OgXmlGetNodeText OgCheckDTD
  OgGetSvnRevision
  OgCksum OgMD5sign
  OgStringProtect OgStringSqueeze OgStringTrim
  OgStringParagTrim OgStringListParagTrim
  OgStringParagIndent OgStringListParagIndent
  OgStringParagComment OgStringListParagComment
  OgStringParagUncomment OgStringListParagUncomment
  OgFileSlurp
  OgDebug OgVerbose
  OgCheckProcess OgKillProcess OgDetachProcess
  OgPlatformPath
  OgIsPathInGitRepo
);

###
# SYSTEM UTILITY FUNCTIONS
#

use constant OS_NONE    => 0;
use constant OS_WINDOWS => 1;
use constant OS_LINUX   => 2;
our %STRINGS_OS = (
  OS_NONE()    => "unknown",    # Note: required paren() to avoid hash quoting
  OS_WINDOWS() => "win",
  OS_LINUX()   => "linux"
);

#
# OS "tag" names
#
# The tags are used to differentiate OS versions among themselves, mostly based
# on binary compatibility, e.g.:
# - all Redhat and Centos 5.x are compatible and thus have the same tag (redhat-5).
# - ubuntu 11.04 and 11.10 aren't necessarily compatible and thus have different tags.
#
# Entries in the lookup table below must have the following format:
#   OS name (regex) => OS tag (composite string)
#
# with:
# - OS name: Regular perl regex, with parentheses for block matching
# - OS tag:  Regular perl string concatenation within a q{}, with $1..$n
#            corresponding to regex blocks
#
# regex reference: http://stackoverflow.com/a/4881005
#
our %STRINGS_OS_TAG = (
  # OS name (regex)      => OS tag (composite string)
  q{win.*}               => q{"win"},           # WinXP => win, windows => win, Win7 => win
  q{redhat\w*-(\d+)\..*} => q{"redhat-" . $1},  # RedHatES-4.7 => redhat-5
  q{red hat.* release (\d+)\..*} => q{"redhat-" . $1},  # Red Hat Enterprise Linux Server release 5.8 (Tikanga)
  q{centos-(\d+)\..*}    => q{"redhat-" . $1},  # CentOS-5.5 => redhat-5
  q{centos.* release (\d+)\..*}    => q{"redhat-" . $1}   # CentOS 6.x => redhat-6
  # no transformations for ubuntu, use full OS name
);

use constant ARCH_NONE   => 0;
use constant ARCH_X86_32 => 1;
use constant ARCH_X86_64 => 2;
our %STRINGS_ARCH = (
  ARCH_NONE()   => "unknown",
  ARCH_X86_32() => "x86",
  ARCH_X86_64() => "x86_64"
);

our $arch_cache_sysOs;
our $arch_cache_sysArch;

use constant CYGPATH_UNKNOWN => 0;
use constant CYGPATH_UNIX    => 1;
use constant CYGPATH_WINDOWS => 2;
use constant CYGPATH_MIXED   => 3;
our %STRINGS_CYGPATH = (
  CYGPATH_UNKNOWN() => "unknown",
  CYGPATH_UNIX()    => "unix",
  CYGPATH_WINDOWS() => "windows",
  CYGPATH_MIXED()   => "mixed"
);

#
# Return TRUE is path is absolute
#
sub OgPathIsAbsolute($)
{
  my ($path) = @_;
  return $path =~ /^[ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz]:/ || $path =~ /^\//;
}

#
# Returns OS and ARCH type of running platform
#
sub OgCheckPlatform ()
{
  my $sysOs = $::arch_cache_sysOs;
  if (!defined($sysOs))
  {
    $::arch_cache_sysOs = `uname -s`;
    $sysOs = $::arch_cache_sysOs;
  }

  my $sysArch = $::arch_cache_sysArch;
  if (!defined($sysArch))
  {
    $::arch_cache_sysArch = `uname -m`;
    $sysArch = $::arch_cache_sysArch;
  }
  my ( $os, $arch );

  chomp($sysOs);
  chomp($sysArch);

  if ( length($sysOs) == 0 ) {
    print STDERR ("Erreur, uname didn't return anything\n") if $::VERBOSE;
    $os = OS_NONE;
  }
  elsif ( $sysOs =~ /CYGWIN/i ) {
    $os = OS_WINDOWS;
  }
  elsif ( $sysOs =~ /Linux/i ) {
    $os = OS_LINUX;
  }
  else {
    print STDERR ("Error: unknown OS ($sysOs)\n") if $::VERBOSE;
    $os = OS_NONE;
  }

  if ( length($sysArch) == 0 ) {
    print STDERR ("Error: uname didn't return anything\n") if $::VERBOSE;
    $arch = ARCH_NONE;
  }
  elsif ( $sysArch =~ /86(-|_)64/i ) {
    $arch = ARCH_X86_64;
  }
  elsif ( $sysArch =~ /86/i ) {
    $arch = ARCH_X86_32;
  }
  else {
    print STDERR ("Error: Unknown architecture ($sysArch)\n") if $::VERBOSE;
    $arch = ARCH_NONE;
  }

  return ( $os, $arch );
}

#
# Returns OS and ARCH type of $platform passed as argument
#
sub OgParsePlatform ($)
{
  my ($platform) = @_;
  my ( $os, $arch );

  if ( $platform =~ /(\w+?)(\d{2})/ ) {
    if ( $1 eq "win" ) {
      $os = OS_WINDOWS;
    }
    elsif ( $1 eq "linux" ) {
      $os = OS_LINUX;
    }
    else {
      $os = OS_NONE;
    }
    if ( $2 eq "32" ) {
      $arch = ARCH_X86_32;
    }
    elsif ( $2 eq "64" ) {
      $arch = ARCH_X86_64;
    }
    else {
      $arch = ARCH_NONE;
    }
  }
  else {
    print STDERR ("Error: Can't parse platform: $platform\n") if $::VERBOSE;
    return ( OS_NONE, ARCH_NONE );
  }

  return ( $os, $arch );
}

#
# Returns name of $os as string. If optional %$string param is passed, first
# looks up $os in %$string and falls back to @STRINGS_OS otherwise.
#
sub OgOsString ($;$)
{
  my ( $os, $strings ) = @_;

  if ( $strings->{$os} ) {
    return $strings->{$os};
  }
  else {
    return ( $STRINGS_OS{$os} );
  }
}

#
# Returns version of $os as string
#
sub OgOsVersion ($)
{
  my ($os) = @_;
  my $version = "";

  if ( $os eq ogutil::OS_WINDOWS ) {
    my $ver = `cmd.exe /C ver`;
    $ver =~ s/(\r?\n)+//g;
    if ($ver =~ /(.+)\[(.+)\]/) {
      my $distrib = OgStringTrim( $1 );
      my $release = OgStringTrim ( $2 );

      $distrib =~ s/Microsoft\s*//ig;
      $release =~ s/version\s*//ig;

      # For now just print distribution, we're totally not yet to the point
      # where testing specific releases becomes tractable
      #$version = "$distrib-$release";
      $version = "$distrib";
    }
    else {
      $version = "Windows";
    }
  }
  else {
    if ( system("which lsb_release >/dev/null 2>&1") == 0 ) {

      my @lsb_out = split( /\n/, `lsb_release -i -r -c`, 3 );

      # LSB compliant system, easy !
      my ( undef, $distrib )  = split( /:/, $lsb_out[0], 2 );
      my ( undef, $release )  = split( /:/, $lsb_out[1], 2 );
      my ( undef, $codename ) = split( /:/, $lsb_out[2], 2 );

      $distrib = OgStringTrim( $distrib );
      $release = OgStringTrim( $release );
      $codename = OgStringTrim( $codename );

      if ($distrib =~ /RedHat/i) {
        $distrib =~ s/RedHatEnterprise/RedHat/g;
        (my $minor = $codename) =~ s/NahantUpdate\s*//ig;
        $version = "$distrib-$release.$minor";
      } elsif ($distrib =~ /CentOS/i) {
        $version = "$distrib-$release";
      } elsif ($distrib =~ /Ubuntu/i) {
        $version = "$distrib-$release";
      } else {
        $version = "$distrib-$release-$codename"
      }
    }
    else {
      if ( -f "/etc/redhat-release" ) {
        $version = `cat /etc/redhat-release`;
      }
      elsif ( -f "/etc/SuSE-release" ) {
        $version = `cat /etc/redhat-release`;
      }
      elsif ( -f "/etc/debian_version" ) {
        $version = `cat /etc/debian_version`;
      }
      else {
        $version = "Linux";
      }
    }
  }

  $version =~ s/(\r?\n)+//g;
  #$version =~ s/\s+//g;

  return ( $version );
}


#
# Provide OS tag string based on %STRINGS_OS_TAG substitutions
#
sub OgOsTagString ($) {
  my ($os) = @_;
  my $osStr = "";

  my $osVer = OgOsVersion($os);
  while (my ($from, $to) = each(%STRINGS_OS_TAG) ) {
    if ($osVer =~ /^$from$/i) {
      ($osStr = $osVer) =~ s/^$from$/$to/eei;
      last;
    }
  }
  if (length($osStr) == 0) {
    $osStr = lc($osVer);
  }

  return($osStr);
}


#
# Returns name of $arch as string. If optional %$string param is passed, first
# looks up $arch in %$string and falls back to @STRINGS_ARCH otherwise.
#
sub OgArchString($;$)
{
  my ( $arch, $strings ) = @_;

  if ( $strings->{$arch} ) {
    return $strings->{$arch};
  }
  else {
    return ( $STRINGS_ARCH{$arch} );
  }
}

#
# Returns a list out of a path
# ex: /opt/pertimm    ->  (, opt, pertimm)
#     d:\opt\pertimm  ->  (d:, opt, pertimm)
#
sub OgPathToList ($)
{
  my ($path) = @_;

  my ( $os, $arch ) = OgCheckPlatform();
  if ( $os == OS_WINDOWS ) {
    return ( split( /\\/, $path ) );
  }
  elsif ( $os == OS_LINUX ) {

    # Note: if path is absolute, split will return a list with nothing as
    # first element. KEEP IT: dir() takes that as a marker of absolute path
    return ( split( /\//, $path ) );
  }

  # TODO: Error
  return ( () );
}

#
# Converts Cygwin paths
#  unix:     /cygdrive/c/windows
#  windows:  d:\windows
#  mixed:    d:/windows
#
sub OgCygpath ($$)
{
  my ( $path, $outType ) = @_;
  my $inType = CYGPATH_UNKNOWN;

  # Checking input format
  if ( $path =~ /^\w:(\/|\\)/ ) {

    # Drive letter -> Windows or Mixed
    if ( $1 eq "/" ) {
      $inType = CYGPATH_MIXED;
    }
    elsif ( $1 eq "\\" ) {
      $inType = CYGPATH_WINDOWS;
    }
    else {
      $inType = CYGPATH_WINDOWS;
    }
  }
  elsif ( $path =~ /^\// ) {

    # Unix absolute path
    $inType = CYGPATH_UNIX;
  }
  else {

    # Relative path, either Unix or Windows
    if ( $path =~ /\// ) {
      $inType = CYGPATH_UNIX;
    }
    elsif ( $path =~ /\\/ ) {
      $inType = CYGPATH_WINDOWS;
    }
    else {
      $inType = CYGPATH_UNKNOWN;
    }
  }

  # Splitting path into list (could use OgPathToList when fixed)
  my @lpath;
  if ( $inType == CYGPATH_UNIX || $inType == CYGPATH_MIXED ) {
    @lpath = split( /\//, $path );
  }
  elsif ( $inType == CYGPATH_WINDOWS ) {
    @lpath = split( /\\/, $path );
  }
  else {

    #TODO: error
    @lpath = ();
  }

  # Formatting output path
  if ( scalar(@lpath) == 0 ) {

    # No path
    $path = "";
  }
  elsif ( $outType == CYGPATH_UNIX ) {
    if ( $inType == CYGPATH_UNIX ) {

      # In Unix - Out Unix
      $path = join( "/", @lpath );
    }
    elsif ( $inType == CYGPATH_WINDOWS || $inType == CYGPATH_MIXED ) {

      # In Windows/Mixed - Out Unix
      if ( $lpath[0] =~ /^(\w):$/ ) {

        # Input path is absolute
        $path = join( "/", "", "cygdrive", $1, @lpath[ 1 .. $#lpath ] );
      }
      else {

        # Input path is relative
        $path = join( "/", @lpath );
      }
    }
  }
  elsif ( $outType == CYGPATH_WINDOWS ) {
    if ( $inType == CYGPATH_UNIX ) {

      # In Unix - Out Windows
      if ( $lpath[0] eq "" && $lpath[1] eq "cygdrive" ) {

        # Input path is absolute with drive letter
        $path = join( "\\", "$lpath[2]:", @lpath[ 3 .. $#lpath ] );
      }
      else {

        # Input path is relative
        $path = join( "\\", @lpath );
      }

      # FIXME: add absolute with no drive letter (e.g.: /usr)
    }
    elsif ( $inType == CYGPATH_WINDOWS || $inType == CYGPATH_MIXED ) {

      # In Windows/Mixed - Out Windows
      $path = join( "\\", @lpath );
    }
  }
  elsif ( $outType == CYGPATH_MIXED ) {
    if ( $inType == CYGPATH_UNIX ) {

      # In Unix - Out Mixed
      if ( $lpath[0] eq "" && $lpath[1] eq "cygdrive" ) {

        # Input path is absolute with drive letter
        $path = join( "/", "$lpath[2]:", @lpath[ 3 .. $#lpath ] );
      }
      else {

        # Input path is relative
        $path = join( "/", @lpath );
      }

      # FIXME: add absolute with no drive letter (e.g.: /usr)
    }
    elsif ( $inType == CYGPATH_WINDOWS || $inType == CYGPATH_MIXED ) {

      # In Windows/Mixed - Out Mixed
      $path = join( "/", @lpath );
    }
  }
  else {
    $path = "";
  }

  return ($path);
}

#
# Test function of OgCygPath
#
sub TestOgCygpath ()
{
  my @test = (
    "Empty",            '',
    "Unix absolute 1",  '/cygdrive/d/toto/titi',
    "Unix absolute 2",  '/usr/bin/toto',           # FIXME: won't work
    "Unix relative",    'toto/titi',
    "Windows absolute", 'd:\toto\titi',
    "Windows relative", 'toto\titi',
    "Mixed absolute",   'd:/toto/titi',
  );
  for ( my $i = 0 ; $i < scalar(@test) ; $i += 2 ) {
    foreach my $type ( ( CYGPATH_UNIX, CYGPATH_WINDOWS, CYGPATH_MIXED ) ) {
      my $path = OgCygpath( $test[ $i + 1 ], $type );
      print "OgCygpath(\"$test[$i+1]\") \[$test[$i] -> $STRINGS_CYGPATH{$type}\]: $path\n";
    }
  }
}

##
# XML UTILITY FUNCTIONS
#

#
# @fn check dtd with factory's xmllint
# due to bug with original xmllint from linux distribution
#
# \param dtdPath
# \param xmlFile
#
sub OgCheckDTD ($$)
{
  my ( $dtdPath, $xmlFile ) = @_;
  my ( $os, $arch ) = OgCheckPlatform();

  # Convert Path Dos to Unix ("\" -> "/" )
  my $OG_TOOL_PATH = "$ENV{OG_TOOL_PATH}";
  $OG_TOOL_PATH =~ s/\\/\//g;

  my $cmd = "xmllint";
  if ( system("which $cmd > /dev/null") != 0 ) {
    if ( $os == ogutil::OS_WINDOWS ) {
      $cmd = file( $OG_TOOL_PATH, "bin", "win32", "xmllint" );
    }
    elsif ( $os == ogutil::OS_LINUX ) {
      $cmd = file( $OG_TOOL_PATH, "bin", "linux", "xmllint" );
    }
  }

  if ( $os == ogutil::OS_WINDOWS ) {
    # xmllint --path option only takes URI format (i.e. no Windows path)
    $dtdPath = `cygpath -u $dtdPath`;
    chomp($dtdPath);
  }

  system("$cmd --path $dtdPath --valid $xmlFile >/dev/null 2>&1");

  return( $? == 0 );
}

#
# Returns the text content of an element
#
sub OgXmlGetNodeText ($)
{
  # Text node is always last according to DOM spec
  my ($element) = @_;

  if (!defined($element))
  {
   return ("");
  }
  my $child = $element->getLastChild();
  return ("") unless defined($child);

  if ( $child->getNodeType() == XML::DOM::TEXT_NODE
    || $child->getNodeType() == XML::DOM::CDATA_SECTION_NODE )
  {
    return ( $child->getNodeValue() );
  }

  return ("");
}

##
# SVN UTILITY FUNCTIONS
#

#
# Get repository revision number
#
# Svn revision can be used on git repository too
#
sub OgGetSvnRevision ($)
{
  my ($pse_path) = @_;
  my $currentdir = `pwd`;

  my $svnRev = 0;

  chdir($pse_path) || die("Invalid chdir $! ($pse_path)\n");

  my $is_git_respoitory = 0;
  if (system("git rev-parse --is-inside-work-tree") == 0)
  {
    $is_git_respoitory = 1;
  }

  # chosee betwen SVN and SVN
  if (OgIsPathInGitRepo($pse_path))
  {

    # meme algo que dans la DOC

    # récupération et formatage de la date du dernier commit
    my $git_date = `git log -1 --format=%ci | sed s/'[: -]'/''/g | head -c12`;

    # récupération de la clef de commit
    my $git_short_id = `git log -1 --pretty=format:%h`;

    $svnRev = "${git_date}_${git_short_id}";

  }
  else
  {

    my $svnInfoXml = `svn info --xml`;
    if ( !defined($svnInfoXml) || $? != 0 ) {
      print("Warning, can't fetch SVN revision, using 0.\n");
      return (0);
    }

    my $parser         = new XML::DOM::Parser;
    my $svnInfoXmlRoot = $parser->parse($svnInfoXml);
    if ( !defined($svnInfoXmlRoot) ) {
      print("Warning, can't fetch SVN revision, using 0.\n");
      return (0);
    }

    my @svnRevList = $svnInfoXmlRoot->xql('/info/entry/commit/@revision');
    if ( scalar(@svnRevList) == 0 ) {
      print("Warning, can't fetch SVN revision, using 0.\n");
      return (0);
    }

    $svnRev = OgXmlGetNodeText( $svnRevList[0] );

    $svnInfoXmlRoot->dispose;

  }

  if (defined($currentdir) && $currentdir ne "")
  {
    chdir($currentdir);
  }

  return ($svnRev);
}

##
# FILE UTILITY FUNCTIONS
#

#
# Returns the checksum and file size of a file passed as argument
#
sub OgCksum ($)
{
  my ($fn) = @_;

  my $md5 = Digest::MD5->new;
  open( my $fh, "$fn" ) or die("Can't open file $fn\n");
  binmode($fh) or die("Can't binmode file $fn\n");
  $md5->addfile($fh);
  my $digest = $md5->hexdigest;
  close($fh);

  my $size = -s $fn;

  return ( $digest, $size );
}

#
# Compute md5 sign (same result between linux and windows)
#
sub OgMD5sign($)
{
  my ($fn) = @_;
  my $target = "";

  my ( $os, $arch ) = OgCheckPlatform();
  if ( $os == ogutil::OS_LINUX ) {
    $target = $fn . "_tmp";
    `unix2dos -q -n $fn $target`;
  }
  else {
    $target = "$fn";
  }
  my $md5 = Digest::MD5->new;
  open( my $fh, "$target" ) or die("Can't open file $target\n");
  binmode($fh) or die("Can't binmode file $target\n");
  $md5->addfile($fh);
  my $digest = $md5->hexdigest;
  close($fh);

  my $size = -s $target;
  if ( $os == ogutil::OS_LINUX ) {
    `rm $target`;
  }
  return ( $digest, $size );
}

##
# STRING UTILITY FUNCTIONS
#

#
# Protects all quote symbols in a string, so that the string can
# itself be quoted safely
#
sub OgStringProtect ($)
{
  my ($str) = @_;

  return ("") unless $str;

  $str =~ s/\"/\\\"/g;
  $str =~ s/\'/\\\'/g;

  return ($str);
}

#
# Removes empty lines surrounding a paragraph
# Does not change heading/trailing spaces
#
sub OgStringSqueeze ($)
{
  my ($str) = @_;

  return ("") unless $str;

  $str =~ s/^([ \t]*\n)*//g;
  $str =~ s/(\n[ \t]*)*$//g;

  return ($str);
}

#
# Removes spaces at begining and end of string
#
sub OgStringTrim ($)
{
  my ($str) = @_;

  return ("") unless $str;

  $str =~ s/^\s+//;
  $str =~ s/\s+$//;

  return ($str);
}

#
# Trims paragraph but keeps relative indentation (list)
# Removes n spaces from all lines in string, where n will be the smallest
# number of heading spaces amongst all lines in the string.
#
# TODO: could be more efficient
#
sub OgStringListParagTrim (@)
{
  my (@list) = @_;
  my $indent;

  return ( () ) unless scalar(@list);

  foreach my $line (@list) {
    if ( $line =~ /^(\s+)/ && ( !$indent || length($1) < $indent ) ) {
      $indent = length($1);
    }
  }
  $indent = 0 unless $indent;

  @list = map { s/^\s{$indent}//; $_; } @list;    # need to explicitely return $_ in map
  return (@list);
}

#
# Trims paragraph (removes all forms of whitespaces)
# but keeps relative indentation (string)
#
sub OgStringParagTrim ($)
{
  my ($str) = @_;

  return ("") unless $str;

  $str = join( "\n", OgStringListParagTrim( split( /\n/, $str ) ) );

  return ($str);
}

#
# Indents paragraph (list)
# Adds $indent spaces to each line in list
#
sub OgStringListParagIndent ($@)
{
  my ( $indent, @list ) = @_;

  return ( () ) unless scalar(@list);

  @list = map { " " x $indent . $_ } @list;

  return (@list);
}

#
# Indents paragraph (string)
#
sub OgStringParagIndent ($$)
{
  my ( $indent, $str ) = @_;

  return ("") unless $str;

  $str = join( "\n", OgStringListParagIndent( $indent, split( /\n/, $str ) ) );

  return ($str);
}

#
# Comments paragraph (list)
# Adds '# ' to each line
#
sub OgStringListParagComment (@)
{
  my (@list) = @_;

  return ( () ) unless scalar(@list);

  @list = map { $_ = "# $_" } @list;

  return (@list);
}

#
# Comments paragraph (string)
#
sub OgStringParagComment ($)
{
  my ($str) = @_;

  return ("") unless $str;

  $str = join( "\n", OgStringListParagComment( split( /\n/, $str ) ) );

  return ($str);
}

#
# Uncomments paragraph (list)
# Removes '\s*# ' from each line
#
sub OgStringListParagUncomment (@)
{
  my (@list) = @_;

  return ( () ) unless scalar(@list);

  @list = map { s/\s*#//; $_; } @list;

  return (@list);
}

#
# Uncomments paragraph (string)
#
sub OgStringParagUncomment ($)
{
  my ($str) = @_;

  return ("") unless $str;

  $str = join( "\n", OgStringListParagUncomment( split( /\n/, $str ) ) );

  return ($str);
}


##
# FILE UTILITY FUNCTIONS
#

#
# Return entire file content as scalar variable
#
# Example:
# open( my $fh, $filename ) or die "Can't open file '$filename'\n";
# my $text = OgFileSlurp($fh);
# close($fh);
#
sub OgFileSlurp ($)
{
  my ($filehandle) = @_;

  my $text = do {
    local( $/ );
    <$filehandle>
  };

  return($text);
}


##
# MISCELLANEOUS UTILITY FUNCTIONS
#

#
# Prints message only if DEBUG mode is activated
#
sub OgDebug (@)
{
  print STDERR @_ if $::DEBUG;
}

#
# Prints message only if VERBOSE mode is activated
#
sub OgVerbose (@)
{
  print STDOUT @_ if $::VERBOSE;
}

##
# SYSTEM UTILITY FUNCTIONS
#

#
# Check if a process 'name' exists
#
sub OgCheckProcess($)
{
  my ($name) = @_;
  my ( $os, $arch ) = OgCheckPlatform();
  my $res;
  if ( $os == ogutil::OS_WINDOWS ) {
    $res = `tasklist | grep "$name.exe" | wc -l`;
  }
  else {
    $res = `ps -aef | grep "$name" | grep -v grep | wc -l`;
  }
  return $res > 0;
}

#
# Kill a process by its name
#
sub OgKillProcess($)
{
  my ($procName) = @_;
  my ( $os, $arch ) = OgCheckPlatform();
  if ( $os == ogutil::OS_WINDOWS ) {
    my @result = split( /\n/, `tasklist` );
    $procName .= ".exe";
    foreach my $line (@result) {
      my ( $pname, $pid, $session_name, $session_num, $mem ) = split( / +/, $line );
      if ( $procName eq $pname ) {
        print("killing $pname $pid\n");
        my $res = `taskkill /F /T /PID $pid`;
      }
    }
  }
  else {
    my @result = split( /\n/, `ps -e -o pid,command | grep $procName | grep -v grep` );
    foreach my $line (@result) {
      $line =~ s/^ +//g;
      my @psTab = split( / /, $line );
      kill( 9, $psTab[0] );
    }
  }
}

#
# Fork and run a program and return its pid
#
sub OgDetachProcess($)
{
  my ($cmd) = @_;

  my $pid = fork();
  if ( $pid == 0 ) {    # child
    $pid = $$;

    exec($cmd) or exit(1);
  }
  return $pid;
}


###
# PERTIMM SPECIFIC UTILITY FUNCTIONS
#

sub OgPlatformPath ($) {
  my ($path) = @_;
  my ($os, $arch) = OgCheckPlatform();

  my $osTag = OgOsTagString($os);
  unless (-d dir($path, $osTag)) {
    # Check if we have a reasonnable fallback
    # This will work on older branches that don't have OS-specific paths
    $osTag = undef;
    $osTag = "linux" if ($os == OS_LINUX   && -d dir($path, "linux"));
    $osTag = "win"   if ($os == OS_WINDOWS && -d dir($path, "win"));
    if (defined($osTag)) {
      print "Warning: using non OS specific path (this is OK if you're using an older branch)\n";
    }
  }
  return(undef) unless defined($osTag);

  my $archTag = OgArchString($arch, {
    ARCH_X86_32() => "x86",
    ARCH_X86_64() => "x86-64"  # We must use x86-64, not x86_64
  });

  return(dir($path, $osTag, $archTag));
}

sub OgIsPathInGitRepo ($)
{
  my ($path) = @_;
  my $currentdir = `pwd`;

  my $svnRev = 0;

  chdir($path) || die("OgIsPathInGitRepo : Invalid chdir $! ($path)\n");

  my $result = system("git rev-parse --is-inside-work-tree >/dev/null 2>&1");
  my $is_git_respoitory = 0;
  if ($result == 0)
  {
    $is_git_respoitory = 1;
  }

  if (defined($currentdir) && $currentdir ne "")
  {
    chdir($currentdir);
  }

  return $is_git_respoitory;
}

1;
