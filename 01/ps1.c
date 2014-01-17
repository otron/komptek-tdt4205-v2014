#include <stdio.h>
#include <stdlib.h>

// A struct for tree nodes, with pointers to the parent and child nodes
// and a int to store the value at the node.
typedef struct Node{
    struct Node* left;
    struct Node* right;
    int value;
} Node;


// Returns a random number between 0 and n
// For simplicity, the random number generator is not seeded,
// hence the same random numbers will be generated on every execution
int get_random_number(int n){
    return rand() % n;
}


// Return a (dynamically allocated) array of length size,
// filled with random numbers between 0 and n
int* create_random_array(int size, int n){
	// dynamically allocate enough memory
	// (the dynamically allocated array should hold
	// "size" numbers between 0 and n, where n is an integer
	int* array = malloc(size * sizeof(int));

	// iterate through the empty array and populate it
	for (int i = 0; i < size; i++) {
		array[i] = get_random_number(n);
	}
	return array;
}


// Should print the contents of array of length size
void print_array(int* array, int size){
	for (int i = 0; i < size; i++) {
		printf("%i ", array[i]);
	}
	printf("\n");
}


// Quicksort!
// (NOTE: IT DOESN'T WORK. YET I DID NOT HAVE THE HEART
//  TO REMOVE IT)
int quicksort_partition(int* array, int p, int r) {
	int pivot = array[r];
	int i = (p-1);
	int temp;
	for (int j = p; p < r; p++) {
		if (array[j] <= pivot) {
			i++;
			temp = array[j];
			array[j] = array[i];
			array[i] = temp;
		}
	}
	i++;
	array[r] = array[i];
	array[i] = pivot;
	return i;
}
void quicksort(int* array, int p, int r) {
	// from page 171 of "Introduction to Algorithms", by Cormen et. al
	// (you know, the one known colloquially as "Cormen")
	if (p < r) {
		int q = quicksort_partition(array, p, r);
		quicksort(array, p, (q-1));
		quicksort(array, (q+1), r);
	}
}
void bubblesort(int* array, int size) {
	int swapped = 0;
	for (int i = 1; i < size; i++) {
		if (array[i-1] > array[i]) {
			int temp = array[i];
			array[i] = array[i-1];
			array[i-1] = temp;
			swapped = 1;
		}
	}
	if (swapped == 1) {
		bubblesort(array, size);
	}
}
// Should sort the numbers in array in increasing order
void sort(int* array, int size){
	// let's implement quicksort, whoo.
//	quicksort(array, 0, size-1);
	bubblesort(array, size);
}





// Inserts the node into the tree rooted at the node pointed to by root
void insert_node(Node** root, Node* node){
	// wait so I just insert the node somewhere in the tree?
	// no, apparently it's a binary tree.
	// weird how I had to look up the slides from the recitation to
	// figure that out. You'd think they'd put it in the assignment text.

	if (node == NULL) {
		return;
	}

	if (*root == NULL) {
		*root = node;
		//*root is a Node*, right? Yeah let's roll with that.
		return;
	}

	if (node->value < (*root)->value) {
		insert_node(&((*root)->left), node);
	} else {
		insert_node(&((*root)->right), node);
	}
}


// Searches for the number n in the tree rooted at root.
// Should return 1 if the number is present, and 0 if not.
int search(Node* root, int n){
	if (root == NULL)
		return 0;

	if (n == root->value)
		return 1;

	if (n < root->value) {
		return search(root->left, n);
	} else {
		return search (root->right, n);
	}
}


// Returns a dynamically allocated node, with all fields set to NULL/0
Node* create_blank_node(){
	Node* blank = (Node*)malloc(sizeof(Node));
	return blank;
}


// Builds a tree of all the numbers in an array
Node* create_tree(int* array, int size){
	Node* root = create_blank_node();
	root->value = array[0];
	for (int i = 1; i < size; i++) {
		Node* node = create_blank_node();
		node->value = array[i];
		insert_node(&root, node);
	}
	return root;
}


// Prints all the nodes of the tree.
void print_tree(Node* n, int offset){
	if (n == NULL) {
		return;
	}
	for (int i = 0; i < offset; i++) {
		printf(" ");
	}
	printf("%i\n", n->value);
	print_tree(n->left, offset+1);
	print_tree(n->right, offset+1);
}


// Computes x^2
double x_squared(double x){
    return x*x;
}


// Computes x^3
double x_cubed(double x){
    return x*x*x;
}


// Computes the definite integral of the function using the rectangle method
double integrate(double (*function)(double), double start, double end, double stepsize){
	return 0;
}


int main(int argc, char** argv){

    // Creates an array with random values
    int* array = create_random_array(10, 10);

    // Prints the values of the array, e.g:
    // 3 6 7 5 3 5 6 2 9 1
    print_array(array, 10);

    // Sorts the array
    sort(array, 10);

    // Prints the sorted array, e.g:
    // 1 2 3 3 5 5 6 6 7 9
    print_array(array, 10);

    // Create another random array
    int* new_array = create_random_array(10,10);

    // Print the second array
    print_array(new_array, 10);

    // Create a tree with the values in the new array
    Node* root = create_tree(new_array, 10);

    // Print the tree
    print_tree(root, 0);

    // Search for the values 3 and 11 in the tree
    // and print the results
    int found_3 = search(root, 3);
    int found_11 = search(root, 11);
    printf("%d, %d\n", found_3, found_11);

    // Integrate x^2 and x^3 from 0 to 1.
    // Should be approx 1/3 and 1/4.
    printf("%f\n", integrate(&x_squared, 0, 1, 0.001));
    printf("%f\n", integrate(&x_cubed, 0, 1, 0.001));
}




