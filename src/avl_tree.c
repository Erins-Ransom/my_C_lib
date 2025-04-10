/*  
    @file avl_tree.c
 
    @version 1.0

    @brief An AVL Tree index that stores pointers (void *) provided a comparison 
    function (int (*cmp)(const void *, const void *)).

    DESCRIPTION:

    An AVL tree, named after it's inventors [1], is a self-balancing binary 
    search tree that maintains the following invariants. 

    For every node in the tree:
        (1) node->left->entry < node->entry < node->right->entry;
        (2) | node->left->height - node->right->height | < 2;

    Assuming these are maintained, an AVL tree with n entries will have a 
    height of O(log n).  Lookups, insertions, and deletions are all O(log n)
    operations since they are limited by the depth of the tree.  If an 
    insertion or deletion breaks invariant (2), it can only be along the 
    path from the root to the added or removed entry. The work required to 
    reestablish invariant (2) is also limited by the height of the tree. 
    
    This implimentation also maintains a doubly linked list over the entries
    in the tree for quick in-order traversal.
    
    [1] Adelson-Velsky, Georgy; Landis, Evgenii (1962). "An algorithm for the 
        organization of information". Proceedings of the USSR Academy of 
        Sciences (in Russian). 146: 263–266. English translation by Myron 
        J. Ricci in Soviet Mathematics - Doklady, 3:1259–1263, 1962.

    E-mail: erins.ransom@gmail.com

    LICENSE:
    Copyright (c) 2025 Erin Ransom

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "avl_tree.h"

#define max(X, Y) (((X) > (Y)) ? (X) : (Y))

typedef struct _avl_node
{
    void *data;
    size_t height;
    struct _avl_node *left;
    struct _avl_node *right;
    struct _avl_node *next;
    struct _avl_node *prev;
} avl_node;

/* 
    A Height-Balanced AVL Tree index that stores pointers (void *) and  
    also maintains a doubly linked list for iteration through entries. 
*/
struct _avl_tree
{
    avl_node *root;                 // Root of the AVL Tree
    avl_node *start;                // Begining of the linked list
    avl_node *end;                  // End of the linked list
    size_t  item_count;             // Current number of entries
    int (*cmp)(const void *, const void *);     // Comparision function, see below.

/*
    cmp() DESCRIPTION

        The cmp() function compares the data referenced by two pointers, 
        ptr1 and ptr2.

        cmp() returns an interger indicating the result of the comparison 
        as follows:

        - 0, if ptr1 and ptr2 have the same key;
        - a negative value if ptr1 has a key less than that of ptr2;
        - a positive value if ptr1 has a key greater than that of ptr2.
*/
};

/*
    An Iterator for accessing pointers stored in an AVL Tree.
*/
struct _avl_iter
{
    avl_tree *tree;     // Reference to the AVL Tree
    avl_node *cur;      // Reference to the current entry
};

/*
    Initializes a new AVL Tree that will use the provided comparison 
    function.  See the description of cmp() above.
*/
avl_tree *
new_avl_tree(int (*cmp)(const void *, const void *))
{
    avl_tree *new = (avl_tree *) malloc(sizeof(avl_tree));
    assert(new);
    new->start = NULL;
    new->end = NULL;
    new->item_count = 0;
    new->cmp = cmp;
    return new;
}

/*
    Returns an unsigned integer (size_t) indicating the number of entries
    stored in <tree>.
*/
size_t
avl_count(avl_tree *tree)
{
    return tree->item_count;
}

/*
    Returns an unsigned integer (size_t) indicating the current height of 
    <tree>.
*/
size_t
avl_height(avl_tree *tree)
{
    return tree->root->height;
}

