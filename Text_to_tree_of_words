#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define ADDSIZE 100
struct tree{
	char *word;
	int count;
	struct tree *left;
	struct tree *right;
};

//fuction to add new element to the tree  
struct tree *addelem ( struct tree *tr, char *w ){
	if (tr == NULL){
		tr = (struct tree*)malloc(sizeof(struct tree));
		tr->word = strdup(w);
		tr->count = 1;
		tr->left = NULL;
		tr->right = NULL;
	}
	else {
		int comp; // flag which shows if words are equal
		comp = strcmp (w, tr->word);
		if (comp == 0) 	tr->count ++;
		else if (comp > 0) tr->right = addelem(tr->right, w);
		else tr->left = addelem(tr->left, w);
	}
	
	return tr;		
}

//function to creat resulting tree (based on the frequency of words in the text)
struct tree *sortedtree (struct tree *sorted, char *w, int freq){
	if (sorted == NULL){
		sorted = (struct tree*)malloc(sizeof(struct tree));
		sorted->word = strdup(w);
		sorted->count = freq;
		sorted->left = NULL;
		sorted->right = NULL;
		
	}
	else{
		struct tree *p;
		if (freq == sorted->count){
			p = (struct tree*)malloc(sizeof(struct tree));
			p->word = strdup(w);
			p->count = freq;
			p->left = sorted->left;
			p->right = NULL;
			sorted->left = p;
		}
		else if (freq < sorted->count) sorted->left = sortedtree(sorted->left, w, freq);
		else sorted->right = sortedtree(sorted->right, w, freq); 
		
	}
		
	return sorted;
}

//fuction to trace all words in the initial tree 
struct tree *creatingtree (struct tree *tr, struct tree *initial){
	if (initial){
		
		tr = creatingtree(tr, initial->left);
		tr = sortedtree(tr, initial->word, initial->count);
		tr = creatingtree(tr, initial->right);
	
	}
	return tr;
}

//function to print tree 
void printtree (struct tree *tr, double total, FILE *out){
	if (tr != NULL){
		double freq;
		printtree(tr->right, total, out);
		freq = (double)(tr->count / total);
		fprintf(out, "%s %d %f \n", tr->word, tr->count, freq);
		printtree(tr->left, total, out);
	}
}

//function to delete the tree 
void deletetree(struct tree *tr){
	if (tr != NULL){
		deletetree(tr->right);
		deletetree(tr->left);
		free(tr->word);
		free(tr);
	}
}

int main(int argc, char **argv){
	double tcount = 0;
	int ch;
	char *inpword;
	int kinp = 0;
	int kout =0;
	struct tree *vocab;
	struct tree *sortedvocab;
	vocab = NULL;
	sortedvocab = NULL;
	FILE *inpsource = stdin;
	FILE *outsource = stdout;
	
	if (argc == 3){
		if (!strcmp(argv[1], "-i")) {
			inpsource = fopen(argv[2], "r");
			kinp++;
		}
		else if (!strcmp(argv[1], "-o")) {
			outsource = fopen(argv[2], "w");
			kout++;
		}
	}
	else if (argc == 5){
		inpsource = fopen(argv[2], "r");
		outsource = fopen(argv[4], "w");
		kinp=kout=1;
	}
				
	do{																	// reading words from the file and creating a tree
		inpword = malloc(0);
		int strlength = 0;
		int numstr = 0;
		int key = 0;
			
		while (isalpha(ch=getc(inpsource)) || isdigit(ch)){
			if (strlength == numstr ) {
				inpword = realloc(inpword, numstr = strlength + ADDSIZE);
			}
			inpword[strlength] = ch;
			strlength ++;
			key = 1;
		}
		
		if (key){
				inpword[strlength] = '\0';
				tcount++;							                    // means we found a word 
				vocab = addelem (vocab, inpword);
				free(inpword);
			}
			
		if (ispunct(ch)){
			tcount++;	
												// if we found a punctuation mark
			inpword = malloc(2);
			inpword[0] = ch;
			inpword[1] = '\0';
			vocab = addelem (vocab, inpword);
			free(inpword);
		}
		
	}
	while (ch != EOF);
	
	sortedvocab = creatingtree(sortedvocab, vocab); 					// sorting a tree 
	printtree (sortedvocab, tcount, outsource); 						// printing a tree 
	deletetree(vocab);													// deleting a tree
	deletetree(sortedvocab);
	if (kinp) fclose(inpsource);
	if (kout) fclose(outsource);
return 0;
}
