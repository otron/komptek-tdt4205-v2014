#include "bindnames.h"
extern int outputStage; // This variable is located in vslc.c
char* thisClass;

int b_d(node_t* root, int stack_offset);
int b_c(node_t* root, int stack_offset);

int variable_size = 4;
int func_offset = 8;
int method_offset = 12;
int local_var_offset = -4;
int class_field_offset = 0;

void visit_children(node_t *root, int stackOffset);
node_t* getVariableListFromFunctionNode(node_t* function_node);

int bind_default ( node_t *root, int stackOffset)
{
	return b_d(root,stackOffset);
}

void visit_children(node_t *root, int stackOffset) {
	for (int i = 0; i < root->n_children; i++) {
		if (root->children[i] != NULL) {
			root->children[i]->bind_names(root->children[i], stackOffset);
		}
	}
}


int bind_function ( node_t *root, int stackOffset)
{
	// when this happens we just entered a function node, right?
	// what we want to do here is a scope_add() first
	// and then we'll want to look at the parameters and call bind_names() on them with the right offset
	// wait, root could be a method as well, yeah?
	// ungh. OK. 
	// anyway, add a scope.
	scope_add();

	if (root->children == NULL) {
		// we can't have a function without _any_ children, I think.
		scope_remove();
		return; // I hope we're not supposed to use the return values for anything here
	}

	node_t* vList = getVariableListFromFunctionNode(root);
	function_symbol_t* fst = function_get(root->label);
	if (fst == NULL) {
		scope_remove();
		return;
	}
	int offset = (fst->nArguments - 1) * variable_size + stackOffset;
	//we're assuming stackOffset is either 8 or 12 depending on where this
	// was called from
	for (int i = 0; i < fst->nArguments; i++, offset -= variable_size) {
		vList->children[i]->bind_names(vList->children[i], offset);
	}

	// and now for the statement_list, no?
	// yes
	if (root->children[1] == NULL) {
		// oh, ok.
		scope_remove();
		return;
	}
	node_t* sList = root->children[1];
	for (int i = 0; i < sList->n_children; i++) {
		sList->children[i]->bind_names(sList->children[i],
				stackOffset);
	}

	// that should be all the children, right?
	// let's hope so
	scope_remove();
	if(outputStage == 6)
		fprintf ( stderr, "FUNCTION: Start: %s\n", root->label);



	if(outputStage == 6)
		fprintf ( stderr, "FUNCTION: End\n");

}

int bind_declaration_list ( node_t *root, int stackOffset)
{
	for (int i = 0; i < root->n_children; i++) {
		if (root->children[i] != NULL) {
			root->children[i]->bind_names(root->children[i],
					local_var_offset*i);
		}
	}

	if(outputStage == 6)
		fprintf ( stderr, "DECLARATION_LIST: Start\n");



	if(outputStage == 6)
		fprintf ( stderr, "DECLARATION_LIST: End\n");

}

int bind_class ( node_t *root, int stackOffset)
{

	class_symbol_t* cst = (class_symbol_t*) malloc(sizeof(class_symbol_t));
	
	int fields = 0; // I'm not sure if we need this
	cst->symbols = ght_create(8);
	cst->functions = ght_create(8);
	// I'm not sure /when/ I should do this
	// so I'm doing it here, because as far as I can guess
	// class_insert_x would be putting stuff into this
	class_add(root->label, cst);

	// look at them children yo
	if (root->children == NULL || root->n_children > 2) 
		return;
	// get the declaration_ and function_list
	node_t* dec_list = root->children[0];
	node_t* fun_list = root->children[1];

	for (int i = 0; i < dec_list->n_children; i++) {
		node_t* child = dec_list->children[i];
		if (child != NULL) {
			// y'know, I'm not sure why I've got this NULL check here
			//  if child is NULL, then we've that's just a bogus node
			// and not a field, but I let the variable offset scale
			// with i below. So on the one hand I'm conscious about segfaults
			// but on the other hand I'm assuming there'll be no NULL nodes
			// weeeird.

			// child should be a declaration_t node
			// which I need to create a symbol table entry for
			// remember, these declarations are for class fields
			symbol_t* st_entry = create_symbol(child, i*class_field_offset);
			class_insert_field(root->label, st_entry->label, st_entry);
		}
	}
	// and now the function_list
	for (int i = 0; i < fun_list->n_children; i++) {
		node_t* child = fun_list->children[i];
		if (child != NULL) {
			// child is a function node
			function_symbol_t *fst = create_function_symbol(child);
			class_insert_method(root->label, child->label, fst);
		}
	}
	// the children should be taken care of by now
	// and we should now be able to delve into the bodies of the functions
	// or methods, whichever.
	visit_children(fun_list, method_offset);

	if(outputStage == 6)
		fprintf(stderr, "CLASS: Start: %s\n", root->children[0]->label);

	

	if(outputStage == 6)
			fprintf(stderr, "CLASS: End\n");

}

