#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <stdlib.h>


int cacheSize=0;
int blockSize=0;
int assoc=0;
int replace=0;

int noOfBlocks=0;
int sets=0;
int setSize=0;

int tagBits=0;
int indexBits=0;
int offSet=0;

int cacheAccesses=0;
int readAccesses=0;
int writeAccesses=0;

int cacheMisses=0;
int compMisses=0;
int capMisses=0;
int confMisses=0;

int readMisses=0;
int writeMisses=0;
int dirtyBlocksEvic=0;


bool* convertHexToBinary(std::string s){
    bool* binaryForm = new bool[32];

    int i=0;
    int j=0;
    
    if(s[j] == '0' && s[j] == 'x') j=2;

    while(i<32){

        int digit = 0;
        if(s[j] >= '0' && s[j] <= '9') digit = ((int)s[j]) - 48;
        if(s[j] >= 'A' && s[j] <= 'F') digit = ((int)s[j]) - 65 + 10;
        if(s[j] >= 'a' && s[j] <= 'f') digit = ((int)s[j]) - 97 + 10;

        for(int k = 3; k>=0; k--){
            binaryForm[i+k] = digit%2;
            digit = digit/2;
        }

        i+=4;
        j++;
    }

    return binaryForm;
}

int convertBinaryToInt(bool *b, int low, int high){
    int integer=0;
    int multiplier = 1;
    for(int i=high; i>=low; i--){
    	integer += b[i]*multiplier;
    	multiplier*=2;
    }
    return integer;
}



bool binarySearch(int *A, int size, int n){
    if(size==0) return 0;

    int low = 1;
    int high = size;

    while(low<=high){
        int mid = low + (high - low)/2;
        if(A[mid] == n) return 1;
        if(A[mid] > n) high = mid-1;
        if(A[mid] < n) low = mid+1; 
    }

    return 0;
}

int* insertElement(int *A, int size, int n){
    int* B = new int[size+1];\
    bool inserted=0;

    for(int i=0; i<size+1; i++){
        if(!inserted && n<=A[i]){
            inserted = 1;
            B[i] = n;
        }

        else B[i] = A[i - inserted];
    } 

    return B;
}




struct cacheBlock{
    int tag;
    int validBit;
    int dirtyBit;
    struct cacheBlock* next;
};


bool searchS0(struct cacheBlock* A, int T){
    struct cacheBlock* temp = A->next;
    while(temp != NULL){
        if(temp->tag == T && temp->validBit==1) return 1;
        temp = temp->next;
    }

    return 0;
}

void LRU1(struct cacheBlock* A, int T, int request){
    struct cacheBlock* temp = A->next;
    struct cacheBlock* prev = A;
    while(temp != NULL){
        if(temp->tag == T){
            struct cacheBlock* S = new struct cacheBlock;
            S->tag = T;
            S->validBit = 1;
            S->dirtyBit = temp->dirtyBit;
            S->next = NULL;

            if(request == 1 && S->dirtyBit == 1){
             	dirtyBlocksEvic++;
             	std::cout << "1" << std::endl;
             }
            if(request == 1) S->dirtyBit = 1;
            
            if(prev == A){
                A->next = A->next->next;
                S->next = A->next;
                A->next = S;
            }

            else{
                prev->next = temp->next;
                S->next = A->next;
                A->next = S;
            }

            break;
        }

        prev=temp;
        temp=temp->next;
    }
}


void insertCacheBlock(int place, struct cacheBlock* A, int T, int request, int* valids, int indexS){

    struct cacheBlock* temp = A;
    int i=0;

    struct cacheBlock* S = new struct cacheBlock;
    S->tag = T;
    S->validBit = 1;
    S->dirtyBit = request;
    S->next=NULL;

    if(place == 0){
        S->next = A->next;
        A->next = S;
    }

    else{    
        while(temp->next!=NULL && i<place){
            temp=temp->next;
            i++;
        }

        S->next = temp->next;
        temp->next = S->next;
    }
    
    valids[indexS]++;

	temp = A;
	i=0;
	while(temp->next!=NULL && i<setSize){
		temp=temp->next;
		i++;
	}

	if(temp->next!=NULL){
	
		if(request==1){
			if(temp->next-> dirtyBit == 1) dirtyBlocksEvic++;
		}

		if(assoc != 1){
			if(assoc==0) capMisses++;
			else confMisses++;

			cacheMisses++;
			if(request == 0) readMisses++;
			if(request == 1) writeMisses++;
		}
		
		temp->next = NULL;
		valids[indexS]--;		
	}
}





