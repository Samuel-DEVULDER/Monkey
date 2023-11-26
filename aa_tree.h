/*
 * AA tree to efficiently compact vec3
 *
 * (c) 2018 S.Devulder
 */

#ifndef AA_TREE_H
#define AA_TREE_H
 
#include "vectors.h"

struct aa_node {
	void *data;
	int level;
	struct aa_node *left, *right;
};

typedef int (*aa_cmpFcn)(const void *,const void *);

typedef struct {
	int size;
	struct aa_node *root;
	aa_cmpFcn cmpFcn;
} aa_tree;

typedef void (*aa_visitor)(struct aa_node *node, void *data);

_REG void aa_init(_A0(aa_tree *tree),_A1(aa_cmpFcn cmpFcn));
_REG void aa_exit(_A0(aa_tree *tree));
_REG const void *aa_add(_A0(aa_tree *tree), _A1(const void *data));
void *aa_visit(struct aa_node *root, aa_visitor func, void *data);

#endif
