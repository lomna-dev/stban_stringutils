#ifndef STBAN_SPLAYMAP_INCLUDED_H
#define STBAN_SPLAYMAP_INCLUDED_H

#include <stdlib.h>

/* #include <assert.h> */
/* #define STBAN_SM_ASSERT(X) assert(X) */

/* This assert causes segfault to read stack trace
 * Don't use this on systems with no segfaults 
 * or in systems where NULL can be written to
 */
#define STBAN_SM_ASSERT(X) do{if(!(X)) ((char*)NULL)[0] = 0; }while(0)

#define STBAN_SM_KEY_TYPE char*
#define STBAN_SM_VALUE_TYPE int
/* #define STBAN_SM_KEY_CMP_FUNC(X,Y) ((X) - (Y)) */
#include <string.h>
#define STBAN_SM_KEY_CMP_FUNC(X,Y) (strcmp((X),(Y)))

typedef struct stban_sm_tree_node stban_sm_tree_node;
typedef struct stban_sm stban_sm;

struct stban_sm_tree_node
{
  STBAN_SM_KEY_TYPE key;
  STBAN_SM_VALUE_TYPE value;
  stban_sm_tree_node *left;
  stban_sm_tree_node *right;
};

struct stban_sm
{
  size_t size;
  size_t threshold;
  stban_sm_tree_node *root;
};

void stban_sm_insert(stban_sm *map, STBAN_SM_KEY_TYPE key, STBAN_SM_VALUE_TYPE value);
/* get function will splay the key which was accessed */
/* view function will not splay the key and is better to use if you want to avoid a splay operation */
STBAN_SM_VALUE_TYPE stban_sm_get(stban_sm *map, STBAN_SM_KEY_TYPE key, STBAN_SM_VALUE_TYPE return_on_fail);
STBAN_SM_VALUE_TYPE stban_sm_view(stban_sm *map, STBAN_SM_KEY_TYPE key, STBAN_SM_VALUE_TYPE return_on_fail);
/* if return 0, then key was not found */
int stban_sm_set(stban_sm *map, STBAN_SM_KEY_TYPE key, STBAN_SM_VALUE_TYPE value);
int stban_sm_splay(stban_sm *map, STBAN_SM_KEY_TYPE key, int force_splay);
/* returns value of key that was deleted */
STBAN_SM_VALUE_TYPE stban_sm_delete(stban_sm *map, STBAN_SM_KEY_TYPE key, STBAN_SM_VALUE_TYPE return_on_fail);
void stban_sm_clear(stban_sm *map);

void stban_sm_left_rotate(stban_sm_tree_node **node);
void stban_sm_right_rotate(stban_sm_tree_node **node);

#endif // STBAN_SPLAYMAP_INCLUDED_H

#ifdef STBAN_SPLAYMAP_IMPLEMENTATION

static size_t stban_sm_insert_recursive(stban_sm_tree_node **current,
					stban_sm_tree_node *node_to_insert,
					size_t depth, size_t threshold);
static size_t stban_sm_splay_recursive(stban_sm_tree_node **current,
				       STBAN_SM_KEY_TYPE key,
				       size_t depth, size_t threshold);
static void stban_sm_clear_recursive(stban_sm_tree_node **node);

void stban_sm_left_rotate(stban_sm_tree_node **node)
{
  if(node[0]->right == NULL)
    STBAN_SM_ASSERT(0 && "right child should not be NULL for left rotate");
  stban_sm_tree_node *child = node[0]->right;
  stban_sm_tree_node *beta = node[0]->right->left;
  node[0]->right->left = node[0];
  node[0]->right = beta;
  node[0] = child;
}

void stban_sm_right_rotate(stban_sm_tree_node **node)
{
  if(node[0]->left == NULL)
    STBAN_SM_ASSERT(0 && "left child should not be NULL for right rotate");
  stban_sm_tree_node *child = node[0]->left;
  stban_sm_tree_node *beta = node[0]->left->right;
  node[0]->left->right = node[0];
  node[0]->left = beta;
  node[0] = child;
}

static size_t stban_sm_splay_recursive(stban_sm_tree_node **current,
				       STBAN_SM_KEY_TYPE key,
				       size_t depth, size_t threshold)
{
  depth++;
  
  if(STBAN_SM_KEY_CMP_FUNC(current[0]->key, key) == 0)
    return depth;
  else if(STBAN_SM_KEY_CMP_FUNC(current[0]->key, key) > 0)
    {
      if(current[0]->left == NULL)
	return 0;
      else
	depth = stban_sm_splay_recursive(&(current[0]->left), key,
					 depth, threshold);

      if(depth > threshold)
	stban_sm_right_rotate(current);
    }
  else
    {
      if(current[0]->right == NULL)
	return 0;
      else
	depth = stban_sm_splay_recursive(&(current[0]->right), key,
					 depth, threshold);

      if(depth > threshold)
	stban_sm_left_rotate(current);
    }

  return depth;
}

static size_t stban_sm_insert_recursive(stban_sm_tree_node **current,
					stban_sm_tree_node *node_to_insert,
					size_t depth, size_t threshold)
{
  depth++;
  
  if(STBAN_SM_KEY_CMP_FUNC(current[0]->key, node_to_insert->key) == 0)
    STBAN_SM_ASSERT(0 && "Key already exists in the map");
  else if(STBAN_SM_KEY_CMP_FUNC(current[0]->key, node_to_insert->key) > 0)
    {
      if(current[0]->left == NULL)
	{
	  current[0]->left = node_to_insert;
	  if(depth > threshold)
	    stban_sm_right_rotate(current);
	}
      else
	{
	  depth = stban_sm_insert_recursive(&(current[0]->left),
					    node_to_insert, depth, threshold);
	  if(depth > threshold)
	    stban_sm_right_rotate(current);
	}
    }
  else
    {
    
      if(current[0]->right == NULL)
	{
	  current[0]->right = node_to_insert;
	  if(depth > threshold)
	    stban_sm_left_rotate(current);
	}
      else
	{
	  depth = stban_sm_insert_recursive(&(current[0]->right),
					    node_to_insert, depth, threshold);
	  if(depth > threshold)
	    stban_sm_left_rotate(current);
	}
    }

  return depth;
}

