/*

  FILE: crawler.c

  AUTHOR: Sang Jin Lee (for Campbell's CS23 W11)

  Description:

  Inputs: ./crawler [SEED URL] [TARGET DIRECTORY WHERE TO PUT THE DATA] [MAX CRAWLING DEPTH]

  Outputs: For each webpage crawled the crawler program will create a file in the 
  [TARGET DIRECTORY]. The name of the file will start a 1 for the  [SEED URL] 
  and be incremented for each subsequent HTML webpage crawled. 

  Each file will include the URL associated with the saved webpage and the
  depth of search in the file. The URL will be on the first line of the file 
  and the depth on the second line. The HTML for the webpage 
  will start on the third line.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crawler.h"
#include "html.h"
#include "hash.h"
#include "header.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Define the dict structure that holds the hash table 
// and the double linked list of DNODES. Each DNODE holds
// a pointer to a URLNODE. This list is used to store
// unique URLs. The search time for this list is O(n).
// To speed that up to O(1) we use the hash table. The
// hash table holds pointers into the list where 
// DNODES with the same key are maintained, assuming
// the hash(key) is not NULL (which implies the URL has
// not been seen before). The hash table provide quick
// access to the point in the list that is relevant
// to the current URL search. 

DICTIONARY* dict = NULL; 


// This is the table that keeps pointers to a list of URL extracted
// from the current HTML page. NULL pointer represents the end of the
// list of URLs.

// POINTER TO URL_LIST
char *url_list[MAX_URL_PER_PAGE]; 

int isDirectory(char *path) {
	struct stat buffer;
	if ((stat(path, &buffer) == 0))
		return 0;
	else
		return 1;
}

/*

(2) *initLists* Initialize any data structure and variables

*/

void initLists() {
	// malloc and initialize dictionary
	dict = (DICTIONARY*)calloc(1, sizeof(DICTIONARY));
	dict->start = dict->end = NULL;	
}


/*

(3) page = *getPage(seedURL, current_depth, target_directory)* Get HTML page

*/

char *getPage(char* url, int depth, char* path) {

	static int i = 1;
	
	char* PTR_wgetCom = (char *)calloc(1, SIZE_OF_WGET_COM);
	
	getcwd(path, 0);//store the current working dir in fullPath (automatic malloc).

	// print html to temp file.
	sprintf(PTR_wgetCom, "wget -qO - \"%s\" > %s", url,TEMP_FILE_NAME); // write wget to temp file.
	int j = 0;		
	if(system(PTR_wgetCom) != 0) { // if we can't get the URL...
		while ((j < 3) && (system(PTR_wgetCom) != 0)) { system(PTR_wgetCom); j++; } // ... try up to 3 times to get the page...
		if((system(PTR_wgetCom) != 0)) return NULL; //... and if it still fails, then return null
	}

	free(PTR_wgetCom); // free up the malloc'd space.

	// get html string from file and store in buffer
		FILE *fp;
		int size = 0;
		if((fp=fopen(TEMP_FILE_NAME, "rb")) == NULL)
			perror("File open");
		else {
			fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			rewind(fp);
		}
	
	// malloc buffer/temp
		char *buffer;
		buffer = (char *)calloc(1, sizeof(char) * size + 1);

	// read in html from file
		fread(buffer, 1, size, fp);
		//free(temp); // since we are done with the temp string, free up the memory =)
		fclose(fp);

	// write the depth/url and html to "path/i"
		FILE *writeFile;
		
		// filename to use for saving
		char fileNumber[10];
		
		// string that holds the url, depth, and buffer to write to the file in ./data
		char *writeToFile;
		// malloc the size of the buffer, url, and depth plus 2 for the line breaks
		writeToFile = (char *)calloc(1, strlen(buffer) + strlen(url) + sizeof(depth) + 2);

		sprintf(writeToFile, "%s\n%d\n%s",url,depth,buffer);
		
		// create variable to hold the full path (to the file to be written)
		char *fullPath;
                fullPath = (char *)calloc(1, strlen(path));

		// copy the directory data from path into fullPath...
		strncpy(fullPath, path, strlen(path));

		// ...and then add the file number onto the end
		sprintf(fileNumber, "/%d", i);
		strncat(fullPath, fileNumber, sizeof(path) + sizeof(fileNumber));

		// write the content of writetoFile  to "path/i"
		writeFile=fopen(fullPath, "w");
		
		if( writeFile == NULL ) {
			perror("File open");
		}
		else {
			fwrite(writeToFile, 1, strlen(writeToFile), writeFile);
		}

		// free space for fullPaht, writeTofile and close the writeFile
		free(fullPath);
		free(writeToFile);
		fclose(writeFile);

	// add \0 to end of string
		strncat(buffer, "\0", sizeof(char) * size + 1);
		
	// increment temp file number
	i++;

	// return the buffer (html)
	return buffer;
}

