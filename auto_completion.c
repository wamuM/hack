#include "auto_completion.h"

int startsWith(char* s1, char* s2){
    while(*s1 != '\0' && *s2 != '\0'){
        if(*s1 != *s2)return 0;
        ++s1;++s2;
    }
    return 1;
}

int populate(char** ch, char* s, int* filled){
    ch[(*filled)++] = s;
    // Return true if done
    return (*filled) >= MAX_NUM_SUGGEST;
}
void generate_suggestions(char *input, graph *g, char **output){
    int filled = 0;
    for(int i = 0; i < g->node_len; ++i){
        if(startsWith(g->nodes[i].name, input)){
            if(populate(output, input,&filled))return;
        }
    }
}
