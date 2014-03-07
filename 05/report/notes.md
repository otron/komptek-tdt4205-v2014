These are my notes.
cool

# Problem 1
Is it type checking or typechecking?
type-checking according to wikipedia.
type checking according to the dragon book.
I am going to use "type checking" because the dragon book, hooo!

6.3 pg 370 (Types and declarations)
"Type checking uses logical rules to reason about the behaviour of a program at run time.
Specifically, it ensures that the types of the operands match the type expected by an operator.
For example, the && operator in Java expects its two operands to be booleans; the result is also of type boolean."
(Operator expectations for expressions are language-specific)

6.5 pg 386 (Type Checking)
Type checking is when you verify that the type expressions found in the source program conform to the collection of logical rules called the type system for the source language.
(I wrote about this in the last theory assignment, did I not?)


All right, what kind of type checking did we do in PS04 code?
(well I never got that far but I was supposed to have done it)

In PS4 we were supposed to check that the types of the operands and result of unary and binary expressions match with the given rules (these rules are language-dependent).
In other words we did the type checking for some expressions.

What about PS5?
We are supposed to check that the number of arguments (and their types) matches the function declaration for function and method calls.
And that the type of the LHS and RHS of assignments matches.

Something about class_field_access expressions?
Also there is supposedly a type problem in err_callFunc.vsl
I am not sure what it is -- the function Two() is void and does not return anything, so that should be good
is the error that the print statement requires that its expressions have a non-void type? (i.e. that PRINT is not defined for operands of type VOID) 
I do not know, guy.

Other vsl programs that (according to recitation 5) have type errors:
addingBool.vsl, addingFloat2.vsl, addingFloat3.vsl, err_addingText.vsl, err_addingVoid.vsl, err_callFunc2.vsl
addingBool.vsl: attempts to BOOL = INT + BOOL. No such entry exists in the type system on the recitation 4 slides.
addingFloat2.vsl: FLOAT = INT + FLOAT. Again, no such entry exists in the type system on the recitation 4 slides.
addingFloat3.vsl: INT = FLOAT + INT
err_addingText.vsl: INT = INT + STRING
err_addingVoid.vsl: attempts to declare VOID a
(yes, void is a type)

Slides!
http://www.cs.cornell.edu/courses/cs412/2008sp/lectures/lec17.pdf
"Types are predicate on values"
type = set of possible values

# Problem 2
I am looking for information about how a stack looks and the rules and what not governing it.
I think "run-time stack" might be a relevant keyword here, as "stack" is just a data structure.
book?
I think this is in the slides, yo.

from recitation 5:
"The space used by a function on the stack is called a frame or activation record"
"When a function is called, a new frame is created and pushed on top of the stack."
When the function returns, the frame is popped.
sp points to the top of the stack. fp points to the start of the current frame.

There is a "function call protocol" in the slides, should probably just use that one
The stack grows upwards, its top is the bottom-most element (visually)

so, what? We push the frame for main onto the stack, then a and b, then a is set to 5 -- how is this represented on the stack?
is it important at all?
eh, I should probably just try and make it look like I got the jist of it
you know, make sure I got the right frames and put the local variables on there
I am anxious re. that I do not understand why the example in recitation 5 has parameters -> return address -> fp -> local variables (sp)
I do not know if the parameters are from the previous frame or the current one
If I could figure that out this would be v. eaz

> What.
From the CU lecture notes it looks like, yes, indeed, local variables are placed on the stack before parameters.

"the stack pointer is adjusted by one word for each register that is saved or restored" (from the dmkons book)
ooh, yeah, the recitation 5 slides even say so:
1: caller saves registers on stack
2: caller pushes parameters on stack
3: caller saves return address in link register
4: caller jumps to called function address
5: callee saves link register on stack
6: callee saves old fp on stack
7: callee sets new fp to top of stack
8: callee executes function (overwriting registers if necessary)
9: callee sets sp to its fp
10: callee restores old fp
11: callee stores result/return value in r0
12: callee jumps back to caller and pops return address
13: caller removes parameteres, restores registers, uses result.

that was not so hard. I can just mechanically follow that!


# Problem 3
This is the ARM assembly code generation one
this is also the last thing I will work on in this theory assignment.
I think this is only covered in the recitation 5 slides
Perhaps some control flow stuff is covered in the book or what not
wait, what do I actually need to do here?

Each function gets a label.
variables are assigned to a register

PUSH { rN } stores the contents of rN at the top of the stack
POP { rN } removes the content from the top of the stack and puts it in rN

can we reference the parameters of a function with LDR rN, [fp+offset]?
ok so when we call a function this happens:
BEFORE THE ACTUAL FUNCTION CALL
registers are saved on the stack
parameters are pushed onto the stack
	"the parameters" are the arguments given to the function that is being called
the return address is saved in link register -- WHAT DOES THIS MEAN?
jump to function address
	jmp %FUNCTIONLABEL%
store link register on stack
store fp on stack
	PUSH { fp }
set fp to top of stack
	MOV sp, fp ; places value in sp in fp

