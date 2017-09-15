/*
 * nls_dataEngine.c
 *
 *  Created on: 11 sept. 2017
 *      Author: sebastien
 */

#include <stdio.h>
#include <stdlib.h>
#include "ogm_nls.h"

/**
 * manage the receiving of a node and dispatch the actions consequently
 * @param ctx
 * @return TRUE for the moment
 */
og_status manageNodeReceived(struct jsonValuesContext * ctx)
{
  if (ctx->jsonNode.type == JSON_BOOLEAN)
  {
    IFE(changeTag(ctx));
    IF(writeJsonNode(ctx))
    {
      NlsThrowError(ctx->lt, "manageNodeReceived : Error on jsonNode.type == JSON_BOOLEAN");
      DPcErr;
    }
  }

  if (ctx->jsonNode.type == JSON_DOUBLE)
  {
    IFE(changeTag(ctx));
    IF(writeJsonNode(ctx))
    {
      NlsThrowError(ctx->lt, "manageNodeReceived : Error on jsonNode.type == JSON_DOUBLE");
      DPcErr;
    }
  }

  if (ctx->jsonNode.type == JSON_INT)
  {
    IFE(changeTag(ctx));
    IF(writeJsonNode(ctx))
    {
      NlsThrowError(ctx->lt, "manageNodeReceived : Error on jsonNode.type == JSON_INT");
      DPcErr;
    }
  }

  if (ctx->jsonNode.type == JSON_NUMBER)
  {
    if (ctx->bIsArray[ctx->IsArrayUsed] == FALSE)
    {
      if(strcmp(ctx->jsonNode.mapKey,"wait")==0)
      {
        // si sleep, on attend le nombre de miliseconds
        int waitTime = atoi(ctx->jsonNode.numberValue);

        if(waitTime != 0)
        {
          OgSleep(waitTime);
        }

      }
    }

    IFE(changeTag(ctx));
    IF(writeJsonNode(ctx))
    {
      NlsThrowError(ctx->lt, "manageNodeReceived : Error on jsonNode.type == JSON_NUMBER");
      DPcErr;
    }
  }

  if (ctx->jsonNode.type == JSON_STRING)
  {
    IFE(changeTag(ctx));

    if (ctx->bIsArray[ctx->IsArrayUsed] == FALSE)
    {
      int iStringLen = ctx->jsonNode.valueSize;

      // si le tag de la string est "name", on répond bonjour
      if ((strcmp(ctx->jsonNode.mapKey, "Answer_Name") == 0) || (strcmp(ctx->jsonNode.mapKey, "Answer_name") == 0))
      {
        char tmpStringValue[DPcPathSize];
        snprintf(tmpStringValue, DPcPathSize, "%.*s", iStringLen, ctx->jsonNode.stringValue);
        snprintf(ctx->jsonNode.stringValue, DPcPathSize, "Hello %s", tmpStringValue);
      }
    }

    IF(writeJsonNode(ctx))
    {
      NlsThrowError(ctx->lt, "manageNodeReceived : Error on jsonNode.type == JSON_STRING");
      DPcErr;
    }

  }

  if (ctx->jsonNode.type == JSON_START_ARRAY)
  {
    IFE(changeTag(ctx));
    IF(writeJsonNode(ctx))
    {
      NlsThrowError(ctx->lt, "manageNodeReceived : Error on jsonNode.type == JSON_START_ARRAY");
      DPcErr;
    }

    if (ctx->IsArrayUsed < maxArrayLevel)
    {
      ctx->IsArrayUsed++;
      ctx->bIsArray[ctx->IsArrayUsed] = TRUE;
    }
    else
    {
      // level max atteint, on ne fait plus rien et on envoie une erreur
    }
  }

  if (ctx->jsonNode.type == JSON_END_ARRAY)
  {
    IF(writeJsonNode(ctx))
    {
      NlsThrowError(ctx->lt, "manageNodeReceived : Error on jsonNode.type == JSON_END_ARRAY");
      DPcErr;
    }

    if (ctx->IsArrayUsed > 0)
    {
      ctx->IsArrayUsed--;
    }
    else
    {
      // level max atteint, on ne fait plus rien et on envoie une erreur
    }
  }

  if (ctx->jsonNode.type == JSON_START_MAP)
  {
    if (ctx->bIsArray[ctx->IsArrayUsed] == FALSE)   // si la map n'est pas un élément d'un array -> pas de tag
    {
      int iMapLen = ctx->jsonNode.mapSize;
      char tmpMapKey[DPcPathSize];
      snprintf(tmpMapKey, DPcPathSize, "%.*s", iMapLen, ctx->jsonNode.mapKey);
      snprintf(ctx->jsonNode.mapKey, DPcPathSize, "Answer_%s", tmpMapKey);
    }

    IF(writeJsonNode(ctx))
    {
      NlsThrowError(ctx->lt, "manageNodeReceived : Error on jsonNode.type == JSON_START_MAP");
      DPcErr;
    }

    if (ctx->IsArrayUsed < maxArrayLevel)
    {
      ctx->IsArrayUsed++;
      ctx->bIsArray[ctx->IsArrayUsed] = FALSE;
    }
    else
    {
      // level max atteint, on ne fait plus rien et on envoie une erreur
    }
  }

  if (ctx->jsonNode.type == JSON_END_MAP)
  {
    IF(writeJsonNode(ctx))
    {
      NlsThrowError(ctx->lt, "manageNodeReceived : Error on jsonNode.type == JSON_END_MAP");
      DPcErr;
    }

    if (ctx->IsArrayUsed > 0)
    {
      ctx->IsArrayUsed--;
    }
    else
    {
      // level max atteint, on ne fait plus rien et on envoie une erreur
    }
  }

  return TRUE;
}

og_status changeTag(struct jsonValuesContext * ctx)
{
  if (ctx->bIsArray[ctx->IsArrayUsed] == FALSE)
  {
    int iMapLen = ctx->jsonNode.mapSize;
    char tmpMapKey[DPcPathSize];
    snprintf(tmpMapKey, DPcPathSize, "%.*s", iMapLen, ctx->jsonNode.mapKey);
    snprintf(ctx->jsonNode.mapKey, DPcPathSize, "Answer_%s", tmpMapKey);
  }
  DONE;
}
