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
}


