#include "simplifynodes.h"

extern int outputStage; // This variable is located in vslc.c

void simplify_children(Node_t* root, int depth);
void transfer_children(Node_t* fromNode, Node_t* toNode);


// calls simplify() on all the children of root
void simplify_children(Node_t* root, int depth) {
	for (int i = 0; i < root->n_children; i++) {
		if (root->children[i] != NULL) 
			root->children[i]->simplify(root->children[i], depth+1);
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
	fromNode->children = realloc(fromNode->children, sizeof(node_t*) * fromNode->n_children);
}


Node_t* simplify_default ( Node_t *root, int depth )
{
	// I am just going to assume that this one is ok.	
	simplify_children(root, depth);
	return root;
}


Node_t *simplify_types ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

	simplify_children(root, depth);

	/*
	// STEP 1: Check if root's base data type is CLASS_TYPE
	if (root->data_type.base_type == CLASS_TYPE) {
		// STEP 2: what if our assumption that root only has one child and that it's a variable-type node don't hold up?
		if (root->n_children != 1) {
			// well this is an error
			// but we're not doing anything about it for now
		}
		// we could check if the child has a label or whatever, but wouldn't we want to trust that our parser or scanner doesn't give us a borked parse tree?
		// That is a legit question. I don't know, y'see.
		// STEP 3: get that variable
		Node_t* goblin_king = root->children[0];
		root->data_type.class_name = goblin_king->label;
		// STEP 4: Let there be no trace of the now-forgotten child.
		// yes I know it's the goblin_king that typically steals children I've seen Labyrinth but just run with it ok? OK.
		goblin_king->label = NULL;
		node_finalize(goblin_king);
	}	


	// STEP end: YOU GET NOTHING.
	*/
	return root;
}


Node_t *simplify_function ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

	simplify_children(root, depth);
	return root;
}


Node_t *simplify_class( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

	simplify_children(root, depth);
	return root;
}


Node_t *simplify_declaration_statement ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

	simplify_children(root, depth);
	return root;
}


Node_t *simplify_single_child ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );
	
	simplify_children(root, depth);
	return root;
	
}

Node_t *simplify_list_with_null ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

	simplify_children(root, depth);
	return root;

}


Node_t *simplify_list ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );
	// I'm just going to leave the fprintf stuff alone, k?

	// STEP 1: call simplify on all of root's children
	simplify_children(root, depth);


	// STEP 2:
	// look at the type of the first child
	node_t* child = root->children[0];
	if (child == NULL)
		return; //I do not want any segfaults please

	// if the type of root is the same as the type of child we should make do with the simplification
	// which is to say we should steal the children of child
	if (child->nodetype.index == root->nodetype.index) {
		// need to expand the children array of root to (root->n_children + child->n_children - 1)
		int newsize = root->n_children + child->n_children - 1;
		if (newsize == 0)
			return;

		node_t** oldChildren = root->children;
		root->children = (node_t**) malloc(sizeof(node_t*) * newsize); // no segfaults

		for (int i = 0; i < root->n_children - 1; i++) {
			root->children[i] = oldChildren[i+1];
			oldChildren[i] = NULL;
		}
		oldChildren[0] = NULL;
		// no segfaults
		free(oldChildren); // free the space we made for all those node_t* pointers
		// no segfaults
		// place the children of child in root->children
		for (int i = root->n_children - 1, j = 0; j < child->n_children; i++, j++) {
			root->children[i] = child->children[j];
			child->children[j] = NULL;
		}
		// no segfaults


	}
	/*
	// STEP 2: compare root's type to the type of its left child
	// the fuck.
	// (we are assuming that there'll only ever be another list of the same type in the left child
	if (root->children == NULL)
		return root;
	if (root->children[0] == NULL)
		return root;
	if (root->children[1] == NULL)
		return root;

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
	*/
	// STEP N: return NULL because the recitations slides said that was cool.
	return root;
}



Node_t *simplify_expression ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s (%s) \n", depth, ' ', root->nodetype.text, root->expression_type.text );
		
	simplify_children(root, depth);
	return root;
}

