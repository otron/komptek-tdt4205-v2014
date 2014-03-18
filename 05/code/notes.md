mmm yeah, notes. Programming!
I hope git is installed on asti.
oh we are not using asti? or is this asti?
idk man
Gotta set up the git config so I can clone my repo

# Problem 1
Type checking!
typecheck_expression()
typecheck_assignment()
equal_types()

I think the easiest would be to start with equal_types()
ok so equal_types() has two arguments and they are both data_type_t (dtt)
dtt is a struct declared in include/symtab.h 
and yeah, ok, equal_types() should return true or 1 or whatever if the two arguments are the same type
but where is that defined? I mean, the requirements for having type equality that is
meh, I guess I just have to make something up then

well I couldn't figure it out using the gcc extension typeof because printf wouldn't let me use it so I am just going to...
hm
well compare some stuff
but like, what if both arguments are of the same class
oh, there's a base_type called CLASS_TYPE
WELL OK
I just did a bunch of checks that seemed to make sense at the moment
like, if their base data type isn't equal then they're not equal
and if they are both class instances then the class names should match
aaand if they are both arrays then their size and array type should match

so in typecheck_expression(), what are we supposed to return if there's an error?
They don't mention that anywhere it seems.
apparently it's type_error()

but hey! maybe the data_type_t definition has some answers.
because typecheck_expression() is supposed to return a data_type_t something.

ooh I gotta figure out how the arguments are laid out
also how do I access the symbol table?
same procedure for accessing it as in PS4
but I'm getting segfaults when I try to retrieve a function entry on some tests
because root->label == NULL
I don't know why it would do that
perhaps the label is somewhere else?

Hah! Calling type_error() if root's nodetype.index doesn't equal METH_CALL_E or FUNC_CALL_E prevents the segfault!
gotta find the number of arguments the function node has and compare it to the number of arguments the function should have 
wait we could count the number of argument-nodes before we do method/function specific stuff
I think root will have a child that's an expression_list and then we just count the number of nodes there
yup -- I took a look at the correct output from vsl program funcall.vsl in assignment 4:
a function call with arguments goes like [root, variable, [expression_list, expression1, ..., expression n]]
while a function call without arguments goes like [root, variable, nil]
(tree structure thing like in the qtree package, remember?)
Let's see if method calls are the same, mm?
method calls are like [root, className, methodName, [expression_list, ...]]
hmm, with nested classes it goes like [root, [expression(class_field), className1, ...], variable(methodName), [expression_list, ...]]
>>> so what I want to do is iterate through the children until I find an expression_list and then count its children

ok how do I get the class name from a method?
case 1: single class (class.method(), no class1.class2.[...].method()) 
case 2: nested classes

s.sumAndMultiply(10); // case 1
EXPRESSION()(METH_CALL)
	VARIABLE()("s")()
	VARIABLE()("sumAndMultiply")()
	EXPRESSION_LIST()()
		CONSTANT(INTEGER)(10)()

i.o.sum() // case 2
EXPRESSION()(METH_CALL)
	EXPRESSION()(CLASS_FIELD)
		VARIABLE()("o")()
     	VARIABLE()("i")()
	  VARIABLE()("sum")()
	  (nil)

(the nil in case 2 is where the expression list should be)
ok so according to this the first child of root will either be a VARIABLE or an EXPRESSION (type CLASS_FIELD)
so first we examine the first child in order to know how we'll get at the class name

Aand this approach does not give any segfaults! Whooo!

