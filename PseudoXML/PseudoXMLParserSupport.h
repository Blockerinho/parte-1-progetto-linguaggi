#ifndef PARSUP_H
#define PARSUP_H

#include "Absyn.h"

/* dato un albero, verifica che tutti gli import siano prima di tutte le
sezioni; se è così ritorna 0, altrimenti ritorna 1. */
int check_toplevel_tag_order(SourceFile p);

/* dato un albero, verifica che in ogni sezione tutti gli inherit siano prima
di tutti i field; se è così ritorna 0, altrimenti ritorna 1. */
int check_sublevel_tag_order(SourceFile p);

#endif
