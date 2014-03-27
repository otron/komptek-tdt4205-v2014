#include "simplifynodes.h"

extern int outputStage; // This variable is located in vslc.c

void simplify_children(Node_t* root, int depth);
void transfer_children(Node_t* fromNode, Node_t* toNode);
void steal_children(Node_t* target, Node_t* victim);


// calls simplify() on all the children of root
void simplify_children(Node_t* root, int depth) {
	for (int i = 0; i < root->n_children; i++) {
		if (root->children[i] != NULL) 
			root->children[i]->simplify(root->children[i], depth+1);
	}
}

// steals the children of victim and gives them to target and removes victim from target's
void steal_children(Node_t* target, Node_t* victim) {
	int newSize = target->n_children + victim->n_children - 1;
	// sigh, we need to expand target->children
	// now for some reason realloc(target->children, sizeof(node_t*) * newsize) segfaults
	// OOH WAIT DOES IT SEGFAULT IF newsize == target->n_children? I SHOULD CHECK THAT OUT
	// probably not gonna, though
	node_t** new_generation; // array to holds the new number of children
	new_generation = (node_t**) malloc(sizeof(node_t*) * newSize);

	// ok so the children of victim should come first in the new generation
	int counter = 0;
	for (; counter < victim->n_children; counter++) {
		new_generation[counter] = victim->children[counter];
		victim->children[counter] = NULL; // so we can finalize victim later
		// when we are done with it
		// without it mucking up our AST
	}
	//free(victim->children);
	// so counter should now be equal to the last index in the new_generation
	// which means target's non-victim children should take their place from counter+1 and upwards
	counter++; 
	for (int i = 1; counter < newSize && i < target->n_children; counter++, i++) {
		new_generation[counter] = target->children[i];
		target->children[i] = NULL; // so we can free the empty space once we're done 
	}

	free(target->children); // this should free the memory space pointed to by target->children
	target->children = new_generation; // join target with its new family

	target->n_children = newSize; // gotta remember to update this or else we'll have inaccessible children
	// which would be bad for all the tiny dancers
	// hm, I got that tiny dancer song by elton john stuck on my mind
	// or actually just the "HOLD ME CLOSER TINY DANCERRRR"-bit

}
// trnsfers all the children of fromNode to toNode
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

	if (root->data_type.base_type == CLASS_TYPE) {
		//cases we're not equipped to handle
		if (root->n_children != 1)
			return root;
		if (root->children[0] == NULL)
			return root; // ????
		if (root->children[0]->nodetype.index != VARIABLE) 
			return root; 
		root->data_type.class_name = STRDUP(root->children[0]->label);
		root->n_children = 0;
		free(root->children); // wait or is it free() I'm supposed to use here?
		root->children = NULL;
	}

	// STEP end: YOU GET NOTHING.
	return root;
}


Node_t *simplify_function ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

	simplify_children(root, depth);

	// should move the first two children up into function
	// steal their info, not their kids
	node_t* type_kid = root->children[0];
	node_t* var_kid = root->children[1];

	root->data_type = type_kid->data_type; //the data_type "field" isn't a pointer so this should create a copy, right?
	root->label = STRDUP(var_kid->label);
	// sheesh, why does finalizing the nodes cause segfaults?
	// Since I don't understand where they would get referenced from
	// i am assuming it's node_finalize's fault
	// but the code works fine without finalizing them so hey, what's the problem anyway
	// nothing really, just a mild annoyance.
	// like how realloc() keeps segfaulting
	//node_finalize(type_kid);
	//node_finalize(var_kid);

	
	root->children[0] = NULL;
	root->children[1] = NULL;
	node_t** new_gen = malloc(sizeof(node_t*) * (root->n_children - 2)); 
	// because realloc keeps segfaulting on me and I don't want to leave memory hangin'

	for (int i = 2, j = 0; i < root->n_children; i++, j++) {
		new_gen[j] = root->children[i];
		root->children[i] = NULL;
	}
	free(root->children); 
	root->children = new_gen;
	root->n_children = root->n_children - 2; // we lost two children today.

	return root;
}


