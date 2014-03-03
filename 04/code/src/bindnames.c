#include "bindnames.h"
extern int outputStage; // This variable is located in vslc.c
char* thisClass;

int b_d(node_t* root, int stack_offset);
int b_c(node_t* root, int stack_offset);

void visit_children(node_t *root, int stackOffset);

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
	visit_children(root, stackOffset);

	if(outputStage == 6)
		fprintf ( stderr, "FUNCTION: Start: %s\n", root->label);



	if(outputStage == 6)
		fprintf ( stderr, "FUNCTION: End\n");

}

int bind_declaration_list ( node_t *root, int stackOffset)
{
	visit_children(root, stackOffset);

	if(outputStage == 6)
		fprintf ( stderr, "DECLARATION_LIST: Start\n");



	if(outputStage == 6)
		fprintf ( stderr, "DECLARATION_LIST: End\n");

}

int bind_class ( node_t *root, int stackOffset)
{

	visit_children(root, stackOffset);
	if(outputStage == 6)
		fprintf(stderr, "CLASS: Start: %s\n", root->children[0]->label);

	

	if(outputStage == 6)
			fprintf(stderr, "CLASS: End\n");

}

function_symbol_t* create_function_symbol(node_t* function_node)
{

}

int bind_function_list ( node_t *root, int stackOffset)
{
	visit_children(root, stackOffset);
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

}

int bind_declaration ( node_t *root, int stackOffset)
{
	visit_children(root, stackOffset);

	if(outputStage == 6)
		fprintf(stderr, "DECLARATION: parameter/variable : '%s', offset: %d\n", root->label, stackOffset);

}

int bind_variable ( node_t *root, int stackOffset)
{
	visit_children(root, stackOffset);
	// root is a variable node (nodetype_t = variable_n)
	symbol_t* st_entry;
	st_entry->stack_offset = stackOffset;
	st_entry->label = root->label;
	st_entry->type = 


	if(outputStage == 6)
		fprintf ( stderr, "VARIABLE: access: %s\n", root->label);

}

int bind_expression( node_t* root, int stackOffset)
{
	visit_children(root, stackOffset);
	if(outputStage == 6)
		fprintf( stderr, "EXPRESSION: Start: %s\n", root->expression_type.text);

	

	if(outputStage == 6)
		fprintf( stderr, "EXPRESSION: End\n");

}


