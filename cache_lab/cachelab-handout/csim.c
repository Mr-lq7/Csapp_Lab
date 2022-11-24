#include "cachelab.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <getopt.h>

int h, v, s, S, E, b, t;

typedef struct {
    int valid_bit; //每行一个有效位
    int tag_bits; //t个标记位
    int timestamp;  //时间戳,为LRU算法做准备
}cache_line;

cache_line** cache = NULL;

char tracePath[100] = {0};
FILE *fp;
int hits = 0, misses = 0, evictions = 0;

//print -h content
void printHelp() {
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
            "Options:\n"
            "  -h         Prints usage info.\n"
            "  -v         Displays trace info.\n"
            "  -s <s>   Number of set index bits.\n"
            "  -E <E>   Number of lines per set.\n"
            "  -b <b>   Number of block offset bits.\n"
            "  -t <tracefile>  Trace file.\n\n"
            "Examples:\n"
            "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
            "  linux>  ./csim-ref -v -s 4 -E 1 -b 4 -t traces/yi.trace\n");
}

// 用二维数组模拟Cache
void initCache() {
    cache = (cache_line **)malloc(sizeof(cache_line *) * S);
    for (int i = 0; i < S; i++) {
        *(cache + i) = (cache_line *)malloc(sizeof(cache_line) * E);
    }
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < E; j++) {
            cache[i][j].valid_bit = 0;
            cache[i][j].tag_bits = -1;
            cache[i][j].timestamp = -1;
        }
    }

}

//进行LRU策略
void update (unsigned int addr) {
    int set_index = (addr >> b) & ((-1U) >> (64-s));
    int tag = addr >> (b+s);

    //查看tag有没有命中
    for (int i = 0; i < E; i++) {
        if (cache[set_index][i].tag_bits == tag) {
            cache[set_index][i].timestamp = 0;
            hits++;
            printf("hit\n");
            return;
        }
    }

    //查看有无空行
    for (int i = 0; i < E; i++) {
        if (cache[set_index][i].valid_bit == 0) {
            cache[set_index][i].valid_bit = 1;
            cache[set_index][i].tag_bits = tag;
            cache[set_index][i].timestamp = 0;
            misses++;
            printf("miss\n");
            return;
        }
    }

    int max_timestamp = -2;
    int max_timestamp_index = -1;
    //需要替换, 利用LRU策略进行替换. 替换掉最大的时间戳
    misses++;
    evictions++;
    printf("evictions\n");
    for (int i = 0; i < E; i++) {
        if (cache[set_index][i].timestamp > max_timestamp) {
            max_timestamp = cache[set_index][i].timestamp;
            max_timestamp_index = i;
        }
    }
    cache[set_index][max_timestamp_index].tag_bits = tag;
    cache[set_index][max_timestamp_index].timestamp = 0;
    cache[set_index][max_timestamp_index].valid_bit = 1;

}

void update_timestamp() {
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < E; j++) {
            if (cache[i][j].valid_bit) {
                cache[i][j].timestamp++;
            }
        }
    }
}

void read_trace() {
    
    //fp在main函数中已经赋值
    char op;
    unsigned int addr;
    int size;
    while (fscanf(fp, " %c %x,%d", &op, &addr, &size) > 0) {
        // fflush(fp);
        if (v)
            printf("%c %d %d \n", op, addr, size);
        switch(op) {
            case 'L':
                update(addr);
                break;
            case 'S':
                update(addr);
                break;
            case 'M':
                update(addr); //load
                update(addr); //store
                break;
        }
        update_timestamp();
    }
    fclose(fp);
    for (int i = 0; i < S; i++) {
        free(cache[i]);
    }
    free(cache);
    return;


}
int main(int argc, char *argv[])
{

    char c;
    /* parse command line args */
    while ((c = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (c) {
            case 'h': /* help */
                printHelp();
                break;
            case 'v': 
                v = 1;
                break; 
            case 's': 
                s = atoi(optarg);
                break;
            case 'E': 
                E = atoi(optarg);
                break;
            case 'b': 
                b = atoi(optarg);
                break;
            case 't': 
                strcpy(tracePath, optarg);
                break;
            default:
                printf("./csim failed to parse its options.\n");
                exit(1);
                
	    }
    }

    //前置条件
    if (s <= 0 || E <= 0 || b <= 0 || !strlen(tracePath)) {
        printf("Invalid param. \n");
        exit(1);
    }
    

    //打开文件
    if ((fp = fopen(tracePath, "r")) == NULL) {
        printf("Fail to open file!\n");
        exit(1);
    }

    //s被赋值为几路组相联
    S = 1 << s;

    initCache();
    read_trace();
    //hits, misses, evictions

    printSummary(hits, misses, evictions);
    return 0;
}
