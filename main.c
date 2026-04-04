/* COP 3502C PA5
This program is written by: Richard Magiday */
/******************************************************************************
Richard Magiday
cop3502c_cmb_26
03/18/26
problem: CS1 PA4

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

/*==================== FUNCTION PROTOTYPES ====================*/

char *makeStringExact(const char *src);
Cat *createCat(char *name, char *breed, int charm, int traits[]);
BST_Node *createNode(Cat *newCat);
void freeCat(Cat *cat);
void freeTree(BST_Node *root);

int countTraits(Cat *cat);
int getSubtreeSize(BST_Node *root);

BST_Node *insert(BST_Node *root, Cat *newCat, int *depth, int *replaced);
BST_Node *deleteNode(BST_Node *root, char *name);
BST_Node *findNode(BST_Node *root, char *name);
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

/*==================== MAIN ====================*/

int main() {

    BST_Node *root = NULL; /* start with an empty tree */
    int n, q;
    int i;

    scanf("%d", &n); /* read total number of operations */

    for (i = 0; i < n; i++) {

        scanf("%d", &q); /* read which query type this operation is */

        /* query 1: insert a new cat into the leaderboard */
        if (q == 1) {
            char name[MAX_NAME + 1];
            char breed[MAX_NAME + 1];
            int charm;
            int traits[NUM_TRAITS];
            int depth = 0;    /* tracks how deep the node lands in the tree */
            int replaced = 0; /* set to 1 inside insert if a duplicate was found */

            /* read all cat fields from input */
            scanf("%s %s %d %d %d %d %d %d",
                  name, breed, &charm,
                  &traits[0], &traits[1], &traits[2], &traits[3], &traits[4]);

            Cat *newCat = createCat(name, breed, charm, traits); /* build the cat struct */
            root = insert(root, newCat, &depth, &replaced);      /* insert into BST */

            if (replaced == 1)
                printf("Replaced\n");           /* duplicate with strictly more traits */
            else if (replaced == 0)
                printf("Insert: %d\n", depth);  /* new node: print its depth */
            /* replaced == -1: duplicate ignored, no output */
        }

        /* query 2: remove a cat by name */
        else if (q == 2) {
            char name[MAX_NAME + 1];
            scanf("%s", name);
            root = deleteNode(root, name); /* delete if found, no-op if not */
            printf("Deletion Complete\n"); /* always print this regardless */
        }

        /* query 3: find the kth smallest cat alphabetically */
        else if (q == 3) {
            int k;
            scanf("%d", &k);

            /* check that k is a valid rank within the current tree size */
            if (root == NULL || k < 1 || k > getSubtreeSize(root)) {
                printf("NO SMALLEST ELEMENT FOUND\n");
            }
            else {
                BST_Node *ans = findKthSmallest(root, k); /* O(h) lookup using subtree_size */
                printf("%s %s %d\n", ans->cat->name, ans->cat->breed, ans->cat->charm);
            }
        }

        /* query 4: print all cats that match a given trait index and value */
        else if (q == 4) {
            int traitIndex, traitValue;
            int resultSize = 0; /* filterByTrait fills this with the match count */
            int j;

            scanf("%d %d", &traitIndex, &traitValue);

            /* returns a deep-copied, realloc'd array of matching names in alphabetical order */
            char **result = filterByTrait(root, traitIndex, traitValue, &resultSize);

            if (resultSize == 0) {
                printf("NONE FOUND\n");
            }
            else {
                printf("%s:", TRAIT_NAMES[traitIndex]); /* print trait label first */
                for (j = 0; j < resultSize; j++) {
                    printf(" %s", result[j]);
                    free(result[j]); /* free each deep-copied name after printing */
                }
                printf("\n");
            }

            free(result); /* free the outer array */
        }

        /* query 5: mass-delete all cats that match a given trait index and value */
        else if (q == 5) {
            int traitIndex, traitValue;
            scanf("%d %d", &traitIndex, &traitValue);

            /* two-phase: collect matching names, then delete each one */
            int removedCount = removeByTrait(&root, traitIndex, traitValue);

            if (removedCount == 0)
                printf("NONE REMOVED\n");
            else
                printf("%d\n", removedCount); /* print how many were removed */
        }

        /* query 6: print every node in alphabetical order with its subtree size */
        else if (q == 6) {
            if (root == NULL)
                printf("EMPTY\n");
            else
                inorderPrint(root); /* name, charm, subtree_size for each node */
        }
    }

    freeTree(root); /* free all remaining nodes and cat data before exit */
    return 0;
}

