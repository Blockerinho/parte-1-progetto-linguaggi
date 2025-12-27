#include <stdlib.h>
#include <stdio.h>

#include "Absyn.h"
#include "PseudoXMLParserSupport.h"

int check_toplevel_tag_order(SourceFile p) {
  ListTopLevelTag tl = p->u.mainFile_.listtopleveltag_;
  TopLevelTag t;
  int reached_section = 0;
  while (tl != 0) {
    t = tl->topleveltag_;
    switch (t->kind) {
      case is_FileImportTag:
        if (reached_section) {
          return 1;
        }
        break;
      case is_SectionTag:
        reached_section = 1;
        break;
      default:
        fprintf(stderr, "Error: bad kind for TopLevelTag");
        exit(1);
    }
    tl = tl->listtopleveltag_;
  }
  return 0;
}

int check_sublevel_tag_order(SourceFile p) {
  ListTopLevelTag tl = p->u.mainFile_.listtopleveltag_;
  TopLevelTag t;
  ListSubLevelTag sl;
  SubLevelTag s;
  int reached_field = 0;
  while (tl != 0) {
    t = tl->topleveltag_;
    if (t->kind != is_SectionTag) {
      tl = tl->listtopleveltag_;
      continue;
    }
    sl = t->u.sectionTag_.listsubleveltag_;
    while (sl != 0) {
      s = sl->subleveltag_;
      switch (s->kind) {
      case is_InheritTag:
        if (reached_field) {
          return 1;
        }
        break;
      case is_FieldTag:
        reached_field = 1;
        break;
      default:
        fprintf(stderr, "Error: bad kind for SubLevelTag");
        exit(1);
      }
      sl = sl->listsubleveltag_;
    }
    tl = tl->listtopleveltag_;
  }
  return 0;
}
