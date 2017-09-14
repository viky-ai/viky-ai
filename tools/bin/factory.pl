#!/usr/bin/perl -w
#
# Build Script
#

use strict;

# Check env variables - can't do anything without them
BEGIN {
  # Factory relies on OG_TOOL_PATH and OG_REPO_PATH.
  if ( !defined($ENV{OG_TOOL_PATH})) {
    print("Erreur: La variable d'environnement OG_TOOL_PATH n'existe pas.\n");
    exit(1);
  }
  if ( !defined($ENV{OG_REPO_PATH})) {
    print("Erreur: La variable d'environnement OG_REPO_PATH n'existe pas.\n");
    exit(1);
  }
}

use XML::DOM;
use XML::XQL;
use XML::XQL::DOM;
use Getopt::Std;
use Path::Class;
use Time::Local;
use lib "$ENV{OG_TOOL_PATH}/bin";
use ogutil;
#use Data::Dumper;

sub usage ();
sub main ();

# FACTORY NEW WRAPPER FUNCTIONS
# TODO: temporary wrappers to clean up main()
# These should probably absorb the functions they're calling
sub mainMakeComponents ($$$);
sub mainMakeProduct ($$$);
sub mainMakeInstances ($$);
sub mainMakeResources ($$);
sub mainShipProduct ($$$$);
sub mainPackageProduct ($$$$);

# FACTORY FUNCTIONS
sub initFactory ();
sub parseComponentsManifest ($);
sub parseResourcesManifest ($);
sub makeProduct ($$$$$$$$);
sub makeComponents ($$$$@);
sub makeResources ($$$$);
sub makeTarget ($$$$$$$$);
sub makeInstancePack ($$);
sub parseShipTree ($$$$$$);
sub runTest ($$$$$$);
sub genPackage ($$$);
sub packageLinux ($$$$$$$$$$$$);
sub genRpmChangelog($);

# HELPER FUNCTIONS
sub initShipPath ($$);
sub openProductManifest ($);
sub getProductInfo ($);
sub getProductVersion ($);
sub getElement ($$);
sub getUseElements ($$);
sub setGlobalVarTab ($$);
sub subsVar ($@);
sub cmpShipItem;
sub OgWriteBDComponentReport ($$$$);
sub hideMsgFilter ($$);
sub OgBeginBuildReport ($);
sub OgEndBuildReport ($$);
sub OgBuildCommercialName ($$);

# DEPRECATED
sub readMakefileInfo ($);
sub dumpComponents (@);
sub getResult ($);

my $OgToolPath;
my $OgRepositoryPath;
my $COROOT;
my ( $os, $arch );
my $makeCMD;
my $logPath;
my $reportFile;

my $COMPONENT_FLAG = 1;
my $INSTANCE_FLAG  = 2;
my $RESOURCE_FLAG  = 4;
my $SHIP_FLAG      = 8;

my $parser = new XML::DOM::Parser;

my $DEFAULT_SHIP_PATH = "$ENV{OG_REPO_PATH}/ship";

my $companyName     = "Pertimm Inc";
my $legalCopyright  = "Copyright © 2006-2014 Pertimm, Inc.";
my $legalTrademarks = "Pertimm (TM)";
my $lineNumber      = 0;
my $nbHideWarn      = 0;
my @hideMsgTab;
my @highlightMsgTab;
my @componentTab;
my @resourceTab;
my @globalVarTab;
my $target = "";

my @DAYS = qw(Sun Mon Tue Wed Thu Fri Sat);
my @MONTHS = qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);

my $TRUE  = ( 1 == 1 );
my $FALSE = ( 0 == 1 );


##
# USAGE
#
sub usage ()
{
  my $prog_name = $0;

  $prog_name =~ s/.*\/(.*)$/$1/;

  print( "Usage " . $prog_name . " <action> <option> <product name|component name> <target>\n\n" );
  print("<action>:\n");
  print("\t-c\n\t make only a component\n\n");
  print("\t-p\n\t make a product (and all required components)\n\n");
  print("\t-a\n\t make all instances of a product\n\n");
  print("\t-r\n\t make all resources (dico, ...) of a product\n\n");
  print("\t-s\n\t ship a product (make product ; ship)\n\n");
  print("\t-k\n\t pacKage a product (ship ; package)\n\n");
  print("\t-h\n\t help\n\n");
  print("<option>:\n");
  print("\t-l\n\t list elements (products or components)\n\n");
  print("\t-t <path>\n\t set target path for building ship (default: \$OG_REPO_PATH/ship)\n\n");
  print("\t-o\n\t overwrite ship on target path\n\n");
  print("\t-u\n\t run unit test\n\n");
  print("<target>:\n");
  print("\tbuild\n\t build the product/component\n\n");
  print("\trebuild\n\t force re-build of a product/component (except external lib)\n\n");
  print("\trelease\n\t make a release of a product/component (re-build ; test)\n\n");
  print("\tdebug\n\t make a debuggable version of a product/component (clean; debug)\n\n");
  print("\tprofile\n\t make a profiling version of a product/component (clean; profile)\n\n");
  print("\ttest\n\t run automatic tests of a product/component\n\n");
  print("\tclean\n\t clean product/component (except external lib)\n\n");
  print("\tfullclean\n\t clean all product/component\n\n");
}


##
# MAIN
#
my $ret = main();

exit($ret);


