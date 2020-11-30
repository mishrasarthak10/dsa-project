#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define N_CHARS 27

unsigned int totalcount = 0;
typedef struct trienode trienode;
struct trienode
{
  trienode *children[27];
  unsigned int count;
};
void addWord(const char *word, trienode *root);
int indexOf(char c);
trienode * countTextFile(const char *path);
unsigned int getCount(const char *word, const   trienode *root);
void printCounts(FILE *stream, trienode *root);
void printNode(FILE *stream, const trienode *node, char c, int depth);
void freeTree(trienode *root);
typedef struct Text
{
  unsigned long length;
  char *data;
  char *current_word;
} Text;
Text getText(const char *path);
void freeText(Text *text);
void preprocess(Text *text);
const char * getNextWord(Text *text);
char lower(char c);
bool isAlpha(char c);
bool isApostrophe(char c);
Text getText(const char *path)
{
  Text text;
  FILE *in_file = NULL;
  size_t bytes_read = 0;
  text.length       = 0ul;
  text.data         = NULL;
  text.current_word = NULL;
  if (!(in_file = fopen(path, "rb")))
  {
    fprintf(stderr, "Couldn't open %s!", path);
    exit(2);
  }
  fseek(in_file, 0, SEEK_END);
  text.length = ftell(in_file);
  if ( !(text.data = calloc(text.length + 1, sizeof(*text.data))) )
  {
    fclose(in_file);
    fprintf(stderr, "Couldn't allocate memory (%zd bytes)!",
            sizeof(*text.data) * text.length + 1);
    exit(3);
  }
  fseek(in_file, 0, SEEK_SET);
  bytes_read = fread(text.data, 1, text.length, in_file);
  fclose(in_file);
  if (bytes_read != text.length)
  {
    if (text.data)
      free(text.data);
    fprintf(stderr, "Failed to read expected number of chars!\n");
    exit(4);
  }
  text.current_word = text.data;
  return text;
}
void freeText(Text *text)
{
  if (text->data)
  {
    free(text->data);
    text->data = NULL;
    text->current_word = NULL;
    text->length = 0;
  }
}
void preprocess(Text *text)
{
  unsigned long char_index;
  char previous = '\0';
  char current  = '\0';
  char next = text->data[0];
  for (char_index = 0ul; char_index < text->length; ++char_index)
  {
    previous = current;
    current = next;
    next = text->data[char_index+1];
    if (isAlpha(current))
    {
      text->data[char_index] = lower(current);
    }
    else if (isAlpha(previous) && isAlpha(next) && current == '\'')
    {
      continue;
    }
    else
    {
      text->data[char_index] = '\0';
    }
  }
}
const char * getNextWord(Text *text)
{
  const char *word = NULL;
  while ((unsigned long)(text->current_word - text->data) < text->length &&
         *text->current_word == '\0')
  {
    ++text->current_word;
  }
  word = text->current_word;
  while ((unsigned long)(text->current_word - text->data) < text->length &&
         *text->current_word != '\0')
  {
    ++text->current_word;
  }
  if (word < (text->data + text->length))
    return word;
  else
    return NULL;
}
bool isAlpha(char c)
{
  return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}
char lower(char c)
{
  return c | 32;
}
void addWord(const char *word, trienode *root)
{
  int character;
  trienode *node = root;
  
  if (word != NULL)
  {
    for (character = 0; word[character] != '\0'; ++character)
    {
      if (node->children[indexOf(word[character])] != NULL)
        node = node->children[indexOf(word[character])]; 
      else
        node = node->children[indexOf(word[character])] = calloc(1, sizeof(trienode));  
    }
    ++(node->count);
    ++totalcount;
  }
}
int indexOf(char c)
{
  return c == '\'' ? 26 : c - 'a';
}
unsigned int getCount(const char *word, const trienode *root)
{
  int character;
  const trienode *node = root;
  for (character = 0; word[character] != '\0'; ++character)
  {
    if (node->children[indexOf(word[character])] != NULL)
      return 0;  
    else
      node = node->children[indexOf(word[character])]; 
  }
  return node->count;
}
trienode * countTextFile(const char *path)
{
  const char *word = NULL;
  Text text = getText(path);
  
  preprocess(&text);
  trienode *count_tree = calloc(1, sizeof(trienode));
  
  while ((word = getNextWord(&text)) != NULL) addWord(word, count_tree);
 
  freeText(&text);

  return count_tree;
}
const int n_chars = N_CHARS;
const char characters[N_CHARS] = "abcdefghijklmnopqrstuvwxyz\'";
void printCounts(FILE *stream, trienode *root)
{
  int i;
  for (i = 0; i < n_chars; ++i)
  {
    if (root->children[i] != NULL)
    {
      printNode(stream, root->children[i], characters[i], 0);
    } 
  }
}
void printNode(FILE *stream, const trienode *node, char c, int depth)
{
  static char stack[64];
  stack[depth] = c;
  if (node->count)
    printf("%25s | %d\n", stack, node->count);
  int i;
  for (i = 0; i < n_chars; ++i)
  {
    if (node->children[i] != NULL)
      printNode(stream, node->children[i], characters[i], depth+1);
  }
  stack[depth] = '\0';
}
void freeTree(trienode *root)
{
  int i;
  for (i = 0; i < n_chars; ++i)
  {
    if (root->children[i] != NULL)
    {
      freeTree(root->children[i]);
    }
  }
  free(root);
}

int main(int argc, char const *argv[])
{
  if (argc != 2)
  {
    printf("Usage: %s <text file>\n", argv[0]);
    return 1;
  }	

  trienode *counts_root = countTextFile(argv[1]);
  printCounts(stdout, counts_root);
  printf("Total no of words : ""%d", totalcount);
  freeTree(counts_root);
  return 0;
}