/*==================== STRING / CAT / NODE HELPERS ====================*/

/* allocate exact memory for a string and copy it */
char *makeStringExact(const char *src) {
    char *res = (char *)malloc((strlen(src) + 1) * sizeof(char));
    strcpy(res, src);
    return res;
}

/* allocate and fill a Cat struct */
Cat *createCat(char *name, char *breed, int charm, int traits[]) {
    Cat *temp = (Cat *)malloc(sizeof(Cat));
    int i;

    temp->name  = makeStringExact(name);
    temp->breed = makeStringExact(breed);
    temp->charm = charm;

    for (i = 0; i < NUM_TRAITS; i++)
        temp->traits[i] = traits[i];

    return temp;
}

/* allocate a BST node wrapping the given Cat */
BST_Node *createNode(Cat *newCat) {
    BST_Node *temp = (BST_Node *)malloc(sizeof(BST_Node));
    temp->cat          = newCat;
    temp->left         = NULL;
    temp->right        = NULL;
    temp->subtree_size = 1;
    return temp;
}

/* free the strings inside a Cat, then the Cat itself */
void freeCat(Cat *cat) {
    if (cat == NULL) return;
    free(cat->name);
    free(cat->breed);
    free(cat);
}

/* post-order free of entire tree */
void freeTree(BST_Node *root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    freeCat(root->cat);
    free(root);
}

/*==================== UTILITY HELPERS ====================*/

/* count how many traits are set to 1 */
int countTraits(Cat *cat) {
    int count = 0, i;
    for (i = 0; i < NUM_TRAITS; i++)
        if (cat->traits[i] == 1)
            count++;
    return count;
}

/* safe subtree size - returns 0 for NULL */
int getSubtreeSize(BST_Node *root) {
    if (root == NULL) return 0;
    return root->subtree_size;
}

/*==================== BST CORE OPERATIONS ====================*/

/*
 * insert - recursively insert newCat into the BST keyed by name.
 * depth tracks how deep the new node landed (root = 0).
 * replaced is set to 1 if an existing node's data was swapped out.
 * subtree_size is updated on the way back up.
 */
BST_Node *insert(BST_Node *root, Cat *newCat, int *depth, int *replaced) {

    /* empty spot - place the node here */
    if (root == NULL)
        return createNode(newCat);

    int cmp = strcmp(newCat->name, root->cat->name);

    if (cmp == 0) {
        /* duplicate name: replace only if strictly more traits */
        if (countTraits(newCat) > countTraits(root->cat)) {
            freeCat(root->cat);
            root->cat = newCat;
            *replaced = 1;
        }
        else {
            /* reject the new cat - free it; -1 means ignored (no output) */
            freeCat(newCat);
            *replaced = -1;
        }
        return root;
    }

    /* increment depth before going deeper */
    (*depth)++;

    if (cmp < 0)
        root->left  = insert(root->left,  newCat, depth, replaced);
    else
        root->right = insert(root->right, newCat, depth, replaced);

    /* update size only when a brand-new node was added (not replaced or ignored) */
    if (*replaced == 0)
        root->subtree_size = 1 + getSubtreeSize(root->left) + getSubtreeSize(root->right);

    return root;
}

/* standard BST search by name */
BST_Node *findNode(BST_Node *root, char *name) {

    if (root == NULL)
        return NULL;

    int cmp = strcmp(name, root->cat->name);

    if (cmp == 0)
        return root;
    else if (cmp < 0)
        return findNode(root->left, name);
    else
        return findNode(root->right, name);
}

/* return leftmost node in subtree (minimum alphabetically) */
BST_Node *minVal(BST_Node *root) {

    /* root stores the minimum */
    if (root->left == NULL)
        return root;

    return minVal(root->left);
}

int isLeaf(BST_Node *node) {
    return (node != NULL && node->left == NULL && node->right == NULL);
}

int hasOnlyLeftChild(BST_Node *node) {
    return (node != NULL && node->left != NULL && node->right == NULL);
}

