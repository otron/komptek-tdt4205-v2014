#include "generator.h"
#include <math.h> // y does this not work

extern int outputStage; // This variable is located in vslc.c
static char* currentClass = NULL;
int peephole = 0;

/* Instructions */
typedef enum {
	COMMMENT, STRING, LABEL, PUSH, POP, MOVE,MOVES,MOVED, CALL, SYSCALL, LEAVE, RET,
	ADD, SUB, MUL, DIV, JUMP, JUMPZERO, JUMPNONZ, DECL, CLTD, NEG, CMPZERO, NIL,
	CMP, SETL, SETG, SETLE, SETGE, SETE, SETNE, CBW, CWDE,JUMPEQ,CVTSD, SET, LOAD, STORE, LOADS, STORES,
	FADD, FSUB, FMUL, FDIV, MOVE32
} opcode_t;

/* Registers */
// stackpointer = r13 = sp, framepointer = r11 / r7 = fp, linkregister (return address) = r14 = lr
static char
	*lr = "lr", *r0 = "r0", *r1 = "r1", *r2 = "r2", *r3 = "r3",
	*fp = "fp", *sp = "sp", *r5 = "r5", *r6 = "r6",
	*d0 = "d0", *d1="d1", *s0 = "s0", *s1 = "s1", *pc = "pc";


/* A struct to make linked lists from instructions */
typedef struct instr {
	opcode_t opcode;
	char *operands[2];
	int offsets[2];
	struct instr *next;
} instruction_t;

/* Start and last element for emitting/appending instructions */
static instruction_t *start = NULL, *last = NULL;


/* Support variables for nested while, for, if and continue.*/
node_t *continue_target;
char *continue_target_label;
int continue_target_depth;
static int while_count = 0;
static int for_count = 0;
static int if_count = 0;

// The counter used for the debug printing.
static int nodeCounter = 0;

/* Provided auxiliaries... */

static void instruction_append ( instruction_t *next )
{
	if ( start != NULL )
	{
		last->next = next;
		last = next;
	}
	else
		start = last = next;
}


static void instruction_add ( opcode_t op, char *arg1, char *arg2, int off1, int off2 )
{
	instruction_t *i = (instruction_t *) malloc ( sizeof(instruction_t) );
	*i = (instruction_t) { op, {arg1, arg2}, {off1, off2}, NULL };
	instruction_append ( i );
}


/*
 * Smart wrapper for "printf". 
 * Makes a comment in the assembly to guide.
 * Also prints a copy to the debug stream if needed.
 */
static void tracePrint( const char * string, ... )
{
	va_list args;
	char buff[1000];
	char buff2[1000];
	// 
	va_start (args, string);
	vsprintf(buff, string, args);
	va_end (args);
	
	sprintf(buff2, "%d %s", nodeCounter++, buff);
	
	if( outputStage == 10 )
    	fprintf(stderr, "%s", buff2);

	instruction_add ( COMMMENT, STRDUP( buff2 ), NULL, 0, 0 );
}


void gen_default ( node_t *root, int scopedepth)
{
	/* Everything else can just continue through the tree */
	if(root == NULL){
		return;
	}

	for ( int i=0; i<root->n_children; i++ )
		if( root->children[i] != NULL ) {
			root->children[i]->generate ( root->children[i], scopedepth );
		}
}


void gen_PROGRAM ( node_t *root, int scopedepth)
{
	/* Output the data segment */
	if( outputStage == 12 )
		strings_output ( stderr );
	instruction_add ( STRING, STRDUP( ".text" ), NULL, 0, 0 );

	tracePrint("Starting PROGRAM\n");

	gen_default(root, scopedepth);//RECUR();

	TEXT_DEBUG_FUNC_ARM();
	TEXT_HEAD_ARM();
	

	/* TODO: Insert a call to the first defined function here */
	// how do I find the first defined function?
	// is it the first function_statement node of root?
	// hmm it might be
	// because the PROGRAM node is going to look like one of the following:
	// [program, [function_list]]
	// [program, [class_list], [function_list]]
	// so we just iterate through its children
	// find the function_list and insert a call to the first child of the function_list
	// yeah, that sounds about right
	node_t* firstFunc;
	for (int i = 0; i < root->n_children; i++) {
		if (root->children[i]->nodetype.index = FUNCTION_LIST) {
			firstFunc = root->children[i]->children[0];
			break;
		}
	}
	// cool this doesn't segfault
	// so now we gotta insert a "call to the first function"
	// which I guess means we gotta add a jump instruction to _firstFunc->label
	// or will just label (without the underscore) suffice?

	// oh, am I supposed to use the CALL opcode here?
	instruction_add(CALL, STRDUP(firstFunc->label), NULL, 0, 0);
	// well it doesn't segfault


	tracePrint("End PROGRAM\n");

	TEXT_TAIL_ARM();
	
	if( outputStage == 12 )
		instructions_print ( stderr );

}



