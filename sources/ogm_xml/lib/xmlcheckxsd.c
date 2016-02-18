/*
 *  Checking an xml file with an xsd schema
 *  Copyright (c) 2012 Pertimm by Patrick Constant
 *  Dev : May 2012
 *  Version 1.0
 */
#include "ogm_xml.h"
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>

static void OgXmlCheckXsdError(void *hmsg, og_string xsd, og_string xml, og_string function_in_error)
{
  og_char_buffer erreur[DOgErrorSize];
  snprintf(erreur, DOgErrorSize, "OgXmlCheckXsd error on '%s' using xsd '%s' on '%s' function.", xml, xsd,
      function_in_error);
  OgErr(OgLogGetErr(hmsg), erreur);
}

static void OgXmlCheckXsdErrorHandler(void *hmsg, xmlErrorPtr xml_error)
{

  int line_number = -1;
  char *msg_error = "unknow error";
  if (xml_error)
  {
    line_number = xml_error->line;
    if (xml_error->message)
    {
      msg_error = xml_error->message;
    }
  }

  og_char_buffer erreur[DOgErrorSize];
  snprintf(erreur, DOgErrorSize, "OgXmlCheckXsd error on line %d: %s", line_number, msg_error);
  OgErr(OgLogGetErr(hmsg), erreur);

}

static void OgXmlCheckXsdDefaultErrorHandler(void *hmsg, const char *format, ...)
{
  // to avoid stderr and out message
}

PUBLIC(int) OgXmlCheckXsd(void *hmsg, og_string xsd, og_string xml)
{

  // disable stderr output
  xmlSetGenericErrorFunc(hmsg, OgXmlCheckXsdDefaultErrorHandler);
  xmlSetStructuredErrorFunc(hmsg, OgXmlCheckXsdErrorHandler);

  int is_valid = 0;

  xmlDocPtr schema_doc = xmlReadFile(xsd, NULL, XML_PARSE_NONET);
  if (schema_doc != NULL)
  {

    xmlSchemaParserCtxtPtr parser_ctxt = xmlSchemaNewDocParserCtxt(schema_doc);
    if (parser_ctxt != NULL)
    {

      xmlSchemaPtr schema = xmlSchemaParse(parser_ctxt);
      if (schema != NULL)
      {

        xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);
        if (valid_ctxt != NULL)
        {
          is_valid = (xmlSchemaValidateFile(valid_ctxt, xml, 0) == 0);
          if (!is_valid)
          {
            OgXmlCheckXsdError(hmsg, xsd, xml, "xmlSchemaValidateFile");
          }

          xmlSchemaFreeValidCtxt(valid_ctxt);
        }
        else
        {
          /* unable to create a validation context for the schema */
          /* the schema itself is not valid */
          OgXmlCheckXsdError(hmsg, xsd, xml, "xmlSchemaNewValidCtxt");
          is_valid = ERREUR;
        }

        xmlSchemaFree(schema);

      }
      else
      {
        /* the schema itself is not valid */
        OgXmlCheckXsdError(hmsg, xsd, xml, "xmlSchemaParse");
        is_valid = ERREUR;
      }

      xmlSchemaFreeParserCtxt(parser_ctxt);

    }
    else
    {
      /* unable to create a parser context for the schema */
      OgXmlCheckXsdError(hmsg, xsd, xml, "xmlSchemaNewDocParserCtxt");
      is_valid = ERREUR;
    }

    xmlFreeDoc(schema_doc);

  }
  else
  {
    /* the schema cannot be loaded or is not well-formed */
    OgXmlCheckXsdError(hmsg, xsd, xml, "xmlReadFile");
    is_valid = ERREUR;
  }

  return is_valid;
}

