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
	int i_start = toNode->n_children - 1;
	for (int i = i_start, j = 0; i < no_children; i++, j++) {
		toNode->children[i] = fromNode->children[j];
		fromNode->children[j] = NULL;
	}
	fromNode->n_children = 0;
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

	// STEP 2: compare root's type to the type of its left child
	// the fuck.
	// (we are assuming that there'll only ever be another list of the same type in the left child
	if (root->children[0]->nodetype.index == root->nodetype.index) {
		// wiat ok I got it let's swap the right and left child
		// STEP 3: Swap root's left (human) and right child
		Node_t* human = root->children[0];
		root->children[0] = root->children[1];
		root->children[1] = NULL;
		// STEP 4: realloc root->children to fit the children of the human 
		root->n_children = root->n_children - 1 + human->n_children;
		// (we have to fit the children of human and the children of root minus human.
		root->children = realloc(root->children, (root->n_children));
		// STEP 5: steal those kids
		int kid_offset = 1;
		for (int i = 0; i < root->n_children; i++) {
			root->children[i+kid_offset] = human->children[i];
			human->children[i] = NULL;
		}
		node_finalize(human);
	}

	// STEP N: return NULL because the recitations slides said that was cool.
	return NULL;
}



Node_t *simplify_expression ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s (%s) \n", depth, ' ', root->nodetype.text, root->expression_type.text );
		
}