void gen_FUNCTION ( node_t *root, int scopedepth )
{

	// hmm, do I have to increase the scope here?

    tracePrint ( "Starting FUNCTION (%s) with depth %d\n", root->label, scopedepth);
    
	if (root->nodetype.index != FUNCTION) {
		// ????
		return;
	}
	// step 1: make dat label
	instruction_add(LABEL, STRDUP(root->label), NULL, 0, 0);

	// step 2: set up the stack frame
	// remember yo training, james
	// save link register on stack
	// waiit didn't I do this in the theory part?
	// so can't I just copy what I did there?
	// yay!

	// push link register onto stack
	instruction_add(PUSH, lr, NULL, 0, 0);
	// push fp onto stack
	instruction_add(PUSH, fp, NULL, 0, 0);
	// set fp to sp
	instruction_add(MOVE, fp, sp, 0, 0);

	// load parameters?
	// oh no we don't have to, we should just generate code for the function body
	// and by that I think they want us to traverse the tree
	// this is step 3 (execute function)
	for (int i = 0; i < root->n_children; i++) {
		node_t* child = root->children[i];
		if (child == NULL) {
			continue;
		}
		child->generate(child, scopedepth+1);	
	}
    

	// step 4
	// set sp to old fp
	instruction_add(MOVE, sp, fp, 0, 0);
	// restore old fp
	instruction_add(POP, fp, NULL, 0, 0);
	// store result/return value in r0
	// wait, can't we just do this in the return statement?
	// yeah let's just assume the return statement generator thing does this
	// pop return address into pc (jump back to caller)
	// oh what, I'm not allowed to make changes to the program counter directly with this framework?
	// WELL FINE
	// pop return address
	// (I made a pc pointer!)
	instruction_add(POP, pc, NULL, 0, 0);
	// and that's it

	

	//Leaving the scope, decreasing depth
	tracePrint ("Leaving FUNCTION (%s) with depth %d\n", root->label, scopedepth);
    
}






void gen_DECLARATION_STATEMENT (node_t *root, int scopedepth)
{
	tracePrint("Starting DECLARATION: adding space on stack\n");

	// I think we should just like
	// push r0 onto the stack here
	instruction_add(PUSH, r0, NULL, 0, 0);
	//(because VSL doesn't make any guarantees about the value of un-initialized variables
	//or so the recitation slides say)
	// it should work like this because parameters are visited before 
	// local variables in the traversal,
	// also declaration statements for local variables are visited in-turn so the stack offset should match
    // stuff should end up in the right place by itself

	tracePrint("Ending DECLARATION\n");
}