/*
    An internal function that initizlizes and returns a reference
    to a new AVL tree entry containing <data>, with <prev> and <next>
    being references to its adjacent entries. New entries are allways
    added as leaf nodes, so their height is initialized to 1.
*/
avl_node *
_new_avl_node(void *data, avl_node *prev, avl_node *next)
{
    avl_node *new = (avl_node *) malloc(sizeof(avl_node));
    assert(new);
    new->data = data;
    new->prev = prev;
    new->next = next;
    new->height = 1;
    new->left = NULL;
    new->right = NULL;
    return new;
}

/*
    Returns the pointer (void *) stored in <tree> that matches <target>
    according to the tree's cmp function.  Returns NULL if no such 
    pointer is found. 
*/
void *
avl_get(avl_tree *tree, void *target)
{
    avl_node *cur = tree->root;
    int cmp = 1;
    while (cur)
    {
        cmp = tree->cmp(cur->data, target);
        if (cmp > 0)
            cur = cur->left;
        else if (cmp < 0)
            cur = cur->left;
        else
            break;
    }
    if (cur)
        return cur->data;
    else
        return NULL;
}

avl_node *
_avl_rotate_cw(avl_tree *tree, avl_node *ptr)
{
    size_t h_left, h_right;
    avl_node *pivot = ptr->left, *swing = pivot->right;

    pivot->right = ptr;
    ptr->left = swing;

    h_left = (ptr->left) ? ptr->left->height : 0;
    h_right = (ptr->right) ? ptr->right->height : 0;
    ptr->height = max(h_left, h_right) + 1;

    h_left = (pivot->left) ? pivot->left->height : 0;
    h_right = (pivot->right) ? pivot->right->height : 0;
    pivot->height = max(h_left, h_right) + 1;

    if (tree->root == ptr)
        tree->root = pivot;

    return pivot;
}

avl_node *
_avl_rotate_ccw(avl_tree *tree, avl_node *ptr)
{
    size_t h_left, h_right;
    avl_node *pivot = ptr->right, *swing = pivot->left;

    pivot->left = ptr;
    ptr->right = swing;

    h_left = (ptr->left) ? ptr->left->height : 0;
    h_right = (ptr->right) ? ptr->right->height : 0;
    ptr->height = max(h_left, h_right) + 1;

    h_left = (pivot->left) ? pivot->left->height : 0;
    h_right = (pivot->right) ? pivot->right->height : 0;
    pivot->height = max(h_left, h_right) + 1;

    if (tree->root == ptr)
        tree->root = pivot;

    return pivot;
}

