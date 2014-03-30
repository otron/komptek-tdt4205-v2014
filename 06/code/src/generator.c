#include "generator.h"
extern int outputStage; // This variable is located in vslc.c
static char* currentClass = NULL;
int peephole = 0;

#define ARM 1


/* Registers and opcodes have been moved to generator.h */

/* stuff I felt like including */
void do_int_arith(node_t* root, int scopedepth);
void do_int_cmp(node_t* root, int scopedepth);
void do_int_logic(node_t* root, int scopedepth);
void do_binary_general_expressions(node_t* root, int scopedepth);
void do_unary_general_expressions(node_t* root, int scopedepth);

static char *lhs = "r1",
			*rhs = "r2";

/* Start and last element for emitting/appending instructions */
static instruction_t *start = NULL, *last = NULL;

/* Support variables for nested while, if and continue.*/
static int while_count = 0;
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


void instruction_add ( opcode_t op, char *arg1, char *arg2, int off1, int off2 )
{
	instruction_t *i = (instruction_t *) malloc ( sizeof(instruction_t) );
	i->opcode = op;
	i->offsets[0] = off1; i->offsets[1] = off2;
	i->operands[0] = arg1; i->operands[1] = arg2;
	i->next = NULL;
	instruction_append ( i );
}

void instruction_add3 ( opcode_t op, char* arg1, char* arg2, char* arg3)
{
	instruction_t *i = (instruction_t *) malloc ( sizeof(instruction_t) );
	i->opcode = op;
	i->offsets[0] = 0; i->offsets[1] = 0;
	i->operands[0] = arg1; i->operands[1] = arg2; i->operands[2] = arg3;
	i->next = NULL;
	instruction_append ( i );
}


static void instructions_finalize ( void ) {};


/*
 * Smart wrapper for "printf". 
 * Makes a comment in the assembly to guide.
 * Also prints a copy to the debug stream if needed.
 */
void tracePrint( const char * string, ... )
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
		if( root->children[i] != NULL )
			root->children[i]->generate ( root->children[i], scopedepth );
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
	
	gp(root,scopedepth);
	

	tracePrint("End PROGRAM\n");

	TEXT_TAIL_ARM();
	
	if( outputStage == 12 )
		instructions_print ( stderr );
	instructions_finalize ();
}

void gen_CLASS (node_t *root, int scopedepth)
{
	// "generate code for methods, should be treated just like functions"

	// generate the label
	// root->class_entry == class ST entry
	//function_symbol_t* fst = class_get_method(root->label, 
	currentClass = root->label;
	for (int i = 0; i < root->n_children; i++) {
		if (root->children[0] != NULL) {
			root->children[0]->generate(root->children[0], scopedepth+1);
		}
	}


}

void gen_FUNCTION ( node_t *root, int scopedepth ) {
	
	// "Change the label generation to use different kinds of labels for methods and functions"
    // Generating label. This may need to be changed to handle labels for methods

	// How can we tell if root is a function or method declaration?
	// I guess that's the challenge here.
	// but hey, it turns out that all function declarations have a scopedepth of 1
	// and all method declarations have a scopedepth of 3
	// FUNCTION_LIST 0 > FUNCTION 1
	// CLASS_LIST 0 > CLASS 1 > FUNCTION_LIST 2 > FUNCTION 3
	// I figured this out by looking at the output from s5 and s7

	int func_SD = 1; //function scopedepth
	int meth_SD = 3; //method scopedepth

	if (scopedepth == func_SD) { //function labeling
		function_symbol_t* entry = root->function_entry;
		int len = strlen(entry->label);
		char *temp = (char*) malloc(sizeof(char) * (len + 3));
		temp[0] = 0;
		strcat(temp, "_");
		strcat(temp, entry->label);
		strcat(temp, ":");

		instruction_add(STRING, STRDUP(temp), NULL, 0, 0);
	}

	if (scopedepth == meth_SD) {
		// hurr, durr
		// it would be super easy if we could somehow figure out
		// the class the method belongs to
		// because then we could use the labeling scheme
		// _CLASS_METH:
		// hmm, I guess we could do this with a static char*
		// that gets set in gen_CLASS
		//  haha: static char* currentClass already exists!
		function_symbol_t* entry = root->function_entry;
		// length of label = len(funcName) + len(className) + 4
		// +4 because functions use +3 and we need an extra underscore here
		int len = strlen(entry->label) + strlen(currentClass); 
		len += 4;
		char* temp = (char*) malloc(sizeof(char) * (len));
		temp[0] = 0; // why do we do this? is it so that strcat() will work?
		strcat(temp, "_");
		strcat(temp, currentClass);
		strcat(temp, "_");
		strcat(temp, entry->label);
		strcat(temp, ":");

		instruction_add(STRING, STRDUP(temp), NULL, 0, 0);

	}

    gf(root,scopedepth);
}