Yikes, I think I might actually be done with typecheck_expression() (per the recitation slides' specifications)
and it doesn't segfault. yowza. the fonz. exclamation. 


so, typecheck_assignment() then
## typecheck_assignment()

so an assignment has a variable on the LHS and an expression on the RHS, right?
typecheck_expression doesn't have the code to compute the type of an arbitrary expression
is there a list of expressions somewhere?
well yeah, in the vsl specification I guess

what do you know, assignment_statement ::= lvalue ASSIGN expression
let's look at the LHS (lvalue) first
lvalue ::= variable | expression '.' variable
is lvalue even a node in the AST or does it get simplified away?
looks like it gets simplified away
well okay. case 1 is pretty simple here (when the LHS is just a VARIABLE), then LHS.type = root->children[0]->data_type 

case 2 is probably more like classname.variable or something
I mean, lvalue does allow expressions before the dot but as far as I know the only expression rule that'll be used in assignment statements is the one that translates expression to lvalue
so we get variable.variable or variable.variable.variable and so on
Could we combine case 1 and 2 into a general case, where we look for the last VARIABLE child of LHS and get its type?
hmm, for case 2 the first child will be an expression of type CLASS_FIELD
for nested cases it'll probably go like it did the last time we dealt with this (in typecheck_expression())
where, y'know, class field continues on and the last VARIABLE in it is the name of the actual variable which is the one we have to look up and fetch its type

aaaand damn, it looks like typecheck_assignment() is done

# Problem 2
assembly code generation
I have a feeling this'll take more time than the last one

gen_PROGRAM()
gen_FUNCTION()
gen_DECLARATION_STATEMENT()
gen_EXPRESSION()
gen_VARIABLE()
gen_CONSTANT()
gen_ASSIGNMENT_STATEMENT()
gen_RETURN()

hmm, which one should I start with?
let's start by looking at generator.c
damn, generator.c is 722 lines long.
I think I am going to start with gen_RETURN()
but before I do that I have to look at the recitation slides and see what's up
and take notes 

I'm supposed to "add the instructions to this list during the tree traversal"?
What list?
Oh, the next slide reveals it: I'm supposed to use instruction_add(opcode_t op, char *arg1, char *arg2, int32_t off1, int32_t  off2)
opcode_t is defined at the top of generator_c
example from the recitation:
instruction_add(PUSH, r0, NULL, 0, 0) generates push {r0}
instruction_add(LOAD, r0, fp, 0, 4) generates ldr, r0, [fp, #4]
labels are generated like this:
instruction_add(LABEL, STRDUP(func_name))

The slides say we only need the following instructions for the assignment:
push, pop, mov, ldr, str and bl
so let's see how instruction_add(...) maps its arguments to those functions
I think off1 is the offset for arg1, and off2 is the offset of arg2
arg1 is the first operand and arg2 is the second operand

instruction_add(PUSH, arg1, arg2, off1, off2):
	push {arg1}
instruction_add(LOAD, ...):
	ldr r0, [fp, #off2]
instruction_add(POP, ...):
	pop {arg1}
instruction_add(MOVE, ...):
	mov arg1, arg2
instruction_add(LOAD, ...):
	ldr arg1, [arg2, #off2]
instruction_add(STORE, ...):
	str arg1, [arg2, #off2]
instruction_add(JUMP, ...):
	bl arg1

wait, BL isn't in the instruction list in generator.c
what?
oh, it's because the opcode_t things don't map directly they are more like
what's it called
you know how lol is an acronym for laughing out load?
like that only more like how rased is X for embarrased
abbreviation, that's probably it
I am unsure about the last one
actually I am unsure about all of them but oh well

anyway what was I doing 
## gen_PROGRAM()
right

hm, we can't add strings in C
and STRDUP() doesn't support format strings
How do I insert a jump to a label? Does it have to include the underscore?
ooh instruction_add(JUMP, STRDUP(label), NULL, 0, 0) should take care of it

I need to take a look at what function nodes look like
the declaration ones that is, inside function lists
hmm ok so if it has no parameters then the first node is NIL/NULL
so I need to check for NULL in the for loop
but after that just calling generate_ should be enough
VARIABLE_LISTS will just go to generate_default I think and that just continues the traversal

ok time for gen_DECLARATION_STATEMENT()
## gen_DECLARATION_STATEMENT()
declaration statements all look the same
all we gotta do is push r0 onto the stack
huh that was easy
hm, VSL doesn't seem to do declarations and initializations at once but hey
I am going to try and do that
or am I?
you know what I'll just make a note of it

## gen_expression()
whooo.
if there are any arguments they'll be in the child that's an expression list
hmm
how do I know what registers the arguments are in?
so if the arguments are variables the expression_list contains VARIABLES
how do I get the value of a variable?
does the symbol table keep track of it?
is it on the stack?
if it is on the stack (which I guess it should be, how do I know which one it is?)

ok so in the function's declaration (which is where we are now more or less)
its parameters are the first A declarations
then its local variables are the next B declarations
what we could do is find its symbol_table entry
buuut we don't have to
all we have to do now is push the local variables on the stack and remember where they are for the length of the function
we can find the number of local variables by running through the children and counting the number of declaration statements
ooor we could ...

hahaha
it's function calls!
not function declarations! hahahaha
i must've forgotten
that's what happens when you take a break
ok so there's two cases
case 1: arguments are constants. Just push the constant onto the stack.
case 2: arguments are variables. How do I find the variables' value?

move32 should have the target register as operand2 and the STRINGX thing as operand 1, what?

I don't think I should be putting all this code into the CONSTANT case
the only difference between constants and variables here is how I get the value
(with constants it's in the node, with variables I've gotta figure it out)
however the variable nodes should store their type
ooorrr that info might be available in the symbol table
ooh yeah, it is! in the function symbol table that is
while constant nodes have it stored in node_t? well they'll have one of those union-things set if it's a constant
and the value is found there
but I think we first want to find the value of the node, regardless of it's a constant or variable

ooh cool, the symbol table entry for variables contain their stack offset!
and stack depth

buut yeah, I'm going to wait with this gen_EXPRESSION thing because I think I'm overthinking it
and that's never good

hmm, what was I doing again?
I added a pc char pointer so I can pop the return address directly into the pc register

right it was the constant thing I was doing
idk how variable is going to turn out because I've no idea how to implement a register management system
but constants? yeah I can do that one

#gen_CONSTANT

hmm, so in the slides they mention that we should put the value of expressions on the top of the stack
that makes sense I guess

added a case to MOVE32 so that I can use it to load ints

also noticed that there's no way to load floats or doubles because the instruction_add thing only accepts integers as parameters 
oooh no we could do it with hex


also my code is segfaulting now
oh it's segfaulting from an earlier change it seems
yeah, there's something in gen_FUNCTION
and it's the generate call on its children
hmm
but why
yyyy daddy
hm, perhaps it's something in gen_RETURN
nope that one's empty
how about gen_DECL -- nope
shit man, I don't understand why this is happening
let's look at the tests that do segfault
hm, callFunc.vsl has a VOID type function
perhaps that's got something to do with it
yeah, I removed the VOID function and it ran without segfaulting
that is hella weird man
also callFunc.s5 and callFunc3.s5 have a nil-child in some EXPRESSION node
yeah, there was something funky in gen_EXPRESSION about not doing a null-check on some children

I'll get back to gen_expression later
ook so constants, right? right

hmm, my String approach segfaults
ahaha, because log10(0) is undefined and string_index might be 0!

let's go back to gen_VARIABLE then

#gen_VARIABLE
oh I just added a push instruction to make room on the stack
or was I not supposed to do that
you know what, I've no idea
I should revisit declaration_statements

#gen_DECLARATION_STATEMENT
dec statements have no children
aaand idk. Let's just leave it like this. pushing r0 onto the stack to increase its size.

#gen_EXPRESSION

#gen_ASSIGNMENT_STATEMENT
LHS is the first child
RHS is the second child and this is an expression
we call generate on the second child and then move the value on the top of the stack into the LHS

#gen_RETURN_STATEMENT
hm, why does calling generate on the first (and only child) cause segfaults in simple.vsl and returnV2.vsl?
it's not because the return statement returns the constant 0 (that's done in other test programs but they don't segfault)
although removing the RETURN statement does fix the segfault
hmm

eh, idk what's wrong with it
I could look at my output and compare it with the correct output and see what's up
but I also need to get started on the BASD assignment
