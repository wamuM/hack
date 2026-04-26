#include "auto_completion.h"
#include <utf8proc.h>
#include <stdlib.h>
#include <string.h>

char* normalize_utf8(const char* input) {
    utf8proc_uint8_t* out = NULL;

    utf8proc_ssize_t result = utf8proc_map(
        (const utf8proc_uint8_t*)input,
        strlen(input),
        &out,
        UTF8PROC_CASEFOLD |    // case-insensitive
        UTF8PROC_DECOMPOSE |   // split accents
        UTF8PROC_STRIPMARK     // remove accents
    );

    if (result < 0) {
        return NULL;
    }

    return (char*)out; // must be freed by caller
}

int startsWith(const char* s1, const char* s2){
    while(*s2 != '\0'){
        if(*s1 != *s2)return 0;
        ++s1;++s2;
    }
    return 1;
}

int populate(char** ch, char* s, int* filled, int max){
    ch[(*filled)++] = s;
    // Return true if done
    return (*filled) >= max;
}

static char** norm_names = NULL;

void normalize_names(graph* g){
    if(norm_names != NULL)return;
    norm_names = calloc(g->node_len,sizeof(char*));
    for(int i = 0; i < g->node_len; ++i){
       char* norm = normalize_utf8(g->nodes[i].name);
       if(!norm)continue;
       norm_names[i] = norm;
    }
}

int generate_suggestions(char *input, graph *g, char **output, int* state, int max){
    if(input[0] == '\0') return 0;

    char* nprefix = normalize_utf8(input);//normalize prefix 
    if (!nprefix)return 0;
    normalize_names(g); //normalize the graph names (idempotent)

    int filled = 0;
    for(int i = 0; i < g->node_len; ++i){
        if(state[i])continue; // Skip nodes that are already in the state
        if(!norm_names[i])continue;// Skip nodes that couldn't be normalized
        if(startsWith(norm_names[i], nprefix)){ 
            if(populate(output, g->nodes[i].name,&filled, max))break;
        }
    }
    free(nprefix);
    return filled;
}
