#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_USERS 110

//  Red-Black Tree

typedef enum { RED, BLACK } Color;

typedef struct RBNode {
    int key;
    void *value;
    Color color;
    struct RBNode *left, *right, *parent;
} RBNode;

static RBNode *rbRoot = NULL;

RBNode* newRBNode(int key, void *value) {
    RBNode *node = malloc(sizeof(RBNode));
    node->key = key;
    node->value = value;
    node->color = RED;
    node->left = node->right = node->parent = NULL;
    return node;
}

void rbRotateLeft(RBNode *x);
void rbRotateRight(RBNode *x);
void rbInsertFixup(RBNode *z);

void rbInsert(int key, void *value) {
    RBNode *z = newRBNode(key, value), *y = NULL, *x = rbRoot;
    while (x) { y = x; x = (key < x->key) ? x->left : x->right; }
    z->parent = y;
    if (!y) rbRoot = z;
    else if (key < y->key) y->left = z;
    else y->right = z;
    rbInsertFixup(z);
}

void* rbSearch(int key) {
    RBNode *x = rbRoot;
    while (x) {
        if (key == x->key) return x->value;
        x = (key < x->key) ? x->left : x->right;
    }
    return NULL;
}

void rbRotateLeft(RBNode *x) {
    RBNode *y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) rbRoot = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

void rbRotateRight(RBNode *x) {
    RBNode *y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) rbRoot = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
}

void rbInsertFixup(RBNode *z) {
    while (z->parent && z->parent->color == RED) {
        RBNode *gp = z->parent->parent;
        if (z->parent == gp->left) {
            RBNode *y = gp->right;
            if (y && y->color == RED) {
                z->parent->color = y->color = BLACK;
                gp->color = RED;
                z = gp;
            } else {
                if (z == z->parent->right) { z = z->parent; rbRotateLeft(z); }
                z->parent->color = BLACK;
                gp->color = RED;
                rbRotateRight(gp);
            }
        } else {
            RBNode *y = gp->left;
            if (y && y->color == RED) {
                z->parent->color = y->color = BLACK;
                gp->color = RED;
                z = gp;
            } else {
                if (z == z->parent->left) { z = z->parent; rbRotateRight(z); }
                z->parent->color = BLACK;
                gp->color = RED;
                rbRotateLeft(gp);
            }
        }
    }
    rbRoot->color = BLACK;
}

// Graph gösterim

typedef struct AdjNode {
    int id;
    struct AdjNode *next;
} AdjNode;

typedef struct User {
    int id;
    char name[32];
    AdjNode *friends;
} User;

// Kullanıcı listesi isim araması için
static User *userList[MAX_USERS+1];

User* createUser(int id, const char *name) {
    User *u = malloc(sizeof(User));
    u->id = id;
    strncpy(u->name, name, sizeof(u->name)-1);
    u->name[sizeof(u->name)-1] = '\0';
    u->friends = NULL;
    if (id <= MAX_USERS) userList[id] = u;
    return u;
}

void addFriendship(int u_id, int v_id) {
    User *u = rbSearch(u_id);
    User *v = rbSearch(v_id);
    if (!u || !v) return;
    AdjNode *an = malloc(sizeof(AdjNode)); an->id = v_id; an->next = u->friends; u->friends = an;
    an = malloc(sizeof(AdjNode)); an->id = u_id; an->next = v->friends; v->friends = an;
}

// user id bulmma
int findUserIdByName(const char *name) {
    for (int i = 1; i <= MAX_USERS; i++) {
        if (userList[i] && strcmp(userList[i]->name, name) == 0)
            return i;
    }
    return -1;
}

// Arkadaslik düzeyleri
void describeFriendshipLevels(int start_id) {
    int *level = malloc((MAX_USERS+1)*sizeof(int));
    int *parent = malloc((MAX_USERS+1)*sizeof(int));
    bool *visited = malloc((MAX_USERS+1)*sizeof(bool));
    for (int i = 0; i <= MAX_USERS; i++) { level[i] = INT_MAX; visited[i] = false; parent[i] = -1; }
    int *queue = malloc((MAX_USERS+1)*sizeof(int));
    int head=0, tail=0;
    visited[start_id]=true; level[start_id]=0; queue[tail++]=start_id;
    while(head<tail) {
        int uid=queue[head++];
        User *u=rbSearch(uid);
        for(AdjNode *p=u->friends;p;p=p->next) {
            if(!visited[p->id]){
                visited[p->id]=true;
                level[p->id]=level[uid]+1;
                parent[p->id]=uid;
                queue[tail++]=p->id;
            }
        }
    }
    printf("\nArkadaslik Duzeyleri ve Yollari (Baslangic: %s)\n", userList[start_id]->name);
    for(int i=1;i<=MAX_USERS;i++){
        if(i==start_id||!visited[i]) continue;
        int path[ MAX_USERS+1 ], count=0, cur=i;
        while(cur!=-1){ path[count++]=cur; if(cur==start_id) break; cur=parent[cur]; }
        for(int j=count-1;j>=0;j--){
            printf("%s", userList[path[j]]->name);
            if(j>0) printf(" - ");
        }
        printf(" (Duzey %d)\n", level[i]);
    }
    free(level); free(parent); free(visited); free(queue);
}