/*
    Adds an entry for <data> to <tree>, maintaining that for all nodes in 
    <tree>:
    (1) node->left->data < node->data < node->right;
    (2) node->height = max(node->left->height, node->right->height) + 1;
    (3) node->prev->data < node->data < node->next->data.  
    
    If an entry matching <data> already exists, in <tree>, its contents are 
    overwritten with <data>.  

    Returns an unsigned integer (size_t) indicating the number of entries
    stored in the tree after this operation. 
*/
size_t
avl_put(avl_tree *tree, void *data)
{
    // We traverse the tree using a stack of node pointers and a trace 
    // of the path traversed (bit_trail) so it's easy to re-trace the 
    // path for rebalancing without re-comparing keys. 
    // stack[i] : current node
    // stack[i-1] : parent of current node
    avl_node *move_up = NULL, *stack[(tree->root) ? tree->root->height+2 : 2];
    unsigned int bit_trail = 0;
    int i = 1, j = 1, cmp = 1, h_right, h_left;

    // Traverse the tree until we find the target key or
    // an empty leaf node. 
    stack[0] = NULL;
    stack[1] = tree->root;
    while (stack[i])
    {
        cmp = tree->cmp(stack[i]->data, data);
        if (!cmp)
            break;
        i++;
        bit_trail <<= 1;
        if (cmp > 0)
            stack[i] = stack[i-1]->left;
        else
        {
            bit_trail |= 1;
            stack[i] = stack[i-1]->right;
        }
    }
    // If the key already exists, we update it's value and return 
    // the current entry count. 
    if (!cmp)
    {
        stack[i]->data = data;
        return tree->item_count;
    }
    // If we reach a leaf node, we add a new entry to the tree 
    // while maintaining our linked list. 
    if (!stack[i])
    {
        if (!stack[i-1])
        {   // Tree is empty, new entry is the root/start/end.
            tree->root = _new_avl_node(data, NULL, NULL);
            tree->start = tree->root;
            tree->end = tree->root;
        }
        else if (cmp > 0)
        { // New entry is the left child of the previous node.
            stack[i-1]->left = _new_avl_node(data, stack[i-1]->prev, stack[i-1]);
            if (stack[i-1]->prev)
                stack[i-1]->prev->next = stack[i-1]->left;
            else 
                tree->start = stack[i-1]->left;
            stack[i-1]->prev = stack[i-1]->left;
        }
        else
        { // New entry is the right child of the previous node.
            stack[i-1]->right = _new_avl_node(data, stack[i-1], stack[i-1]->next);
            if (stack[i-1]->next)
                stack[i-1]->next->prev = stack[i-1]->right;
            else
                tree->end = stack[i-1]->right;
            stack[i-1]->next = stack[i-1]->right;
        }
        // Increase the item count.
        tree->item_count++;
    }

    // Now we retrace our path, updating the hieght of each node 
    // and rebalance the tree if needed.
    i--;
    while (stack[i])
    { /* When rebalancing the tree we have 4 cases.  Let "a" be the 
         first unblanced node we reach re-tracing our path and let 
         "b" and "c" be the child and grandchild of "a" along the 
         path from the root to the new entry.  We then define our 
         cases by the path from "a" to "c":
         CASE 0: LEFT LEFT      (00)
         CASE 1: LEFT RIGHT     (01)
         CASE 2: RIGTH LEFT     (10)
         Case 3: RIGHT RIGHT    (11)                                */

        h_left = (stack[i]->left) ? stack[i]->left->height : 0;
        h_right = (stack[i]->right) ? stack[i]->right->height : 0;

        if (h_left - h_right > 1 && ((bit_trail >> j-2) & 3) == 0)
        { /* CASE 0:
                      a                          b
                     / \                       /   \
                    b   T4                    c     a
                   / \          ===>         / \   / \
                  c  T3       Clockwise     T1 T2 T3 T4
                 / \          
                T1 T2                                               */

            // Rotate Clockwise at "a"
            stack[i] = _avl_rotate_cw(tree, stack[i]);
            // Swing the parent pointer
            if (!stack[i-1])
                tree->root = stack[i];
            else if ((bit_trail >> j) & 1)
                stack[i-1]->right = stack[i];
            else
                stack[i-1]->left = stack[i];
        }
        else if (h_left - h_right > 1 && ((bit_trail >> j-2) & 3) == 1)
        { /* CASE 1:
                a                         a                         c
               / \                       / \                      /   \
              b  T4                     c   T4                   b     a
             / \         ===>          / \         ===>         / \   / \
            T1  c       Counter       b  T3      Clockwise     T1 T2 T3 T4
               / \     Clockwise     / \          
              T2 T3                 T1 T2                                     */
            
            // Rotate Counter Clocwise at "b"
            stack[i]->left = _avl_rotate_ccw(tree, stack[i]->left);
            // Rotate Clockwise at "a"
            stack[i] = _avl_rotate_cw(tree, stack[i]);
            // Swing the parent pointer
            if (!stack[i-1])
                tree->root = stack[i];
            else if ((bit_trail >> j) & 1)
                stack[i-1]->right = stack[i];
            else
                stack[i-1]->left = stack[i];
        }
        else if (h_left - h_right < -1 && ((bit_trail >> j-2) & 3) == 2)
        { /* CASE 2:
                a                     a                             c
               / \                   / \                          /   \
              T1  b                 T1  c                        a     b
                 / \     ===>          / \         ===>         / \   / \
                c  T4  Clockwise      T2  b       Counter      T1 T2 T3 T4
               / \                       / \     Clockwise               
              T2 T3                     T3 T4                             */
            
            // Rotate Clockwise at "b" 
            stack[i]->right = _avl_rotate_cw(tree, stack[i]->right);
            // Rotate Counter Clockwise at "a"
            stack[i] = _avl_rotate_ccw(tree, stack[i]);
            // Swing the parent pointer
            if (!stack[i-1])
                tree->root = stack[i];
            else if ((bit_trail >> j) & 1)
                stack[i-1]->right = stack[i];
            else
                stack[i-1]->left = stack[i];
        }
        else if (h_left - h_right < -1 && ((bit_trail >> j-2) & 3) == 3)
        { /* CASE 3:
                a                             b
               / \                          /   \
              T1  b                        a     c
                 / \         ===>         / \   / \
                T2  c       Counter      T1 T2 T3 T4
                   / \     Clockwise               
                  T3 T4                                                  */
            
            // Rotate Counter Clockwise at "a"
            stack[i] = _avl_rotate_ccw(tree, stack[i]);
            // Swing the parent pointer
            if (!stack[i-1])
                tree->root = stack[i];
            else if ((bit_trail >> j) & 1)
                stack[i-1]->right = stack[i];
            else
                stack[i-1]->left = stack[i];
        }
        else
        {
            cmp = stack[i]->height;
            // Update the height of the current node
            stack[i]->height = max(h_left, h_right) + 1;
            cmp -= stack[i]->height;
            if (cmp == 0)
            {   // If the height of the current node does not change, we are done;
                // otherwise, continue up the tree.
                break;
            }
        }
        i--;
        j++;    
    }
    return tree->item_count;
}