void gen_PRINT_STATEMENT(node_t* root, int scopedepth)
{
	tracePrint("Starting PRINT_STATEMENT\n");

	for(int i = 0; i < root->children[0]->n_children; i++){

		root->children[0]->children[i]->generate(root->children[0]->children[i], scopedepth);

		//Pushing the .INTEGER constant, which will be the second argument to printf,
		//and cause the first argument, which is the result of the expression, and is
		//allready on the stack to be printed as an integer
		base_data_type_t t = root->children[0]->children[i]->data_type.base_type;
		switch(t)
		{
		case INT_TYPE:
			instruction_add(STRING, STRDUP("\tmovw	r0, #:lower16:.INTEGER"), NULL, 0,0);
			instruction_add(STRING, STRDUP("\tmovt	r0, #:upper16:.INTEGER"), NULL, 0,0);
			instruction_add(POP, r1, NULL, 0,0);
			break;

		case FLOAT_TYPE:
			instruction_add(LOADS, sp, s0, 0,0);
			instruction_add(CVTSD, s0, d0, 0,0);
			instruction_add(STRING, STRDUP("\tfmrrd	r2, r3, d0"), NULL, 0,0);
			instruction_add(STRING, STRDUP("\tmovw	r0, #:lower16:.FLOAT"), NULL, 0,0);
			instruction_add(STRING, STRDUP("\tmovt	r0, #:upper16:.FLOAT"), NULL, 0,0);
			
			// And now the tricky part... 8-byte stack alignment :(
			// We have at least 4-byte alignment always.
			// Check if its only 4-byte aligned right now by anding that bit in the stack-pointer.
			// Store the answer in r5, and set the zero flag.
			instruction_add(STRING, STRDUP("\tandS	r5, sp, #4"), NULL, 0,0);
			// Now use the zero flag as a condition to optionally change the stack-pointer
			instruction_add(STRING, STRDUP("\tpushNE	{r5}"), NULL, 0,0);
			break;
			
		case BOOL_TYPE:
			instruction_add(STRING, STRDUP("\tmovw	r0, #:lower16:.INTEGER"), NULL, 0,0);
			instruction_add(STRING, STRDUP("\tmovt	r0, #:upper16:.INTEGER"), NULL, 0,0);
			instruction_add(POP, r1, NULL, 0,0);
			break;

		case STRING_TYPE:
			instruction_add(POP, r0, NULL, 0,0);
			break;

		default:
			instruction_add(PUSH, STRDUP("$.INTEGER"), NULL, 0,0);
			fprintf(stderr, "WARNING: attempting to print something not int, float or bool\n");
			break;
		}
		
		instruction_add(SYSCALL, STRDUP("printf"), NULL,0,0);

		// Undo stack alignment.
		if(t == FLOAT_TYPE) {
			// Redo the zero flag test on r5, as it will give the same answer as the first test on sp.
			instruction_add(STRING, STRDUP("\tandS	r5, #4"), NULL, 0,0);
			// Conditionally remove the alignment. 
			instruction_add(STRING, STRDUP("\tpopNE	{r5}"), NULL, 0,0);
		}
	}

	instruction_add(SET, r0, NULL, 0x0A, 0);
	instruction_add(SYSCALL, STRDUP("putchar"), NULL, 0,0);

	tracePrint("Ending PRINT_STATEMENT\n");
}


void gen_EXPRESSION ( node_t *root, int scopedepth )
{
	
	tracePrint ( "Starting EXPRESSION of type %s\n", (char*) root->expression_type.text);

	// by "currently only function calls", it's implied that method calls aren't covered yet, right?
	node_t* expr_list; 
	switch(root->expression_type.index){
		case FUNC_CALL_E:
			//now I know the recitation slides say that step 1 would be to push all the arguments
			// but isn't this where the functions get called?
			// so shouldn't we do all that stuff the caller has to do according to the recitation slides?
			// haha no because we should just save the registers on the stack
			// and what I've done is just put everything in r0 all the time
			instruction_add(PUSH, r0, NULL, 0, 0);

			// step 1: Push all the arguments
			// this mean we are calling generate on the children of the expr_list-child, right?
			if (root->n_children > 2)
				return; // yeah I don't know how this would happen but I don't want it to
			expr_list = root->children[1];
			if (expr_list != NULL) {
				for (int i = 0; i < expr_list->n_children; i++) {
					if (expr_list->children[i] != NULL) {
						expr_list->children[i]->generate(expr_list->children[i], scopedepth);
					}
				}
			}
			// otherwise there are no arguments

			// step 2: jump to the label
			// the "label" is the function name with an underscore in front of it
			// the function name is found in the first child of root
			char* lbl = root->children[0]->label;
			instruction_add(JUMP, lbl, NULL, 0, 0);
			// remove parameters
			if (expr_list != NULL) {
				for (int i = 0; i < expr_list->n_children; i++) {
					instruction_add(POP, r1, NULL, 0, 0);
				}
			}

			instruction_add(PUSH, r0, NULL, 0, 0); // push return value onto stack

		default:
			break;
	}

	tracePrint ( "Ending EXPRESSION of type %s\n", (char*) root->expression_type.text);
}

