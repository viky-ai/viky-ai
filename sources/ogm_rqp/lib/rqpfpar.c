/*
 *  Main function for parsing a request
 *  Copyright (c) 2006-2008 Pertimm by Patrick Constant
 *  Dev : May,June 2006, March 2007, December 2008
 *  Version 1.3
*/
#include "ogm_rqp.h"


#define DOgMaxProximityValueSize    10


struct tree_depth_info {
  int subtree_number;
  int depth;
  };


static int IsBooleanOperator(struct og_ctrl_rqp *, unsigned char *, int, int, struct og_boolean_operator *);
static int AddImplicitOperator(struct og_ctrl_rqp *);



/*
 *  Parses the Unicode request into a simple structure.
 */

int RqpFirstParse(struct og_ctrl_rqp *ctrl_rqp)
{
  int uni_tree_id_length;
  unsigned char uni_tree_id[DPcPathSize];
  int in_dquote = 0, in_func_dquote = 0, start_dquote = 1;
  struct og_boolean_operator cbo, *bo = &cbo;
  int i, c, cbefore = 0, end = 0, state = 1, start = 0;
  unsigned char *s = ctrl_rqp->request;
  int end_of_word, is_boolean, depth = 0;
  int is = ctrl_rqp->request_length;
  int j, cj, cjm1, coma, in_quotes;
  char erreur[DOgErrorSize];
  int parenthesis_level = 0;

  int subtree_number = (-1), current_subtree_number = (-1), tree_depth = 0;
  struct tree_depth_info tree_min_depth[100];

  ctrl_rqp->NodeUsed = 0;

  for (i = 0, c = 0; !end; i += 2)
  {
    cbefore = c;
    if (i >= is)
    {
      c = ' ';
      end = 1;
    }
    else c = (s[i] << 8) + s[i + 1];
    switch (state)
    {
      /** before or between words **/
      case 1:
        if (c == '(') depth++;
        else if (c == ')')
        {
          if (tree_depth > 0 && tree_min_depth[tree_depth - 1].depth == depth)
          {
            if (tree_depth > 1)
            {
              subtree_number = tree_min_depth[tree_depth - 2].subtree_number;
              tree_depth--;
            }
            else
            {
              subtree_number = (-1);
              tree_depth = 0;
            }
          }
          depth--;
        }
        else if (cbefore != '\\' && c == '"')
        {
          if (in_dquote)
          {
            in_dquote = 0;
            depth--;
          }
          else
          {
            in_dquote = 1;
            start_dquote = 1;
            depth++;
          }
        }
        else if (!PcIsspace(c))
        {
          start = i;
          state = 2;
        }
        break;

        /** in words **/
      case 2:
        end_of_word = 0;
        if (c == '(')
        {
          unsigned char *subtree_name_uni = "\0s\0u\0b\0t\0r\0e\0e";
          int isubtree_name_uni = 14;
          unsigned char *without_subtree_name_uni = "\0w\0i\0t\0h\0o\0u\0t\0_\0s\0u\0b\0t\0r\0e\0e";
          int iwithout_subtree_name_uni = 30;

          og_bool is_subtree = (((i - start) == isubtree_name_uni)
              && !Ogmemicmp(subtree_name_uni, s + start, isubtree_name_uni));

          og_bool is_without_subtree = (((i - start) == iwithout_subtree_name_uni)
              && !Ogmemicmp(without_subtree_name_uni, s + start, iwithout_subtree_name_uni));

          /** format is tree(name, subtree) we want to be equivalent to (subtree) **/
          if (is_subtree || is_without_subtree)
          {
            for (coma = 0, in_quotes = 0, j = i + 2; j < is; j++)
            {
              cj = (s[j] << 8) + s[j + 1];
              cjm1 = (s[j - 2] << 8) + s[j - 1];
              if (cjm1 != '\\' && cj == '"')
              {
                if (in_quotes) in_quotes = 0;
                else in_quotes = 1;
              }
              if (cj == ',')
              {
                if (!in_quotes)
                {
                  coma = 1;
                  break;
                }
              }
            }
            if (!coma)
            {
              sprintf(erreur, "RqpFirstParse: tree function does not have a marker followed by coma");
              OgErr(ctrl_rqp->herr, erreur);
              DPcErr;
            }
            uni_tree_id_length = j - (i + 2);
            memcpy(uni_tree_id, s + (i + 2), uni_tree_id_length);
            OgTrimUnicode(uni_tree_id_length, uni_tree_id, &uni_tree_id_length, uni_tree_id);
            OgTrimUnicodeChar(uni_tree_id_length, uni_tree_id, &uni_tree_id_length, uni_tree_id, '"');
            OgTrimUnicode(uni_tree_id_length, uni_tree_id, &uni_tree_id_length, uni_tree_id);
            i = j;
            depth++; /** same operation as if in state 1 to erase the tree function **/
            current_subtree_number++;
            subtree_number = current_subtree_number;
            og_status status = RqpSubtreeAddId(ctrl_rqp, subtree_number, uni_tree_id_length, uni_tree_id,
                (is_without_subtree ? DOgRqpSubtreeTypeWithout : DOgRqpSubtreeTypeNormal));
            IFE(status);
            tree_min_depth[tree_depth].subtree_number = subtree_number;
            tree_min_depth[tree_depth].depth = depth;
            tree_depth++;
            state = 1;
            break;
          }
          parenthesis_level++;
          in_func_dquote = 0;
          state = 3;
          break;
        }
        if (cbefore == '=' && c == '"')
        {
          state = 4;
          break;
        }
        if (c == '(' || c == ')' || (cbefore != '\\' && c == '"') || PcIsspace(c)) end_of_word = 1;
        if (end_of_word)
        {
          IFE(is_boolean = IsBooleanOperator(ctrl_rqp, s, start, i - start, bo));
          IFE(RqpAddNode(ctrl_rqp, bo, start, i - start, depth, in_dquote, start_dquote, 0, subtree_number));
          start_dquote = 0;
          state = 1;
        }
        if (c == '(') depth++;
        else if (c == ')')
        {
          if (tree_depth > 0 && tree_min_depth[tree_depth - 1].depth == depth)
          {
            if (tree_depth > 1)
            {
              subtree_number = tree_min_depth[tree_depth - 2].subtree_number;
              tree_depth--;
            }
            else
            {
              subtree_number = (-1);
              tree_depth = 0;
            }
          }
          depth--;
        }
        else if (cbefore != '\\' && c == '"')
        {
          if (in_dquote)
          {
            in_dquote = 0;
            depth--;
          }
          else
          {
            in_dquote = 1;
            depth++;
          }
        }
        break;
        /** in functions which can have sub-functions, thus parenthesis levels **/
      case 3:
        if (c == ')')
        {
          if (!in_func_dquote) parenthesis_level--;
        }
        else if (c == '(')
        {
          if (!in_func_dquote) parenthesis_level++;
        }
        if (c == ')' && parenthesis_level == 0)
        {
          if (!in_func_dquote)
          {
            IFE(is_boolean = IsBooleanOperator(ctrl_rqp, s, start, i + 2 - start, bo));
            IFE(RqpAddNode(ctrl_rqp, bo, start, i + 2 - start, depth, in_dquote, start_dquote, 1, subtree_number));
            start_dquote = 0;
            state = 1;
          }
        }
        else if (cbefore != '\\' && c == '"')
        {
          if (in_func_dquote)
          {
            if (!(i > 0 && s[i - 1] == '\\')) in_func_dquote = 0;
          }
          else in_func_dquote = 1;
        }
        break;
        /** in double quote from attribute="word1 word2 word3" **/
      case 4:
        if (cbefore != '\\' && c == '"')
        {
          IFE(is_boolean = IsBooleanOperator(ctrl_rqp, s, start, i - start, bo));
          IFE(RqpAddNode(ctrl_rqp, bo, start, i + 2 - start, depth, in_dquote, start_dquote, 0, subtree_number));
          state = 1;
        }
        break;
    }
  }

  if (ctrl_rqp->loginfo->trace & DOgRqpTraceFirstParse)
  {
    OgMsg(ctrl_rqp->hmsg, "", DOgMsgDestInLog, "RqpFirstParse: after basic processing");
    IFE(OgRqpLogTree(ctrl_rqp));
  }

#if 0
  IFE(RqpChangeMinusOperator(ctrl_rqp));
  if (ctrl_rqp->loginfo->trace & DOgRqpTraceFirstParse)
  {
    OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
        , "RqpFirstParse: after RqpChangeMinusOperator");
    IFE(OgRqpLogTree(ctrl_rqp));
  }