Node_t *simplify_class( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

	simplify_children(root, depth);

	// For class nodes, the label of the variable node can be moved up into the class node

	// the variable node is the first child of the class node
	// according to the grammar, anyway

	// steal that label
	root->label = STRDUP(root->children[0]->label);

	// create secondary pointer to the first child
	node_t* holdit = root->children[0];
	// dno why, though. Felt like it might come in handy?
	root->children[0] = NULL;

	// move all children one space up
	// malloc etc etc
	node_t** new_gen = (node_t**) malloc(sizeof(node_t*) * (root->n_children - 1));
	for (int i = 0, j = 1; j < root->n_children; i++, j++) {
		new_gen[i] = root->children[j];
		root->children[j] = NULL;
	}
	root->n_children -= 1; //reduce number of children by 1
	free(root->children);
	root->children = new_gen;


	return root;
}


Node_t *simplify_declaration_statement ( Node_t *root, int depth )
{
	if(outputStage == 4)
		fprintf ( stderr, "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

	simplify_children(root, depth);
	// declaration statements only ever have two children
	// at least according to the grammar
	if (root->children == NULL) 
		return root; // ????
	if (root->n_children != 2)
		return root; // ????

	// save those references
	node_t* type_child = root->children[0];
	node_t* var_child = root->children[1];

	// steal that data
	root->data_type = type_child->data_type;
	root->label = STRDUP(var_child->label);
	
	root->children[0] = NULL;
	root->children[1] = NULL;
	root->n_children = 0;
	free(root->children);

	// sigh, node_finalize segfaults
	// we're gonna have memory leaks aren't we?

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
	// should deal with declaration_list and function_list
	// their grammar is like
	// list := list element | nil
	// but that doesn't really matter at this point:
	// if the first child is NULL then we eliminate it and reduce the size of children by 1
	// which we will do by moving the second child to the first child's place and then setting n_children to 1
	// haha no we're doing the malloc thing
	node_t* goblin = root->children[0];
	if (goblin == NULL) {
		if (root->n_children > 1 && root->children[1] != NULL) {
			goblin = root->children[1]; // why create a new variable when we've got one we're not using?
			// free the children
			root->children[1] = NULL; // first child is already null
			free(root->children);
			// malloc space for the remaining child
			root->children = (node_t**) malloc(sizeof(node_t*));
			root->children[0] = goblin;
			root->n_children = 1;
		}
		return root;
	} else if (root->nodetype.index == goblin->nodetype.index) {
		// non-null goblin: >1 children
		// we should steal goblin's children
		//steal_children(root, goblin);

		// step 1: calculate new number of children
		int new_n_kids = root->n_children + goblin->n_children - 1;
		// step 2: malloc a new node_t** pointer
		node_t** new_gen = (node_t**) malloc(sizeof(node_t*) * new_n_kids);

		// step 3: place goblin's children in new_gen
		int counter = 0; // used to store goblin->n_children/first unused index of new_gen
		for (int i = 0; i < goblin->n_children; i++, counter++) {
			new_gen[i] = goblin->children[i];
			goblin->children[i] = NULL;
		}
		// empty goblin
		goblin->n_children = 0;
		free(goblin->children);
		root->children[0] = NULL;

		// move the children of into new_gen
		for (int i = 1; i < root->n_children; i++, counter++) {
			new_gen[counter] = root->children[i];
			root->children[i] = NULL;
		}
		free(root->children);
		root->children = new_gen;
		
		//node_finalize(goblin); // this is going to segfault right?
	}
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
	node_t* goblin = root->children[0]; // it's called goblin because "child" got confusing
	if (goblin == NULL)
		return root; //I do not want any segfaults please

	// if the type of root is the same as the type of goblin we should make do with the simplification
	// which is to say we should steal the children of goblin 
	if (goblin->nodetype.index == root->nodetype.index) {
		// ok cool, we're gonna steal the children of goblin 
		// after we've done that we need to discard goblin
		// if goblin has n children, root->children needs to be expanded to accomodate (root->n_children + n - 1)
		int newSize = root->n_children + goblin->n_children - 1;
		
		if (newSize == 0)
			return root; //what?
		if (newSize == root->n_children) {
			// this happens if goblin->n_children == 1
			// we don't need to expand root->children, just replace goblin with goblin's only child
			root->children[0] = goblin->children[0];
			goblin->children[0] = NULL;

		} else {
			steal_children(root, goblin);
		}
		// we should probably finalize goblin now
		// although I think that causes segfaults for some reason
		//node_finalize(goblin);
		return root;


		/*
		// need to expand the children array of root to (root->n_children + child->n_children - 1)
		int newsize = root->n_children + child->n_children - 1;
		if (newsize == 0)
			return root;

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
		*/


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

