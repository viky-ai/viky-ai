#!/usr/bin/perl -w
#
# Description:
#  Script used to generate makefile for specific platforms from the XML
#  description of a project.
#

sub printUsage {
  (my $name = $0) =~ s/.*\/(.*)$/$1/;
  print <<END;
Usage: $name [options] TARGET PROJECT_DIR
  Required:
    TARGET       Platform target to build makefile for (win32, linux32/64)
    PROJECT_DIR  Directory of the project you want to generate makefile for
                 Must contain makefile.xml (if not given, take current dir)
                 Use 'all' to generate all components makefile
                 (see sources/manifest.xml)
  Options:
    -verbose    Prints some more or less useful warnings and infos about process
END
  exit;
}

use strict;
use XML::DOM;
use XML::XQL;
use XML::XQL::DOM;
use Getopt::Long;
use Data::Dumper;
use Path::Class;
use Cwd 'abs_path';
use lib "$ENV{OG_TOOL_PATH}/bin";
use ogutil;

my @PLATFORMS = ("win32", "linux32", "linux64");  #TODO: move to ogutil.pm ?

my $COROOT;
my $OG_TOOL_PATH;
my $OG_REPO_PATH;
my $VERBOSE = 0;

my $parser = new XML::DOM::Parser;

my $type;
my $project_name;
my @sources;
my @tests;
my @options;

my $template;
my $makefile;
my $coroot_cygpath;
my $repo_path_cygpath;
my $rctools_cygpath;
my $gentesttools_cygpath;
my $sources_string;
my $options_string;
my $tests_string;
my $gentest_string;

my @componentTab;

##  @fn value parseComponentsManifest($filePath)
# Open component manifest file and parse it
#
# \param configPath manifest file path
#
sub parseComponentsManifest($) {
  my ($filePath)=@_;
  
  my $componentsManifestXmlDoc;

  # Read config file
  if (!-f $filePath) {
    die("Error: Component manifest $filePath doesn't exist");
  }
    
  # dtd check config
  OgCheckDTD( file("$OG_REPO_PATH", "products"), $filePath ) or warn "invalid $filePath";

  # parse config
  $componentsManifestXmlDoc=$parser->parsefile($filePath);

  # get all Components
  my @componentNodeList = $componentsManifestXmlDoc->xql("components_manifest/component");
  my $size = @componentNodeList;
  if ($size==0) {
    warn("Error, empty component list in file $filePath\n");
    return(-1);
  }
  my $componentItem;
  foreach $componentItem (@componentNodeList) {
    push(@componentTab,{ name  => $componentItem->getAttribute("name") });
  }
  
  # free memory
  $componentsManifestXmlDoc->dispose;
}

##  @fn value openMakefile($configPath)
# Open config file and parse it
#
# \param configPath configuration file path
#
sub openMakefileDesc($) {
  my $filePath=shift;
  
  # Read config file
  if (!-f $filePath) {
    # TODO: exception
    warn("Error: File $filePath doesn't exist");
    return("");
  }
  
  # dtd check config
  OgCheckDTD( file("$OG_REPO_PATH", "tools", "makefile"), $filePath) or warn "invalid $filePath";
 
  # parse config
  return $parser->parsefile($filePath);
}

##  @fn value closeConfig()
# Close config file and free memory
#
#
sub closeMakefileDesc($) {
  my ($makefileXmlDoc)=@_;
  $makefileXmlDoc->dispose;
}

