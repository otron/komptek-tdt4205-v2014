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
	/* STUFF 
		nodetype_t 			=> struct
		base_data_type_t 	=> enum
		data_type_t			=> struct
		expression_type_t	=> struct
	 */

	// Step 1: Create an empty node
	node_t* blank = (node_t*) malloc(sizeof(node_t));

	// Step 2: I guess like, just, assign the given label to the new node?
	blank->label = label;

	// Step 3: the node's type, then? Oh look it's a 1-1 thing.
	blank->nodetype = type;

	// Step 4: expression_type!
	blank->expression_type = expression_type;

	// Step 5: the children. the node_t** at blank->children should
	//	have room for n_children children, which means malloc
	//	should allocate (n_children * sizeof(node_t*)) memory space
	blank->children = (node_t**) malloc(sizeof(node_t*) * n_children);
	blank->n_children = n_children;
	// Step 5-a: populatin'.
	for (int i = 0; i < n_children; i++) {
		blank->children[i] = va_arg(child_list, node_t*);
		// if I am correct about how va_lists work, then va_arg()
		// should return the next node_t* when called.
		// haha, imagine that, something hinging on me being correct.
	}

	// Step 6: base_type
	// huh, do I have to create a new data_type_t struct instance?
	// ooor can I just set the base type of blank's data_type?
	// node_t.data_type isn't dynamically allocated, after all.
	// That's how it works, right?
	blank->data_type.base_type = base_type;
	return blank;
}


/* Should free the memory of the argument node */
void node_finalize ( node_t *discard ) {
	//right, so...
	// just, like, free all the parts of a node_t struct that are dynamically allocated?
	if (discard != NULL) {
		// free the label (char array)
		if (discard->label != NULL)
			free(discard->label);

		// free the children
		if (discard->children != NULL) {
			destroy_subtree(NULL, discard);
			free(discard->children);
		}
		free(discard);
	}
}

/* Should recursively free the memory for the subtree below the given node */
void destroy_subtree ( FILE *output, node_t *discard ) {
	if (discard != NULL)
		if (discard->children != NULL)
			for (int i = 0; i < discard->n_children; i++) {
				node_finalize(discard->children[i]);
			}
}
