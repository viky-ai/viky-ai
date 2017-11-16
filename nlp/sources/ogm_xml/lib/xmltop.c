/**
 *  Getting the top tag of an XML file
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : September 2006
 *  Version 1.0
 */
#include "ogm_xml.h"

/**
 * Gets the top tag of an XML file. the file should start with:
 * <?xml version="1.0" encoding="ISO-8859-1"?>
 * <top_tag>
 */
PUBLIC(int) OgXmlTopTag(void *herr, int is, unsigned char *s, int top_tag_size, unsigned char *top_tag)
{
  int state = 1, start = 0;
  og_bool found = FALSE;

  top_tag[0] = 0;

  for (int i = 0; i < is && !found; i++)
  {
    switch (state)
    {

      case 1:

        /** before <? **/
        if (i + 1 <= is && !memcmp(s + i, "<?", 2))
        {
          state = 2;
        }

        /**
         * No xml prolog use by PJ
         * before <top_tag
         * avoids such tags as "<!DOCTYPE3 or "<!ENTITY"
         */
        else if (i + 3 <= is && s[i] == '<' && s[i + 1] != '!')
        {
          state = 4;
        }
        break;

        /** before ?> **/
      case 2:
        if (i + 1 <= is && !memcmp(s + i, "?>", 2))
        {
          state = 3;
        }
        break;

        /** before <top_tag **/
      case 3:
        /** avoids such tags as "<!DOCTYPE3 or "<!ENTITY" **/
        if (i + 3 <= is && s[i] == '<' && s[i + 1] != '!')
        {
          start = i + 1;
          state = 4;
        }
        break;

        /** before top_tag> **/
      case 4:
        if (s[i] == '>')
        {
          int length = i - start;
          snprintf(top_tag, top_tag_size, "%.*s", length, s + start);
          OgTrimString(top_tag, top_tag);
          found = TRUE;
        }
        break;
    }
  }

  return (found);
}
