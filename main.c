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


int main() {

    BST_Node *root = NULL;
    int n, q;
    int i;

    scanf("%d", &n);

    for (i = 0; i < n; i++) {

        scanf("%d", &q);

        /* query 1: insert */
        if (q == 1) {
            char name[MAX_NAME + 1];
            char breed[MAX_NAME + 1];
            int charm;
            int traits[NUM_TRAITS];
            int depth = 0;    /* tracks insertion depth */
            int replaced = 0; /* -1 ignored, 0 new node, 1 replaced */

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

        /* query 2: delete by name */
        else if (q == 2) {
            char name[MAX_NAME + 1];
            scanf("%s", name);
            root = deleteNode(root, name);
            printf("Deletion Complete\n");
        }

        /* query 3: kth smallest */
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

        /* query 4: filter by trait */
        else if (q == 4) {
            int traitIndex, traitValue;
            int resultSize = 0;
            int j;

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

        /* query 5: remove all matching a trait */
        else if (q == 5) {
            int traitIndex, traitValue;
            scanf("%d %d", &traitIndex, &traitValue);

            int removedCount = removeByTrait(&root, traitIndex, traitValue);

            if (removedCount == 0)
                printf("NONE REMOVED\n");
            else
                printf("%d\n", removedCount);
        }

        /* query 6: inorder print */
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

char *makeStringExact(const char *src) {
    char *res = (char *)malloc((strlen(src) + 1) * sizeof(char));
    strcpy(res, src);
    return res;
}

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

BST_Node *createNode(Cat *newCat) {
    BST_Node *temp = (BST_Node *)malloc(sizeof(BST_Node));
    temp->cat = newCat;
    temp->left = NULL;
    temp->right = NULL;
    temp->subtree_size = 1;
    return temp;
}

void freeCat(Cat *cat) {
    if (cat == NULL) return;
    free(cat->name);
    free(cat->breed);
    free(cat);
}

void freeTree(BST_Node *root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    freeCat(root->cat);
    free(root);
}

int countTraits(Cat *cat) {
    int count = 0, i;
    for (i = 0; i < NUM_TRAITS; i++)
        if (cat->traits[i] == 1)
            count++;
    return count;
}

int getSubtreeSize(BST_Node *root) {
    if (root == NULL) return 0;
    return root->subtree_size;
}

BST_Node *insert(BST_Node *root, Cat *newCat, int *depth, int *replaced) {

    if (root == NULL)
        return createNode(newCat);

    int cmp = strcmp(newCat->name, root->cat->name);

    if (cmp == 0) {
        if (countTraits(newCat) > countTraits(root->cat)) {
            freeCat(root->cat);
            root->cat = newCat;
            *replaced = 1;
        }
        else {
            /* reject duplicate; -1 signals no output */
            freeCat(newCat);
            *replaced = -1;
        }
        return root;
    }

    (*depth)++;

    if (cmp < 0)
        root->left  = insert(root->left,  newCat, depth, replaced);
    else
        root->right = insert(root->right, newCat, depth, replaced);

    /* only count up if a new node was actually added */
    if (*replaced == 0)
        root->subtree_size = 1 + getSubtreeSize(root->left) + getSubtreeSize(root->right);

    return root;
}

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

BST_Node *minVal(BST_Node *root) {

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

BST_Node *deleteNode(BST_Node *root, char *name) {

    BST_Node *save_node, *new_del_node;
    char *save_name;

    if (root == NULL)
        return NULL;

    int cmp = strcmp(name, root->cat->name);

    if (cmp < 0) {
        root->left = deleteNode(root->left, name);
    }
    else if (cmp > 0) {
        root->right = deleteNode(root->right, name);
    }
    else {
        /* case 1: leaf */
        if (isLeaf(root)) {
            freeCat(root->cat);
            free(root);
            return NULL;
        }

        /* case 2a: only left child */
        if (hasOnlyLeftChild(root)) {
            save_node = root->left;
            freeCat(root->cat);
            free(root);
            return save_node;
        }

        /* case 2b: only right child */
        if (hasOnlyRightChild(root)) {
            save_node = root->right;
            freeCat(root->cat);
            free(root);
            return save_node;
        }

        /* case 3: two children - replace with in-order successor */
        new_del_node = minVal(root->right);
        save_name = makeStringExact(new_del_node->cat->name);

        freeCat(root->cat);
        root->cat = createCat(new_del_node->cat->name,
                               new_del_node->cat->breed,
                               new_del_node->cat->charm,
                               new_del_node->cat->traits);

        root->right = deleteNode(root->right, save_name);
        free(save_name);
    }

    root->subtree_size = 1 + getSubtreeSize(root->left) + getSubtreeSize(root->right);

    return root;
}

BST_Node *findKthSmallest(BST_Node *root, int k) {

    int leftSize = getSubtreeSize(root->left);

    if (k == leftSize + 1)
        return root;
    else if (k <= leftSize)
        return findKthSmallest(root->left, k);
    else
        return findKthSmallest(root->right, k - leftSize - 1);
}

void inorderPrint(BST_Node *root) {
    if (root == NULL) return;

    inorderPrint(root->left);
    printf("%s %d %d\n", root->cat->name, root->cat->charm, root->subtree_size);
    inorderPrint(root->right);
}

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

    if (idx == 0)
        result = (char **)realloc(result, sizeof(char *));
    else
        result = (char **)realloc(result, idx * sizeof(char *));

    return result;
}

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

int removeByTrait(BST_Node **root, int traitIndex, int traitValue) {

    int count = 0, i, totalNodes;
    char **names;

    if (*root == NULL) return 0;

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
