<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">
  <html>
    <head>
	 <title><xsl:value-of select="report/info/title" /></title>	
           <meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1" />
	  <link rel="stylesheet" media="screen" type="text/css" title="Design" href="pertimmReport.css" />
    </head>
    <body>
	<div id="en_tete">
               <h1><center><xsl:value-of select="report/info/title" /></center></h1>
	</div>
	<div id="cartouche">
	    <h2>hostname : <xsl:value-of select="report/info/hostname" /><br/>date : <xsl:value-of select="report/info/date" /></h2>
	</div>

          <xsl:for-each select="//component">
	  <div id="corps">
	    <h2>Component: <xsl:value-of select="@name"/> Build: <xsl:value-of select="@status"/> </h2>
	  <!--   <p><b>nb  Warnings:<xsl:value-of select ="@nbWarn"/> nb Errors: <xsl:value-of select="@nbErr" /></b>/<p> -->
	     <p><b>Warnings:<xsl:value-of select ="@nbWarn"/></b> </p> 
	     <p><b>nb Errors: <xsl:value-of select="@nbErr" /></b></p>
	     <xsl:for-each select="warning">
		<p><em>Warning: <xsl:value-of select="."/></em></p>
   	     </xsl:for-each>
 	     <xsl:for-each select="error">
		<p><em>Error: <xsl:value-of select="."/></em></p>
   	     </xsl:for-each>
	    </div>
	 </xsl:for-each>	
	 <div id="pied_de_page">
	</div>
    </body>
  </html>
  </xsl:template>
</xsl:stylesheet>
