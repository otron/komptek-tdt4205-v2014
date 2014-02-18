#include "simplifynodes.h"

extern int outputStage; // This variable is located in vslc.c

void simplify_children(Node_t* root, int depth);
void transfer_children(Node_t* fromNode, Node_t* toNode);


// calls simplify() on all the children of root
void simplify_children(Node_t* root, int depth) {
	for (int i = 0; i < root->n_children; i++) {
		root->children[i]->simplify(root->children[i], depth++);
	}
}

// transfers all the children of fromNode to toNode
void transfer_children(Node_t* fromNode, Node_t* toNode) {
	// Step 1:
	// oh cool, realloc is a thing.
	int no_children = fromNode->n_children + toNode->n_children;
		// it's funny how "No." is short for "numero" which means "number of"
	toNode->children = (Node_t**) realloc(toNode->children, sizeof(node_t*) * no_children);

		


}


Node_t* simplify_default ( Node_t *root, int depth )
{
	
}


Node_t *simplify_types ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

}


Node_t *simplify_function ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

}


Node_t *simplify_class( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );
}


Node_t *simplify_declaration_statement ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

}


Node_t *simplify_single_child ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );
	
}

Node_t *simplify_list_with_null ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

}


Node_t *simplify_list ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );
	// I'm just going to leave the fprintf stuff alone, k?

	// STEP 1: call simplify on all of root's children
	simplify_children(root, depth);

	// STEP 2: ????
	// the fuck.
	if (root->children[0]->nodetype == root->nodetype) {
		// wiat ok I got it let's swap the right and left child
		Node_t* child = root->children[0];
		root->children[0] = root->children[1];

	}

		
	for (int i = 0; i < (root->n_children); i++) {
		child = root->children[i];
		if (child->nodetype == root->nodetype) {
			// uh, we have to remove child from root->children
			// we can just set the children[]-entry to NULL and realloc, right?
			root->children[i] = NULL;

			transferChildren(child, root);
		}
	}

}



Node_t *simplify_expression ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s (%s) \n", depth, ' ', root->nodetype.text, root->expression_type.text );
		
}