#endif

  IFE(RqpCleanMultipleOperator(ctrl_rqp));
  if (ctrl_rqp->loginfo->trace & DOgRqpTraceFirstParse)
  {
    OgMsg(ctrl_rqp->hmsg, "", DOgMsgDestInLog, "RqpFirstParse: after first RqpCleanMultipleOperator");
    IFE(OgRqpLogTree(ctrl_rqp));
  }

  IFE(RqpSwapExceptOperator(ctrl_rqp));
  if (ctrl_rqp->loginfo->trace & DOgRqpTraceFirstParse)
  {
    OgMsg(ctrl_rqp->hmsg, "", DOgMsgDestInLog, "RqpFirstParse: after RqpSwapExceptOperator");
    IFE(OgRqpLogTree(ctrl_rqp));
  }

  IFE(RqpCleanMultipleOperator(ctrl_rqp));
  if (ctrl_rqp->loginfo->trace & DOgRqpTraceFirstParse)
  {
    OgThrMessageLog(ctrl_rqp->hmutex, DOgMlogInLog, ctrl_rqp->loginfo->where, 0,
        "RqpFirstParse: after second RqpCleanMultipleOperator");
    IFE(OgRqpLogTree(ctrl_rqp));
  }

  IFE(AddImplicitOperator(ctrl_rqp));
  if (ctrl_rqp->loginfo->trace & DOgRqpTraceFirstParse)
  {
    OgThrMessageLog(ctrl_rqp->hmutex, DOgMlogInLog, ctrl_rqp->loginfo->where, 0,
        "RqpFirstParse: after AddImplicitOperator");
    IFE(OgRqpLogTree(ctrl_rqp));
  }

  IFE(NormalizeDepth(ctrl_rqp));
  if (ctrl_rqp->loginfo->trace & DOgRqpTraceFirstParse)
  {
    OgThrMessageLog(ctrl_rqp->hmutex, DOgMlogInLog, ctrl_rqp->loginfo->where, 0, "RqpFirstParse: after NormalizeDepth");
    IFE(OgRqpLogTree(ctrl_rqp));
  }

  DONE;
}






