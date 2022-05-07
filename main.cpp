#include <iostream>
#include <random>
#include <windows.h>
#include <set>

using namespace std;

typedef int robj;

typedef struct zskiplistNode {

    robj *obj;
    double score;

    struct zskiplistNode *backward;

    struct zskiplistLevel {
        struct zskiplistNode *forward;
        unsigned int span;
    } level[]; //创建新的zskiplistNode节点的时候，level层数数组[]大小时随机产生的，见zslInsert->zslRandomLevel

} zskiplistNode; //存储在zskiplist跳跃表结构中


typedef struct zskiplist { //zslCreate
    struct zskiplistNode *header, *tail;
    unsigned long length;
    int level;
} zskiplist;

zskiplistNode *createNode(int level, double score = 0) {

    auto p = (zskiplistNode *) malloc(sizeof(zskiplistNode) + level * sizeof(zskiplistNode::zskiplistLevel));
    p->score = score;
    return p;
}
const int ZSKIP_LEVEL_MAX = 32;

zskiplist *createList(int level)
{
    auto *zs = new zskiplist();
    zs->length = 0;
    zs->header = createNode(level);
    zs->tail = nullptr;
    zs->level = level;
}
default_random_engine e;
const double ZSKIPLIST_P = 0.25;
int zslRandomLevel() {
    int level = 1;
    while ((e()&0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
        level += 1;
    return (level<ZSKIP_LEVEL_MAX) ? level : ZSKIP_LEVEL_MAX;
}

int search(zskiplist* zs,double score)
{
    auto p = zs->header;
    for(int i = zs->level - 1 ; i >= 0 ; i --)
    {
        while (p && p->level[i].forward && p->score < score) {
            p = p->level[i].forward;
        }
        if( p && p->score == score)
        {
//            cout << "find:" << i << " " << p << "score:" << p->score << endl;
            return i;
        }
    }
    return -1;
}


int maxlevel = 0;
void insert(zskiplist* zs,double score)
{
    zskiplistNode *update[ZSKIP_LEVEL_MAX];
    int level = zslRandomLevel();
    maxlevel = max(maxlevel, level);
//    cout << "level:" << level << endl;
    auto cur = createNode(level,score);
    auto p = zs->header;
    for(int i = zs->level - 1 ; i>= 0 ; i --)
    {
        while(p->level[i].forward && p->level[i].forward->score < score)
        {
            p = p->level[i].forward;
        }
        update[i] = p; // 至关重要，要不然底下不知道level不知道从哪一个更新
    }
    // 找到了p指针
    cur->backward = p;
    if (p->level[0].forward)
    {
        p->level[0].forward->backward = cur;
    }
//    cout << "p:" << p  << "header:" << zs->header << endl;
    for (int i = 0; i < level; ++i) {
        cur->level[i].forward = update[i]->level[i].forward;
        update[i]->level[i].forward = cur;
//        cout << "link:" << i  << "s:" << score << endl;
//        cout << " I:" << i << "  p:" << p->score << " cur:" << cur->score << endl;
    }



}

bool remove(zskiplist* zs,double score)
{
    zskiplistNode *update[zs->level];
    auto p = zs->header;
    for(int i = zs->level - 1 ; i>= 0 ; i --)
    {
        while(p->level[i].forward && p->level[i].forward->score < score)
        {
            p = p->level[i].forward;
        }
        update[i] = p; // 至关重要，要不然底下不知道level不知道从哪一个更新, update  每一层的 p 的上一个
    }
    auto cur = p->level[0].forward;
    if (cur && cur->score == score)
    {
        cout << "remove p" << endl;
        // 删除 p 指针
        if(cur->level[0].forward)
        {
            cur->level[0].forward->backward = cur->backward; // p 前面的指针 的后面指向 p 的后面
        }

        // 更改与p相关的，解除链表关系
        for (int i = 0; i < zs->level; ++i) {
            if (update[i]->level[i].forward == cur)
            {
                update[i]->level[i].forward = cur->level[i].forward;
            }
        }

    }




}



const int TEST_MAX = 1e6;

void test_skiplist()
{
    zskiplist* list = createList(ZSKIP_LEVEL_MAX);

    for (int i = 1; i <= TEST_MAX; ++i) {
        insert(list, i / (TEST_MAX * 1.0));
    }

    for (int i = 1; i <= TEST_MAX; ++i) {
        search(list, i / (TEST_MAX * 1.0));
    }
}

void test_set()
{
    std::set<double> s;
    for (int i = 1; i <= TEST_MAX; ++i) {
        s.insert(i / (TEST_MAX * 1.0));
    }

    for (int i = 1; i <= TEST_MAX; ++i) {
        s.find(i / (TEST_MAX * 1.0));
    }

}

void count_test_skiplist()
{
    DWORD Start = GetTickCount();
    test_skiplist();
    DWORD End = GetTickCount();
    cout << End - Start << endl;
}

void count_test_set()
{
    DWORD Start = GetTickCount();
    test_set();
    DWORD End = GetTickCount();
    cout << End - Start << endl;
}
int main() {

//    count_test_skiplist();
//    count_test_set();

    zskiplist* list = createList(ZSKIP_LEVEL_MAX);
    for (int i = 10; i >= 1; --i) {
        insert(list,i / (10 * 1.0));
    }

    for (int i = 5; i >= 1; --i) {
        remove(list,i / (10 * 1.0));
    }

    auto p = list->header;
    while (p && p->level[0].forward) {
        p = p->level[0].forward;
    }
    auto tail = p;
    while (tail)
    {
        cout << tail->score << endl;
        tail = tail->backward;
    }

    for (int i = 0; i < list->level; ++i) {
        auto p = list->header;
        cout << "layer:" << i << endl;
        while (p) {
            cout << p->score << " ";
            p = p->level[i].forward;
        }
        cout << endl;
    }
    return 0;
}