void gen_VARIABLE ( node_t *root, int scopedepth )
{

	tracePrint ( "Starting VARIABLE\n");

	if (root->nodetype.index != VARIABLE) {
		// eh
		return;
	}
	// "Load from memory using stack offset from symbol table"
	symbol_t* st = symbol_get(root->label);
	// wait, load from memory? Into what? A register?
	// which register? What kind of rules govern this?
	// I am just going to put it in r0
	instruction_add(LOAD, r0, fp, 0, st->stack_offset);
	// how do we know what register it is in later?
	// man this is weird
	// wait are we supposed to push it onto the stack?
	// like with constants?
	instruction_add(PUSH, r0, NULL, 0, 0);
	// yeah why not, might be an expression after all.

	tracePrint ( "End VARIABLE %s, stack offset: %d\n", root->label, root->entry->stack_offset);
}

void gen_CONSTANT (node_t * root, int scopedepth)
{
	tracePrint("Starting CONSTANT\n");
	if (root->nodetype.index != CONSTANT) {
		return;
	}

	switch (root->data_type.base_type) {
		case FLOAT_TYPE:
			//	instruction_add(MOVE32, root->float_const, r0, 0, 0);
			break;
		case DOUBLE_TYPE:
			// wait weren't all data types in VSL 4 bytes?
			// which makes it weird that doubles are a thing because those are typically 8 bytes, right?
			// idk man
			//	instruction_add(MOVE32, root->double_const, r0, 0, 0);
			break;
		case INT_TYPE:
			instruction_add(MOVE32, NULL, r0, root->int_const, 0);
			break;
		case BOOL_TYPE: ;
						// I am assuming that the vsl stuff uses 0 for false
						// and that anything that's not 0 is true
						int val = 1;
						if (root->bool_const == 0)
							val = 0;
						instruction_add(MOVE, r0, NULL, val, 0);
						break;
		case STRING_TYPE: ;
						  // do that thing described in the recitation slides with STRINGX
						  // how do I do a lookup in the string table?
						  // oh node_t has a field called string_index
						  // that was easy
						  // or well, I have to pass ".STRING%i", string_index to instruction_add
						  // which is... less easy 

						  // step 1: find the length of string_index in characters
						  int indexLength = 7; // lengthOf(".STRING") = 7
						  if (root->string_index != 0) // log10(0) is undefined and idk what it returns but it causes segfaults
							  indexLength += ((int) ceil(log10(root->string_index)));

						  //step 2: allocate enough space for the string/char pointer
						  char* strArg = malloc(sizeof(char) * (indexLength+1));
						  // step 3: store the formatted string in strArg
						  sprintf(strArg, ".STRING%d", root->string_index); 

						  instruction_add(MOVE32, strArg, r0, 0, 0);

						  // step 4: free memory used by strArg?
						  free(strArg);
						  break;
		case CLASS_TYPE:
						  // ???
						  break;
		case VOID_TYPE:
						  // ???
						  break;
		case ARRAY_TYPE:
						  // hm
						  break;
		default:
						  break;
	}
	// move constant to the top of the stack
	instruction_add(PUSH, r0, NULL, 0, 0);




	tracePrint("End CONSTANT\n");
}

void gen_ASSIGNMENT_STATEMENT ( node_t *root, int scopedepth )
{

	tracePrint ( "Starting ASSIGNMENT_STATEMENT\n");

	if (root->nodetype.index != ASSIGNMENT_STATEMENT)
		return; // ????

	// LHS = child[0]
	// RHS = child[1]
	node_t* LHS = root->children[0];
	node_t* RHS = root->children[1];

	// step 1: call generate on the RHS
	RHS->generate(RHS, scopedepth);
	// result of the RHS expression or whatever is now on the top of the stack
	// LHS should be a variable
	// step 2: put top-of-stack value into LHS

	if (LHS->nodetype.index == VARIABLE) {
		symbol_t* st = symbol_get(LHS->label);
		if (st == NULL)
			return; // ???
		instruction_add(POP, r1, NULL, 0, 0);
		instruction_add(STORE, r1, fp, 0, st->stack_offset);
		// yeah so the thing is I don't think this'll work
		// because I'm assuming that the variable is a local one in this scope, no?
		// anyway whatever
	}
	// if LHS isn't a variable then idk what to do



	tracePrint ( "End ASSIGNMENT_STATEMENT\n");
}

void gen_RETURN_STATEMENT ( node_t *root, int scopedepth )
{

	tracePrint ( "Starting RETURN_STATEMENT\n");

	// so my code goes here, then? ok. Let's try that
	// just gotta store the value returned by the expression in r0
	// yeah we should just call generate on the child and then pop the stack into r0

	node_t* child = root->children[0];
	if (child != NULL) {
		child->generate(child, scopedepth);
		instruction_add(POP, r0, NULL, 0, 0);
	}


	tracePrint ( "End RETURN_STATEMENT\n");
}





	static void
