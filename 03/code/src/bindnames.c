#include "bindnames.h"
extern int outputStage; // This variable is located in vslc.c
char* thisClass;

int bind_default ( node_t *root, int stackOffset)
{
	if (root->children != NULL) {
		for (int i = 0; i < root->n_children; i++) {
			if (root->children[i] != NULL)
				root->children[i]->bind_names(root->children[i], stackOffset);
		}

	}
	
}

int bind_constant ( node_t *root, int stackOffset)
{
	if(outputStage == 6)
		fprintf ( stderr, "CONSTANT\n");
	
	// do something with the string constant
	// what nodes get assigned this? CONSTANT nodes, OK.
	// so I've gotta look for the ones that are of type STRING_CONST and do stuff with those
	
	if (root->data_type.base_type == STRING_TYPE) {
		root->string_index = strings_add(root->string_const);
	}
	

	return 0;
}


