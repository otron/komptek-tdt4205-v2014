#include "typecheck.h"
extern int outputStage;

data_type_t td(node_t* root);
data_type_t te(node_t* root);
data_type_t tv(node_t* root);

void type_error(node_t* root){
	fprintf(stderr, "Type error at:\n");
	if(root != NULL){
		fprintf(stderr,"%s", root->nodetype.text);
		if(root->nodetype.index == EXPRESSION){
			fprintf(stderr," (%s)", root->expression_type.text);
		}
		fprintf(stderr,"\n");
	}
	fprintf(stderr,"Exiting\n");
	exit(-1);
}

int equal_types(data_type_t a, data_type_t b) {
	// morelike equal rites
	// am i rite or what
	// yeah my man rincewind knows what's up
	// wait, the rules for type equality in VSL aren't specified anywhere
	// at least not in the recitation slides
	// ... what?

	if (a.base_type != b.base_type) {
		return FALSE;
	}
	// so from here on we can assume that a.base_type == b.base_type 

	//same class?
	if (a.base_type == CLASS_TYPE) {
		//compare class_names
		if (strcmp(a.class_name, b.class_name) == 0) {
			return TRUE;
		}
		return FALSE;
	}

	// arrays?
	if (a.base_type == ARRAY_TYPE) {
		if (a.array_type != b.array_type)
			return FALSE;
		if (a.n_dimensions != b.n_dimensions)
			return FALSE;
		// if we get here they have the same number of dimensions
		for (int i = 0; i < a.n_dimensions; i++) {
			if (a.dimensions[i] != b.dimensions[i])
				return FALSE;
		}
		// well okay they check out
		return TRUE;
	}

	// so if we get here they're not arrays or some class
	// which means they should be equal as long as the base type
	// is the same, right?
	return TRUE;
	// let's hope so
}

data_type_t typecheck_default(node_t* root)
{
	return td(root);
}

data_type_t typecheck_expression(node_t* root)
{
	data_type_t toReturn;

	if(outputStage == 10)
		fprintf( stderr, "Type checking expression %s\n", root->expression_type.text);

	toReturn = te(root);
	// this was here when I got here
	
	//Insert additional checking here
	// and by "here" you mean below "here", right?
	// All right, that's cool.
	// so, expressions!
	// we should check that the number of arguments (and their types) matches the function declaration for function and method calls

	// so I gotta find out if root represents a function or method call
	// root->expression_type.index == METH_CALL_E || FUNC_CALL_E ?

	if (root->nodetype.index != EXPRESSION) {
		// what are we even doing here
		type_error(root);
	}

	// assume root->nodetype == expression_n

	if (root->expression_type.index != METH_CALL_E ||
			root->expression_type.index != FUNC_CALL_E) {
		// we should break here I think
		type_error(root);
	}

	if (root->label == NULL)
		type_error(root);

	function_symbol_t* fst = (function_symbol_t*) malloc(sizeof(function_symbol_t));

	// count the number of argument child nodes root has
	int child_count = 0;
	node_t* expr_list; // store the expression_list for later
	for (int i = 0; i < root->n_children; i++) {
		expr_list = root->children[i];
		if (expr_list->nodetype.index = EXPRESSION_LIST) {
			child_count = expr_list->n_children;
			break;
		}
	}

	if (root->expression_type.index == FUNC_CALL_E) {
		// step 1: find its symbol table entry
		fst = function_get(root->label);
	}

	if (root->expression_type.index == METH_CALL_E) {
		// step 1: find the class name
		char* className;

		// step 2: look at the first child
		node_t* firstChild = root->children[0];

		if (firstChild->nodetype.index = VARIABLE) {
			//case 1, no nested class. e.g. class.method()
			// class name == label of first child
			className = firstChild->label;
		}
		else if (firstChild->nodetype.index == EXPRESSION &&
				firstChild->expression_type.index == CLASS_FIELD_E) {
			// case 2: nested class(es).
			// class name (where the method was declared)
			// is the last VARIABLE node in firstChild.
			className = firstChild->children[firstChild->n_children - 1]->label;

		}
		else { //case 3: error
			type_error(root);
		}
		// step 2: get the ST entry
		fst = class_get_method(className, root->label);
	}

	if (fst == NULL)
		type_error(root);
	// if we don't do that here then we're just going to get segfaults down the road
	// and I do not want that

	// step 2: uh, compare child_count with fst's no. parameters
	if (fst->nArguments != child_count)
		type_error(root); // argument count mismatch
	
	// step 3: compare the types of fst's (the declaration's) parameters
	// to the types of the children of expr_list
	// we can use equal_types(...) for this!
	// just throw type_error() if equal_types(...) doesn't return TRUE/1!
	
	// at this point fst->nArguments == child_count == expr_list->n_children
	for (int i = 0; i < fst->nArguments; i++) {
		// compare fst->argument_types[i] and expr_list->children[i]->data_type
		int equal = equal_types(fst->argument_types[i], expr_list->children[i]->data_type);
		if (equal == FALSE)
			type_error(root);
	}

	//need to return the function or method's return type
	return fst->return_type;
}

data_type_t typecheck_variable(node_t* root){
	return tv(root);
}

data_type_t typecheck_assignment(node_t* root)
{

}
