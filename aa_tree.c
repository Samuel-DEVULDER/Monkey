/*
 * AA tree to efficiently compact vec3
 * (https://en.wikipedia.org/wiki/AA_tree)
 *
 * (c) 2018 S.Devulder
 */

#include "aa_tree.h"
#include <stdio.h>

static int nullCmp(const void *a, const void *b) {
	return ((long)a) - ((long)b);
}

_REG void aa_init(_A0(aa_tree *tree),_A1(aa_cmpFcn cmpFcn)) {
	 tree->size   = 0;
	 tree->root   = NULL;
	 tree->cmpFcn = cmpFcn==NULL ? nullCmp : cmpFcn;
}

static _REG struct aa_node * disp(_A0(struct aa_node *nd)) {
	if(nd) {
		nd->left  = disp(nd->left); 
		nd->right = disp(nd->right);
		free(nd);
	}
	return NULL;
}

_REG void aa_exit(_A0(aa_tree *tree)) {
	tree->size   = 0;
	tree->root   = disp(tree->root);
	tree->cmpFcn = NULL;
}

static _REG struct aa_node *skew(_A0(struct aa_node *t)) {
	if(t && t->left && t->level == t->left->level) {
			struct aa_node *l = t->left;
			t->left  = l->right;
			l->right = t;
			t = l;
	} 
	return t;
}

static _REG struct aa_node *split(_A0(struct aa_node *t)) {
	if(t && t->right && t->right->right && t->level == t->right->right->level) {
		struct aa_node *r = t->right;
		t->right = r->left;
		r->left  = t;
		r->level++;
		t = r;
	}
	return t;
}

struct insert_aux {
	const void *data;
	aa_tree *tree;
};

static _REG struct aa_node *insert(_A0(struct aa_node *t), _A1(struct insert_aux *aux)) {
	if(!t) {
		t = malloc(sizeof(*t));
		if(!t) {fprintf(stderr, "%s:%d:Out of mem!", __FILE__, __LINE__); exit(1);}
		t->level = 1;
		t->left = t-> right = NULL;
		t->data = (void*)aux->data;
		aux->tree->size++;
	} else {
		int d = (*aux->tree->cmpFcn)(aux->data, t->data);
		if(d==0) {
			aux->data = t->data; // already there
		} else {
			if(d<0)	t->left  = insert(t->left,  aux);
			else	t->right = insert(t->right, aux);
			t = split(skew(t));
		}
	}
	return t;
}

_REG const void *aa_add(_A0(aa_tree *tree), _A1(const void *data)) {
	struct insert_aux aux;
	aux.data   = data;
	aux.tree   = tree;
	tree->root = insert(tree->root, &aux);
	return aux.data;
}

void *aa_visit(struct aa_node *root, aa_visitor func, void *data) {
	if(root) {
		aa_visit(root->left, func, data);
		(*func)(root, data);
		aa_visit(root->right, func, data);
	}
	return data;
}