int hasOnlyRightChild(BST_Node *node) {
    return (node != NULL && node->left == NULL && node->right != NULL);
}

/*
 * deleteNode - 3-case BST deletion matching lecture framework style.
 * Uses successor (min of right subtree) for two-child case.
 * If name is not found, returns root unchanged.
 * subtree_size is updated on the way back up.
 */
BST_Node *deleteNode(BST_Node *root, char *name) {

    BST_Node *save_node, *new_del_node;
    char *save_name;

    /* name not in tree - nothing to do */
    if (root == NULL)
        return NULL;

    int cmp = strcmp(name, root->cat->name);

    if (cmp < 0) {
        /* target is in left subtree */
        root->left = deleteNode(root->left, name);
    }
    else if (cmp > 0) {
        /* target is in right subtree */
        root->right = deleteNode(root->right, name);
    }
    else {
        /* found the node to delete */

        /* case 1: leaf node */
        if (isLeaf(root)) {
            freeCat(root->cat);
            free(root);
            return NULL;
        }

        /* case 2a: only a left child */
        if (hasOnlyLeftChild(root)) {
            save_node = root->left;
            freeCat(root->cat);
            free(root);
            return save_node;
        }

        /* case 2b: only a right child */
        if (hasOnlyRightChild(root)) {
            save_node = root->right;
            freeCat(root->cat);
            free(root);
            return save_node;
        }

        /* case 3: two children - use in-order successor (min of right subtree) */
        new_del_node = minVal(root->right);

        /* copy successor's cat data into this node */
        save_name = makeStringExact(new_del_node->cat->name);

        freeCat(root->cat);
        root->cat = createCat(new_del_node->cat->name,
                               new_del_node->cat->breed,
                               new_del_node->cat->charm,
                               new_del_node->cat->traits);

        /* now delete the successor from the right subtree */
        root->right = deleteNode(root->right, save_name);
        free(save_name);
    }

    /* update size on the way back up */
    root->subtree_size = 1 + getSubtreeSize(root->left) + getSubtreeSize(root->right);

    return root;
}

/*==================== QUERY HELPERS ====================*/

/*
 * findKthSmallest - O(h) using subtree_size.
 * leftSize = number of nodes in left subtree.
 * If k == leftSize + 1, current node is the answer.
 * If k <= leftSize, answer is in left subtree.
 * Otherwise answer is in right subtree with adjusted k.
 */
BST_Node *findKthSmallest(BST_Node *root, int k) {

    int leftSize = getSubtreeSize(root->left);

    if (k == leftSize + 1)
        return root;
    else if (k <= leftSize)
        return findKthSmallest(root->left, k);
    else
        return findKthSmallest(root->right, k - leftSize - 1);
}

/* inorder print: name, charm, subtree_size */
void inorderPrint(BST_Node *root) {
    if (root == NULL) return;

    inorderPrint(root->left);
    printf("%s %d %d\n", root->cat->name, root->cat->charm, root->subtree_size);
    inorderPrint(root->right);
}

/* collect deep-copied names of matching cats in alphabetical (inorder) order */
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

/*
 * filterByTrait - O(n)
 * malloc char** to tree size, deep copy matching names,
 * realloc down to exact result count before returning.
 */
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

    /* realloc to exact size - if 0 matches keep 1 slot to avoid malloc(0) */
    if (idx == 0)
        result = (char **)realloc(result, sizeof(char *));
    else
        result = (char **)realloc(result, idx * sizeof(char *));

    return result;
}

/* collect names of nodes to delete in phase 1 of removeByTrait */
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

/*
 * removeByTrait - O(n * h)
 * Phase 1: collect all matching names (O(n) traversal)
 * Phase 2: delete each by name (O(h) per delete)
 */
int removeByTrait(BST_Node **root, int traitIndex, int traitValue) {

    int count = 0, i, totalNodes;
    char **names;

    if (*root == NULL) return 0;

    totalNodes = getSubtreeSize(*root);
    names = (char **)malloc(totalNodes * sizeof(char *));

    /* phase 1: gather names */
    collectNamesToDelete(*root, traitIndex, traitValue, names, &count);

    /* phase 2: delete each */
    for (i = 0; i < count; i++) {
        *root = deleteNode(*root, names[i]);
        free(names[i]);
    }

    free(names);
    return count;
}