node_t* getVariableListFromFunctionNode(node_t* function_node) {
	// returns the variable_list found somewhere amongst the node's children
	if (function_node->children == NULL || function_node->n_children == 0) 
		return NULL;

	// let's just try assuming the parameter_list is the first child
	node_t* pmList = function_node->children[0];
	// and hope nothing goes wrong
	
	// and then let's assume the parameter_list only has one child, which is a variable_list
	if (pmList == NULL || pmList->n_children == 0) 
		return NULL;

	// and just return it
	return pmList->children[0];
	// that's a lotta assumptions, man
	/* so this assumes that the function_nodes all have two children:
		[0] parameter_list
		[1] statement_list
		where 0 could be NULL (because functions with 0 parameters are allowed)
		but I'm not sure if this means the function_node only has one child
	*/
}

function_symbol_t* create_function_symbol(node_t* function_node)
{
	function_symbol_t *fst = (function_symbol_t*) malloc(sizeof(function_symbol_t));
	fst->label = function_node->label;
	fst->return_type = function_node->data_type;
	// look for nArguments and argument_types
	
	node_t* var_list;
   	var_list = getVariableListFromFunctionNode(function_node);

	
	if (var_list == NULL || var_list->n_children == 0) {
		fst->nArguments = 0;
		fst->argument_types = NULL;
		return fst;
	}

	fst->nArguments = var_list->n_children;
	fst->argument_types = (data_type_t*) malloc(sizeof(data_type_t) * fst->nArguments);
	
	for (int i = 0; i < var_list->n_children; i++) {
		node_t* child = var_list->children[i];
		fst->argument_types[i] = child->data_type;
	}
	return fst;
	// hey what do you know this doesn't cause segfaults!	
}

int bind_function_list ( node_t *root, int stackOffset)
{
	// this is the one where we have to put all the children in the symbol table before we do anything funky (else)
	if (root->children != NULL) {
		function_symbol_t* fst;
		for (int i = 0; i < root->n_children; i++) {
			// create a ST entry for the child
			fst = create_function_symbol(root->children[i]);
			function_add(fst->label, fst);
		}
	}
	// is it ok to visit the children now?
	int offset = 4;
	visit_children(root, offset);
	if(outputStage == 6)
		fprintf ( stderr, "FUNCTION_LIST: Start\n");

	

	if(outputStage == 6)
		fprintf ( stderr, "FUNCTION_LIST: End\n");

}

int bind_constant ( node_t *root, int stackOffset)
{
	visit_children(root, stackOffset);
	return b_c(root, stackOffset);
}


symbol_t* create_symbol(node_t* declaration_node, int stackOffset)
{
	// create a symbol table entry!
	// don't try creating an empty symbol_t* and filling out its fields because it'll segfault! 
	symbol_t *st_entry = (symbol_t*) malloc(sizeof(symbol_t));
	st_entry->stack_offset = stackOffset;
	st_entry->label = declaration_node->label;
	st_entry->type = declaration_node->data_type;
	return st_entry;
}

int bind_declaration ( node_t *root, int stackOffset)
{
	if (root == NULL)
		return; // I mean, can you ever be too safe when dealing with potential segfaults?
	// So, create the symbol table entry for this declaration?
	// According to the recitation slides, root is a variable
	symbol_t* st_entry = create_symbol(root, stackOffset);
	symbol_insert(st_entry->label, st_entry);

	if(outputStage == 6)
		fprintf(stderr, "DECLARATION: parameter/variable : '%s', offset: %d\n", root->label, stackOffset);

	visit_children(root, stackOffset);
}

int bind_variable ( node_t *root, int stackOffset)
{
	if (root == NULL)
		return;
	// ooh so this is where we go when a variable is used
	// so we should get the symbol table entry
	// and bind it to root->entry, right?
	root->entry = symbol_get(root->label);
	visit_children(root, stackOffset);
	// root is a variable node (nodetype_t = variable_n)


	if(outputStage == 6)
		fprintf ( stderr, "VARIABLE: access: %s\n", root->label);

}

int bind_expression( node_t* root, int stackOffset)
{

	// so, uh, check if it's a function call?
	// how do I do that?
	// I'm just going to try and retrieve the entry from both
	// the function- and variable table and add an entry for the
	// one that isn't NULL
	// NO WAIT
	// Do I even have to?
	// do only functions call end up here?
	// let's try that

	//root->function_entry = function_get(root->label);
	// nope, causes segfaults.

	char* label = root->label;
	if (label == NULL) {
		visit_children(root, stackOffset);
		return;
		// idk if this is the desired behaviour but that's what we're doing.
		// because then we don't have to put the rest of this function
		// inside an if-statement
	}

	// function stuff
	function_symbol_t* fun_entry = function_get(label);
	// no, I don't think this will work.
	// root is an expression node, yeah?
	// I've no idea what its structure is like.
	// but I don't think it's this easy
	// because then idk how we would do class usage
	// I'm not even sure what I should be looking for here
	// its structure is all kinds of different
	if (fun_entry != NULL) {
		// do the function stuff
		//root->function_entry = fun_entry;
	}
	// variable stuff? No, that's taken care of in bind_variable it seems

	// class stuff!

	visit_children(root, stackOffset);
	if(outputStage == 6)
		fprintf( stderr, "EXPRESSION: Start: %s\n", root->expression_type.text);

	

	if(outputStage == 6)
		fprintf( stderr, "EXPRESSION: End\n");

}


