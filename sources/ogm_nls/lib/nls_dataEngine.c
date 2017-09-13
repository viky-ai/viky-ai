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
int manageNodeReceived(struct jsonValuesContext * ctx)
{
  if (ctx->jsonNode.type == JSON_BOOLEAN)
  {
    changeTag(ctx);
    writeJsonNode(ctx);
  }
  if (ctx->jsonNode.type == JSON_DOUBLE)
  {
    changeTag(ctx);
    writeJsonNode(ctx);
  }
  if (ctx->jsonNode.type == JSON_INT)
  {
    changeTag(ctx);
    writeJsonNode(ctx);
  }
  if (ctx->jsonNode.type == JSON_NUMBER)
  {
    changeTag(ctx);
    writeJsonNode(ctx);
  }
  if (ctx->jsonNode.type == JSON_STRING)
  {
    changeTag(ctx);

    if (ctx->bIsArray[ctx->IsArrayUsed] == FALSE)
    {
      int iStringLen = ctx->jsonNode.valueSize;

      // si le tag de la string est "name", on répond bonjour
      if ((strcmp(ctx->jsonNode.mapKey, "Answer_Name") == 0) || (strcmp(ctx->jsonNode.mapKey, "Answer_name") == 0))
      {
        char tmpStringValue[DPcPathSize];
        snprintf(tmpStringValue, DPcPathSize, "%s", ctx->jsonNode.stringValue);
        snprintf(ctx->jsonNode.stringValue, DPcPathSize, "Hello %.*s", iStringLen + 6, tmpStringValue);
      }
    }
    writeJsonNode(ctx);
  }
  if (ctx->jsonNode.type == JSON_START_ARRAY)
  {
    changeTag(ctx);
    writeJsonNode(ctx);

    if (ctx->IsArrayUsed < maxArrayLevel)
    {
      ctx->IsArrayUsed++;
      ctx->bIsArray[ctx->IsArrayUsed] = TRUE;
    }
    else   // level max atteint, on ne fait plus rien et on envoie une erreur
    {

    }
  }
  if (ctx->jsonNode.type == JSON_END_ARRAY)
  {
    writeJsonNode(ctx);

    if (ctx->IsArrayUsed > 0)
    {
      ctx->IsArrayUsed--;
    }
    else   // level max atteint, on ne fait plus rien et on envoie une erreur
    {

    }
  }
  if (ctx->jsonNode.type == JSON_START_MAP)
  {
    if (ctx->bIsArray[ctx->IsArrayUsed] == FALSE)   // si la map n'est pas un élément d'un array -> pas de tag
    {
      int iMapLen = ctx->jsonNode.mapSize;
      char tmpMapKey[DPcPathSize];
      snprintf(tmpMapKey, DPcPathSize, "%.*s", iMapLen,  ctx->jsonNode.mapKey);
      snprintf(ctx->jsonNode.mapKey, DPcPathSize, "Answer_%s", tmpMapKey);
    }

    writeJsonNode(ctx);

    if (ctx->IsArrayUsed < maxArrayLevel)
    {
      ctx->IsArrayUsed++;
      ctx->bIsArray[ctx->IsArrayUsed] = FALSE;
    }
    else   // level max atteint, on ne fait plus rien et on envoie une erreur
    {

    }
  }
  if (ctx->jsonNode.type == JSON_END_MAP)
  {
    writeJsonNode(ctx);

    if (ctx->IsArrayUsed > 0)
    {
      ctx->IsArrayUsed--;
    }
    else   // level max atteint, on ne fait plus rien et on envoie une erreur
    {

    }
  }
  return TRUE;
}

int changeTag(struct jsonValuesContext * ctx)
{
  if (ctx->bIsArray[ctx->IsArrayUsed] == FALSE)
  {
    int iMapLen = ctx->jsonNode.mapSize;
    char tmpMapKey[DPcPathSize];
    snprintf(tmpMapKey, DPcPathSize, "%s", ctx->jsonNode.mapKey);
    snprintf(ctx->jsonNode.mapKey, DPcPathSize, "Answer_%.*s", iMapLen + 7, tmpMapKey);
  }
  return TRUE;
}
