#include <stdio.h>
#include <stdlib.h>

#define STBAN_SPLAYMAP_IMPLEMENTATION
#include "stban_splaymap.h"

void print_tree(stban_sm_tree_node *root, int indent){
  if(root == NULL)
    return;
  
  printf("|");
  for(int i = 0; i < indent; i++)
    printf("-");
  printf(">");
  printf("%s-%d\n",root->key,root->value);

  if(root->left != NULL)
    print_tree(root->left, indent + 1);
  if(root->right != NULL)
    print_tree(root->right, indent + 1);
}

int main(){
  stban_sm sm = {0};
  
  stban_sm_insert(&sm, "hello", 2);
  stban_sm_insert(&sm, "abc", 4);
  stban_sm_insert(&sm, "a", 1);
  stban_sm_insert(&sm, "world", 3);
  stban_sm_insert(&sm, "test", 4);
  stban_sm_insert(&sm, "key", 1);
  stban_sm_set(&sm, "hello", 3);
  
  print_tree(sm.root, 0);
  printf("\ntree size %d\n\n", sm.size);

  stban_sm_delete(&sm, "test", 0);
  stban_sm_delete(&sm, "key", 0);

  print_tree(sm.root, 0);
  printf("tree size %d\n\n", sm.size);

  stban_sm_clear(&sm);

  print_tree(sm.root, 0);
  printf("\ntree size %d\n\n", sm.size);

  stban_sm_insert(&sm, "hello", 2);
  stban_sm_insert(&sm, "abc", 4);
  stban_sm_insert(&sm, "a", 1);
  stban_sm_insert(&sm, "world", 3);
  stban_sm_insert(&sm, "test", 4);
  stban_sm_insert(&sm, "key", 1);

  print_tree(sm.root, 0);
  printf("\ntree size %d\n\n", sm.size);
  
  return 0;
}