/*
    Removes the entry in <tree> that matches <target>, if one exists, 
    maintaining that for each node in <tree>:
    (1) node->left->data < node->data < node->right;
    (2) node->height = max(node->left->height, node->right->height) + 1;
    (3) node->prev->data < node->data < node->next->data. 

    Returns an unsigned integer (size_t) indicating the number of entries
    stored in the tree after this operation. 
*/
size_t
avl_delete(avl_tree *tree, void *target)
{   // We traverse the tree using a stack of node pointers and a trace 
    // of the path traversed (bit_trail) so it's easy to re-trace the 
    // path for rebalancing without re-comparing keys. 
    // stack[i] : current node
    // stack[i-1] : parent of current node
    avl_node *stack[(tree->root) ? tree->root->height+2 : 2];
    unsigned int bit_trail = 0;
    int i = 1, j = 1, cmp = 1, h_right, h_left, target_index;



    // Traverse the tree until we find the target key or
    // an empty leaf node. 
    stack[0] = NULL;
    stack[1] = tree->root;
    while (stack[i])
    {
        cmp = tree->cmp(stack[i]->data, target);
        if (!cmp)
            break;
        i++;
        bit_trail <<= 1;
        if (cmp > 0)
            stack[i] = stack[i-1]->left;
        else
        {
            bit_trail |= 1;
            stack[i] = stack[i-1]->right;
        }
    }
    // If we reach a NULL pointer, the target does not exist in the tree;
    if (!stack[i])
    {
        return tree->item_count;
    }
    // If the target exists and is not already a leaf node,
    // we move nodes up along the longest path to a leaf node
    // from the target. Once we have moved a leaf node, we retrace 
    // the path to the root and rebalance the tree as needed. 
    else if (!cmp)
    {
        target_index = i;

        // First, we remove <target> from the linked list
        if (stack[i]->next)
            stack[i]->next->prev = stack[i]->prev;
        else
            tree->end = stack[i]->prev;
        if (stack[i]->prev)
            stack[i]->prev->next = stack[i]->next;
        else
            tree->start = stack[i]->next;

        // If <target> has a single child, we will overwrite <target> with 
        // its only child.
        if (!(stack[i]->left && stack[i]->right) && stack[i]->height > 1)
        {
            bit_trail <<= 1;
            if (stack[i]->left)
            {
                stack[++i] = stack[i]->left;
            }
            else 
            {
                bit_trail |= 1;
                stack[++i] = stack[i]->right;
            }
        }
        else if (stack[i]->height > 1)
        // Otherwise, we find the predecessor of <target> to overwrite it with.
        {   
            stack[++i] = stack[i]->left; 
            bit_trail <<= 1;
            while (stack[i]->right)
            {
                stack[++i] = stack[i]->right;
                bit_trail = (bit_trail << 1) | 1;
            }
        }

        // If <target> is not a leaf node, we overwrite it with <stack[i]>. 
        if (target_index != i) 
        {
            stack[target_index]->data = stack[i]->data;
            stack[target_index]->next = stack[i]->next;
            stack[target_index]->prev = stack[i]->prev;
            if (stack[i]->prev)
                stack[i]->prev->next = stack[target_index];
            else
                tree->start = stack[target_index];
            if (stack[i]->next)
                stack[i]->next->prev = stack[target_index];
            else
                tree->end = stack[target_index];
        }

        // Swing the parent pointer and free the node.
        if (!stack[i-1])
            tree->root = NULL;
        else if (bit_trail & 1)
            stack[i-1]->right = NULL;
        else
            stack[i-1]->left = NULL;
        free(stack[i]);
        tree->item_count--;

        

        // Now we retrace our path, updating the hieght of each node 
        // and rebalance the tree if needed.
        i--;
        while (stack[i])
        { /* When rebalancing the tree we have 4 cases.  Let "a" be the 
            first unblanced node we reach re-tracing our path and let 
            "b" and "c" be the child and grandchild of "a" along the 
            path from the root to the new entry.  We then define our 
            cases by the path from "a" to "c":
            CASE 0: LEFT LEFT      (00)
            CASE 1: LEFT RIGHT     (01)
            CASE 2: RIGTH LEFT     (10)
            Case 3: RIGHT RIGHT    (11)                                */

            h_left = (stack[i]->left) ? stack[i]->left->height : 0;
            h_right = (stack[i]->right) ? stack[i]->right->height : 0;

            if (h_left - h_right > 1 && ((bit_trail >> j-2) & 3) == 0)
            { /* CASE 0:
                         a                          b
                        / \                       /   \
                       b   T4                    c     a
                      / \          ===>         / \   / \
                     c  T3       Clockwise     T1 T2 T3 T4
                    / \          
                   T1 T2                                               */

                // Rotate Clockwise at "a"
                stack[i] = _avl_rotate_cw(tree, stack[i]);
                // Swing the parent pointer
                if (!stack[i-1])
                    tree->root = stack[i];
                else if ((bit_trail >> j) & 1)
                    stack[i-1]->right = stack[i];
                else
                    stack[i-1]->left = stack[i];
            }
            else if (h_left - h_right > 1 && ((bit_trail >> j-2) & 3) == 1)
            { /* CASE 1:
                    a                         a                         c
                   / \                       / \                      /   \
                  b  T4                     c   T4                   b     a
                 / \         ===>          / \         ===>         / \   / \
                T1  c       Counter       b  T3      Clockwise     T1 T2 T3 T4
                   / \     Clockwise     / \          
                  T2 T3                 T1 T2                                     */
                
                // Rotate Counter Clocwise at "b"
                stack[i]->left = _avl_rotate_ccw(tree, stack[i]->left);
                // Rotate Clockwise at "a"
                stack[i] = _avl_rotate_cw(tree, stack[i]);
                // Swing the parent pointer
                if (!stack[i-1])
                    tree->root = stack[i];
                else if ((bit_trail >> j) & 1)
                    stack[i-1]->right = stack[i];
                else
                    stack[i-1]->left = stack[i];
            }
            else if (h_left - h_right < -1 && ((bit_trail >> j-2) & 3) == 2)
            { /* CASE 2:
                    a                     a                             c
                   / \                   / \                          /   \
                  T1  b                 T1  c                        a     b
                     / \     ===>          / \         ===>         / \   / \
                    c  T4  Clockwise      T2  b       Counter      T1 T2 T3 T4
                   / \                       / \     Clockwise               
                  T2 T3                     T3 T4                             */
                
                // Rotate Clockwise at "b" 
                stack[i]->right = _avl_rotate_cw(tree, stack[i]->right);
                // Rotate Counter Clockwise at "a"
                stack[i] = _avl_rotate_ccw(tree, stack[i]);
                // Swing the parent pointer
                if (!stack[i-1])
                    tree->root = stack[i];
                else if ((bit_trail >> j) & 1)
                    stack[i-1]->right = stack[i];
                else
                    stack[i-1]->left = stack[i];
            }
            else if (h_left - h_right < -1 && ((bit_trail >> j-2) & 3) == 3)
            { /* CASE 3:
                    a                             b
                   / \                          /   \
                  T1  b                        a     c
                     / \         ===>         / \   / \
                    T2  c       Counter      T1 T2 T3 T4
                       / \     Clockwise               
                      T3 T4                                                  */
                
                // Rotate Counter Clockwise at "a"
                stack[i] = _avl_rotate_ccw(tree, stack[i]);
                // Swing the parent pointer
                if (!stack[i-1])
                    tree->root = stack[i];
                else if ((bit_trail >> j) & 1)
                    stack[i-1]->right = stack[i];
                else
                    stack[i-1]->left = stack[i];
            }
            else
            {
                cmp = stack[i]->height;
                // Update the height of the current node
                stack[i]->height = max(h_left, h_right) + 1;
                cmp -= stack[i]->height;
                if (cmp == 0)
                {   // If the height of the current node does not change, we are done;
                    // otherwise, continue up the tree.
                    break;
                }
            }
            i--;
            j++;    
        } 
    }
    return tree->item_count;   
}