/*

(4) *extractURLs(page, SEED_URL)* Extract all URLs from SEED_URL page and update
    the URLsList accordingly.

*/

char **extractURLs(char* html_buffer, char* current) {
	
	// initialize url_list to NULL
	int i;
	for(i = 0; i <= MAX_URL_PER_PAGE; i++) {
		url_list[i] = NULL;
	}

	i = 0; // reset i back to 0

	// copy the current url as the first item in the url_list
    url_list[i] = calloc(1, MAX_URL_LENGTH);
	strncpy(url_list[i], current, MAX_URL_LENGTH);

	i++;

	// get the urls
	int pos = 0;
	char *result;
	result = (char *)calloc(1, MAX_URL_LENGTH);

	// get all of the links in the document (ignore things like pdf files, images, etc)
	while ((pos = GetNextURL(html_buffer, current, result, pos)) > 0) {

		// is the URL in the domain we specified?
		int is_in_domain = strncmp(URL_PREFIX, result, strlen(URL_PREFIX));

		if ( is_in_domain == 0) { 
			if(NormalizeURL(result) == 1) { // if URL has the correct prefix and is a pure text file...
				// malloc the url index
				url_list[i] = calloc(1, MAX_URL_LENGTH);

				// copy the parsed url to the url_list
				strncpy(url_list[i], result, MAX_URL_LENGTH);
				strncat(url_list[i], "\0", MAX_URL_LENGTH);

				i++;
			}
		} else {
			continue;
		}

		BZERO(result, MAX_URL_LENGTH);
	}

	free(result); // free up the memory for result.	
	
	return url_list;
}

/*
(5) *free(page)* Done with the page so release it

(6) *updateListLinkToBeVisited(URLsLists, current_depth + 1)*  For all the URL 
    in the URLsList that do not already exist already on the NODE list then 
    add a URLNODE (URL name, depth, visited) to the NODE list (start, end). 
*/

URLNODE *makeURLNODE(int depth, char* buffer) {
	// malloc space for the URLNODE
        URLNODE* n = calloc(1, sizeof(URLNODE));

	// set the depth and visited status of the node
        n->depth = depth;
        n->visited = 0;

	// BZERO the node url
        BZERO(n->url, MAX_URL_LENGTH);

	// copy the buffer (url) into the node.
        strncpy(n->url, buffer, MAX_URL_LENGTH);

	// return the URLNODE
	return n;
}

void addDNODE(int depth, char* url, int h) {

	if((dict->start) == NULL) { // if there are no dnodes in the list of dnodes
		// malloc the start and end nodes
		dict->start = dict->end = calloc(1, sizeof(DNODE));
		dict->start->prev = dict->start->next = NULL;

		// set the dnode as the starting dnode (since the list of dnodes is empty).
		dict->hash[h] = dict->start;
		dict->start->data = makeURLNODE(depth, url); // set the data to the URL node returned
		BZERO(dict->start->key, KEY_LENGTH);
		strncpy(dict->start->key, url, KEY_LENGTH);
	} else {
		// malloc the node
		dict->hash[h] = calloc(1, sizeof(DNODE));

		// place the dnode at the end of the list of dnodes
		//dict->hash[h] = dict->end;
		dict->hash[h]->prev = dict->end;
		dict->hash[h]->next = NULL;
		dict->end->next = dict->hash[h];
		dict->end = dict->hash[h]; // make the current node the last node (end).

		dict->hash[h]->data = makeURLNODE(depth, url); // set the data to the URL node returned by makeURLNODE
		BZERO(dict->hash[h]->key, KEY_LENGTH);
		strncpy(dict->hash[h]->key, url, KEY_LENGTH);
	}
}

