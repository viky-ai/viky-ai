/**
 * Retrait d'une chaine de caracteres dans l'automate.
 *
 * An extended string is encoded in 9 bits or more
 * so that we can 'encode' extra-characters.
 *
 * Copyright (c) 1997-2014 Pertim by Brice Ruzand
 * Dev: July 2014
 * Version 1.0
 */
#include "ogm_aut.h"

/**
 * Retrait d'une chaîne de caractères dans l'automate.
 *
 * Attention, il n'est pas possible de supprimer des lignes de
 * description dans le cas où l'automate est minimisé.
 * Cependant, cette vérification n'est pas effectuée.
 *
 * A rupture string is encoded by adding 1 to each letter
 * and putting zero as a char at the rupture. This enables
 * a sorting identical to the repository sorting.
 */
PUBLIC(int) OgAutDelRup(void *handle, int iline, int rupture, unsigned char *line)
{
  struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *) handle;
  int il,l[DPcAutMaxBufferSize];

  IFE(OgAutRupToExt(ctrl_aut,iline,rupture,line,&il,l));

  IFE(AutDline(ctrl_aut, l));

  DONE;
}