/*
    Initializes and returns a pointer to a new iterator for <tree>.  The 
    iterator will be initialized pointing to the first entry stored in 
    <tree>.
*/
avl_iter *
new_avl_iter(avl_tree *tree)
{
    avl_iter *new = (avl_iter *) malloc(sizeof(avl_iter));
    new->tree = tree;
    new->cur = tree->start;
    return new;
}

/*
    Moves <iter> to the entry that matches <target>, if it exists.  The 
    current entry will be set to NULL if no match is found. 
*/
void
avl_iter_seek(avl_iter *iter, void *target)
{
    iter->cur = avl_get(iter->tree, target);
}

/*
    Moves <iter> to its first entry.
*/
void
avl_iter_seek_start(avl_iter *iter)
{
    iter->cur = iter->tree->start;
}

/*
    Moves <iter> to its last entry.
*/
void
avl_iter_seek_end(avl_iter *iter)
{
    iter->cur = iter->tree->end;
}

/*
    Advances <iter> to its next entry. The current entry will be set to 
    NULL if <iter> is currently set to its last entry. 
*/
void
avl_iter_next(avl_iter *iter)
{
    if (iter->cur)
        iter->cur = iter->cur->next;
}

/*
    Moves <iter> to its previous entry. The current entry will be set to 
    NULL if <iter> is currently set to its first entry. 
*/
void 
avl_iter_prev(avl_iter *iter)
{
    if (iter->cur)
        iter->cur = iter->cur->prev;
}

