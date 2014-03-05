#include "typecheck.h"
extern int outputStage;

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



data_type_t typecheck_default(node_t* root)
{

	if (root->children != NULL) {
		for (int i = 0; i < root->n_children; i++) {
			node_t* child = root->children[i];
			if (child != NULL) {
				child->typecheck(child);
				// I get the feeling this should return something
				// but when there's multiple children and it's not an expression
				// what am I supposed to do?
			}
		}
	}
}

data_type_t typecheck_expression(node_t* root)
{

	if(outputStage == 10)
		fprintf( stderr, "Type checking expression %s\n", root->expression_type.text);

	// so yeah I don't actually know how the expression nodes will be laid out
	// will a + expression have two or three children? Does the '+' node get simplified away?
	// the slides from recitation 3 aren't clear on this 
	// add, minus, mult, div (+, -, *, /)
	if (root->expression_type.index == ADD_E ||
			root->expression_type.index == SUB_E ||
			root->expression_type.index == MUL_E ||
			root->expression_type.index == DIV_E ) {
		// need to check the data type of both involved child-expressions
		// and compare it to the root's data type
		// wait how do I get the root's data type?	
		if (root->
		
	}
}

data_type_t typecheck_variable(node_t* root)
{
	// this should just return the type of the variable, right?
	// guess we could look it up in the symbol table
	// wait or could we just return its type because that would be way easier
	// but we probably can't

	if (root->nodetype.index == VARIABLE) {
		symbol_t* st = symbol_get(root->label);
		return st->type;
	}

	if (root->nodetype.index == FUNCTION) {
		function_symbol_t* fst = function_get(root->label);
		return fst->return_type;
	}

}