int getHash(char* url) {
	return (int)(hash1(url) % (MAX_HASH_SLOT - 1));
}

void addClusterDNODE(int depth, char* url, DNODE* currentNode, int h) {

	/* UNCOMMENT PRINTING FOR COLLISION DETECTION INFORMATION */	

	//printf("Collision at currentURL:     %s\n", url);
	//printf("Current node is:             %s\n", currentNode->key);

	//printf("Hash value at currentURL is: %d\n", getHash(url));
	//printf("Hash value at current Node:  %d\n", getHash(currentNode->key));


	while(getHash(currentNode->key) == h) { // go to the last dnode in the cluster
			currentNode = currentNode->next;
	}

	// the while loop above puts us one value after the current cluster, so move back one.
	currentNode = currentNode->prev;

	// define/malloc new dnode to be initialized
	DNODE* dnodeToAdd;
	dnodeToAdd = malloc(sizeof(DNODE));
	MALLOC_CHECK(dnodeToAdd);

	// place the dnode at the end of the cluster
	// if the end of the cluser happens to be the end of the list of dnodes...
	if ((currentNode->next) == NULL) { // ...place the dnode at the end of the list of dnodes
                dnodeToAdd->prev = currentNode;
                dnodeToAdd->next = NULL;
                currentNode->next = dnodeToAdd;
                dict->end = dnodeToAdd;    // make the current node the last node (end).

                dnodeToAdd->data = makeURLNODE(depth, url); // set the data to the URL node returned by makeURLNODE
                BZERO(dnodeToAdd->key, KEY_LENGTH);
                strncpy(dnodeToAdd->key, url, KEY_LENGTH);
	} else {			  // otherwise just add it at the end of the cluster
		dnodeToAdd->prev = currentNode;
		currentNode->next->prev = dnodeToAdd;
		dnodeToAdd->next = currentNode->next;
		currentNode->next = dnodeToAdd;

		dnodeToAdd->data = makeURLNODE(depth, url);
		BZERO(dnodeToAdd->key, KEY_LENGTH);
		strncpy(dnodeToAdd->key, url, KEY_LENGTH);
	}
}


int uniqueURL(DNODE* currentNode, char* url, int h) {
	// initialize the string to store the URL
        char* currentURL = currentNode->key;

	while(getHash(currentURL) == h) {
		if(strcmp(currentURL, url) == 0) return 1;	// if the key in the current node is the url, then return 1
		currentNode = currentNode->next;		// go to the next dnode
		currentURL = currentNode->key;
	}
	return 0; // if we got to the end of the cluster without finding a match, return 0
}

void updateListLinkToBeVisited(char **url_list, int depth) {
	
	int i = 0;
	int h;	// store the hash

	// for every url in the list (until the url_list is null or until the predefined limit is reached)
	while((i < MAX_URL_PER_PAGE) && (url_list[i] != NULL)) {
		h = getHash(url_list[i]);	// get hash index of for the url

		if ((dict->hash[h]) == NULL) { // if the hash is empty...
			addDNODE(depth, url_list[i], h);	// then simply add a dnode!
		} else if (uniqueURL(dict->hash[h], url_list[i], h) == 0) // otherwise, if the URL is unique
			addClusterDNODE(depth, url_list[i], dict->hash[h], h);  // then add the dnode at the end of the cluster (grr...).
		i++;
	}
}


/*

(7) *setURLasVisited(SEED_URL)* Mark the current URL visited in the URLNODE.

*/
 
void setURLasVisited(char* url) {
	
	int h;
	h = getHash(url);
	DNODE* currentDNODE = dict->hash[h];

	while(strcmp((currentDNODE->key),url) != 0) {	// cycle through cluster until we get to the correct dnode
		currentDNODE = currentDNODE->next;
	}

	URLNODE* url_node = (URLNODE *)currentDNODE->data;

	url_node->visited = 1; // set the current url as visited
}