/*
    Returns the data pointer (void *) for the current entry of <iter>. The 
    return value will be NULL if <iter> has been set a non-existing entry. 
*/
void *
avl_iter_data(avl_iter *iter)
{
    if (iter->cur)
        return iter->cur->data;
    else
        return NULL;
}

/* 
    Runs a check on <tree> asserting that for every node in <tree>:
    (1) node->left->data < node->data < node->right;
    (2) node->height = max(node->left->height, node->right->height) + 1;
    (3) node->prev->data < node->data < node->next->data. 
*/
void 
avl_validate(avl_tree *tree)
{
    avl_iter *iter;
    avl_node *ptr;
    avl_node **stack;
    void *prev_entry;
    int i = 1;
    int h_left;
    int h_right;

    // If the tree is empty, verify it has been initialized.
    if (!tree->root)
    {
        assert(!tree->start);
        assert(!tree->end);
        return;
    }

    // We do a breadth first traversal of the tree using a stack of node
    // pointers, checking invariants (1) & (2) for each entry.  
    stack = (avl_node **) malloc(sizeof(avl_node *) * tree->root->height + 2);
    stack[0] = NULL;
    stack[1] = tree->root;

    // While the stack is not empty
    while (stack[i])
    {
        // Pop the top of our stack 
        ptr = stack[i--];
        h_left = (ptr->left) ? ptr->left->height : 0;
        h_right = (ptr->right) ? ptr->right->height : 0;
        // Check (2)
        if (ptr->height != max(h_left, h_right) + 1)
        {
            fprintf(stdout, "Invalid Tree: Unbalanced\n\t");
            exit(1);
        }
        // Check (1) for each child and add them to the stack. 
        if (h_right)
        {
            if (tree->cmp(ptr->data, ptr->right->data) >= 0)
            {
                fprintf(stdout, "Invalid Tree: Bad Right Child");
                exit(1);
            }
            assert(tree->cmp(ptr->data, ptr->right->data) < 0);
            stack[++i] = ptr->right;
        }
        if (h_left)
        {
            if (tree->cmp(ptr->data, ptr->left->data) <= 0)
            {
                fprintf(stdout, "Invalid Tree: Bad Left Child");
                exit(1);
            }
            assert(tree->cmp(ptr->data, ptr->left->data) > 0);
            stack[++i] = ptr->left;
        }   
    }
    // Validate (4) going forwards.
    iter = new_avl_iter(tree);
    while (prev_entry = avl_iter_data(iter))
    {
        avl_iter_next(iter);
        if (avl_iter_data(iter))
            assert(tree->cmp(avl_iter_data(iter), prev_entry) > 0);
    }
    // Validate (4) going backwards.
    avl_iter_seek_end(iter);
    while (prev_entry = avl_iter_data(iter))
    {
        avl_iter_prev(iter);
        if (avl_iter_data(iter))
            assert(tree->cmp(avl_iter_data(iter), prev_entry) < 0);
    }
    free(stack);
}

/*
    Prints each entry in <tree> in order using <print_data>.
*/
void print_avl_tree(avl_tree *tree, void (*print_data)(void *data, FILE *out))
{
    char *buff;
    avl_iter *iter;

    if (!tree)
    {
        fprintf(stdout,"Tree has not been initialized.\n");
        return;
    }
    if (!tree->item_count)
    {
        fprintf(stdout, "Tree is empty.\n");
        return;
    }

    buff = (char *) malloc(tree->root->height*2+2);
    for (int i=0; i<tree->root->height*2; i++)
        buff[i] = '-';
    
    buff[tree->root->height*2] = '>';
    buff[tree->root->height*2+1] = 0;

    fprintf(stdout, "AVL Tree:\n");
    iter = new_avl_iter(tree);
    while (avl_iter_data(iter))
    {
        fprintf(stdout, "%s", buff + iter->cur->height * 2);
        print_data(avl_iter_data(iter), stdout);
        fprintf(stdout, "\n");
        avl_iter_next(iter);
    }

    free(buff);
    free(iter);
}