/*
 *  Boolean operator are AND, OR, EXCEPT=NOT, and proximity operators
 *  which are interpreted as an AND. proximity operators are:
 *  <op< or <op>, where op can be:
 *   - a number, the number of words between the two elements
 *   - w (word, equivalent to 1) s (sentence) � (paragraph) p (page)
*/

static int IsBooleanOperator(struct og_ctrl_rqp *ctrl_rqp, unsigned char *s, int start, int length, struct og_boolean_operator *boolean_operator)
{
int i,c,is_proximity=0,same_order=0;
int idigit; char digit[DPcPathSize];

memset(boolean_operator,0,sizeof(struct og_boolean_operator));

if (length==6 && !Ogmemicmp(s+start,"\0a\0n\0d",length)) boolean_operator->name = DOgBooleanAnd;
else if (length==4 && !Ogmemicmp(s+start,"\0o\0r",length)) boolean_operator->name = DOgBooleanOr;
else if (length==6 && !Ogmemicmp(s+start,"\0n\0o\0t",length)) boolean_operator->name = DOgBooleanExcept;
else if (length==12 && !Ogmemicmp(s+start,"\0e\0x\0c\0e\0p\0t",length)) boolean_operator->name = DOgBooleanExcept;
else if (s[start]==0 && s[start+1]=='<') {
  if (s[start+length-2]==0) {
    if (s[start+length-1]=='<') { is_proximity=1; same_order=1; }
    else if (s[start+length-1]=='>') { is_proximity=1; }
    }
  if (is_proximity) {
    if (length==6) {
      if (s[start+2]==0) {
        c=s[start+3];
        if (isdigit(c)) {
          boolean_operator->name = DOgBooleanAnd;
          boolean_operator->proximity_type=DOgProximityWord;
          digit[0]=c; digit[1]=0; boolean_operator->proximity_value=atoi(digit);
          boolean_operator->same_order=same_order;
          }
        else if (c=='w') {
          boolean_operator->name = DOgBooleanAnd;
          boolean_operator->proximity_type=DOgProximityWord;
          boolean_operator->proximity_value=1;
          boolean_operator->same_order=same_order;
          }
        else if (c=='s') {
          boolean_operator->name = DOgBooleanAnd;
          boolean_operator->proximity_type=DOgProximitySentence;
          boolean_operator->proximity_value=1;
          boolean_operator->same_order=same_order;
          }
        else if (c==167) { /* � */
          boolean_operator->name = DOgBooleanAnd;
          boolean_operator->proximity_type=DOgProximityParagraph;
          boolean_operator->proximity_value=1;
          boolean_operator->same_order=same_order;
          }
        else if (c=='p') {
          boolean_operator->name = DOgBooleanAnd;
          boolean_operator->proximity_type=DOgProximityPage;
          boolean_operator->proximity_value=1;
          boolean_operator->same_order=same_order;
          }
        }
      }
    /** We accept 10 digit maximum **/
    else if (length>6 && length <= 4+DOgMaxProximityValueSize*2) {
      int is_digit=1;
      for (idigit=0,i=2; i<length-2; i+=2) {
        if (s[start+i]==0 && isdigit(s[start+i+1])) digit[idigit++]=s[start+i+1];
        else { is_digit=0; break; }
        }
      if (is_digit) {
        digit[idigit]=0;
        boolean_operator->name = DOgBooleanAnd;
        boolean_operator->proximity_type=DOgProximityWord;
        boolean_operator->proximity_value=atoi(digit);
        boolean_operator->same_order=same_order;
        }
      }
    }
  }

if (boolean_operator->name == DOgBooleanNil) return(0);
return(1);
}