sub parseMakefileDesc($;$) {
  my ($makefileXmlDoc,$platform)=@_;
  my $fileItem;
  my @filesTab;
  my @testsTab;
  my @optionTab;
  my $optionItem;
  
  my ($os, $arch) = OgParsePlatform($platform);
  my $strOs = OgOsString($os);
  my $strArch = OgArchString($arch);
  
  $type=getNodeText($makefileXmlDoc->xql("makefile/type"));
  $project_name=getNodeText($makefileXmlDoc->xql("makefile/project_name"));
  @filesTab =$makefileXmlDoc->xql("makefile/sources/file");

  foreach $fileItem (@filesTab) {
    push(@sources,getNodeText($fileItem));
  }
  # check if this platform is describe in makefile.xml
  if ($type ne "extern") {
    my @tab=$makefileXmlDoc->xql("makefile/platforms/platform[\@name='$strOs']");
    if ($#tab<0) {
      $type="notdefined";
    }
  }
  # add specific platforms sources
  @filesTab =$makefileXmlDoc->xql("makefile/platforms/platform[\@name='$strOs']/sources/file");
  foreach $fileItem (@filesTab) {
    push(@sources,getNodeText($fileItem));
  }

  # add test plan files
  @testsTab =$makefileXmlDoc->xql("makefile/tests/file");
  foreach $fileItem (@testsTab) {
    push(@tests,getNodeText($fileItem));
  }

  @optionTab=$makefileXmlDoc->xql("makefile/platforms/platform[\@name='$strOs']/option");
  foreach $optionItem (@optionTab) {
    my $optArch = $optionItem->getAttribute("arch");
    if (length($optArch) == 0 || $optArch eq $strArch) {
      push(@options, {
        name => $optionItem->getAttribute("name"),
        value => getNodeText($optionItem)
      });
    }
  }
}


## @fn private value getNodeText()
# return a xml text node from an element
#
# \return element
##
sub getNodeText($) {
  my $element= shift;
  my $child;
  my @childs=$element->getChildNodes();
  foreach $child (@childs) { # search text node
    if ($child->getNodeType()==TEXT_NODE ||
  $child->getNodeType()==CDATA_SECTION_NODE) {
      return $child->getNodeValue();
    }
  }
  return "";
}

sub constructSourceString($) {
  my ($platform)=@_;
  my ($os, $arch) = OgParsePlatform($platform);
  
  $sources_string =  "";
  if($os == ogutil::OS_LINUX) {
    foreach(@sources) {
      $sources_string = $sources_string." $_";
    }
  } elsif($os == ogutil::OS_WINDOWS) {
    foreach(@sources) {
      s/\.cpp/\.obj/;
      s/\.c/\.obj/;
      s/\.rc/\.res/;
      $sources_string = $sources_string." \$(TARGET)\\$_";
    }
  }
}

sub constructOptionsString() {
  $options_string = "";
    foreach my $option (@options) {
      $options_string = $options_string .$option->{name}." = ".$option->{value}."\n";
    }
}

sub constructTestsString() {
  $tests_string = "";
  foreach my $test (@tests) {
    $tests_string = $tests_string.$test." ";
  }
  if (length($tests_string)>0) { # activate test generation
    $gentest_string="GENTEST";
  }
  else {
    $gentest_string="";
  }
}

#
# Search a component in componentTab
#
sub getComponent($) {
  my ($name)=@_;
  my $componentItem;
  my $i;

  my $size=@componentTab;
  for ($i=0; $i<$size; $i++) {
    if ($componentTab[$i]{name} =~$name) {
      return $i;
    }
  }
  return -1;
}

