#include "tree.h"
#include "symtab.h"

// Print a node and all children recursively. 
void node_print ( FILE *output, node_t *root, int nesting )
{
    if ( root != NULL )
    {
        // Print 'nesting' number of ' ' before node text description
        fprintf ( output, "%*c%s", nesting, ' ', root->nodetype.text );

        // Print the data_type of the node, statements etc should be set to NO_TYPE
    	switch (root->data_type.base_type)
    	{
        	case INT_TYPE:
        		fprintf ( output, "(INTEGER" );
        		break;
        	case FLOAT_TYPE:
        		fprintf ( output, "(FLOAT" );
        		break;
        	case BOOL_TYPE:
        		fprintf ( output, "(BOOL" );
        		break;
        	case STRING_TYPE:
        		fprintf ( output, "(STRING" );
        		break;
        	case VOID_TYPE:
        		fprintf ( output, "(VOID" );
        		break;
        	case DOUBLE_TYPE:
        		fprintf ( output, "(DOUBLE" );
        		break;
        	case CLASS_TYPE:
        		if(root->data_type.class_name != NULL )
        			fprintf ( output, "(CLASS: %s", root->data_type.class_name );
        		else
        			fprintf ( output, "(CLASS: NULL" );
        		break;
        	case ARRAY_TYPE:
        		fprintf ( output, "(ARRAY" );
        		break;
        	case NO_TYPE:
        		fprintf ( output, "(" );
        		break;

        	default:
        		fprintf ( output, "(ERROR: No type set" );
        		break;
        }

		int dim = root->data_type.n_dimensions;
		for(int i = 0; i < dim; i++)
			fprintf ( output, "[%d]", root->data_type.dimensions[i] );

        fprintf ( output, ")" );
        
        // Print the value of constants
        if ( root->nodetype.index == CONSTANT ) {
        	switch (root->data_type.base_type)
        	{
            	case INT_TYPE:
            		fprintf ( output, "(%d)", root->int_const );
            		break;
            	case FLOAT_TYPE:
            		fprintf ( output, "(%f)", root->float_const );
            		break;
            	case DOUBLE_TYPE:
            		fprintf ( output, "(%f)", root->double_const);
            		break;
            	case BOOL_TYPE:
            		fprintf ( output, "(%s)", root->bool_const ? "True" : "False" );
            		break;
            	case STRING_TYPE:
            		fprintf ( output, "(%s)", root->string_const );
            		break;

            	default:
            		fprintf ( output, "(ERROR: No type set for constant/index)" );
            		break;
            }
        }

        // Print the label of nodes where it is set,
        if(root->label != NULL){
        	fprintf ( output, "(\"%s\")", root->label );
        }

        // Print expression type where it is set

        if(root->expression_type.text != NULL){
        	fprintf ( output, "(%s)", root->expression_type.text);
        }


        fputc ( '\n', output );
        for ( int i=0; i<root->n_children; i++ )
            node_print ( output, root->children[i], nesting+1 );
    }
    else
        fprintf ( output, "%*c%p\n", nesting, ' ', root );
}


// Print a node and all children recursively. 
void node_print_entries ( FILE *output, node_t *root, int nesting )
{
    if ( root != NULL )
    {
        // Print 'nesting' number of ' ' before node text description
        fprintf ( output, "%*c%s", nesting, ' ', root->nodetype.text );
        if ( root->nodetype.index == CONSTANT ) {
            if ( root->label != NULL )
                fprintf ( output, "(\"%s\"), ", root->label );
            fprintf ( output, "ID integer: %d", root->int_const );
            
        }
        if ( root->nodetype.index == VARIABLE || root->nodetype.index == FUNCTION )
        {
            if ( root->label != NULL )
                fprintf ( output, ", (\"%s\")", root->label );
            else
                fprintf ( output, "%p", NULL );
            
            if ( root->entry != NULL ) {
                fprintf ( output, ", depth: %d, stack_offset: %d", root->entry->depth, root->entry->stack_offset );
                    if ( root->entry->label != NULL )
                        fprintf ( output, ", entry label: \"%s\"", root->entry->label );
            }
            else
                fprintf ( output, ", %p", NULL );
        }
        fputc ( '\n', output );
        for ( int i=0; i<root->n_children; i++ )
            node_print_entries ( output, root->children[i], nesting+1 );
    }
    else
        fprintf ( output, "%*c%p\n", nesting, ' ', root );
}


node_t * node_init ( nodetype_t type,
		char* label,
		base_data_type_t base_type,
		expression_type_t expression_type,
		int n_children,
		va_list child_list )
{
	// create an empty node
	node_t* blank = (node_t*)malloc(sizeof(node_t));

	// stuff it
	// NAME YOUR CREATION
	blank->label = label;
	// Don't forget the children
	blank->n_children = n_children;
	blank->children = (node_t**)malloc(n_children * sizeof(node_t*)); //will this work?
	// it shooouuullddd (?) malloc enough room for n_children node_t-pointers

	// from reading up on va_list it seems like the thing is to call
	// va_arg(child_list, node_t*) to get the next argument in the va_list
	// no wait does va_arg(child_list) just return the next entry?
	// I'll try both!
	for (int i = 0; i < n_children; i++) {
		blank->children[i] = va_arg(child_list, node_t*);
	}

	// type stuff
	blank->nodetype = type;
	blank->expression_type = expression_type;

	// base_data_type_t is an enum defined in symtab.h
	// Hm, I think I have to create an instance of the data_type_t struct
	blank->data_type.base_type = base_type; 
	// naw let's just see if this works first.
}


/* Should free the memory of the argument node */
void node_finalize ( node_t *discard )
{
	//right, so...
	// just, like, free all the parts of a node_t struct that are dynamically allocated?
	if (discard != NULL) {
		// free the label (char array)
		if (discard->label != NULL)
			free(discard->label);

		// free the children
		if (discard->children != NULL) {
			for (int i = 0; i < discard->n_children; i++) {
				if (discard->children[i] != NULL)
					free (discard->children[i]);
			}
			free(discard->children);
		}
	}
}


void destroy_subtree ( FILE *output, node_t *discard )
{

}