instructions_print ( FILE *stream )
{
	instruction_t *this = start;

	while ( this != NULL )
	{
		switch ( this->opcode ) // ARM
		{
			case PUSH:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tpush\t{%s}\n", this->operands[0] );
				else
					fprintf ( stream, "\tpushl TODO\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;
			case POP:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tpop\t{%s}\n", this->operands[0] );
				else
					fprintf ( stream, "\tpopl TODO\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;

			case MOVE32:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 ) {
					/*
					   snprintf(buff2, 100, "\tmovw	r0, #:lower16:%s", buff);
					   instruction_add(STRING, STRDUP(buff2), NULL, 0,0);
					   snprintf(buff2, 100, "\tmovt	r0, #:upper16:%s", buff);
					   instruction_add(STRING, STRDUP(buff2), NULL, 0,0);
					 */
					fprintf ( stream, "\tmovw\t%s, #:lower16:%s\n",
							this->operands[1], this->operands[0]+1
							);
					fprintf ( stream, "\tmovt\t%s, #:upper16:%s\n",
							this->operands[1], this->operands[0]+1
							);
					break;
				}
				// if offsets are not 0

				fprintf (stream, "\tmovw\t%s, #:lower16:%i\n",
						this->operands[1], this->offsets[0]);
				fprintf (stream, "\tmovt\t%s, #:upper16:%i\n",
						this->operands[1], this->offsets[0]);

				break; 
			case MOVE:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tmov\t%s, %s\n",
							this->operands[0], this->operands[1]
							);
				//Should not be used, for legacy support only
				else if ( this->offsets[0] != 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tldr\t%s, [%s, #%d]\n", 
							this->operands[1], this->operands[0], this->offsets[0]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "\tstr\t%s, [%s, #%d]\n",
							this->operands[0], this->operands[1], this->offsets[1]
							);
				break;


			case LOAD:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tldr\t%s, [%s]\n",
							this->operands[0], this->operands[1]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "\tldr\t%s, [%s, #%d]\n", 
							//this->offsets[0], this->operands[0], this->operands[1] "\tmovl\t%d(%s),%s\n",
							this->operands[0], this->operands[1], this->offsets[1]
							);
				else if ( this->offsets[0] != 0 && this->offsets[1] == 0 )
					fprintf ( stream, "ERROR, LOAD format not correct\n",
							//this->operands[0], this->offsets[1], this->operands[1] "\tmovl\t%s,%d(%s)\n"
							this->operands[0], this->operands[1], this->offsets[1]
							);
				break;

			case LOADS:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tflds\t%s, [%s]\n",
							this->operands[1], this->operands[0]
							);
				else if ( this->offsets[0] != 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tflds\t%s, [%s, #%d]\n", 
							//this->offsets[0], this->operands[0], this->operands[1] "\tmovl\t%d(%s),%s\n",
							this->operands[1], this->operands[0], this->offsets[0]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "ERROR, LOAD format not correct\n",
							//this->operands[0], this->offsets[1], this->operands[1] "\tmovl\t%s,%d(%s)\n"
							this->operands[0], this->operands[1], this->offsets[1]
							);
				break;


			case STORE:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tstr\t%s, [%s]\n",
							this->operands[0], this->operands[1]
							);
				else if ( this->offsets[1] != 0 && this->offsets[0] == 0 )
					fprintf ( stream, "\tstr\t%s, [%s, #%d]\n", 
							//this->offsets[0], this->operands[0], this->operands[1] "\tmovl\t%d(%s),%s\n",
							this->operands[0], this->operands[1], this->offsets[1]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "ERROR, STORE format not correct\n",
							//this->operands[0], this->offsets[1], this->operands[1] "\tmovl\t%s,%d(%s)\n"
							this->operands[0], this->operands[1], this->offsets[1]
							);
				break;

			case STORES:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tfsts\t%s, [%s]\n",
							this->operands[0], this->operands[1]
							);
				else if ( this->offsets[0] != 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tfsts\t%s, [%s, #%d]\n", 
							//this->offsets[0], this->operands[0], this->operands[1] "\tmovl\t%d(%s),%s\n",
							this->operands[0], this->operands[1], this->offsets[0]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "ERROR, STORE format not correct\n",
							//this->operands[0], this->offsets[1], this->operands[1] "\tmovl\t%s,%d(%s)\n"
							this->operands[0], this->operands[1], this->offsets[1]
							);
				break;

			case SET:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "SET ERROR: set\t%s, %s\n",
							this->operands[1], this->operands[0]
							);
				else if ( this->offsets[0] != 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tmov\t%s, #%d\n", 
							//this->offsets[0], this->operands[0], this->operands[1] "\tmovl\t%d(%s),%s\n",
							this->operands[0], this->offsets[0]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "\tmovl\t%s,%d(%s)\n",
							this->operands[0], this->offsets[1], this->operands[1]
							);
				break;

			case MOVES:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tmfcpys\t%s, %s\n",
							this->operands[1], this->operands[0]
							);
				else if ( this->offsets[0] != 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tError: NOT possible for ARM, use load/store\t%d(%s),%s\n",
							this->offsets[0], this->operands[0], this->operands[1]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "\tError: NOT possible for ARM, use load/store\t%s,%d(%s)\n",
							this->operands[0], this->offsets[1], this->operands[1]
							);
				break;

			case MOVED:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tmfcpyd TODO\t%s,%s\n",
							this->operands[1], this->operands[0]
							);
				else if ( this->offsets[0] != 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tError: NOT possible for ARM, use load/store\t%s, [%s,#%d]\n",  
							//this->offsets[0], this->operands[0], this->operands[1]   "\ldr\t%d(%s),%s\n",
							this->operands[1], this->operands[0], this->offsets[0]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "\tError: NOT possible for ARM, use load/store\t%s,%d(%s)\n",
							this->operands[0], this->offsets[1], this->operands[1]
							);
				break;

			case CVTSD:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tfcvtds\t%s,%s\n",
							this->operands[1], this->operands[0]
							);
				else if ( this->offsets[0] != 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tfcvtds TODO\t%d(%s),%s\n",
							this->offsets[0], this->operands[0], this->operands[1]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "\tfcvtds TODO\t%s,%d(%s)\n",
							this->operands[0], this->offsets[1], this->operands[1]
							);
				break;



			case ADD:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tadd\t%s, %s\n",
							this->operands[1], this->operands[0]
							);
				else if ( this->offsets[0] != 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\taddl\t%d(%s),%s\n",
							this->offsets[0], this->operands[0], this->operands[1]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "\taddl\t%s,%d(%s)\n",
							this->operands[0], this->offsets[1], this->operands[1]
							);
				break;

			case FADD:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tfadds\t%s, %s, %s\n",
							this->operands[1], this->operands[1], this->operands[0]
							);
				break;

			case SUB:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tsub\t%s, %s\n",
							this->operands[1], this->operands[0]
							);
				else if ( this->offsets[0] != 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tsubl\t%d(%s),%s\n",
							this->offsets[0], this->operands[0], this->operands[1]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "\tsubl\t%s,%d(%s)\n",
							this->operands[0], this->offsets[1], this->operands[1]
							);
				break;

			case FSUB:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tfsubs\t%s, %s, %s\n",
							this->operands[1], this->operands[1], this->operands[0]
							);
				else
					fprintf ( stream, "Not supported...\tfsub\t%s, %s\n",
							this->operands[1], this->operands[0]
							);
				break;

			case MUL:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tmul\t r0, %s\n", this->operands[0] ); 
				else
					fprintf ( stream, "Not supported...\tmul\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;
			case FMUL:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tfmuls\t %s, %s, %s\n", this->operands[1], this->operands[1], this->operands[0] ); 
				else
					fprintf ( stream, "Not supported...\tfmul\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;
			case DIV:
				if ( this->offsets[0] == 0 &&  this->operands[1] == NULL)
					fprintf ( stream, "\tsdiv\tr0, r0, %s\n", this->operands[0] );
				else  if ( this->offsets[0] == 0)
					fprintf ( stream, "\tsdiv\t%s, %s, %s\n", this->operands[1], this->operands[1], this->operands[0] );
				else
					fprintf ( stream, "\tidivl TODO\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;
			case FDIV:
				if ( this->offsets[0] == 0 &&  this->operands[1] == NULL)
					fprintf ( stream, "\tfdivs\ts0, s0, %s\n", this->operands[0] );
				else  if ( this->offsets[0] == 0)
					fprintf ( stream, "\tfdivs\t%s, %s, %s\n", this->operands[1], this->operands[1], this->operands[0] );
				else
					fprintf ( stream, "\tidivl TODO\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;
			case NEG:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tnegl\t%s\n", this->operands[0] );
				else
					fprintf ( stream, "\tnegl\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;

			case DECL:
				fprintf ( stream, "\tsubs\t%s, #1\n", this->operands[0]); // The s turn on flag updates
				break;
			case CLTD:
				fprintf ( stream, "\t#cltd TODO\n" );
				break;
			case CBW:
				fprintf ( stream, "#\tcbw# Useless on ARM\n" );
				break;
			case CWDE:
				fprintf ( stream, "#\tcwde # Useless on ARM\n" );
				break;
			case CMPZERO:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tteq\t%s, $0\n", this->operands[0] ); // Test equal
				else
					fprintf ( stream, "\tcmpl\t$0,%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;
			case CMP:
				if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tcmp\t%s,%s\n",
							this->operands[1], this->operands[0]
							);
				else if ( this->offsets[0] != 0 && this->offsets[1] == 0 )
					fprintf ( stream, "\tcmpl\t%d(%s),%s\n",
							this->offsets[0], this->operands[0], this->operands[1]
							);
				else if ( this->offsets[0] == 0 && this->offsets[1] != 0 )
					fprintf ( stream, "\tcmpl\t%s,%d(%s)\n",
							this->operands[0], this->offsets[1], this->operands[1]
							);
				break;
			case SETL:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tmov\t %s, #0\t\n\tmovlt\t %s, #1\n", this->operands[0], this->operands[0] );
				else
					fprintf ( stream, "ERROR: TODO\tsetl\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;
			case SETG:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tmov\t %s, #0\t\n\tmovgt\t %s, #1\n", this->operands[0], this->operands[0] );
				else
					fprintf ( stream, "ERROR: TODO\tsetg\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;
			case SETLE:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tmov\t %s, #0\t\n\tmovle\t %s, #1\n", this->operands[0], this->operands[0] );
				else
					fprintf ( stream, "ERROR: TODO\tsetle\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;
			case SETGE:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tmov\t %s, #0\t\n\tmovge\t %s, #1\n", this->operands[0], this->operands[0] );
				else
					fprintf ( stream, "ERROR: TODO\tsetge\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;
			case SETE:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tmov\t %s, #0\t\n\tmoveq\t %s, #1\n", this->operands[0], this->operands[0] );
				else
					fprintf ( stream, "ERROR: TODO\tsete\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;
			case SETNE:
				if ( this->offsets[0] == 0 )
					fprintf ( stream, "\tmov\t %s, #0\t\n\tmovne\t %s, #1\n", this->operands[0], this->operands[0] );
				else
					fprintf ( stream, "ERROR: TODO\tsetnq\t%d(%s)\n",
							this->offsets[0], this->operands[0]
							);
				break;

			case CALL: case SYSCALL:
				fprintf ( stream, "\tbl\t" );
				if ( this->opcode == CALL )
					fputc ( '_', stream );
				fprintf ( stream, "%s\n", this->operands[0] );
				break;
			case LABEL: 
				fprintf ( stream, "_%s:\n", this->operands[0] );
				break;

			case JUMP:
				fprintf ( stream, "\tb\t%s\n", this->operands[0] );
				break;
			case JUMPZERO:
				fprintf ( stream, "\tbeq\t%s\n", this->operands[0] ); // Same as equal on ARM  (?)
				break;
			case JUMPEQ:
				fprintf ( stream, "\tbeq\t%s\n", this->operands[0] );
				break;
			case JUMPNONZ:
				fprintf ( stream, "\tbne\t%s\n", this->operands[0] );// Same as !zero on ARM  (?)
				break;

			case LEAVE: 
				// Same as "leave"
				fprintf ( stream, "\tmov\tsp, fp\n");
				fprintf ( stream, "\tpop\t{fp}\n");
				break;
			case RET:   
				fprintf ( stream, "\tpop\t{pc}\n");
				break;

			case STRING:
				fprintf ( stream, "%s\n", this->operands[0] );
				break;

			case COMMMENT:
				fprintf ( stream, "#%s", this->operands[0] );
				break;

			case NIL:
				break;

			default:
				fprintf ( stderr, "Error in instruction stream\n" );
				break;
		}
		this = this->next;

	}
}