sub genMakefile($$$) {
  my ($platform, $componentName, $projectdir) = @_;
  my ($os, $arch) = OgParsePlatform($platform);

  my $makefileXml = file($projectdir, "makefile.xml");
  unless (-f $makefileXml) {
    print("Error: $makefileXml does not exist\n");
    return(-1);
  }
  my $makefileXmlDoc = openMakefileDesc($makefileXml);
  if (length($makefileXmlDoc) == 0) {
    warn("Empty makefile description");
    return(-1);
  }
  parseMakefileDesc($makefileXmlDoc,$platform);
  if ($type eq "extern") {
    printf("Skip $projectdir (extern component)\n");
    return;
  }

  # Debug
  if($VERBOSE) {
    print "- type\n";
    print Dumper($type);
    print "- project_name\n";
    print Dumper($project_name);
    print "- sources\n";
    print Dumper(@sources);
    print "- options\n";
    print Dumper(@options);
  }

  die ("Need a valid project type ($type) in $projectdir/makefile.xml") if ($type ne "lib" && $type ne "prog" && $type ne "notdefined");
  
  $template= file("$OG_TOOL_PATH","makefile",OgOsString($os),"makefile_$type.tpl");
  $makefile= file("$projectdir","makefile.$platform");
  
  if ($type eq "notdefined") {
    printf("Undefined $projectdir for $platform => Gen empty makefile.$platform\n");
  }
  else {
    print("Gen $project_name ($projectdir) makefile.$platform\n");
  }
  constructSourceString($platform);
  constructTestsString();
  constructOptionsString();

  open TEMPLATE,"<$template" or die("Cannot open $template");
  open MAKEFILE,">$makefile" or die("Cannot open $makefile");
  
  while(<TEMPLATE>){
    s/\$\$\$\$OG_REPO_PATH\$\$\$\$/$repo_path_cygpath/;
    s/\$\$\$\$COROOT\$\$\$\$/$coroot_cygpath/ if defined($COROOT);
    s/\$\$\$\$OPTIONS\$\$\$\$/$options_string/;
    s/\$\$\$\$COMPONENT_NAME\$\$\$\$/$componentName/;
    s/\$\$\$\$GENRC_PATH\$\$\$\$/$rctools_cygpath/;
    s/\$\$\$\$GENTEST_PATH\$\$\$\$/$gentesttools_cygpath/;
    s/\$\$\$\$SOURCES\$\$\$\$/$sources_string/;
    s/\$\$\$\$SRCTESTS\$\$\$\$/$tests_string/;
    s/\$\$\$\$GENTESTS\$\$\$\$/$gentest_string/;
    print MAKEFILE "$_";
  }
 
  close(TEMPLATE);
  close(MAKEFILE);
  
  return(0);
  
}

#################
# MAIN FUNCTION #
#################

GetOptions('help' => \&printUsage,
  'verbose' => \$VERBOSE,
);

# Check Env Var
if (! exists $ENV{OG_REPO_PATH}) {
  print("Erreur, la variable d'environnement OG_REPO_PATH n\'existe pas\n");
  exit(1);
}
# Convert Path Dos to Unix ("\" -> "/" )
($OG_TOOL_PATH = "$ENV{OG_TOOL_PATH}") =~ s/\\/\//g;
($OG_REPO_PATH = "$ENV{OG_REPO_PATH}") =~ s/\\/\//g;
if (defined($ENV{COROOT})) {
  ($COROOT = "$ENV{COROOT}") =~ s/\\/\//g;
}

# Get real absolute path
$OG_REPO_PATH = abs_path("$OG_REPO_PATH");

# Getting parameters
if (scalar(@ARGV)<1) {
  print("Unsufficient parameters\n");
  printUsage();
}
my $platform = $ARGV[0];
my $projectdir = (scalar(@ARGV) > 1) ? $ARGV[1] : ".";

# Checking parameters
if (scalar(grep(/^$platform$/, @PLATFORMS)) == 0) {
  print("Error: Need a valid platform (" . join(", ", @PLATFORMS) . ")\n");
  printUsage();
}

# Setting constant paths that will be used in genMakefile
$rctools_cygpath = "$OG_TOOL_PATH/bin/genRC.pl";
$gentesttools_cygpath = "$OG_TOOL_PATH/bin/gentest.pl";
$coroot_cygpath = $COROOT if defined($COROOT);
$repo_path_cygpath = $OG_REPO_PATH;
if ($platform eq "win32") {
  #$repo_path_cygpath = OgCygpath($OG_REPO_PATH, ogutil::CYGPATH_WINDOWS);
  $coroot_cygpath = `cygpath -w $COROOT` if defined($COROOT);
  $repo_path_cygpath = `cygpath -w $OG_REPO_PATH`;
}

parseComponentsManifest(file("$OG_REPO_PATH","sources","manifest.xml"));
if ($projectdir eq "all") {
  print("Generate all components\n");
  
  my $size=@componentTab;

  for (my $i=0; $i<$size; $i++) {
    my $componentName = $componentTab[$i]{name};
    genMakefile($platform, $componentName, dir("$OG_REPO_PATH", "sources", $componentName));
    @sources = ();
    @options = ();
    @tests= ();
  }
} elsif (-d $projectdir) {
  genMakefile($platform, $projectdir, $projectdir);
} else {
  genMakefile($platform, $projectdir, dir("$OG_REPO_PATH", "sources", $projectdir));
}
print("Done\n");

exit(0)
#################