/*
 *  Implicit operator is the AND operator
 *  a b c becomes a AND b AND c
 *  a (b c) becomes a AND (b AND c).
 *  (a b) c becomes (a AND b) AND c.
*/

static int AddImplicitOperator(struct og_ctrl_rqp *ctrl_rqp)
{
int subtree_number_before,subtree_number_after;
int depth_before,depth_after;
struct node *node;
int i,j,found;

for (i=0; i+1<ctrl_rqp->NodeUsed; i++) {
  if (ctrl_rqp->Node[i].boolean_operator.name==DOgBooleanNil &&
      ctrl_rqp->Node[i+1].boolean_operator.name==DOgBooleanNil) {
    int nb_nodes_to_move,depth=ctrl_rqp->Node[i].depth,in_dquote=0,start_dquote=0;
    if (depth > ctrl_rqp->Node[i+1].depth) depth=ctrl_rqp->Node[i+1].depth;
    if (ctrl_rqp->Node[i].in_dquote && ctrl_rqp->Node[i+1].in_dquote) {
      if (ctrl_rqp->Node[i+1].start_dquote) start_dquote=1;
      else in_dquote=1;
      }
    /** Just to make sure we have enough room at the end **/
    IFE(RqpAddNode(ctrl_rqp,0,0,0,depth,0,0,0,0));
    nb_nodes_to_move = ctrl_rqp->NodeUsed-i-2;
    memmove(ctrl_rqp->Node+i+2,ctrl_rqp->Node+i+1,nb_nodes_to_move*sizeof(struct node));
    node = ctrl_rqp->Node+i+1;
    memset(node,0,sizeof(struct node));
    node->father=(-1); node->son=(-1);
    node->before=(-1); node->next=(-1);
    node->start=0; node->length=0;
    node->boolean_operator.name=DOgBooleanAnd;
    if (in_dquote) {
      node->in_dquote=1;
      node->boolean_operator.proximity_type=DOgProximityWord;
      node->boolean_operator.proximity_value=1;
      node->boolean_operator.same_order=1;
      }
    if (start_dquote) node->depth=depth-1;
    else node->depth=depth;
    depth_before = ctrl_rqp->Node[i].depth;
    depth_after = ctrl_rqp->Node[i+2].depth;
    subtree_number_before = ctrl_rqp->Node[i].subtree_number;
    subtree_number_after = ctrl_rqp->Node[i+2].subtree_number;
    if (depth_before == depth_after) {
      if (subtree_number_before != subtree_number_after) {
        /** get the upper subtree_number from either tree **/
        j=ctrl_rqp->Node[i].father; found=0;
        while (j>=0) {
          if (ctrl_rqp->Node[j].subtree_number != subtree_number_before) {
            node->subtree_number = ctrl_rqp->Node[j].subtree_number;
            found=1;
            }
          j = ctrl_rqp->Node[j].father;
          }
        if (!found) node->subtree_number = -1;
        }
      else node->subtree_number = subtree_number_before;
      }
    else if (depth_before < depth_after) {
      node->subtree_number = subtree_number_before;
      }
    else {
      node->subtree_number = subtree_number_after;
      }
    i++;
    }
  }

DONE;
}