/*

(8) URLToBeVisited = *getAddressFromTheLinksToBeVisited(current_depth)* Get the next 
    URL to be visited from the NODE list (first one not visited from start)
*/

char *getAddressFromTheLinksToBeVisited(int *depth) {
	
	// starting from the fist dnode (set the currentDNODE as the starting dnode)
	DNODE* currentDNODE = dict->start;
	URLNODE* url_node = (URLNODE *)currentDNODE->data;

	while((url_node->visited) == 1) {	// iterate through the dnodes until we get to one that isn't visited
		if (currentDNODE->next == NULL) // if we get to the end, then return null
			return NULL;

		currentDNODE = currentDNODE->next;		// move the dnode down
		url_node = (URLNODE *)currentDNODE->data;	// update the url_node
	}

	*depth = url_node->depth; // update the currentDepth
	
	return currentDNODE->key;
}

void cleanUp() {

	/* Free up each element in the url_list */
	int i;
        for(i = 0; i <= MAX_URL_PER_PAGE; i++) {
                free(url_list[i]);
        }

	/* Free up the dnodes and their corresponding urlNode */
	DNODE* currentDNODE = dict->start->next;	   // we start on the second one because we free the previos node's data.

	while((currentDNODE->next) != NULL) {	
		free((URLNODE *)currentDNODE->prev->data); // free the previous node's urlNode.
		free(currentDNODE->prev);		   // free the previous node.
		currentDNODE = currentDNODE->next;
	}

	// Then, free the last node
	free((URLNODE *)currentDNODE->data);
	free(currentDNODE);

	/* Free dictionary */
	free(dict);

}

// argv[1] = SEED_URL
// argv[2] = DIRECTORY
// argv[3] = CRAWLING DEPTH

int main(int argc, char *argv[]) {
	
	int currentDepth = 0;
	int crawlDepth;	
	sscanf(argv[3], "%d", &crawlDepth);
	
	// MALLOC AND BZERO THE URL TO BE VISITED
	char *URLToBeVisited = calloc(1, MAX_URL_LENGTH);	// the url to visit

	URLToBeVisited = argv[1];
	
	/* CHECK ARGUMENTS! */
	// check the number of args
	if (argc != 4) { printf("Error! Usage: [SEED_URL] [TARGET_DIRECTORY] [CRAWLING_DEPTH]\n"); return 1; }

	int is_in_domain = strncmp(URL_PREFIX, argv[1], strlen(URL_PREFIX));

	if (is_in_domain != 0) { printf("Error! %s is not in domain %s\n", argv[1], URL_PREFIX); return 1; }
	
	// check each argument
	if (isDirectory(argv[2]) != 0) { printf("Error! %s is an invalid directory\n", argv[2]); return 1; }
	if (crawlDepth > MAXDEPTH) { printf("Error! The crawing depth you entered (%d) is greater than 4\n", crawlDepth); return 1; }

	/* INITIALIZE THE DICTIONARY (dict) */
	initLists();

	do {

		if (currentDepth > crawlDepth) {
			setURLasVisited(URLToBeVisited);
			continue;
		}
			
		
		/* GET HTML PAGE, SAVE TO TEMP FILE */ 
		page = getPage(URLToBeVisited, currentDepth, argv[2]);
		if(page == NULL) { // if the page can't be opened, set it as visited and move on
			setURLasVisited(URLToBeVisited);
			perror("Can't crawl source URL\n");
			continue;
		}
		
		/* EXTRACT URLS + UPDATE THE DNODES (ADD URLS) */
		updateListLinkToBeVisited((extractURLs(page, URLToBeVisited)), currentDepth + 1);
		
		// FREE ZE PAGE (TAG AND RELASE TO THE WILD)
		free(page);
		
		/* SET THE CURRENT URL AS VISITED */
		setURLasVisited(URLToBeVisited);
	
		printf("VISITING URL... %s\n", URLToBeVisited);
		
		/* SLEEP FOR PREDETERMINED INTERVAL */
		//sleep(INTERVAL_PER_FETCH);
	} while ((URLToBeVisited = getAddressFromTheLinksToBeVisited(&currentDepth)) != NULL)
	
	/* CLEAN UP */
	free(URLToBeVisited);
	cleanUp();
	
  return 0;
}