void gen_DECLARATION_STATEMENT (node_t *root, int scopedepth)
{
	gd(root,scopedepth);
}


void gen_PRINT_STATEMENT(node_t* root, int scopedepth)
{
	tracePrint("Starting PRINT_STATEMENT\n");

	for(int i = 0; i < root->children[0]->n_children; i++){

		root->children[0]->children[i]->generate(root->children[0]->children[i], scopedepth+1);

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

	instruction_add(MOVE32, STRDUP("#0x0A"), r0, 0,0);
	instruction_add(SYSCALL, STRDUP("putchar"), NULL, 0,0);

	tracePrint("Ending PRINT_STATEMENT\n");
}


void gen_EXPRESSION ( node_t *root, int scopedepth )
{
	/*
	   CLASS_FIELD_E 	-- find address of part before '.', add offset from part after
	   THIS_E 			-- like a variable with offset 8
	   NEW_E 			-- call malloc
	   METH_CALL_E 		-- like function call with extra argument (THIS)
	   Arithmetic, comparison, logical expressions
	 */
	/*
	 * Expressions:
	 * Handle any nested expressions first, then deal with the
	 * top of the stack according to the kind of expression
	 * (single variables/integers handled in separate switch/cases)
	 */
	tracePrint ( "Starting EXPRESSION of type %s\n", (char*) root->expression_type.text);


	// the idea is to handle expressions that don't require special treatment based on their types first
	switch(root->expression_type.index) {
		case FUNC_CALL_E:
			ge(root, scopedepth);
			break;

		case METH_CALL_E: ;
			// like a function call but with an extra argument (THIS)
			// remember it's a function CALL, not a declaration
			// so don't try calling gen_FUNCTION() here

			// step whatever: get the label for the method
			// this is "_CLASS_METHOD:"
			// I did some thinking and if we want to support infinitely-nested classes and method calls
			// then that is fine
			// because no matter how many times we go class.class.class.class.class.class....class.method()
			// the only thing that matters is 1) the name of the method and 2) the name of the class the method belongs to
			// and the method itself (or the variable containing its name) is always the second child of root
			//		(the third child is the EXPRESSION_LIST with the arguments)

			// the class name is either the first child of root (e.g. class.method()) OR
			// the second child of the first child of root (e.g. class.class.method())
			// so we have to look at the first child's type
			
			char *methodName, *class_name;

			if (root->children[0]->nodetype.index == VARIABLE) {
				class_name = root->children[0]->label;
			} else if (root->children[0]->nodetype.index == EXPRESSION &&
					root->children[0]->expression_type.index == CLASS_FIELD_E) {
				class_name = root->children[0]->children[1]->label;
			}
			methodName = root->children[1]->label;

			
			int labelSize = 4 + strlen(methodName) +
				strlen(class_name);
			char* jmpLabel = (char*) malloc(sizeof(char) * labelSize);
			jmpLabel[0] = 0;
			strcat(jmpLabel, "_");
			strcat(jmpLabel, class_name);
			strcat(jmpLabel, "_");
			strcat(jmpLabel, methodName);
			strcat(jmpLabel, ":");

			// ok now for the rest of the steps
			// step 1: save registers on stack
			// there are no registers to save here as we put everything on the stack

			// step 2: push parameters onto the stack
			// i.e. call gen_default on the third child
			gen_default(root->children[2], scopedepth); // gen_default checks if the node argument is null so it's cool

			// step 3: save return address in link register
			// this should be done automatically by the jump instruction
			// step 4: jump to the function/method address
			instruction_add(JUMP, STRDUP(jmpLabel), NULL, 0, 0);
			instruction_add(STRING, STRDUP("#JUMP!"), NULL, 0, 0);
			// the return address is the next instruction
			free(jmpLabel);

			// step 5: caller removes parameters, restores registers and uses result
			// hmm, ok I guess
			// result should be in r0
			int paramCount = class_get_method(class_name, methodName);
			for (int i = 0; i < paramCount; i++) {
				instruction_add(POP, r1, NULL, 0, 0);
			}

			// save returned value on the stack
			instruction_add(PUSH, r0, NULL, 0, 0);

			break;

		case CLASS_FIELD_E:
			// find address of part before '.', add offset from part after
			// "we compute an address and then load the value from that address and push it into the stack)"

			// step 1: evaluate the part before the . to get the base address
			root->children[0]->generate(root->children[0], scopedepth+1);
			// I think this should work on nested class fields as well (e.g. class1.class2.field)
			// base address is now on top of the stack

			//step 2: find the offset of the second child
			// what
			// oh, do I look up the label of the second child in the class' symbol table and get its offset from there?
			// that makes sense
			char* className = root->children[0]->label;
			char* fieldName = root->children[1]->label;
			int second_offset = class_get_symbol(className, fieldName)->stack_offset;

			//step 3: load the value at address = base (child 0) + offset (child 1)
			// load the base address
			instruction_add(POP, r1, NULL, 0, 0);
			instruction_add(LOAD, r0, r1, 0, second_offset); // should load the field's value into r0

			//step 4: push it onto the stack
			instruction_add(PUSH, r0, NULL, 0, 0);

			break;
		case THIS_E:
			// like a variable, but with offset 8
			// I was like, "shouldn't the type of this be a class?"
			// but then I realized I have no idea what I am doing
			// oh, just load fp+8 and then push it to the stack, yeah?
			instruction_add(LOAD, r0, fp, 0, 8); // 8 is stack offset of "THIS"
			instruction_add(PUSH, r0, NULL, 0, 0);
			break;
		case NEW_E:
			// call _malloc
			// what
			// I can't find any definition of _malloc anywhere in this project
			//%TODO: NEW expressions
			break;

		default:
			// expression is type specific or whatever
			// I found these in generator.h, isn't that cool?
			switch (root->data_type.base_type) {
				case INT_TYPE:
					gen_int_expression(root, scopedepth);
					break;
				case FLOAT_TYPE:
					gen_float_expression(root, scopedepth);
					break;
				case BOOL_TYPE:
					gen_bool_expression(root, scopedepth);
					break;
				case STRING_TYPE:
					gen_string_expression(root, scopedepth);
					break;
				default: ;
			}

			break;
	}

	tracePrint ( "Ending EXPRESSION of type %s\n", (char*) root->expression_type.text);
}
void gen_int_expression(node_t* root, int scopedepth) {
	int size = strlen("# begin INT expression, type ") + strlen(root->expression_type.text);
	char* temp = (char*) malloc(sizeof(char) * (size + 1));
	temp[0] = 0;
	strcat(temp, "# begin INT expression, type ");
	strcat(temp, STRDUP(root->expression_type.text));
	instruction_add(STRING, STRDUP(temp), NULL, 0, 0);

	switch(root->expression_type.index) {
			// Arithmetics!
			// all of these are basically the same so I put them
			// in their own function

		// Binary arithmetic expressions!
		case ADD_E:
		case SUB_E:
		case MUL_E:
		case DIV_E:
			do_int_arith(root, scopedepth);
			break;
		// unary arithmetic expressions!
		case UMINUS_E:
			do_unary_general_expressions(root, scopedepth);
			instruction_add(NEG, r0, lhs, 0, 0);
			//NEG is like the arithmetic negation, right?
			// otherwise I'd have to do
			// MOV r1 #0 # load 0 into r1
			// SUB r0 r1 r0 # r0 = r1 - r0 == 0 - r0
			// 0 - A = -A
			// 0 - (-A) = -(-A) = A
			instruction_add(PUSH, r0, NULL, 0, 0);
			break;

		// Comparison!
		// they're all binary operations
		// and the CMP function checks out all of them
		case LEQUAL_E:
		case GEQUAL_E:
		case EQUAL_E:
		case NEQUAL_E: //!=
		case LESS_E:
		case GREATER_E:
			do_int_cmp(root, scopedepth);
			break;

			// Logic!
			// implemented as boolean logic
			// binary expressions
		case AND_E:
		case OR_E:
			do_int_logic(root, scopedepth);
			break;

			// unary expression
		case NOT_E: //! (logical negation)
			// xor it with 111111... I guess?
			// can use the EOR instruction for logical exclusive OR
			// I am going to get back to this
			//%TODO: implement logical NOT
			do_unary_general_expressions(root, scopedepth);
			// instructions that compute NOT lhs go here
			break;

		default:
			;
			// this should kick in if I forgot to handle an expression
			int tisize = strlen("#FORGOT: ") + 1;
			tisize += strlen(root->expression_type.text);
			char* tits = (char*) malloc(sizeof(char) * tisize);
			instruction_add(STRING, STRDUP(tits), NULL, 0, 0);
			break;
	}
}

// calls generate on the first (and only) child of root
// then pops the result of that expression into lhs/r1
void do_unary_general_expressions(node_t* root, int scopedepth) {
	root->children[0]->generate(root->children[0], scopedepth+1);
	instruction_add(POP, lhs, NULL, 0, 0);
}

void do_int_cmp(node_t* root, int scopedepth) {
	// result gets stored in the status register
	do_binary_general_expressions(root, scopedepth);
	instruction_add(CMP, r1, r2, 0, 0); // compare r1 and r2
}

void do_int_logic(node_t* root, int scopedepth) {
	do_binary_general_expressions(root, scopedepth);

	opcode_t operator;

	switch(root->expression_type.index) {
		case AND_E:
			operator = MUL;
			break;
		case OR_E:
			operator = ADD;
			break;
	}

	instruction_add3(operator, r0, lhs, rhs);
	instruction_add(PUSH, r0, NULL, 0, 0);
}

// calls generate on the first two children of root
// (root is assumed to be a binary integer expression node)
// and pushes its childrens' result values into lhs (r1) and rhs (r2)
void do_binary_general_expressions(node_t* root, int scopedepth) {
	root->children[0]->generate(root->children[0], scopedepth+1);
	root->children[1]->generate(root->children[1], scopedepth+1);
	instruction_add(POP, rhs, NULL, 0, 0); // right operand in r2
	instruction_add(POP, lhs, NULL, 0, 0); // left operand in r1
}
// calls generate on the first two children of root
// and does the integer arithmetic stuff
void do_int_arith(node_t* root, int scopedepth) {
	// it would be nice if some documentation was available for this
	// like, how do you suppose we should go about figuring out that
	// we need to use instruction_add3? It's not even mentioned
	// in the recitation slides. You could at least specify
	// something like "oh and by the way you should probably
	// have a look through the source code because most of it
	// isn't covered anywhere." That would be nice.

	do_binary_general_expressions(root, scopedepth);
	opcode_t oppy;

	switch (root->expression_type.index) {
		case ADD_E:
			oppy = ADD;
			break;
		case SUB_E:
			oppy = SUB;
			break;
		case MUL_E:
			oppy = MUL;
			break;
		case DIV_E:
			oppy = DIV;
			break;
	}

	instruction_add3(oppy, r0, lhs, rhs);
	instruction_add(PUSH, r0, NULL, 0, 0);
	instruction_add(STRING, STRDUP("#buttsdone"), NULL, 0, 0);
}
void gen_float_expression(node_t* root, int scopedepth) {
	// %TODO: (OPTIONAL) implement floating point arithmetic
	instruction_add(STRING, STRDUP("# begin float expression"), NULL, 0, 0);
}
void gen_string_expression(node_t* root, int scopedepth) {
	instruction_add(STRING, STRDUP("# begin string expression"), NULL, 0, 0);
}
void gen_bool_expression(node_t* root, int scopedepth) {
	gen_int_expression(root, scopedepth);
	// I see potential problems that might arise from handling boolean specific expressions as integers
	//instruction_add(STRING, STRDUP("# begin boolean expression"), NULL, 0, 0);
}

void gen_VARIABLE ( node_t *root, int scopedepth )
{
	gv(root,scopedepth);
}

void gen_CONSTANT (node_t * root, int scopedepth)
{
	gc(root,scopedepth);
}

void gen_ASSIGNMENT_STATEMENT ( node_t *root, int scopedepth )
{
	 tracePrint ( "Starting ASSIGNMENT_STATEMENT\n");

	//Generating the code for the expression part of the assignment. The result is
	//placed on the top of the stack
	root->children[1]->generate(root->children[1], scopedepth+1);

    // Left hand side may be a class field, which should be handled in this assignment
	if(root->children[0]->expression_type.index == CLASS_FIELD_E){
		//%TODO: write code that handles assignments of class fields

	}
	// or a variable, handled in previous assignment
	else{
		ga(root,scopedepth);
	}

	tracePrint ( "End ASSIGNMENT_STATEMENT\n");
}

void gen_RETURN_STATEMENT ( node_t *root, int scopedepth )
{
	gr(root,scopedepth);
}


void gen_WHILE_STATEMENT ( node_t *root, int scopedepth )
{
	// generate labels, jumps and code
	//%TODO: while statements

}


void gen_IF_STATEMENT ( node_t *root, int scopedepth )
{
	// generate labels, jumps and code
	//%TODO: IF Statements

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
		        	if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
		        		fprintf ( stream, "\tmovw\t%s, #:lower16:%s\n",
		        		          this->operands[1], this->operands[0]+1
		        		          );
		        		fprintf ( stream, "\tmovt\t%s, #:upper16:%s\n",
		        			      this->operands[1], this->operands[0]+1
		        		          );
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
		        	if ( this->operands[2] == NULL){
		        		//Legacy support
		                fprintf ( stream, "\tadd\t%s, %s\n",
		                        this->operands[1], this->operands[0]
		                        );
		        	}
		            else{
		            	fprintf ( stream, "\tadd\t%s, %s, %s\n",
		            			this->operands[0], this->operands[1], this->operands[2]
		            			);
		            }
		            break;
		        
		        case FADD:
		            if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
		                fprintf ( stream, "\tfadds\t%s, %s, %s\n",
		                 		this->operands[1], this->operands[1], this->operands[0]
		                	   );
		            break;
		        
		        case SUB:
		            if ( this->operands[2] == NULL )
		                fprintf ( stream, "\tsub\t%s, %s\n",
		                        this->operands[1], this->operands[0]
		                        );
		            else{
		            	fprintf ( stream, "\tsub\t%s, %s, %s\n",
		            			this->operands[0], this->operands[1], this->operands[2]
		            			);
		            }
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
		        		fprintf(stream, "\tmul\t%s,%s,%s\n",
		        				this->operands[0], this->operands[1], this->operands[2]);

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

		                fprintf ( stream, "\tsdiv\t%s, %s, %s\n",
		                		this->operands[0], this->operands[1], this->operands[2] );

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
		        case CMP:
		            if ( this->offsets[0] == 0 && this->offsets[1] == 0 )
		                fprintf ( stream, "\tcmp\t%s,%s\n",
		                        this->operands[0], this->operands[1]
		                        );
		            break;
		        case FCMP:
		        	fprintf( stream, "\tfcmps\t%s,%s\n", this->operands[0], this->operands[1]);
		        	fprintf( stream, "\tvmrs APSR_nzcv, FPSCR\n");
		        	break;

		        case MOVGT:
		        	fprintf(stream, "\tmovgt\t %s, %s\n", this->operands[0], this->operands[1]);
		        	break;
		        case MOVGE:
		        	fprintf(stream, "\tmovge\t %s, %s\n", this->operands[0], this->operands[1]);
		        	break;
		        case MOVLT:
		        	fprintf(stream, "\tmovlt\t %s, %s\n", this->operands[0], this->operands[1]);
		        	break;
		        case MOVLE:
		        	fprintf(stream, "\tmovle\t %s, %s\n", this->operands[0], this->operands[1]);
		        	break;
		        case MOVEQ:
		        	fprintf(stream, "\tmoveq\t %s, %s\n", this->operands[0], this->operands[1]);
		        	break;
		        case MOVNE:
		        	fprintf(stream, "\tmovne\t %s, %s\n", this->operands[0], this->operands[1]);
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
		            fprintf ( stream, "\tbeq\t%s\n", this->operands[0] );
		            break;
		        case JUMPEQ:
		            fprintf ( stream, "\tbeq\t%s\n", this->operands[0] );
		            break;
		        case JUMPNE:
		            fprintf ( stream, "\tbne\t%s\n", this->operands[0] );
		            break;
		        case JUMPNONZ:
		            fprintf ( stream, "\tbne\t%s\n", this->operands[0] );
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

