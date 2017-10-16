#!/usr/bin/perl -w -U
# $Id$
#
# Description:
#  Script use to initialize the /build directory with good content of /sources/private
#

sub printUsage {
  print <<END;
Usage: buildinit.pl [options]

Options:
  verbose    Prints some more or less useful warnings and infos about process
  debug      Enable debug mode
  update     Do not reset 'build' directory, only update it
  rh73       Force OS/hardware to Linux Redhat 7.3/x86
END
  exit;
}

use strict;
use Getopt::Long;
use File::Path qw(rmtree);
use Path::Class qw(file dir);
use File::Copy qw(copy);
use lib "$ENV{OG_TOOL_PATH}/bin";
use ogutil;

our $VERBOSE = 0;
our $DEBUG = 0;


##
# Main
#

my $update = 0;
my $rh73 = 0;
GetOptions('help' => \&printUsage,
  'verbose!' => \$VERBOSE,
  'debug!' => \$DEBUG,
  'update!' => \$update,
  'rh73' => \$rh73
);

# Check Env Var
if (! exists $ENV{OG_REPO_PATH}) {
  print("Erreur, la variable d'environnement OG_REPO_PATH n\'existe pas\n");
  exit(1);
}

my $OG_REPO_PATH = OgCygpath($ENV{OG_REPO_PATH}, ogutil::CYGPATH_UNIX);

my ($os, $arch) = OgCheckPlatform();

if ($os == ogutil::OS_NONE || $arch == ogutil::ARCH_NONE) {
  print "Error: Invalid platform.\n";
  exit(1);
}

my $buildpath = dir($ENV{OG_REPO_PATH}, "build");

OgVerbose("- OG_REPO_PATH = '$OG_REPO_PATH'\n");
OgVerbose("- buildpath    = '$buildpath'\n");
OgVerbose("- update       = $update\n");

if (!$update) {
  OgVerbose("Resetting 'build' directory.\n");
  rmtree("$buildpath");
  die("Build directory cannot be deleted ($buildpath)") if (-d $buildpath);
}

system("mkdir -p $buildpath/debug/bin");
system("mkdir -p $buildpath/debug/link");
system("mkdir -p $buildpath/release/bin");
system("mkdir -p $buildpath/release/link");

##
# Functions
#

#
# Recursive copy
#
# Should be replaced by File::Copy::Recursive with filter on ".svn"
# when recursive copy will be available (for real) on CPAN
#
sub OgCopy {
  my ($from, $to, $recursive, $update) = @_;

  defined($recursive) or $recursive = 0;
  $from = "$from";  # So we can take Path::Class objects as input
  $to = "$to";

  if (-d $from && !$recursive) {
    die("Can't copy directory without recursive flag on");
  }

  if (-d $from && -f $to) {
    die("Can't copy a directory to a file");
  }

  if (-f $from) {
    if (-d $to) {
      OgCopy($from, file($to, $from), 0, $update);
    } else {
      OgDebug("Copying file $from\n");
      copy($from, $to);
      my $mode = (stat($from))[2] & 07777;
      chmod($mode, $to);
    }
  } else {
    opendir(DIR, $from) || warn("Can't open directory $from: $!\n");
    my @items = grep {!/^\./} readdir(DIR);  # will also drop ".svn"
    closedir (DIR);
    foreach my $item (@items) {
      if (-f file($from, $item)) {
        OgCopy(file($from, $item), file($to, $item), 0, $update);
      } else {
        my $fromSub = dir($from, $item);
        my $toSub = dir($to, $item);
        if (-d $toSub) {
          if ($update) {
            OgCopy($fromSub, $toSub, 1, $update);
          } else {
            warn("$toSub already exists: $!");
          }
        } else {
          mkdir($toSub) || warn("Can't create directory $toSub: $!");
          OgCopy($fromSub, $toSub, 1, $update);
        }
      }
    }
  }
}