int main(){

    std::cin >> cacheSize >> blockSize >> assoc >> replace;

    noOfBlocks = cacheSize/blockSize;

    sets = 1;
    setSize = noOfBlocks;

    if(assoc == 1){
        sets = noOfBlocks;
        setSize = 1;
    }

    if(assoc > 1){
        sets = noOfBlocks/assoc;
        setSize = assoc;
    }

    indexBits = log2(sets);
    offSet = log2(blockSize);
    
    tagBits = 32 - (indexBits + offSet);

    struct cacheBlock** Cache = new struct cacheBlock*[sets]; 
    
    for(int i=0; i<sets; i++){
        Cache[i] =  new struct cacheBlock;
        Cache[i]->tag = 0;
        Cache[i]->validBit = 0;
        Cache[i]->dirtyBit = 0;
        Cache[i]->next = NULL;
    }

    bool** trees;
    if(replace == 2){
        trees = new bool*[sets];
        for(int i=0; i<sets; i++){
            trees[i] = new bool[2*setSize - 1];
            for(int j=0; j<2*setSize-1; j++) trees[i][j] = 0;
        }
	
        
        for(int i=0; i<sets; i++){
            struct cacheBlock* SetPLRU = Cache[i];
            for(int j=0; j<setSize; j++){
                SetPLRU->next = new struct cacheBlock;

                SetPLRU->next->tag = 0;
                SetPLRU->next->validBit = 0;
                SetPLRU->next->dirtyBit = 0;
                SetPLRU->next->next = NULL; 

                SetPLRU = SetPLRU->next;               
            }
        }
    }
	

    int dataSize=1;
    int *data = new int[dataSize];
    data[0] = 0;
    
    int *valids = new int[sets];

    for(int i=0; i<sets; i++) valids[i] = 0;

    std::ifstream input("input.txt");
    
    std::string s;
    
    int tagS=0;

    do{
        getline(input, s);
        
        if(!s.length()) break;
        bool* binary = convertHexToBinary(s);

        cacheAccesses++;
        int request=0;
        
        std::string::iterator i = s.end();
        i--;
        if(*i == 'w') request=1;
        
        

        if(request == 0) readAccesses++;
        if(request == 1) writeAccesses++;

        tagS    = convertBinaryToInt(binary, 0,                 tagBits-1);
        int indexS  = convertBinaryToInt(binary, tagBits,           tagBits+indexBits-1);
        int offS    = convertBinaryToInt(binary, tagBits+indexBits, 31);
        int blockAdd= convertBinaryToInt(binary, 1,                 tagBits+indexBits-1);

        int comp=0;
        int cap=0;
        int conf=0;
        

        if(!binarySearch(data, dataSize, blockAdd)){
            comp++;
            compMisses++;
            if(request == 0) readMisses++;
            if(request == 1) writeMisses++;
            data = insertElement(data, dataSize, blockAdd);
            dataSize++;
            cacheMisses++;
        }

        bool tagEx = searchS0(Cache[indexS], tagS);
		
        if(replace == 0){
            if(tagEx == 0){
                int random = rand()%valids[indexS];
                insertCacheBlock(random, Cache[indexS], tagS, request, valids, indexS);

            }
        }

        if(replace == 1){
            if(tagEx == 1) LRU1(Cache[indexS], tagS, request);
            else insertCacheBlock(0, Cache[indexS], tagS, request, valids, indexS);
		}

		if(replace == 2){
			if(tagEx==0){
			int i=0;
            while(i<setSize-1){
            	int goRight=trees[indexS][i];
                trees[indexS][i] = (trees[indexS][i]+1)%2;
                i = 2*i + goRight;
			}
                
            i -= setSize;
            struct cacheBlock* temp = Cache[indexS]->next;

            while(i){
            	temp=temp->next;
                i--;
           	}

			if(temp->validBit == 0){
				temp->tag =  tagS;
               	temp->validBit = 1;
               	temp->dirtyBit = request;
             	valids[indexS]++;
           	}
			
         	else{
              	if(valids[indexS] == setSize){
               		if(assoc==0){
                 		capMisses++;
                   		cacheMisses++;
                    	if(request == 0) readMisses++;
            			if(request == 1) writeMisses++;
                    }
                }
			}
			
            temp->tag = tagS;
            if(request == 1){
              	if(temp->dirtyBit == 1) dirtyBlocksEvic++;
              	else temp->dirtyBit = 1;
              	}
		}
}
    }while(s.length());



    input.close(); 


    

    std::cout << cacheSize << "\t#Cache size\n";
    std::cout << blockSize << "\t#Cache line size\n";

    if(assoc == 0)      std::cout << "Fully-associative cache\n";
    else if(assoc == 1) std::cout << "Direct-mapped cache\n";
    else                std::cout << "Set-associative cache\n";

    if(replace == 0)        std::cout << "Random Replacement\n";
    else if(replace == 1)   std::cout << "LRU Replacement\n";
    else if(replace == 2)   std::cout << "Pseudo-LRU Replacement\n";
    

    std::cout << cacheAccesses   << "\t#Cache accesses\n";
    std::cout << readAccesses    << "\t#Read accesses\n";
    std::cout << writeAccesses   << "\t#Write accesses\n";

    std::cout << cacheMisses     << "\t#Cache misses\n";
    std::cout << compMisses      << "\t#Compulsory misses\n";
    std::cout << capMisses       << "\t#Capacity misses\n";
    std::cout << confMisses      << "\t#Conflict misses\n";

    std::cout << readMisses      << "\t#Read misses\n";
    std::cout << writeMisses     << "\t#Write misses\n";

    std::cout << dirtyBlocksEvic << "\t#Dirty blocks evicted\n";

}