// ortak arkadaslar
void commonFriends(int u1, int u2) {
    User *a=rbSearch(u1), *b=rbSearch(u2);
    if(!a||!b) return;
    bool *mark=calloc(MAX_USERS+1, sizeof(bool));
    for(AdjNode *p=a->friends;p;p=p->next) mark[p->id]=true;
    printf("\nOrtak Arkadaslar (%s, %s):\n", a->name, b->name);
    for(AdjNode *p=b->friends;p;p=p->next) {
        if(mark[p->id]) printf("%s\n", userList[p->id]->name);
    }
    free(mark);
}

// topluluk tespiti
void dfsComponent(int uid, bool *visited) {
    visited[uid]=true;
    printf("%s ", userList[uid]->name);
    for(AdjNode *p=userList[uid]->friends;p;p=p->next) {
        if(!visited[p->id]) dfsComponent(p->id, visited);
    }
}

void detectCommunities() {
    bool *visited=calloc(MAX_USERS+1, sizeof(bool));
    printf("\nTopluluklar:\n");
    for(int i=1;i<=MAX_USERS;i++){
        if(userList[i] && !visited[i]){
            printf("Topluluk: "); dfsComponent(i, visited); printf("\n");
        }
    }
    free(visited);
}

// etki alanı
int countReachable(int uid, bool *visited) {
    visited[uid]=true; int count=1;
    for(AdjNode *p=userList[uid]->friends;p;p=p->next) {
        if(!visited[p->id]) count+=countReachable(p->id, visited);
    }
    return count;
}

void computeInfluence(int u_id) {
    bool *visited=calloc(MAX_USERS+1, sizeof(bool));
    int reach=countReachable(u_id, visited)-1;
    printf("\n%s kullanicisi %d kullaniciya ulasiyor.\n", userList[u_id]->name, reach);
    free(visited);
}

int main() {
    // Oluşturulan kullanıcılar
    struct { int id; const char *name; } users[] = {
        {101,"Ali"},{102,"Ayse"},{103,"Mehmet"},{104,"Fatma"},{105,"Can"},
        {106,"Burak"},{107,"Ceren"},{108,"Deniz"},{109,"Ece"},{110,"Furkan"}
    };
    int n = sizeof(users)/sizeof(users[0]);
    for(int i=0;i<n;i++) rbInsert(users[i].id, createUser(users[i].id, users[i].name));

    // arkadaşlık ilişkileri
    addFriendship(101,102);
    addFriendship(101,103);
    addFriendship(102,104);
    addFriendship(103,104);
    addFriendship(104,105);
    addFriendship(105,106);
    addFriendship(106,107);
    addFriendship(107,108);
    addFriendship(108,109);
    addFriendship(109,110);
    addFriendship(102,103);
    addFriendship(104,106);
    addFriendship(107,109);
    addFriendship(108,110);
    addFriendship(105,110);


    char name1[32], name2[32];
    printf("Kullanici adi giriniz: "); scanf("%31s", name1);
    int id1 = findUserIdByName(name1);
    if(id1<0){ printf("Kullanici bulunamadi.\n"); return 1; }

    // Arkadaşlık düzeyleri ve yollar
    describeFriendshipLevels(id1);

    // Ortak arkadaş analizi
    printf("\nOrtak arkadas analizi icin ikinci kullanici adi giriniz: ");
    scanf("%31s", name2);
    int id2 = findUserIdByName(name2);
    if(id2<0){ printf("Ikinci kullanici bulunamadi.\n"); return 1; }
    commonFriends(id1, id2);

    // Topluluk tespiti
    detectCommunities();

    // Etki alanı hesaplama
    computeInfluence(id1);

    return 0;
}