int stban_sm_splay(stban_sm *map, STBAN_SM_KEY_TYPE key, int force_splay)
{
  size_t success;

  if(map->root == NULL)
    return 0;
  
  if(force_splay)
    success = stban_sm_splay_recursive(&(map->root), key, 0, -1);
  else
    success = stban_sm_splay_recursive(&(map->root), key, 0, map->threshold);
  return !!success;
}

int stban_sm_set(stban_sm *map, STBAN_SM_KEY_TYPE key,
		 STBAN_SM_VALUE_TYPE value)
{
  int found_node = stban_sm_splay(map, key, 0);
  if(!found_node)
    return found_node;

  stban_sm_tree_node *current = map->root;
  while(current != NULL)
    {
      if(STBAN_SM_KEY_CMP_FUNC(current->key, key) == 0)
	{
	  current->value = value;
	  return found_node;
	}
      else if(STBAN_SM_KEY_CMP_FUNC(current->key, key) > 0)
	current = current->left;
      else
	current = current->right;
    }

  return found_node;
}

STBAN_SM_VALUE_TYPE stban_sm_view(stban_sm *map, STBAN_SM_KEY_TYPE key,
				  STBAN_SM_VALUE_TYPE return_on_fail)

{
  if(map->root == NULL)
    return return_on_fail;

  stban_sm_tree_node *current = map->root;
  while(current != NULL)
    {
      if(STBAN_SM_KEY_CMP_FUNC(current->key, key) == 0)
	return current->value;
      else if(STBAN_SM_KEY_CMP_FUNC(current->key, key) > 0)
	current = current->left;
      else
	current = current->right;
    }

  return return_on_fail;
}

STBAN_SM_VALUE_TYPE stban_sm_get(stban_sm *map, STBAN_SM_KEY_TYPE key,
				 STBAN_SM_VALUE_TYPE return_on_fail)
{
  int found_node = stban_sm_splay(map, key, 0);
  if(!found_node)
    return return_on_fail;

  stban_sm_tree_node *current = map->root;
  while(current != NULL)
    {
      if(STBAN_SM_KEY_CMP_FUNC(current->key, key) == 0)
	return current->value;
      else if(STBAN_SM_KEY_CMP_FUNC(current->key, key) > 0)
	current = current->left;
      else
	current = current->right;
    }

  return return_on_fail;
}

STBAN_SM_VALUE_TYPE stban_sm_delete(stban_sm *map, STBAN_SM_KEY_TYPE key,
				    STBAN_SM_VALUE_TYPE return_on_fail)
{
  if(map->root == NULL)
    return return_on_fail;

  stban_sm_tree_node **current = &(map->root);
  while(current[0] != NULL)
    {
      if(STBAN_SM_KEY_CMP_FUNC(current[0]->key, key) == 0)
	break;
      else if(STBAN_SM_KEY_CMP_FUNC(current[0]->key, key) > 0)
	current = &(current[0]->left);
      else
	current = &(current[0]->right);
    }

  if(current[0] == NULL)
    return return_on_fail;
  
  STBAN_SM_VALUE_TYPE return_value = current[0]->value;
  stban_sm_tree_node *to_free = current[0];
  if(current[0]->left == NULL)
    {
      current[0] = current[0]->right;
      free(to_free);
    }
  else if(current[0]->right == NULL)
    {
      current[0] = current[0]->left;
      free(to_free);
    }
  else
    {
      stban_sm_tree_node **successor = &(current[0]->right);
      while(successor[0]->left != NULL)
	successor = &(successor[0]->left);
      to_free = successor[0];
      current[0]->key = successor[0]->key;
      current[0]->value = successor[0]->value;
      successor[0] = successor[0]->right;
      free(to_free);
    }

  map->size -= 1;
  return return_value;
}

void stban_sm_insert(stban_sm *map, STBAN_SM_KEY_TYPE key,
		     STBAN_SM_VALUE_TYPE value)
{
  stban_sm_tree_node *node = malloc(sizeof(stban_sm_tree_node));
  if(node == NULL)
    STBAN_SM_ASSERT(0 && "malloc failed : buy more ram");
  node->key = key;
  node->value = value;
  node->left = NULL;
  node->right = NULL;

  if(map->root == NULL)
    map->root = node;
  else
    stban_sm_insert_recursive(&(map->root), node, 0, map->threshold);
  map->size += 1;
}

static void stban_sm_clear_recursive(stban_sm_tree_node **node)
{
  if(node[0] == NULL)
    return;

  if(node[0]->left != NULL)
    stban_sm_clear_recursive(&(node[0]->left));
  
  if(node[0]->right != NULL)
    stban_sm_clear_recursive(&(node[0]->right));

  node[0]->left = NULL;
  node[0]->right = NULL;

  free(node[0]);
  node[0] = NULL;
}

void stban_sm_clear(stban_sm *map)
{
  stban_sm_clear_recursive(&(map->root));
  map->size = 0;
  map->root = NULL;
}

#endif // STBAN_SPLAYMAP_IMPLEMENTATION