sub main ()
{
  my %options = ();
  my $i;
  my ( $nbErr, $nbWarn, $nbMakeErr ) = ( 0, 0, 0 );
  my $filesSection;

  # Parse Args
  if ( $#ARGV < 0 ) {
    usage();
    exit(1);
  }

  initFactory();
  getopts( "uarocpsklht:", \%options );

  # check target
  $target = $ARGV[$#ARGV];

  if ( defined $options{h} ) {
    usage();
    exit(0);
  }

  if ( !defined $target ) {
    print("Error, target required\n");
    exit(1);
  }

  if ( $target !~ "build"
    && $target !~ "rebuild"
    && $target !~ "release"
    && $target !~ "debug"
    && $target !~ "profile"
    && $target !~ "test"
    && $target !~ "clean"
    && $target !~ "fullclean" )
  {
    print("Error, invalid target ($target)\n");
    usage();
    exit(1);
  }

  if ( !defined $options{a}
    && !defined $options{r}
    && !defined $options{c}
    && !defined $options{p}
    && !defined $options{s}
    && !defined $options{k} )
  {
    print("Error, you must enter an action\n");
    usage();
    exit(1);
  }

  # TODO: Code below needs to be factored even more
  # Optional arguments should be set directly through getopt shortcuts
  if ( defined $options{c} ) {    # make component(s)
    my $doTest = ( defined $options{u} ) ? $TRUE : $FALSE;
    my @components = @ARGV[ 0 .. $#ARGV-1 ];

    ( $nbErr, $nbWarn, $nbMakeErr ) = mainMakeComponents(\@components, $target, $doTest);
  }
  elsif ( defined $options{p} ) {    # make product
    die("Error, can't make multiple products at same time\n") if ( $#ARGV > 1 );
    my $doTest = ( defined $options{u} ) ? $TRUE : $FALSE;
    my $product = $ARGV[0];

    ( $nbErr, $nbWarn, $nbMakeErr ) = mainMakeProduct($product, $target, $doTest);
  }
  elsif ( defined $options{a} ) {    # make product instances
    die("Error, can't make instances for multiple products at same time\n") if ( $#ARGV > 1 );
    my $product = $ARGV[0];

    ( $nbErr, $nbWarn, $nbMakeErr ) = mainMakeInstances($product, $target);
  }
  elsif ( defined $options{r} ) {    ## make resources
    die("Error, can't make ressources for multiple products at same time\n") if ( $#ARGV > 1 );
    my $product = $ARGV[0];

    ( $nbErr, $nbWarn, $nbMakeErr ) = mainMakeResources($product, $target);
  }
  elsif ( defined $options{s} ) {    # build ship
    die("Error, can't ship multiple products at same time\n") if ( $#ARGV > 1 );
    my $product = $ARGV[0];
    my $shipPath = (defined $options{t}) ? $options{t} : $DEFAULT_SHIP_PATH;
    my $cleanShip = (defined $options{o}) ? $FALSE : $TRUE;

    ( $nbErr, $nbWarn, $nbMakeErr ) = mainShipProduct($product, $target, $shipPath, $cleanShip);
  }
  elsif ( defined $options{k} ) {    # build package
    die("Error, can't package multiple products at same time\n") if ( $#ARGV > 1 );
    my $product = $ARGV[0];
    my $shipPath = (defined $options{t}) ? $options{t} : $DEFAULT_SHIP_PATH;
    my $cleanShip = (defined $options{o}) ? $FALSE : $TRUE;

    ( $nbErr, $nbWarn, $nbMakeErr ) = mainPackageProduct($product, $target, $shipPath, $cleanShip);
  }

  return ( $nbErr != 0 || $nbMakeErr != 0 );
}

##
# FACTORY NEW WRAPPER FUNCTIONS
#

sub mainMakeComponents ($$$)
{
  my ( $components, $target, $doTest ) = @_;
  my @itemsTab;
  my $itemStr = "";
  my ($nbErr, $nbWarn, $nbMakeErr);

  parseComponentsManifest( file( $OgRepositoryPath, "sources", "manifest.xml" ) );

  for my $component (@$components) {
    push( @itemsTab, { name => $component } );
    $itemStr .= " $component";
  }

  OgBeginBuildReport("Make component(s) $itemStr");

  ( $nbErr, $nbWarn, $nbMakeErr ) = makeComponents( "0,0,0", "", $target, $doTest, @itemsTab );

  OgEndBuildReport( $nbErr, $nbWarn );

  return($nbErr, $nbWarn, $nbMakeErr);
}

sub mainMakeProduct ($$$)
{
  my ( $product, $target, $doTest ) = @_;
  my ($nbErr, $nbWarn, $nbMakeErr);

  parseComponentsManifest( file( $OgRepositoryPath, "sources", "manifest.xml" ) );
  parseResourcesManifest( file( $OgRepositoryPath, "resources", "manifest.xml" ) );

  OgBeginBuildReport("Make product $product");

  my ( $commercialName, $version, undef, undef, undef, undef, undef, undef )
    = getProductInfo($product);

  setGlobalVarTab( "TARGET", $target );
  setGlobalVarTab( "COROOT", $COROOT ) if defined($COROOT);
  setGlobalVarTab( "OG_REPO_PATH", $OgRepositoryPath );
  setGlobalVarTab( "OG_TOOL_PATH", $OgToolPath );
  setGlobalVarTab( "COMMERCIAL_NAME", OgBuildCommercialName($commercialName, $target) );
  setGlobalVarTab( "DATE",         localtime() );
  setGlobalVarTab( "SVNREV",       OgGetSvnRevision($OgRepositoryPath) );

  ( $nbErr, $nbWarn, $nbMakeErr ) =
    makeProduct( $product, "", $version, $target, "", \{}, $doTest,
    $COMPONENT_FLAG | $INSTANCE_FLAG | $RESOURCE_FLAG );

  OgEndBuildReport( $nbErr, $nbWarn );

  return($nbErr, $nbWarn, $nbMakeErr);
}

sub mainMakeInstances ($$)
{
  my ( $product, $target ) = @_;
  my ($nbErr, $nbWarn, $nbMakeErr);

  setGlobalVarTab( "TARGET", $target );
  setGlobalVarTab( "COROOT", $COROOT ) if defined($COROOT);
  setGlobalVarTab( "OG_REPO_PATH", $OgRepositoryPath );
  setGlobalVarTab( "OG_TOOL_PATH", $OgToolPath );
  setGlobalVarTab( "DATE",         localtime() );
  setGlobalVarTab( "SVNREV",       OgGetSvnRevision($OgRepositoryPath) );

  OgBeginBuildReport("Make instances for $product");

  my ( $commercialName, $version, undef, undef, undef, undef, undef, undef )
    = getProductInfo($product);

  ( $nbErr, $nbWarn, $nbMakeErr ) =
    makeProduct( $product, "", $version, $target, "", \{}, $FALSE, $INSTANCE_FLAG );

  OgEndBuildReport( $nbErr, $nbWarn );

  return($nbErr, $nbWarn, $nbMakeErr);
}

sub mainMakeResources ($$)
{
  my ( $product, $target ) = @_;
  my ($nbErr, $nbWarn, $nbMakeErr);

  setGlobalVarTab( "TARGET", $target );
  setGlobalVarTab( "COROOT", $COROOT ) if defined($COROOT);
  setGlobalVarTab( "OG_REPO_PATH", $OgRepositoryPath );
  setGlobalVarTab( "OG_TOOL_PATH", $OgToolPath );
  setGlobalVarTab( "DATE",         localtime() );
  setGlobalVarTab( "SVNREV",       OgGetSvnRevision($OgRepositoryPath) );

  parseResourcesManifest( file( $OgRepositoryPath, "resources", "manifest.xml" ) );

  OgBeginBuildReport("Make resources for product $product");

  my ( $commercialName, $version, undef, undef, undef, undef, undef, undef )
    = getProductInfo($product);

  ( $nbErr, $nbWarn, $nbMakeErr ) =
    makeProduct( $product, "", $version, $target, "", \{}, $FALSE, $RESOURCE_FLAG );
  OgEndBuildReport( $nbErr, $nbWarn );

  return($nbErr, $nbWarn, $nbMakeErr);
}

sub mainShipProduct ($$$$)
{
  my ($product, $target, $shipPath, $cleanShip) = @_;
  my ($nbErr, $nbWarn, $nbMakeErr);

  initShipPath($shipPath, $cleanShip);

  # (build || rebuild) => release
  if ( $target eq "build" || $target eq "rebuild" ) { $target = "release"; }

  OgBeginBuildReport("Build $target ship for $product");

  my ( $commercialName, $version, undef, undef, undef, undef, undef, undef )
    = getProductInfo($product);

  setGlobalVarTab( "VERSION",         $version );
  setGlobalVarTab( "COMMERCIAL_NAME", OgBuildCommercialName($commercialName, $target));
  setGlobalVarTab( "TARGET",          $target );
  setGlobalVarTab( "COROOT",          $COROOT ) if defined($COROOT);
  setGlobalVarTab( "OG_REPO_PATH",    $OgRepositoryPath );
  setGlobalVarTab( "OG_TOOL_PATH",    $OgToolPath );
  setGlobalVarTab( "SHIPDIR",         $shipPath );
  setGlobalVarTab( "DATE",            localtime() );
  setGlobalVarTab( "SVNREV",          OgGetSvnRevision($OgRepositoryPath) );
  setGlobalVarTab( "ARCH",            OgArchString($arch) );
  if   ( $os == ogutil::OS_WINDOWS ) { setGlobalVarTab( "PLATFORM", "Windows" ); }
  else                               { setGlobalVarTab( "PLATFORM", "Linux" ); }

  ( $nbErr, $nbWarn, $nbMakeErr ) =
    makeProduct( $product, "", $version, $target, $shipPath, [], $FALSE,
    $SHIP_FLAG );

  print("\e[0;32m => Shipping $product ($shipPath) OK\e[0m \n");
  OgEndBuildReport( 0, 0 );

  return($nbErr, $nbWarn, $nbMakeErr );
}

sub mainPackageProduct ($$$$)
{
  my ($product, $target, $shipPath, $cleanShip) = @_;

  initShipPath($shipPath, $cleanShip);

  # (build || rebuild) => release
  if ( $target eq "build" || $target eq "rebuild" ) { $target = "release"; }

  OgBeginBuildReport("Build $target package for $product");

  genPackage( $product, $target, $shipPath );

  print("\e[0;32m => Package $product ($shipPath) OK\e[0m \n");
  OgEndBuildReport( 0, 0 );

  return(0, 0, 0);
}


##
# FACTORY FUNCTIONS
#

#
# Init
#
sub initFactory ()
{
  my $res;

  # Get and format (dos path to unix path) environment variables
  ( $OgToolPath = "$ENV{OG_TOOL_PATH}" ) =~ s/\\/\//g;
  ( $OgRepositoryPath = "$ENV{OG_REPO_PATH}" ) =~ s/\\/\//g;
  if (defined($ENV{COROOT})) {
    ( $COROOT = "$ENV{COROOT}" ) =~ s/\\/\//g;
  }

  $DEFAULT_SHIP_PATH =~ s/\\/\//g;

  # Check OS
  ( $os, $arch ) = OgCheckPlatform();
  if ( $os == ogutil::OS_WINDOWS ) {
    print("Windows System\n");
    if ( $arch == ogutil::ARCH_X86_32 ) {
      $makeCMD = "nmake -f makefile.win32 /nologo";
    }
    elsif ( $arch == ogutil::ARCH_X86_64 ) {
      $makeCMD = "nmake -f makefile.win64 /nologo";
    }
  }
  elsif ( $os == ogutil::OS_LINUX ) {
    print("Linux System\n");
    if ( $arch == ogutil::ARCH_X86_32 ) {
      $makeCMD = "make -f makefile.linux32";
    }
    elsif ( $arch == ogutil::ARCH_X86_64 ) {
      $makeCMD = "make -f makefile.linux64";
    }
  }
  else {
    die("Unknown system\n");
  }

  $logPath = "$OgRepositoryPath/build/log";

  # Make log dir
  if ( !-d $logPath ) {
    print("$logPath does not exists, create it\n");
    $res = `mkdir -p $logPath`;
    $res = `cp $OgToolPath/release/buildReport2html.xsl $logPath`;
  }
}


## @fn value parseComponentsManifest($filePath)
# Open component manifest file and parse it
#
# \param configPath manifest file path
#
sub parseComponentsManifest ($)
{
  my ($filePath) = @_;

  my $componentsManifestXmlDoc;
  my $size;
  my $msgItem;

  # Read config file
  die("Error: File $filePath doesn't exist") unless ( -f $filePath );

  # dtd check config
  OgCheckDTD( file( $OgRepositoryPath, "products" ), $filePath ) or die "invalid $filePath";

  # parse config
  $componentsManifestXmlDoc = $parser->parsefile($filePath);

  # get all Components
  my @componentNodeList = $componentsManifestXmlDoc->xql("components_manifest/component");
  $size = @componentNodeList;
  if ( $size == 0 ) {
    die("Error, empty component list in file $filePath\n");
  }
  my $componentItem;
  foreach $componentItem (@componentNodeList) {
    push(
      @componentTab,
      { name     => $componentItem->getAttribute("name"),
        info     => $componentItem->getAttribute("info"),
        doc      => $componentItem->getAttribute("doc"),
        flags    => $componentItem->getAttribute("flags"),
        parallel => $componentItem->getAttribute("parallel")
      }
    );
  }

  # extract factory config
  my @highlightMsgList =
    $componentsManifestXmlDoc->xql("components_manifest/factory_config/highlight_msg/msg");
  foreach $msgItem (@highlightMsgList) {

    # printf("msg: type=" . $msgItem->getAttribute("type") . " color="
    #   . $msgItem->getAttribute("color") . " [" . OgXmlGetNodeText($msgItem) . "]\n"
    # );
    push(
      @highlightMsgTab,
      { type  => $msgItem->getAttribute("type"),
        color => $msgItem->getAttribute("color"),
        msg   => OgXmlGetNodeText($msgItem)
      }
    );
  }

  my @hideMsgList =
    $componentsManifestXmlDoc->xql("components_manifest/factory_config/hide_msg/msg");
  foreach $msgItem (@hideMsgList) {

    # printf("msg: type=" . $msgItem->getAttribute("type") . " color="
    #   . $msgItem->getAttribute("color") . " [" . OgXmlGetNodeText($msgItem) . "]\n"
    # );
    push(
      @hideMsgTab,
      { type  => $msgItem->getAttribute("type"),
        color => $msgItem->getAttribute("color"),
        msg   => OgXmlGetNodeText($msgItem)
      }
    );
  }

  # free memory
  $componentsManifestXmlDoc->dispose;

  return 0;
}

## @fn value parseComponentsManifest($filePath)
# Open resource manifest file and parse it
#
# \param configPath manifest file path
#
sub parseResourcesManifest ($)
{
  my ($filePath) = @_;

  my $resourcesManifestXmlDoc;
  my $size;
  my $msgItem;

  # Read config file
  die("Error: File $filePath doesn't exist") unless ( -f $filePath );

  # dtd check config
  OgCheckDTD( file( $OgRepositoryPath, "products" ), $filePath ) or die "invalid $filePath";

  # parse config
  $resourcesManifestXmlDoc = $parser->parsefile($filePath);

  # get all Resources
  my @resourceNodeList = $resourcesManifestXmlDoc->xql("resources_manifest/resource");
  $size = @resourceNodeList;
  if ( $size == 0 ) {
    print("Warning, empty resource list in file $filePath\n");
  }
  my $resourceItem;
  foreach $resourceItem (@resourceNodeList) {
    push(
      @resourceTab,
      { name     => $resourceItem->getAttribute("name"),
        info     => $resourceItem->getAttribute("info"),
        doc      => $resourceItem->getAttribute("doc"),
        flags    => $resourceItem->getAttribute("flags"),
        parallel => $resourceItem->getAttribute("parallel")
      }
    );
  }

  # free memory
  $resourcesManifestXmlDoc->dispose;

  return 0;
}

#
# buid a product (or some parts)
# recursively make all parent product
#
sub makeProduct ($$$$$$$$)
{
  my ( $productName, $father, $version, $target, $shipDir, $refSList, $doTest, $what ) = @_;
  my ( $nbErr, $nbWarn, $nbMakeErr ) = ( 0, 0, 0 );
  my $productManifestXmlDoc = openProductManifest($productName);
  my @productNode           = $productManifestXmlDoc->xql("product_manifest");
  my $parent                = $productNode[0]->getAttribute("parent");

  if ( length($parent) > 0 ) {
    my ( $nbErrC, $nbWarnC, $nbMakeErrC ) =
      makeProduct( $parent, $productName, $version, $target, $shipDir, $refSList, $doTest, $what );
    $nbErr     += $nbErrC;
    $nbWarn    += $nbWarnC;
    $nbMakeErr += $nbMakeErrC;
    if ( $nbErr > 0 || $nbMakeErr > 0 ) {    # error detected => stop making product
      print("Building parent Failed\n");
      $productManifestXmlDoc->dispose();
      return ( $nbErr, $nbWarn, $nbMakeErr );
    }
  }

  print("\e[0;36mMaking product $productName\e[0m \n\n");
  if ( $what & $COMPONENT_FLAG ) {           # build component
    print("build components\n");
    my @productUseTab = getUseElements( $productManifestXmlDoc, "component" );

    # check components dependencies
    for ( my $i = 0 ; $i < scalar(@productUseTab) ; $i++ ) {
      if ( getElement( $productUseTab[$i]{name}, \@componentTab ) < 0 ) {
        die(  "Error, unknow component dependency ("
            . $productUseTab[$i]{name}
            . ") for product $productName\n" );
      }
    }
    my ( $nbErrC, $nbWarnC, $nbMakeErrC ) =
      makeComponents( $version, $productName, $target, $doTest, @productUseTab );
    $nbErr     += $nbErrC;
    $nbWarn    += $nbWarnC;
    $nbMakeErr += $nbMakeErrC;
    if ( $nbErr > 0 || $nbMakeErr > 0 ) {
      print("Building components Failed\n");
      $productManifestXmlDoc->dispose();
      return ( $nbErr, $nbWarn, $nbMakeErr );
    }
  }

  if ( $what & $RESOURCE_FLAG ) {    # build resources
    print("build resource\n");
    my @resourceUseTab = getUseElements( $productManifestXmlDoc, "resource" );
    my ( $nbErrC, $nbWarnC, $nbMakeErrC ) =
      makeResources( $version, $productName, $target, \@resourceUseTab );
    $nbErr     += $nbErrC;
    $nbWarn    += $nbWarnC;
    $nbMakeErr += $nbMakeErrC;
    if ( $nbErr > 0 || $nbMakeErr > 0 ) {    # error detected => stop making product
      print("Building resources Failed for product $productName\n");
      $productManifestXmlDoc->dispose();
      return ( $nbErr, $nbWarn, $nbMakeErr );
    }
  }

  # build instance packages
  if ( $what & $INSTANCE_FLAG ) {            # build resources
    my @appList = $productManifestXmlDoc->xql("product_manifest/instances/instance");
    my $nbErrC  = 0;
    foreach my $appNode (@appList) {
      $nbErrC += makeInstancePack( $target, $appNode );
    }
    $nbErr += $nbErrC;
    if ( $nbErr > 0 ) {                      # error detected => stop making product
      print("Building instances Failed for product $productName\n");
      $productManifestXmlDoc->dispose();
      return ( $nbErr, $nbWarn, $nbMakeErr );
    }
  }

  # build ship
  if ( $what & $SHIP_FLAG ) {                # build ship
    print("ship $target $productName to $shipDir\n");

    # get all Components
    my $osStr = OgOsString($os);
    my @shipList =
      $productManifestXmlDoc->xql("product_manifest/packaging[\@platform='$osStr']/ship");
    if ( scalar(@shipList) == 0 ) {
      @shipList = $productManifestXmlDoc->xql("product_manifest/packaging/ship");
      scalar(@shipList) != 0 || die("Error, empty ship list in $productName manifest.xml file\n");
    }
    my $shipNode;
    foreach $shipNode (@shipList) {
      parseShipTree( $target, $shipList[0], 0, length($shipDir), $shipDir, $refSList );
    }

    chdir($shipDir);
  }

  $productManifestXmlDoc->dispose();

  return ( $nbErr, $nbWarn, $nbMakeErr );
}

##
# Build a list of components
##
sub makeComponents ($$$$@)
{    #TODO: *?
  my ( $productVersion, $productName, $target, $doTest, @components ) = @_;

  my $i;
  my $nbAllErr     = 0;
  my $nbAllWarn    = 0;
  my $nbAllMakeErr = 0;
  my $size         = @components;

  # Check components
  for ( $i = 0 ; $i < $size ; $i++ ) {
    if ( getElement( $components[$i]{name}, \@componentTab ) < 0 ) {
      die( "Error, no description for component " . $components[$i]{name} . "\n" );
    }
  }

  # Build each component
  for ( $i = 0 ; $i < $size ; $i++ ) {
    my $id = getElement( $components[$i]{name}, \@componentTab );
    my $targetLogName = $componentTab[$id]{name};
    $targetLogName =~ s/[\/\\]/_/g;    # accept name like ogm_gen/lib ...
    my $logFile = file( "$logPath", "log_" . $target . "_" . $targetLogName . ".txt" );

    print( "making component: $target " . $componentTab[$id]{name} . "\n" );

    my $use_makeopts_parallel = "";
    if($componentTab[$id]{parallel} ne "false" && defined($ENV{'MAKE_OPTS_PARALLEL'}))
    {
      $use_makeopts_parallel = $ENV{'MAKE_OPTS_PARALLEL'};
    }

    my ( $nbErr, $nbWarn, $makeRet ) = makeTarget(
      "sources", $target,
      $components[$i]{name},
      $componentTab[$id]{info},
      $productVersion, $productName, $logFile, $use_makeopts_parallel
    );
    $nbAllErr     += $nbErr;
    $nbAllWarn    += $nbWarn;
    $nbAllMakeErr += $makeRet;

    last if ( $makeRet != 0 || $nbAllErr > 0 );    # break

    if ($doTest) {
      runTest(
        "sources", $target,
        $components[$i]{name},
        $componentTab[$id]{info},
        $productVersion, $productName
      );
    }
  }

  return ( $nbAllErr, $nbAllWarn, $nbAllMakeErr );
}

#
# make a resource (dico)
#
sub makeResources ($$$$)
{
  my ( $productVersion, $productName, $target, $refTab ) = @_;
  my @resources = @$refTab;
  my $i;
  my $nbAllErr     = 0;
  my $nbAllWarn    = 0;
  my $nbAllMakeErr = 0;

  # Check resources
  for ( $i = 0 ; $i < scalar(@resources) ; $i++ ) {
    if ( getElement( $resources[$i]{name}, \@resourceTab ) < 0 ) {
      die( "Error, unknow resource " . $resources[$i]{name} . " for product $productName\n" );
    }
  }

  # Build each resource
  for ( $i = 0 ; $i < scalar(@resources) ; $i++ ) {
    my $id = getElement( $resources[$i]{name}, \@resourceTab );
    my $targetLogName = $resourceTab[$id]{name};
    $targetLogName =~ s/[\/\\]/_/g;    # accept name like ogm_gen/lib ...
    my $logFile = file( "$logPath", "log_" . $target . "_" . $targetLogName . ".txt" );

    my $use_makeopts_parallel = "";
    if($resourceTab[$id]{parallel} ne "false" && defined($ENV{'MAKE_OPTS_PARALLEL'}))
    {
      $use_makeopts_parallel = $ENV{'MAKE_OPTS_PARALLEL'};
    }

    print( "making resource: $target " . $resourceTab[$id]{name} . "\n" );
    my ( $nbErr, $nbWarn, $makeRet ) = makeTarget(
      "resources", $target,
      $resources[$i]{name},
      $resourceTab[$id]{info},
      $productVersion, $productName, $logFile, $use_makeopts_parallel
    );
    $nbAllErr     += $nbErr;
    $nbAllWarn    += $nbWarn;
    $nbAllMakeErr += $makeRet;

    last if ( $makeRet != 0 || $nbAllErr > 0 );    # break
  }

  return ( $nbAllErr, $nbAllWarn, $nbAllMakeErr );
}

##
# Call component's makefile.xxx with target and write report
# Type=sources|resources
##
sub makeTarget ($$$$$$$$)
{
  my ( $type, $target, $cName, $info, $productVersion, $productName, $logFile, $use_makeopts_parallel) = @_;
  my ( $makeRet, $makeMsg );

  #  my $componentName=$componentTab[$id]{name};
  print("\e[0;36mBuild $type $target for $cName\e[0m \n\n");

  # Create empty log file
  open( LOGFILE, ">", $logFile ) || die("Error, can't create file $!");
  close(LOGFILE);
  if ( !chdir( file( $OgRepositoryPath, $type, "$cName" ) ) ) {
    $makeRet = 1;
    $makeMsg = "Error, $! " . file( $OgRepositoryPath, $type, "$cName" ) . "\n";
  }
  else {
    if ( $os == ogutil::OS_WINDOWS ) {
      $makeMsg = `$makeCMD PRODUCT_NAME=$productName $target > \"$logFile\" 2>&1`;
    }
    else {
      $makeMsg = `$makeCMD $use_makeopts_parallel $target > \"$logFile\" 2>&1`;
    }
    $makeRet = $?;
  }

  return OgWriteBDComponentReport( $logFile, $cName, $makeRet, $makeMsg );
}

#
# make an instance package
#
sub makeInstancePack ($$)
{
  my ( $target, $node ) = @_;
  my @nilList;
  my $res;
  my $appName    = $node->getAttribute("name");
  my $currentdir = `pwd`;
  chomp($currentdir);

  if ( $target eq "build" || $target eq "rebuild" ) {
    $target = "release";
  }
  elsif ( $target eq "redebug" ) {
    $target = "debug";
  }
  elsif ( $target eq "clean" || $target eq "fullclean" ) {
    my $appPack = file( $OgRepositoryPath, "build", "release", "instances", $appName . ".ogz" );
    if ( -e $appPack ) {
      $res = `rm $appPack`;
    }
    print("\e[0;32m => clean $appPack\e[0m \n");

    $appPack = file( $OgRepositoryPath, "build", "debug", "instances", $appName . ".ogz" );
    if ( -e $appPack ) {
      $res = `rm $appPack`;
    }
    print("\e[0;32m => clean $appPack\e[0m \n");

    $appPack = file( $OgRepositoryPath, "build", "profile", "instances", $appName . ".ogz" );
    if ( -e $appPack ) {
      $res = `rm $appPack`;
    }
    print("\e[0;32m => clean $appPack\e[0m \n");

    return 0;
  }

  print("\e[0;36mGenerate Instance Pack $appName ($target)\e[0m \n\n");
  setGlobalVarTab( "INSTANCE_NAME", $appName );

  my $path = file( $OgRepositoryPath, "build", $target, "instances", "target" );
  if ( -d $path ) {
    $res = `rm -rf $path`;
  }
  $res = `mkdir -p $path`;

  my $appPack = file( $OgRepositoryPath, "build", $target, "instances", $appName . ".ogz" );
  if ( -e $appPack ) {
    $res = `rm $appPack`;
  }

  # build instance tree
  parseShipTree( $target, $node, 0, length($path), $path, \@nilList );

  # pack instance in .ogz
  chdir($path) || die("Invalid chdir $! ($path)\n");
  $res = `zip -X -r $appPack .`;
  chdir($currentdir) || die("Invalid chdir $! ($currentdir)\n");

  # clean target
  $res = `rm -rf $path`;
  print("\e[0;32m => gen $appPack OK\e[0m \n");

  delGlobalVarTab( "INSTANCE_NAME" );
  return 0;
}

sub parseShipTree ($$$$$$)
{
  my ( $shipTarget, $currentNode, $level, $shipPathLen, $path, $refSList ) = @_;
  my $i;

  my $filesSection = "";

  foreach my $node ( $currentNode->getChildNodes() ) {
    if ( $node->getNodeType() == ELEMENT_NODE ) {
      if ( $node->getTagName() eq "directory" ) {    # dir
        my $dirPath .= $path . "/" . $node->getAttribute("name");
        my $platform = $node->getAttribute("platform");

        if ( $platform eq "" || $platform eq "all" || $platform eq OgOsString($os) ) {
          $dirPath = subsVar( $dirPath, @globalVarTab );
          print("DIR $dirPath\n");                   # TODO:del
          my $res = `mkdir -p $dirPath`;
          if ( $? != 0 ) {
            die("[0;31m Build Ship Error: can't create dir $path \e[0m\n");
          }
          push( @$refSList, { type => "dir", target => $dirPath, level => $level } );
          $filesSection .= "\%dir " . substr( $dirPath, $shipPathLen ) . "\n";  # TODO:linux/windows
          $filesSection .=
            parseShipTree( $shipTarget, $node, $level + 1, $shipPathLen, $dirPath, $refSList );
        }
      }
      elsif ( $node->getTagName() eq "file"
        || $node->getTagName() eq "bin"
        || $node->getTagName() eq "lib" )
      {                                                                         # file
        my $target = subsVar( $path . "/" . $node->getAttribute("name"), @globalVarTab );
        my $source = subsVar( $node->getAttribute("source"),             @globalVarTab );
        my $strip  = subsVar( $node->getAttribute("strip"),              @globalVarTab );
        my $platform = $node->getAttribute("platform");

        if ( $node->getTagName() eq "bin" ) {
          if ( OgOsString($os) eq "win" ) {
            $target .= ".exe";
            $source .= ".exe";
          }
        }
        elsif ( $node->getTagName() eq "lib" ) {
          if ( OgOsString($os) eq "win" ) {
            $target .= ".dll";
            $source .= ".dll";
          }
          else {
            $target .= ".so";
            $source .= ".so";
          }
        }
        if ( $platform eq "" || $platform eq "all" || $platform eq OgOsString($os) ) {
          if ( -e $target ) {
            my $res = `diff $target $source`;
            if ( $? == 0 ) {    # same file, don't copy target
              print("\033[04;33mSKIP COPY $source $target\033[0m \n");
            }
            else {
              print("COPY $source $target\n");
              my $res = `cp -af $source $target`;
              if ( $? != 0 ) {
                die("\e[0;31m Building Ship Error: can't copy $source -> $target \e[0m\n");
              }
            }
          }
          else {
            print("COPY $source $target\n");
            my $res = `cp -af $source $target`;
            if ( $? != 0 ) {
              die("\e[0;31m Building Ship Error: can't copy $source -> $target \e[0m\n");
            }
          }
          if ( defined $strip && $strip eq $shipTarget ) {
            if ( $os == ogutil::OS_LINUX ) {
              print("STRIP $target\n");
              my $res = `strip $target`;
              if ( $? != 0 ) {
                die("\e[0;31m Building Ship Error: can't strip $target \e[0m\n");
              }
            }
          }
          push( @$refSList, { type => "file", target => $target, level => $level } );
          $filesSection .= substr( $target, $shipPathLen ) . "\n";    # TODO:linux/windows
        }
      }
      elsif ( $node->getTagName() eq "copy" ) {                       # file !!! no file section
        my $target   = $path;
        my $source   = subsVar( $node->getAttribute("source"), @globalVarTab );
        my $platform = $node->getAttribute("platform");

        if ( $platform eq "" || $platform eq "all" || $platform eq OgOsString($os) ) {
          print("COPY $source $target\n");
          my $res = `cp -afr $source $target`;
          if ( $? != 0 ) {
            die("\e[0;31m Building Ship Error: can't copy $source -> $target \e[0m\n");
          }
        }
      }
      elsif ( $node->getTagName() eq "lhp" ) {    # lhp
        my $lhpCmd;
        my $lhpName   = subsVar( $node->getAttribute("name"),   @globalVarTab );
        my $lhpSource = subsVar( $node->getAttribute("source"), @globalVarTab );
        my $lhpArgs   = subsVar( OgXmlGetNodeText($node),            @globalVarTab );
        my $platform  = $node->getAttribute("platform");

        if ( $platform eq "" || $platform eq "all" || $platform eq OgOsString($os) ) {
          print("LHP $lhpName $lhpSource $lhpArgs \n");

          my $toolpath = dir($OgToolPath, "bin");
          $lhpCmd = file($toolpath, "lhp.sh");
          my $currentdir = `pwd`;
          chdir($path);

          my $res = `$lhpCmd "$lhpArgs" $lhpName $lhpSource`;

          #  printf("[%s][$?]\n",$res);
          if ( $? != 0 ) {
            chdir $currentdir;
            die("\e[0;31m invalid $lhpCmd \e[0m\n");
          }
          chdir($currentdir);
          push( @$refSList, { type => "file", target => $path . "/" . $lhpName, level => $level } );
        }
      }
      elsif ( $node->getTagName() eq "link" ) {    # link
        my $lnTarget =
          subsVar( $node->getAttribute("target"), @globalVarTab );   # (!) relative links are better
        my $lnName = subsVar( $node->getAttribute("name"), @globalVarTab );
        my $platform = $node->getAttribute("platform");

        if ( $platform eq "" || $platform eq "all" || $platform eq OgOsString($os) ) {
          if ( $os == ogutil::OS_WINDOWS ) {
            print("Skip link $lnTarget $lnName. Only on Linux\n");
          }
          else {
            print("LN -s $lnTarget $lnName \n");
            my $currentdir = `pwd`;
            chdir $path;
            my $res = `ln -fs  $lnTarget $lnName`;
            if ( $? != 0 ) {
              chdir $currentdir;
              die(
                "\e[0;31m Build Ship Error: can't create link $lnTarget $lnName in $path \e[0m\n\n"
              );
            }
            chdir $currentdir;
            push( @$refSList,
              { type => "link", target => $path . "/" . $lnName, level => $level } );
          }
        }
      }
      elsif ( $node->getTagName() eq "rename" ) {    # rename action
        my $mvTarget = subsVar( $path . '/' . $node->getAttribute("target"), @globalVarTab );
        my $mvName   = subsVar( $path . '/' . $node->getAttribute("name"),   @globalVarTab );
        my $platform = $node->getAttribute("platform");

        if ( $platform eq "" || $platform eq "all" || $platform eq OgOsString($os) ) {
          my $res = `pwd`;
          print("RENAME $mvName $mvTarget\n");
          if ( -d $mvTarget ) {
            my $res = `cp -afr $mvName/* $mvTarget`;
            $res = `rm -rf $mvName`;
          }
          else {
            my $res = `mv -f $mvName $mvTarget`;
          }

          if ( $? != 0 ) {
            die("\e[0;31m Build Ship Error: can't rename $mvName $mvTarget in $path \e[0m\n");
          }
          my @tab = @$refSList;
          for ( my $i = 0 ; $i < scalar(@tab) ; $i++ ) {

            if ( $tab[$i]{target} =~ $mvName ) {
              my $tmp =
                $mvTarget . substr( $tab[$i]{target}, length($mvName), length( $tab[$i]{target} ) );
              $tab[$i]{target} = $tmp;    # subs name
            }
          }
        }
      }
      elsif ( $node->getTagName() eq "module" ) {
        my $source    = subsVar( $node->getAttribute("source"), @globalVarTab );
        my $platform  = $node->getAttribute("platform");
        my $osStr     = OgOsString($os);

        if ( $platform eq "" || $platform eq "all" || $platform eq $osStr ) {
          my $makefile = $parser->parsefile("$source/makefile.xml");

          my @tmpNode = $makefile->xql("makefile/platforms/platform[\@name='$osStr']/option[\@name='NAME']");
          my $mdName = OgXmlGetNodeText($tmpNode[0]);

          #build option
          @tmpNode = $makefile->xql("makefile/platforms/platform[\@name='$osStr']/option[\@name='ADD_CFLAGS']");
          my $mdCflags = OgXmlGetNodeText($tmpNode[0]);

          @tmpNode = $makefile->xql("makefile/platforms/platform[\@name='$osStr']/option[\@name='LINKER_NO_UNDEF']");
          my $mdLinkerNoUndef = OgXmlGetNodeText($tmpNode[0]);

          @tmpNode = $makefile->xql("makefile/platforms/platform[\@name='$osStr']/option[\@name='ADD_INC']");
          my $mdInc = OgXmlGetNodeText($tmpNode[0]);

          @tmpNode = $makefile->xql("makefile/platforms/platform[\@name='$osStr']/option[\@name='ADD_LIBR']");
          my $mdLibr = OgXmlGetNodeText($tmpNode[0]);

          my $mdLibd = "";
          @tmpNode = $makefile->xql("makefile/platforms/platform[\@name='$osStr']/option[\@name='ADD_LIBD']");
          if(!defined($tmpNode[0]) || OgXmlGetNodeText($tmpNode[0]) eq "" || OgXmlGetNodeText($tmpNode[0]) =~ m/\$\((ADD_LIB.)\)/)
          {
            $mdLibd = $mdLibr;
          }
          else
          {
           $mdLibd = OgXmlGetNodeText($tmpNode[0]);
          }

          my @sourceFiles = ();
          foreach my $src ($makefile->xql("makefile/sources/file")) {
            push(@sourceFiles, OgXmlGetNodeText($src));
          }
          my $mdSources = join(" ", @sourceFiles);
          my $mdObjects = "\\\$(TARGET)/".join(" \\\$(TARGET)/", @sourceFiles);
          $mdObjects =~ s/\.c/.obj/g;

          # nodes can only be created from the root node of type XML::DOM::Document
          my $root = $node->getParentNode();
          $root = $root->getParentNode() while defined($root->getParentNode());

          my $mdLhp = $root->createElement("lhp");
          $mdLhp->setAttribute("name", "makefile");
          $mdLhp->setAttribute("source", "$OgToolPath/makefile/$osStr/makefile_ssil.tpl");
          $mdLhp->setAttribute("platform", $platform);
          my $lhpVars = "MODULE='$mdName'; SOURCES_LIST='$mdSources'; OBJECTS_LIST='$mdObjects'; OPT_ADD_CFLAGS='$mdCflags'; OPT_ADD_INC='$mdInc'; OPT_ADD_LIBR='$mdLibr'; OPT_ADD_LIBD='$mdLibd'; OPT_LINKER_NO_UNDEF='$mdLinkerNoUndef';";
          $mdLhp->appendChild($root->createTextNode($lhpVars));
          $node->appendChild($mdLhp);

          my $mdFilesH = $root->createElement("copy");
          $mdFilesH->setAttribute("source", "$source/*.h");
          $node->appendChild($mdFilesH);

          my $mdFilesC = $root->createElement("copy");
          $mdFilesC->setAttribute("source", "$source/*.c");
          $node->appendChild($mdFilesC);

          # re-parse modified node
          $filesSection .=
            parseShipTree( $shipTarget, $node, $level, $shipPathLen, $path, $refSList );
        }
      } else {
        $filesSection .=
          parseShipTree( $shipTarget, $node, $level + 1, $shipPathLen, $path, $refSList );
      }
    }
  }

  return $filesSection;
}

##
# Call component's makefile.xxx with target and write report
# Type=sources|resources
##
sub runTest ($$$$$$)
{
  my ( $type, $target, $cName, $info, $productVersion, $productName ) = @_;
  my ( $makeRet, $makeMsg );

  if ( $target eq "clean" ) {
    return 0;
  }
  elsif ( $target eq "build" || $target eq "rebuild" ) {
    $target = "release";
  }
  elsif ( $target eq "redebug" ) {
    $target = "debug";
  }

  my $testTarget = "runtest_$target";
  my $genTestCmd;
  my $runTestCmd;

  if ( $os == ogutil::OS_WINDOWS ) {
    $genTestCmd = "$makeCMD PRODUCT_NAME=$productName GENTEST 2>&1";
    $runTestCmd = "$makeCMD PRODUCT_NAME=$productName $testTarget 2>&1";
  }
  else {
    $genTestCmd = "$makeCMD GENTEST 2>&1";
    $runTestCmd = "$makeCMD $testTarget 2>&1";
  }

  print("\e[0;36mgenerate tests $target for $cName\e[0m \n\n");
  $makeRet = `$genTestCmd`;
  if ( $? != 0 ) {
    print("\e[0;31m => Gen test Failed \e[0m\n$makeRet\n");
  }
  else {
    print("\e[0;32m => Gen test $cName OK\e[0m \n");
    print("\e[0;36mrun tests $target for $cName\e[0m \n\n");
    $makeRet = `$runTestCmd`;
    print("$makeRet\n");
  }
  $makeRet = $?;
}

sub genPackage ($$$)
{
  my ( $productName, $target, $shipDir ) = @_;
  my $fileSection = "";

  my @shipItemsList;
  my ( $commercialName, $version, $summary, $license, $vendor, $url, $description,
    $default_location )
    = getProductInfo($productName);
  my $revision = OgGetSvnRevision($OgRepositoryPath);

  setGlobalVarTab( "VERSION",         $version );
  setGlobalVarTab( "COMMERCIAL_NAME", OgBuildCommercialName($commercialName, $target));
  setGlobalVarTab( "TARGET",          $target );
  setGlobalVarTab( "COROOT",          $COROOT ) if defined($COROOT);
  setGlobalVarTab( "OG_REPO_PATH",    $OgRepositoryPath );
  setGlobalVarTab( "OG_TOOL_PATH",    $OgToolPath );
  setGlobalVarTab( "SHIPDIR",         $shipDir . $default_location );
  setGlobalVarTab( "DATE",            localtime() );
  setGlobalVarTab( "SVNREV",          $revision );
  setGlobalVarTab( "ARCH",            OgArchString($arch) );
  if   ( $os == ogutil::OS_WINDOWS ) { setGlobalVarTab( "PLATFORM", "Windows" ); }
  else                               { setGlobalVarTab( "PLATFORM", "Linux" ); }

  my ( $nbErr, $nbWarn, $nbMakeErr ) =
    makeProduct( $productName, "", $version, $target, $shipDir . $default_location,
    \@shipItemsList, $FALSE, $SHIP_FLAG );

  # sort and elimnate doubloons
  my @list1 = sort cmpShipItem @shipItemsList;
  my @list2;
  my $cItem;
  for my $href (@list1) {
    if ( !exists( $cItem->{target} )
      || !( $cItem->{target} eq $href->{target} && $cItem->{type} eq $href->{type} ) )
    {
      push( @list2, $href );
      $cItem = $href;
    }
  }

  if ( $os == ogutil::OS_LINUX ) {
    packageLinux(
      $productName, $commercialName, $version,     $revision, $summary, $license,
      $vendor,      $url,            $description, $target,   $shipDir, \@list2
    );
  }
  else {

    # build file section from ship
    my $cLevel = 0;
    for my $href (@list2) {
      if ( $cLevel > $href->{level} ) {
        printf( "%d:", $cLevel );
        for ( my $i = 0 ; $i < $cLevel ; $i++ ) { printf("."); }
        printf("</dir>\n");
      }
      $cLevel = $href->{level};
      printf( "%d:", $href->{level} );
      for ( my $i = 0 ; $i < $href->{level} ; $i++ ) { printf("."); }

      #  printf("level: %d type: %s path: [%s]\n",$href->{level},$href->{type},$href->{target});
      if ( $href->{type} eq "dir" ) {
        printf( "<dir level=%d name=%s>\n",
          $href->{level}, substr( $href->{target}, length($shipDir) ) );
      }
      elsif ( $href->{type} eq "file" ) {
        printf( "<file level=%d name=%s\n",
          $href->{level}, substr( $href->{target}, length($shipDir) ) );
      }
    }
    for ( my $i = $cLevel ; $i >= 0 ; $i-- ) {
      printf( "%d:", $cLevel );
      for ( my $j = 0 ; $j < $i ; $j++ ) { printf("."); }
      printf("</dir>\n");
    }
    printf( "%s\n", $fileSection );
  }
}

# WARNING: This code has been customized for PJ
# (it is the only client using RPM straight from the factory, and there
# doesn't seem to be any plan to use it for others, as we have yet another
# way to build RPMs for other clients)
sub packageLinux ($$$$$$$$$$$$)
{
  my (
    $productName, $commercialName, $version,     $revision, $summary, $license,
    $vendor,      $url,            $description, $target,   $shipDir, $hIList
  ) = @_;

  my $workPackPath = file( $OgRepositoryPath, "products", "$productName" );
  my $packageName;
  my $changelog_warnings;
  my $res;

  my $productManifestXmlDoc = openProductManifest($productName);

  # init linux ship path
  $res = `mkdir -p $shipDir/BUILD $shipDir/RPMS $shipDir/SRPMS $shipDir/tmp`;
  if ( $? != 0 ) {
    die("Error $shipDir init\n");
  }

  # set rpm_macros file to change default rpm source dir (root is not necessary any more)
  open( RPM_MACROS_FILE, ">", "$workPackPath/rpm_macros" )
    or die("Cannot open $workPackPath/rpm_macros file");
  print( RPM_MACROS_FILE "\%\_topdir\t$shipDir\n" );
  print( RPM_MACROS_FILE "\%\_tmppath\t$shipDir/tmp\n" );
  close(RPM_MACROS_FILE);

  # set rpm_macros file to change default rpm source dir (root is not necessary any more)
  open( RPMRC_FILE, ">", "$workPackPath/rpmrc" ) or die("Cannot open $workPackPath/rpmrc file");
  print( RPMRC_FILE "buildarchtranslate: i386: i686\n" );
  print( RPMRC_FILE "buildarchtranslate: i486: i686\n" );
  print( RPMRC_FILE "buildarchtranslate: i586: i686\n" );
  print( RPMRC_FILE "buildarchtranslate: ia32e: x86_64\n" );
  print( RPMRC_FILE "buildarchtranslate: amd64: x86_64\n\n" );
  print( RPMRC_FILE
      "macrofiles: /usr/lib/rpm/macros:/etc/rpm/macros.specspo:/etc/rpm/macros.db1:/etc/rpm/macros.cdb:/etc/rpm/macros:$workPackPath/rpm_macros\n"
  );
  close(RPMRC_FILE);

  # FIXME: will only generate elX disttag for RedHat, even on CentOS (ok) or Fedora (ko)
  # The %{dist} variable set by rpmbuild is not available in CentOS or old RH, so since
  # we only care about PJ on RH, it's easier to do it by hand here
  my $disttag = `lsb_release -r -s`;
  if ( $? != 0 ) {
    die("Error: cannot get lsb release\n");
  }
  $disttag =~ s/^(\d+).*$/el$1/;

  # build product_rpm.spec
  # info part
  open( PRODUCT_RPM_FILE, ">", "$workPackPath/product_rpm.spec" )
    or die("Cannot open $workPackPath/product_rpm.spec");
  if ( $target eq "release" || $target eq "build" ) {
    print( PRODUCT_RPM_FILE "Name: $commercialName\n" );    # <productName>-1.0.0-11.i686
    $packageName = $commercialName . "-" . $version . "-" . $revision;
  }
  else {

    # <productName>-debug-1.0.0-11.i686
    print( PRODUCT_RPM_FILE "Name: $commercialName\-$target\n" );
    $packageName = $commercialName . "-" . $target . "-" . $version . "-" . $revision;
  }
  print( PRODUCT_RPM_FILE "Summary: $summary\n" );
  print( PRODUCT_RPM_FILE "Version: $version\n" );
  print( PRODUCT_RPM_FILE "Release: $revision.$disttag\n" );
  print( PRODUCT_RPM_FILE "Group: Applications/pertimm\n" );
  print( PRODUCT_RPM_FILE "License: $license\n" );
  print( PRODUCT_RPM_FILE "Vendor: $vendor\n" );
  print( PRODUCT_RPM_FILE "URL: $url\n" );
  print( PRODUCT_RPM_FILE "AutoReqProv: no\n" );

  my @packageNode =
    $productManifestXmlDoc->xql("product_manifest/packaging/package[\@platform='linux']");
  my $prefix = $packageNode[0]->getAttribute("default_location");
  print( PRODUCT_RPM_FILE "Prefix: $prefix\n" );    # ??
  print( PRODUCT_RPM_FILE "\%description\n$description\n" );

  # sh script part
  setGlobalVarTab( "PACKAGE_NAME", $packageName );
  setGlobalVarTab( "PRODUCT_NAME", $commercialName );

  # TODO:win/linux
  my @scriptNodeList =
    $productManifestXmlDoc->xql("product_manifest/packaging/package[\@platform='linux']/var");

  foreach my $scriptNode (@scriptNodeList) {
    if ( $scriptNode->getAttribute("name") eq "pre_install" ) {
      print(PRODUCT_RPM_FILE "\%pre\n"
          . subsVar( OgXmlGetNodeText($scriptNode), @globalVarTab )
          . "\n" );
    }
    elsif ( $scriptNode->getAttribute("name") eq "post_install" ) {
      print(PRODUCT_RPM_FILE "\%post\n"
          . subsVar( OgXmlGetNodeText($scriptNode), @globalVarTab )
          . "\n" );
    }
    elsif ( $scriptNode->getAttribute("name") eq "pre_uninstall" ) {
      print(PRODUCT_RPM_FILE "\%preun\n"
          . subsVar( OgXmlGetNodeText($scriptNode), @globalVarTab )
          . "\n" );
    }
    elsif ( $scriptNode->getAttribute("name") eq "post_uninstall" ) {
      print(PRODUCT_RPM_FILE "\%postun\n"
          . subsVar( OgXmlGetNodeText($scriptNode), @globalVarTab )
          . "\n" );
    }
    elsif ( $scriptNode->getAttribute("name") eq "changelog" ) {
      my $changelog;
      ($changelog, $changelog_warnings) =
        genRpmChangelog( subsVar( OgXmlGetNodeText($scriptNode), @globalVarTab ) );
      if ( defined($changelog) && $changelog ne "" ) {
        print(PRODUCT_RPM_FILE "\%changelog" . "\n" . "$changelog" . "\n" );
      }
    }
    else {
      die("Error, invalid 'when' for linux script\n");
    }
  }

  # ship part
  print( PRODUCT_RPM_FILE "\%files\n" );
  print( PRODUCT_RPM_FILE "\%defattr(-,root,root)\n" );
  my $fileSection = "";
  for my $href (@$hIList) {

    #  printf("level: %d type: %s path: [%s]\n",$href->{level},$href->{type},$href->{target});
    if ( $href->{type} eq "dir" ) {
      $fileSection .= "\%dir " . substr( $href->{target}, length($shipDir) ) . "\n";
    }
    elsif ( $href->{type} eq "file" ) {
      $fileSection .= substr( $href->{target}, length($shipDir) ) . "\n";
    }
    elsif ( $href->{type} eq "link" ) {
      $fileSection .= substr( $href->{target}, length($shipDir) ) . "\n";
    }
  }
  print( $fileSection. "\n" );

  for my $changelog_warning (@$changelog_warnings) {
    print("Warning, $changelog_warning\n");
  }

  print( PRODUCT_RPM_FILE "$fileSection\n" );
  close(PRODUCT_RPM_FILE);

  # build rpm
  $res =
    `rpmbuild -bb --rcfile $workPackPath/rpmrc --buildroot $shipDir $workPackPath/product_rpm.spec`;
  print("$?\n");

  # installation du package
  # rpm -i ship/RPMS/i686/philippePack-1.0.0-11.i686.rpm
  # desinstallation du package
  #rpm -e philippePack-1.0.0-11.i686
  # clean
  if ( -e "$workPackPath/rpm_macros" )       { unlink("$workPackPath/rpm_macros"); }
  if ( -e "$workPackPath/rpmrc" )            { unlink("$workPackPath/rpmrc"); }
  if ( -e "$workPackPath/product_rpm.spec" ) { unlink("$workPackPath/product_rpm.spec"); }
}


# If changelog entry is invalid, it is still displayed but with an added FIXME
# Invalid dates are ignored and replaced by the date from the previous valid entry
# (this is because rpm changelog requires changelog entries to be sorted by date)
sub genRpmChangelog ($)
{
my ($changelog_path) = @_;
my @warnings = ();
my $previous_date = undef;

if (! -f $changelog_path) {
  push(@warnings, "can't find changelog file at '$changelog_path'");
  return(undef, \@warnings);
}

my $i = 0;
my @changelog;
open(CHANGELOG_FILE, "<$changelog_path");
while (my $line = <CHANGELOG_FILE>) {
  chomp($line);

  if ($line =~ /\*\* DUMMY CHANGELOG \*\*/) {
    close(CHANGELOG_FILE);
    push(@warnings, "dummy changelog found, ignoring");
    return(undef, \@warnings);
  }
  elsif ($line =~ /^\*/) {
    $i += 1;
    my $date = undef;
    my $version = undef;
    my $fixme = 0;

    if ($line =~ /^\*\s+(\d+\.\d+\.\d+[a-z]?)/) {
      $version = $1;
      if ($line =~ /^\*\s+\d+\.\d+\.\d+[a-z]?\s+-\s+(\d{4})-(\d{1,2})-(\d{1,2})/) {
        my ($year, $month, $day) = ($1, $2, $3);
        my $time = timelocal(0, 0, 0, $day, $month - 1, $year);
        my $wday = (localtime($time))[6];
        my $sday = $DAYS[$wday];
        my $smonth = $MONTHS[$month - 1];
        $date = "$sday $smonth $day $year";
        $previous_date = $date;
      }
    }

    unless (defined($date)) {
      if (defined($previous_date)) {
        $date = $previous_date;
      } else {
        $date = `date +"%a %b %d %Y"`;
        chomp($date);
        $previous_date = $date;
      }
      $fixme = 1;
    }
    unless (defined($version)) {
      $version = "X.Y.Z";
      $fixme = 1;
    }

    $changelog[$i] = "* $date - $version" . (($fixme == 1) ? " - FIXME" : "") . "\n";

    if ($fixme == 1) {
      push(@warnings, "invalid changelog entry: $line");
    }
  }
  else {
    $changelog[$i] .= "$line\n";
  }
}
close(CHANGELOG_FILE);

my $changelog_string = join("\n", @changelog);

return($changelog_string, \@warnings);
}


##
# HELPER FUNCTIONS
#

sub initShipPath ($$)
{
  my ( $shipPath, $cleanShip ) = @_;

  if ( $cleanShip == $TRUE ) {
    print("clean ship $shipPath\n");
    system("rm -rf $shipPath ; mkdir -p $shipPath");
  }
  else {
    print("overwrite ship $shipPath\n");
  }
}

sub openProductManifest ($)
{
  my ($productName) = @_;
  my $filePath = file( $OgRepositoryPath, "products", "$productName", "manifest.xml" );

  # dtd check config
  OgCheckDTD( file( $OgRepositoryPath, "products" ), $filePath ) or die "invalid $filePath";

  return $parser->parsefile($filePath);    # parse config and return xml tree
}


# read info from product manifest
sub getProductInfo ($)
{
  my ($productName) = @_;

  my $productManifestXmlDoc = openProductManifest($productName);
  my $version               = getProductVersion($productName);
  if ( $version eq "" ) {
    die("\e[0;31mError, product version not found\n\e[0m");
  }
  my @infoNode       = $productManifestXmlDoc->xql("product_manifest");
  my $commercialName = $infoNode[0]->getAttribute("commercial_name");
  @infoNode = $productManifestXmlDoc->xql("product_manifest/info/summary");
  my $summary = OgXmlGetNodeText( $infoNode[0] );
  @infoNode = $productManifestXmlDoc->xql("product_manifest/info/license");
  my $license = OgXmlGetNodeText( $infoNode[0] );
  @infoNode = $productManifestXmlDoc->xql("product_manifest/info/vendor");
  my $vendor = OgXmlGetNodeText( $infoNode[0] );
  @infoNode = $productManifestXmlDoc->xql("product_manifest/info/url");
  my $url = OgXmlGetNodeText( $infoNode[0] );
  @infoNode = $productManifestXmlDoc->xql("product_manifest/info/description");
  my $description     = OgXmlGetNodeText( $infoNode[0] );
  my $defaultLocation = "";
  my @packageNode =
    $productManifestXmlDoc->xql("product_manifest/packaging/package[\@platform='linux']");

  if ( scalar(@packageNode) != 0 ) {
    $defaultLocation = $packageNode[0]->getAttribute("default_location");
  }
  $productManifestXmlDoc->dispose();

  return ( $commercialName, $version, $summary, $license, $vendor, $url, $description,
    $defaultLocation );
}

sub getProductVersion ($)
{
  my ($productName) = @_;

  my $version               = "";
  my $productManifestXmlDoc = openProductManifest($productName);
  my @infoNode              = $productManifestXmlDoc->xql("product_manifest/info/version");

  if ( scalar(@infoNode) == 0 ) {
    my @productNode = $productManifestXmlDoc->xql("product_manifest");
    my $parent      = $productNode[0]->getAttribute("parent");
    if ( length($parent) > 0 ) {
      $version = getProductVersion($parent);
    }
  }
  else {
    $version =
        $infoNode[0]->getAttribute("major") . "."
      . $infoNode[0]->getAttribute("minor") . "."
      . $infoNode[0]->getAttribute("fix");
  }
  $productManifestXmlDoc->dispose();

  return $version;
}

#
# Search an element in Tab
#
sub getElement ($$)
{
  my ( $name, $rtab ) = @_;
  my @tab = @$rtab;
  my $i;

  for ( $i = 0 ; $i < scalar(@tab) ; $i++ ) {
    if ( $tab[$i]{name} =~ $name ) {
      return $i;
    }
  }

  return -1;
}

## @fn useTab getUseElement($productManifestDoc,"component|resource")
# Extract list of used elements from productManifestXmlDoc
# \param productManifestXmlDoc manifest file path
# \param tag component|resource
#
sub getUseElements ($$)
{
  my ( $productManifestXmlDoc, $tag ) = @_;
  my @useTab;

  # get all Components
  my @useNodeList = $productManifestXmlDoc->xql("product_manifest/use/$tag");
  if ( scalar(@useNodeList) > 0 ) {
    my $useItem;
    foreach $useItem (@useNodeList) {
      push( @useTab, { name => $useItem->getAttribute("name") } );
    }
  }

  return @useTab;
}

sub setGlobalVarTab ($$)
{
  my ( $name, $value ) = @_;
  if ( $name eq "TARGET" ) {
    if ( $value eq "build" || $value eq "rebuild" ) { $value = "release"; }
    elsif ( $value eq "redebug" )   { $value = "debug"; }
    elsif ( $value eq "reprofile" ) { $value = "profile"; }
  }
  push( @globalVarTab, { name => "\%$name\%", value => $value } );
}

sub delGlobalVarTab ($)
{
  my ( $name ) = @_;
  @globalVarTab = grep { $_->{name} ne "\%$name\%" } @globalVarTab;
}

sub subsVar ($@)
{
  my ( $str, @varTab ) = @_;
  my $i;
  my $size = @varTab;

  for ( $i = 0 ; $i < $size ; $i++ ) {
    $str =~ s/$varTab[$i]{name}/$varTab[$i]{value}/g;
  }

  return $str;
}

sub OgBuildCommercialName ($$)
{
  my ( $name, $target ) = @_;

  if ( $target eq "redebug" ) { $target = "debug"; }
  if ( $target eq "debug" )   { $name   = "$name-debug"; }

  return $name;
}

#
# compare target from ship list to sort them
#
sub cmpShipItem
{
  my $res;

  return $res = $a->{target} cmp $b->{target};
}

##
# Log build component result
##
sub OgWriteBDComponentReport ($$$$)
{
  my ( $logFile, $component, $makeRet, $makeMsg ) = @_;
  my $buffer = "";
  my ( $nbWarn, $nbErr ) = ( 0, 0 );

  open( LOG_FILE, "<", $logFile ) or die "Cannot open $logFile";
  my @compilLog = <LOG_FILE>;
  close(LOG_FILE);

  if ( $makeRet != 0 ) {
    $buffer = $makeMsg . "\n";
    print("$makeMsg");
  }

  foreach my $line (@compilLog) {
    $lineNumber++;
    if ( $makeRet != 0 ) {    # print log if makefile return an arror
      print("$lineNumber: $line");
    }
    else {

      # hide message
      if ( hideMsgFilter( $line, $buffer ) == 1 ) {    # continue;
        if ( $line =~ /: warning/ ) {
          $buffer = $buffer . "\t\t<warning><![CDATA[$lineNumber: $line]]></warning>\n";
          $nbWarn++;
          print("$lineNumber: $line");
        }
        if ( $line =~ /: error/
          || $line =~ /^LINK : fatal error/
          || $line =~ /^NMAKE : fatal erro/ )
        {
          $nbErr++;
          $buffer = $buffer . "\t\t<error><![CDATA[$lineNumber: $line]]></error>\n";
          print("\e[0;31m$lineNumber: $line\e[0m");
        }
      }
    }
  }

  open( REPORT_FILE, ">>", $reportFile )
    || throw Error::FatalException( -text => "can't open report $reportFile" );
  if ( $makeRet != 0 || $nbErr > 0 ) {
    print( REPORT_FILE
        "\t<component name='$component' status='FAIL' nbWarn='$nbWarn' nbErr='$nbErr' logFile='file://$logFile'>\n"
    );
    print("\e[0;31m => Build $component ERROR\e[0m \n");
  }
  else {
    print( REPORT_FILE
        "\t<component name='$component' status='SUCCESS' nbWarn='$nbWarn' nbErr='$nbErr'>\n" );
    print("\e[0;32m => Build $component OK\e[0m \n");
  }
  print( REPORT_FILE $buffer );
  print( REPORT_FILE "\t</component>\n" );
  close(REPORT_FILE);

  return ( $nbErr, $nbWarn, $makeRet );
}

sub hideMsgFilter ($$)
{
  my ( $msg, $buffer ) = @_;
  my $hidePatternElement;
  my $hideStr;
  my $i;
  my $size = @hideMsgTab;

  for ( $i = 0 ; $i < $size ; $i++ ) {
    if ( $msg =~ /$hideMsgTab[$i]{msg}/ ) {
      $buffer = $buffer . "\t\t<warning_hide><![CDATA[$lineNumber: $msg]]></warning_hide>\n";
      $nbHideWarn++;
      return 0;
    }
  }

  return 1;
}

sub OgBeginBuildReport ($)
{
  my ($title) = @_;

  $reportFile = "$logPath/BuildReport.xml";
  my $hostname = `hostname`;
  chomp($hostname);
  my $date = `date`;
  chomp($date);
  open( REPORT_FILE, ">", $reportFile )
    || throw Error::FatalException( -text => "can't create report $reportFile" );
  printf( REPORT_FILE "<?xml version='1.0' encoding='ISO-8859-1'?>\n" );
  printf( REPORT_FILE "<?xml-stylesheet type='text/xsl' href='buildReport2html.xsl'?>\n" );
  printf( REPORT_FILE "<report>\n" );
  printf( REPORT_FILE "\t<info>\n" );
  printf( REPORT_FILE "\t\t<title><![CDATA[$title]]></title>\n" );
  printf( REPORT_FILE "\t\t<date>$date</date>\n" );
  printf( REPORT_FILE "\t\t<hostname><![CDATA[$hostname]]></hostname>\n" );
  printf( REPORT_FILE "\t</info>\n" );
  close(REPORT_FILE);
}

sub OgEndBuildReport ($$)
{
  my ( $nbErr, $nbWarn ) = @_;

  open( REPORT_FILE, ">>", $reportFile )
    || throw Error::FatalException( -text => "can't open report $reportFile" );
  printf( REPORT_FILE "\t<summary nbErr='$nbErr' nbWarn='$nbWarn' />\n" );
  printf( REPORT_FILE "</report>\n" );
  close(REPORT_FILE);
}


##
# DEPRECATED
#

#
# read makefile.xml info
#
sub readMakefileInfo ($)
{
  my ($makefileXml) = @_;
  my $project_name  = "";
  my $buildedName   = "";
  my $revision      = "";
  my $version       = "";

  # Read makefile.xml file
  if ( !-f $makefileXml ) {
    die("Error: File $makefileXml doesn't exist");
  }

  # dtd check config
  OgCheckDTD( file( $OgToolPath, "makefile" ), $makefileXml ) or die "invalid $makefileXml";

  # parse config
  my $makefileXmlDoc = $parser->parsefile($makefileXml);
  my $type           = OgXmlGetNodeText( $makefileXmlDoc->xql("makefile/type") );

  if ( $type ne "extern" ) {
    $project_name = OgXmlGetNodeText( $makefileXmlDoc->xql("makefile/project_name") );
    my @buildedNameNode =
      $makefileXmlDoc->xql("makefile/platforms/platform[\@name='win']/option[\@name='NAME']");
    $buildedName = OgXmlGetNodeText( $buildedNameNode[0] );

    my @revisionNode = $makefileXmlDoc->xql("makefile/revision");
    if ( !defined $revisionNode[0] ) {
      $revision = "0";
    }
    else {
      $revision = OgXmlGetNodeText( $revisionNode[0] );
      $revision =~ /Revision: (\w*)/;
      $revision = $1;
    }

    my @versionNode = $makefileXmlDoc->xql("makefile/version");
    if ( !defined $versionNode[0] ) {
      $version = "-1";
    }
    else {
      $version = OgXmlGetNodeText( $versionNode[0] );
    }
  }

  $makefileXmlDoc->dispose;

  return ( $type, $project_name, $buildedName, $revision, $version );
}

## @fn  dumpComponents($title,@cluster)
# Dump cluster
#
# \param title
# \param cluster
#
sub dumpComponents (@)
{
  my (@componentTab) = @_;
  my $server;
  my $i;
  my $size = @componentTab;

  for ( $i = 0 ; $i < $size ; $i++ ) {
    print( "component   : " . $componentTab[$i]{name} . "\n" );
    print( "\t info     : " . $componentTab[$i]{info} . "\n" );
    print( "\t doc      : " . $componentTab[$i]{doc} . "\n" );
    print( "\t flags    : " . $componentTab[$i]{flags} . "\n" );
    print( "\t parallel : " . $componentTab[$i]{parallel} . "\n" );
  }
}

sub getResult ($)
{
  my ($curpos) = @_;
  my $line;

  for ( ; ; ) {
    while ( defined( $line = <LOGFILE> ) ) {
      $curpos = tell(LOGFILE);
      if ( $line =~ /^RESULT:[\w\.]*/ ) {
        print("$curpos ===> $line");
        return ( $line, $curpos );
      }

      # else {
      #   print("$curpos: $line ");
      # }
    }
    seek( LOGFILE, $curpos, 0 );
  }
}

