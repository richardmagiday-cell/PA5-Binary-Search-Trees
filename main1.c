/******************************************************************************
Richard Magiday
cop3502c_cmb_26
03/18/26
problem: CS1 PA5

cd PA5
gcc main.c
Get-Content in1.txt | .\a.exe > out_test1.txt
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 25
#define NUM_TRAITS 5

typedef struct Cat {
    char *name;
    char *breed;
    int charm;
    int traits[NUM_TRAITS];
} Cat;

typedef struct BST_Node {
    Cat *cat;
    struct BST_Node *left;
    struct BST_Node *right;
    int subtree_size;
} BST_Node;

const char *TRAIT_NAMES[NUM_TRAITS] = {
    "friendly", "grumpy", "playful", "lazy", "curious"
};

char *makeStringExact(const char *src);
Cat *createCat(char *name, char *breed, int charm, int traits[]);
BST_Node *createNode(Cat *newCat);
void freeCat(Cat *cat);
void freeTree(BST_Node *root);

int countTraits(Cat *cat);
int getSubtreeSize(BST_Node *root);
void recomputeSizes(BST_Node *root);

BST_Node *insert(BST_Node *root, Cat *newCat, int *depth, int *replaced);
BST_Node *deleteNode(BST_Node *root, char *name);
BST_Node *findNode(BST_Node *root, char *name);
BST_Node *parent(BST_Node *root, BST_Node *node);
BST_Node *minVal(BST_Node *root);
int isLeaf(BST_Node *node);
int hasOnlyLeftChild(BST_Node *node);
int hasOnlyRightChild(BST_Node *node);

BST_Node *findKthSmallest(BST_Node *root, int k);
void inorderPrint(BST_Node *root);
void collectTraitMatches(BST_Node *root, int traitIndex, int traitValue,
                         char **result, int *idx);
char **filterByTrait(BST_Node *root, int traitIndex, int traitValue, int *resultSize);
void collectNamesToDelete(BST_Node *root, int traitIndex, int traitValue,
                          char **names, int *count);
int removeByTrait(BST_Node **root, int traitIndex, int traitValue);


int main() {

    BST_Node *root = NULL;
    int n, q, i;

    scanf("%d", &n);

    for (i = 0; i < n; i++) {

        scanf("%d", &q);

        // Query 1: insert a new cat into the tree.
        if (q == 1) {
            char name[MAX_NAME + 1];
            char breed[MAX_NAME + 1];
            int charm;
            int traits[NUM_TRAITS];
            int depth = 0;
            int replaced = 0; // -1 = ignored, 0 = new node, 1 = replaced

            scanf("%s %s %d %d %d %d %d %d",
                  name, breed, &charm,
                  &traits[0], &traits[1], &traits[2], &traits[3], &traits[4]);

            Cat *newCat = createCat(name, breed, charm, traits);
            root = insert(root, newCat, &depth, &replaced);

            if (replaced == 1)
                printf("Replaced\n");
            else if (replaced == 0)
                printf("Insert: %d\n", depth);
        }

        // Query 2: delete a cat by name.
        else if (q == 2) {
            char name[MAX_NAME + 1];
            scanf("%s", name);
            root = deleteNode(root, name);
            printf("Deletion Complete\n");
        }

        // Query 3: find the kth smallest cat alphabetically.
        else if (q == 3) {
            int k;
            scanf("%d", &k);

            if (root == NULL || k < 1 || k > getSubtreeSize(root)) {
                printf("NO SMALLEST ELEMENT FOUND\n");
            }
            else {
                BST_Node *ans = findKthSmallest(root, k);
                printf("%s %s %d\n", ans->cat->name, ans->cat->breed, ans->cat->charm);
            }
        }

        // Query 4: print all cats matching a trait index and value.
        else if (q == 4) {
            int traitIndex, traitValue, resultSize, j;
            resultSize = 0;

            scanf("%d %d", &traitIndex, &traitValue);

            char **result = filterByTrait(root, traitIndex, traitValue, &resultSize);

            if (resultSize == 0) {
                printf("NONE FOUND\n");
            }
            else {
                printf("%s:", TRAIT_NAMES[traitIndex]);
                for (j = 0; j < resultSize; j++) {
                    printf(" %s", result[j]);
                    free(result[j]);
                }
                printf("\n");
            }

            free(result);
        }

        // Query 5: remove all cats matching a trait index and value.
        else if (q == 5) {
            int traitIndex, traitValue, removedCount;
            scanf("%d %d", &traitIndex, &traitValue);

            removedCount = removeByTrait(&root, traitIndex, traitValue);

            if (removedCount == 0)
                printf("NONE REMOVED\n");
            else
                printf("%d\n", removedCount);
        }

        // Query 6: print every node in alphabetical order.
        else if (q == 6) {
            if (root == NULL)
                printf("EMPTY\n");
            else
                inorderPrint(root);
        }
    }

    freeTree(root);
    return 0;
}

// Allocates exactly enough memory for src and copies it.
char *makeStringExact(const char *src) {
    char *res = (char *)malloc((strlen(src) + 1) * sizeof(char));
    strcpy(res, src);
    return res;
}

// Allocates and fills a Cat struct with the given fields.
Cat *createCat(char *name, char *breed, int charm, int traits[]) {
    Cat *temp = (Cat *)malloc(sizeof(Cat));
    int i;

    temp->name = makeStringExact(name);
    temp->breed = makeStringExact(breed);
    temp->charm = charm;

    for (i = 0; i < NUM_TRAITS; i++)
        temp->traits[i] = traits[i];

    return temp;
}

// Allocates a BST node wrapping the given cat pointer.
BST_Node *createNode(Cat *newCat) {
    BST_Node *temp = (BST_Node *)malloc(sizeof(BST_Node));
    temp->cat = newCat;
    temp->left = NULL;
    temp->right = NULL;
    temp->subtree_size = 1;
    return temp;
}

// Frees the strings inside a Cat, then the Cat itself.
void freeCat(Cat *cat) {
    if (cat == NULL) return;
    free(cat->name);
    free(cat->breed);
    free(cat);
}

// Frees every node and cat in the tree using a postorder traversal.
void freeTree(BST_Node *root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    freeCat(root->cat);
    free(root);
}

// Returns how many of a cat's traits are set to 1.
int countTraits(Cat *cat) {
    int count = 0, i;
    for (i = 0; i < NUM_TRAITS; i++)
        if (cat->traits[i] == 1)
            count++;
    return count;
}

// Returns the subtree size stored in root, or 0 if root is NULL.
int getSubtreeSize(BST_Node *root) {
    if (root == NULL) return 0;
    return root->subtree_size;
}

// Recomputes subtree_size for every node via a postorder traversal.
// Called after parent-pointer deletions that cannot update sizes on the way up.
void recomputeSizes(BST_Node *root) {
    if (root == NULL) return;
    recomputeSizes(root->left);
    recomputeSizes(root->right);
    root->subtree_size = 1 + getSubtreeSize(root->left) + getSubtreeSize(root->right);
}

// Inserts newCat into the BST rooted at root. Tracks insertion depth in
// *depth and sets *replaced to 1 if a duplicate was swapped out, or -1
// if the duplicate was rejected. Returns the updated root.
BST_Node *insert(BST_Node *root, Cat *newCat, int *depth, int *replaced) {

    int cmp;

    // Inserting into an empty tree.
    if (root == NULL)
        return createNode(newCat);

    cmp = strcmp(newCat->name, root->cat->name);

    // Duplicate name: replace only if the new cat has strictly more traits.
    if (cmp == 0) {
        if (countTraits(newCat) > countTraits(root->cat)) {
            freeCat(root->cat);
            root->cat = newCat;
            *replaced = 1;
        }
        else {
            freeCat(newCat);
            *replaced = -1; // rejected; no output for this case
        }
        return root;
    }

    (*depth)++;

    // Element should be inserted to the right.
    if (cmp > 0) {

        // There is a right subtree to insert the node.
        if (root->right != NULL)
            root->right = insert(root->right, newCat, depth, replaced);

        // Place the node directly to the right of root.
        else
            root->right = createNode(newCat);
    }

    // Element should be inserted to the left.
    else {

        // There is a left subtree to insert the node.
        if (root->left != NULL)
            root->left = insert(root->left, newCat, depth, replaced);

        // Place the node directly to the left of root.
        else
            root->left = createNode(newCat);
    }

    // Only update size if a new node was actually added.
    if (*replaced == 0)
        root->subtree_size = 1 + getSubtreeSize(root->left) + getSubtreeSize(root->right);

    return root;
}

// Returns a pointer to the node whose cat has the given name, or NULL
// if no such node exists in the subtree rooted at current_ptr.
BST_Node *findNode(BST_Node *current_ptr, char *name) {

    // Check if there are nodes in the tree.
    if (current_ptr != NULL) {

        // Found the name at the root.
        if (strcmp(name, current_ptr->cat->name) == 0)
            return current_ptr;

        // Search to the left.
        if (strcmp(name, current_ptr->cat->name) < 0)
            return findNode(current_ptr->left, name);

        // Or search to the right.
        else
            return findNode(current_ptr->right, name);
    }
    else
        return NULL; // No node found.
}

// Returns the parent of the node pointed to by node in the tree rooted at
// root. If the node is the root of the tree, or the node doesn't exist in
// the tree, NULL will be returned.
BST_Node *parent(BST_Node *root, BST_Node *node) {

    // Take care of NULL cases.
    if (root == NULL || root == node)
        return NULL;

    // The root is the direct parent of node.
    if (root->left == node || root->right == node)
        return root;

    // Look for node's parent in the left side of the tree.
    if (strcmp(node->cat->name, root->cat->name) < 0)
        return parent(root->left, node);

    // Look for node's parent in the right side of the tree.
    else if (strcmp(node->cat->name, root->cat->name) > 0)
        return parent(root->right, node);

    return NULL; // Catch any other extraneous cases.
}

// Returns a pointer to the node storing the minimum value in the tree
// with the root, root. Will not work if called with an empty tree.
BST_Node *minVal(BST_Node *root) {

    // Root stores the minimal value.
    if (root->left == NULL)
        return root;

    // The left subtree of the root stores the minimal value.
    else
        return minVal(root->left);
}

// Returns 1 if node is a leaf node, 0 otherwise.
int isLeaf(BST_Node *node) {
    return (node->left == NULL && node->right == NULL);
}

// Returns 1 iff node has a left child and no right child.
int hasOnlyLeftChild(BST_Node *node) {
    return (node->left != NULL && node->right == NULL);
}

// Returns 1 iff node has a right child and no left child.
int hasOnlyRightChild(BST_Node *node) {
    return (node->left == NULL && node->right != NULL);
}

// Will delete the node storing name in the tree rooted at root. The
// function returns a pointer to the root of the resulting tree.
BST_Node *deleteNode(BST_Node *root, char *name) {

    BST_Node *delnode, *new_del_node, *save_node;
    BST_Node *par;
    char *save_name;
    Cat *save_cat;

    delnode = findNode(root, name); // Get a pointer to the node to delete.

    // Name not in tree - nothing to do.
    if (delnode == NULL)
        return root;

    par = parent(root, delnode); // Get the parent of this node.

    // Take care of the case where the node to delete is a leaf node.
    if (isLeaf(delnode)) {

        // Deleting the only node in the tree.
        if (par == NULL) {
            freeCat(root->cat);
            free(root);
            return NULL;
        }

        // Deletes the node if it's a left child.
        if (strcmp(name, par->cat->name) < 0) {
            freeCat(par->left->cat);
            free(par->left);
            par->left = NULL;
        }

        // Deletes the node if it's a right child.
        else {
            freeCat(par->right->cat);
            free(par->right);
            par->right = NULL;
        }

        recomputeSizes(root);
        return root;
    }

    // Take care of the case where the node to be deleted only has a left child.
    if (hasOnlyLeftChild(delnode)) {

        // Deleting the root node of the tree.
        if (par == NULL) {
            save_node = delnode->left;
            freeCat(delnode->cat);
            free(delnode);
            return save_node;
        }

        // Deletes the node if it's a left child.
        if (strcmp(name, par->cat->name) < 0) {
            save_node = par->left;
            par->left = par->left->left;
            freeCat(save_node->cat);
            free(save_node);
        }

        // Deletes the node if it's a right child.
        else {
            save_node = par->right;
            par->right = par->right->left;
            freeCat(save_node->cat);
            free(save_node);
        }

        recomputeSizes(root);
        return root;
    }

    // Takes care of the case where the deleted node only has a right child.
    if (hasOnlyRightChild(delnode)) {

        // Node to delete is the root node.
        if (par == NULL) {
            save_node = delnode->right;
            freeCat(delnode->cat);
            free(delnode);
            return save_node;
        }

        // Deletes the node if it's a left child.
        if (strcmp(name, par->cat->name) < 0) {
            save_node = par->left;
            par->left = par->left->right;
            freeCat(save_node->cat);
            free(save_node);
        }

        // Deletes the node if it's a right child.
        else {
            save_node = par->right;
            par->right = par->right->right;
            freeCat(save_node->cat);
            free(save_node);
        }

        recomputeSizes(root);
        return root;
    }

    // If your code reaches here it means delnode has two children.
    // Find the new physical node to delete (in-order successor).
    new_del_node = minVal(delnode->right);
    save_name = makeStringExact(new_del_node->cat->name);
    save_cat = createCat(new_del_node->cat->name,
                         new_del_node->cat->breed,
                         new_del_node->cat->charm,
                         new_del_node->cat->traits);

    deleteNode(root, save_name); // Now delete the successor.

    // Restore the data to the original node to be deleted.
    freeCat(delnode->cat);
    delnode->cat = save_cat;

    free(save_name);
    return root;
}

// Returns a pointer to the node with the kth smallest name in the subtree
// rooted at root. Uses stored subtree sizes to find it in O(log n).
BST_Node *findKthSmallest(BST_Node *root, int k) {

    int numNodesLeft = getSubtreeSize(root->left);

    if (numNodesLeft >= k)
        return findKthSmallest(root->left, k);
    else if (numNodesLeft == k - 1)
        return root;
    else
        return findKthSmallest(root->right, k - numNodesLeft - 1);
}

// Prints every node in the subtree rooted at root in alphabetical order.
void inorderPrint(BST_Node *root) {
    if (root == NULL) return;

    inorderPrint(root->left);
    printf("%s %d %d\n", root->cat->name, root->cat->charm, root->subtree_size);
    inorderPrint(root->right);
}

// Traverses the subtree rooted at root in order, appending the name of
// each cat whose traits[traitIndex] equals traitValue to result.
void collectTraitMatches(BST_Node *root, int traitIndex, int traitValue,
                         char **result, int *idx) {

    if (root == NULL) return;

    collectTraitMatches(root->left, traitIndex, traitValue, result, idx);

    if (root->cat->traits[traitIndex] == traitValue) {
        result[*idx] = makeStringExact(root->cat->name);
        (*idx)++;
    }

    collectTraitMatches(root->right, traitIndex, traitValue, result, idx);
}

// Returns a heap-allocated array of cat names whose traits[traitIndex]
// equals traitValue. Sets *resultSize to the number of matches found.
char **filterByTrait(BST_Node *root, int traitIndex, int traitValue, int *resultSize) {

    int idx = 0;
    char **result;

    *resultSize = 0;

    if (root == NULL) {
        result = (char **)malloc(sizeof(char *));
        return result;
    }

    result = (char **)malloc(getSubtreeSize(root) * sizeof(char *));

    collectTraitMatches(root, traitIndex, traitValue, result, &idx);

    *resultSize = idx;

    // Shrink the array down to the actual number of matches.
    if (idx == 0)
        result = (char **)realloc(result, sizeof(char *));
    else
        result = (char **)realloc(result, idx * sizeof(char *));

    return result;
}

// Traverses the subtree in order and copies the name of each cat whose
// traits[traitIndex] equals traitValue into names.
void collectNamesToDelete(BST_Node *root, int traitIndex, int traitValue,
                          char **names, int *count) {

    if (root == NULL) return;

    collectNamesToDelete(root->left, traitIndex, traitValue, names, count);

    if (root->cat->traits[traitIndex] == traitValue) {
        names[*count] = makeStringExact(root->cat->name);
        (*count)++;
    }

    collectNamesToDelete(root->right, traitIndex, traitValue, names, count);
}

// Deletes all cats whose traits[traitIndex] equals traitValue. Returns
// the number of cats removed.
int removeByTrait(BST_Node **root, int traitIndex, int traitValue) {

    int count = 0, i, totalNodes;
    char **names;

    if (*root == NULL) return 0;

    // Collect matching names first so tree traversal isn't affected by deletions.
    totalNodes = getSubtreeSize(*root);
    names = (char **)malloc(totalNodes * sizeof(char *));

    collectNamesToDelete(*root, traitIndex, traitValue, names, &count);

    for (i = 0; i < count; i++) {
        *root = deleteNode(*root, names[i]);
        free(names[i]);
    }

    free(names);
    return count;
}
