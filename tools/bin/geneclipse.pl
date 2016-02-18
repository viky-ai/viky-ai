#!/usr/bin/perl -w
#
# Generate Eclipse CDT project config
#

sub printUsage {
  (my $name = $0) =~ s/.*\/(.*)$/$1/;
  print <<END;
Generate Eclipse CDT configuration from manifest.xml
Usage: $name [options] TARGET PROJECT_DIR
  Required:
    TARGET       Platform target to build Code::Blocks project config
    PROJECT_DIR  Directory of the project you want to generate Eclipse CDT conf for
                 Must contain makefile.xml (if not given, take current dir)
                 Use 'all' to generate all components
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
use lib "$ENV{OG_TOOL_PATH}/bin";
use ogutil;
use File::Basename;
use Cwd qw( realpath );

sub parseComponentsManifest ($);
sub genNBP ($);
sub parseMakefileDesc ($);
sub buildIncludeString ();
sub applyTemplate ($$);

my @PLATFORMS = ("win32", "linux32", "linux64");  #TODO: move to ogutil.pm ?
my %CBPLATFORMS = ("win32" => "Windows", "linux32" => "Unix", "linux64" => "Unix");

my $OG_TOOL_PATH;
my $OG_REPO_PATH = dir(OgPathToList($ENV{OG_REPO_PATH}));
my ($OS, $ARCH);

my $platform;
my $projectType;
my $projectName;
my $projectDir;
my $projectDir_new;
my $projectSourceDir;
my $projectId;
my $projectBranch;
my $number_of_cpus = 4;
my $outputDebug = "";
my $outputRelease = "";
my @includeList;
my $includeString;
my @sourceList;

my $parser = new XML::DOM::Parser;

my $VERBOSE = 0;

GetOptions('help' => \&printUsage,
  'verbose' => \$VERBOSE,
);

# Check Env Var
unless(exists($ENV{OG_REPO_PATH})) {
  print("Erreur, la variable d'environnement OG_REPO_PATH n'existe pas\n");
  exit(1);
}

# Getting parameters
unless(scalar(@ARGV) >= 1) {
  print("Error: Missing parameter\n");
  printUsage();
}

($OG_TOOL_PATH = "$ENV{OG_TOOL_PATH}") =~ s/\\/\//g;

$platform = $ARGV[0];
$projectDir = (scalar(@ARGV) > 1) ? $ARGV[1] : ".";

($OS, $ARCH) = OgParsePlatform($platform);

# Checking parameters
if(scalar(grep(/^$platform$/, @PLATFORMS)) == 0) {
  print("Error: Need a valid platform (" . join(", ", @PLATFORMS) . ")\n");
  printUsage();
}

if($projectDir eq "all") {
  print("Generating all components\n");

  my @componentList = parseComponentsManifest(file("$OG_REPO_PATH","sources","manifest.xml"));

  for(my $i = 0; $i < scalar(@componentList); $i++) {
    genNBP(file("$OG_REPO_PATH","sources", $componentList[$i]{name}));
  }
} else {
  genNBP($projectDir);
}

exit(0);



#
# Functions
#

##  @fn value parseComponentsManifest($filePath)
# Open component manifest file and parse it
#
# \param configPath manifest file path
#
sub parseComponentsManifest ($) {
  my ($filePath) = @_;

  # Read config file
  unless(-f $filePath) {
    die("Error: Component manifest $filePath doesn't exist");
  }

  # dtd check config
  OgCheckDTD( file("$OG_REPO_PATH", "products"), $filePath ) or warn "invalid $filePath";

  # parse config
  my $doc = $parser->parsefile($filePath);

  # get all Components
  my @componentNodeList = $doc->xql("components_manifest/component");
  if (scalar(@componentNodeList) == 0) {
    die("Error, empty component list in file $filePath\n");
  }

  my @componentList = ();
  foreach my $item (@componentNodeList) {
    push(@componentList, { name => $item->getAttribute("name") });
  }

  # free memory
  $doc->dispose;

  return(wantarray ? @componentList : \@componentList);
}


sub genNBP ($) {
  my ($projectDir) = @_;

  $projectDir_new = $projectDir;

  unless(parseMakefileDesc(file("$projectDir_new","makefile.xml")) == 0) {
    return(-1);
  }

  if($projectType eq "extern") {
    printf("Skip $projectDir_new (extern component)\n");
    return(0);
  }

  $projectBranch = "trunk";
  if($OG_REPO_PATH =~ /\/trunk\/ogmios$/g)
  {
    $projectBranch = "trunk";
  }
  elsif($OG_REPO_PATH =~ /\/stable\/ogmios$/g)
  {
    $projectBranch = "stable";
  }
  elsif($OG_REPO_PATH =~ /\/branch\/ogmios$/g)
  {
    $projectBranch = "branch";
  }

  print("Gen ${projectName}_${projectType}_${projectBranch} ($projectDir_new) : Eclipse CDT config.\n");
  # Debug
  if($VERBOSE) {
    print "- type\n";
    print Dumper($projectType);
    print "- project_name\n";
    print Dumper($projectName);
    print "- sources\n";
    print Dumper(@sourceList);
  }

  if($projectType ne "lib" && $projectType ne "prog") {
    warn("Need a valid project type ($projectType) in $projectDir/makefile.xml");
    return(-1);
  }

  # generate makefile
  `genmake.pl $platform $projectDir`;

  $includeString = buildIncludeString();

  if (! -e "$projectDir_new/.settings") {
    `mkdir $projectDir_new/.settings`;
  }

  # Generate a random id between 1000000000 and 9999999999
  $projectId = int(rand(9000000000)) + 1000000000 - 1;

  my $conf_template = file($ENV{OG_TOOL_PATH}, "template", "eclipse", "project.xml");
  my $conf_target = file("$projectDir_new", ".project");
  applyTemplate($conf_template, $conf_target);

  $conf_template = file($ENV{OG_TOOL_PATH}, "template", "eclipse", "cproject.xml");
  $conf_target = file("$projectDir_new", ".cproject");
  applyTemplate($conf_template, $conf_target);

  $conf_template = file($ENV{OG_TOOL_PATH}, "template", "eclipse", "language.settings.xml");
  $conf_target = file("$projectDir_new", ".settings", "language.settings.xml");
  applyTemplate($conf_template, $conf_target);

  # include conf
  if(${projectType} =~ /lib/ )
  {
    my $module_name = "${projectName}_${projectType}_${projectBranch}";
    if ( $projectName =~ /ebusiness-.*/ )
    {
      $module_name = "${projectName}_${projectBranch}";
    }

    # create project file
    my $include_cproject_file = "${OG_REPO_PATH}/sources/include/.cproject";
    unless ( -e $include_cproject_file)
    {
      $projectName = "include";
      $conf_template = file($ENV{OG_TOOL_PATH}, "template", "eclipse", "cproject.xml");
      $conf_target = file($include_cproject_file);
      applyTemplate($conf_template, $conf_target);

      # disable build on include
      my $sed_cmd_1 = "sed -i -e 's|enableCleanBuild=\"true\" enabledIncrementalBuild=\"true\"|enableCleanBuild=\"false\" enabledIncrementalBuild=\"false\"|' ${include_cproject_file}";
      system($sed_cmd_1);
    }

    # create project file
    my $include_project_file = "${OG_REPO_PATH}/sources/include/.project";
    unless ( -e $include_project_file)
    {
      $projectName = "include";
      $conf_template = file($ENV{OG_TOOL_PATH}, "template", "eclipse", "project.xml");
      $conf_target = file($include_project_file);
      applyTemplate($conf_template, $conf_target);
    }

    # update reference projects list
    my $module_already_in_file = `grep \"<project>${module_name}</project>\"  ${include_project_file} | wc -l`;
    if ($module_already_in_file == 0)
    {
      my $sed_cmd_2 = "sed -i -e 's|</projects>|  <project>${module_name}</project>\\n  </projects>|' ${include_project_file}";
      system($sed_cmd_2);
    }

  }

  return(0);
}


sub applyTemplate ($$) {
  my ($template, $target) = @_;

  unless(open(TPL, "<$template") != 0) {
     warn("Cannot open $template");
     return(-1);
  }
  unless(open(TGT, ">$target") != 0) {
    warn("Cannot open $target");
    close(TPL);
    return(-1);
  }

  my $module_name = "${projectName}_${projectType}_${projectBranch}";
  if ( $projectName =~ /ebusiness-.*/ || $projectName =~ /include/ )
  {
    $module_name = "${projectName}_${projectBranch}";
  }

  while(<TPL>) {
    s/____MODULE_ID____/$projectId/g;
    s/____MODULE_NAME____/$module_name/g;
    s/____MAKEFILE____/makefile.$platform/g;
    s/____INCLUDES____/$includeString/g;
    s/____NUMBER_OF_CPUS____/$number_of_cpus/g;
    s/____SOURCESPATH____/$projectSourceDir/g;
    print TGT "$_";
  }
  close(TPL);
  close(TGT);

  return(0);
}


sub parseMakefileDesc ($) {
  my ($makefileDesc) = @_;

  # Read config file
  unless(-f $makefileDesc) {
    # TODO: exception
    warn("File $makefileDesc doesn't exist");
    $projectType = "";
    $projectName = "";
    @sourceList = ();
    return(-1);
  }

  # dtd check config
  OgCheckDTD( file($ENV{OG_TOOL_PATH}, "makefile"), $makefileDesc) or warn "invalid $makefileDesc";

  # parse config
  my $doc = $parser->parsefile($makefileDesc);

  unless(length($doc) != 0) {
    warn("Empty makefile description");
    $projectType = "";
    $projectName = "";
    @sourceList = ();
    return(-1);
  }

  my @r = $doc->xql("makefile/type");
  $projectType = OgXmlGetNodeText($r[0]);

  @r = $doc->xql("makefile/project_name");
  $projectName = OgXmlGetNodeText($r[0]);

  if ( $projectType eq "extern" ) {
    print "Skipping external project $projectName\n";
    return(0);
  }

  $projectSourceDir = "";

  if ( $projectName =~ /ogm_ssil.*/ )
  {
    my $pn = "";

    $pn = realpath(dirname($makefileDesc)."/../../../..");
    if ( basename($pn) =~ /ebusiness-.*/ )
    {
     $projectName = basename($pn);
     $projectDir_new = $pn;
    }

    $pn = realpath(dirname($makefileDesc)."/../../..");
    if ( basename($pn) =~ /ebusiness-.*/ )
    {
     $projectName = basename($pn);
     $projectDir_new = $pn;
    }

    $pn = realpath(dirname($makefileDesc)."/../..");
    if ( basename($pn) =~ /ebusiness-.*/ )
    {
     $projectName = basename($pn);
     $projectDir_new = $pn;
    }

    $pn = realpath(dirname($makefileDesc)."/..");
    if ( basename($pn) =~ /ebusiness-.*/ )
    {
     $projectName = basename($pn);
     $projectDir_new = $pn;
    }

    $pn = realpath(dirname($makefileDesc));
    if ( basename($pn) =~ /ebusiness-.*/ )
    {
     $projectName = basename($pn);
     $projectDir_new = $pn;
    }

    $projectSourceDir = realpath(dirname($makefileDesc));
    $projectSourceDir =~ s/$projectDir_new//g;
  }

  my @fileList = $doc->xql("makefile/sources/file");
  @sourceList = ();
  foreach my $src (@fileList) {
    push(@sourceList, OgXmlGetNodeText($src));
  }

  @includeList = (
    join("/", OgPathToList($ENV{OG_REPO_PATH}), "sources", "include"),
    join("/", OgPathToList($ENV{OG_REPO_PATH}), "sources", "include", "glib-2.0")
  );

  # add specific platforms sources
  my $strOs = OgOsString($OS);
  @fileList = $doc->xql("makefile/platforms/platform[\@name='$strOs']/sources/file");
  foreach my $src (@fileList) {
    push(@sourceList, OgXmlGetNodeText($src));
  }

  my $ext = "";
  if ($projectType eq "lib") {
    if ($OS == ogutil::OS_WINDOWS) {
      $ext = ".dll";
    } elsif ($OS == ogutil::OS_LINUX) {
      $ext = ".so";
    }
  } elsif ($projectType eq "prog") {
    if ($OS == ogutil::OS_WINDOWS) {
      $ext = ".exe";
    } elsif ($OS == ogutil::OS_LINUX) {
      $ext = "";
    }
  }

  my @out = $doc->xql("makefile/platforms/platform[\@name='$strOs']/option[\@name='NAME']");
  if (scalar(@out) != 0) {
    my $outputFile = OgXmlGetNodeText($out[0]);
    $outputDebug = join("/", OgPathToList($ENV{OG_REPO_PATH}), "build", "debug", "bin", $outputFile . $ext);
    $outputRelease = join("/", OgPathToList($ENV{OG_REPO_PATH}), "build", "release", "bin", $outputFile . $ext);  #FIXME: windows support
  }

  $doc->dispose;

  return(0);
}


sub buildIncludeString () {
  my $string = "";

  foreach my $inc (@includeList) {
    $string .= "            <entry kind=\"includePath\" name=\"$inc\"/>\n";
  }

  return($string);
}

