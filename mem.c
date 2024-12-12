//**********************************/
// name: 敬培全
// data: 2024.12.11
// description: 内存管理
//**********************************/

#include <stdio.h>
#include <stdlib.h>

#define MEMORY_SIZE 65536 // 假设内存大小为64MB

typedef struct SubAreaNode {
    int address;               // 分区起始地址
    int size;                  // 分区大小
    int state;                 // 分区状态(0空闲,1占用)
    int taskNo;                // 记录作业号
    struct SubAreaNode *prior; // 分区的前向指针
    struct SubAreaNode *next;  // 分区的后继指针
} SubAreaNode;

SubAreaNode *head = NULL; // 定义全局的头指针

// 初始化内存分区链表
void initializeMemory() {
    head = (SubAreaNode *)malloc(sizeof(SubAreaNode));
    head->address = 0;
    head->size = MEMORY_SIZE;
    head->state = 0; // 初始状态为空闲
    head->taskNo = 0;
    head->prior = head->next = NULL;
}

// 显示内存分区
void displayMemory() {
    SubAreaNode *current = head;
    printf("当前内存分区情况:\n");
    printf("+---------------+---------------+---------------+---------------+\n");
    printf("|  内存地址     |  大小(单位KB) |  状态         |  作业号       |\n");
    printf("+---------------+---------------+---------------+---------------+\n");
    while (current) {
        printf("|  %-12d\t", current->address);
        printf("|  %-12d\t", current->size);
        if (current->state == 0) {
            printf("|  \033[34m%-12s\033[0m\t", "空闲");
        } else {
            printf("|  \033[32m%-12s\033[0m\t", "占用");
        }
        printf("|  %-12d\t", current->taskNo);
        printf("|\n");
        current = current->next;
    }
    printf("+---------------+---------------+---------------+---------------+\n");
}

// 查找合适的分区（首次适配策略）
SubAreaNode *findFirstFit(int size) {
    SubAreaNode *current = head;
    while (current) {
        if (current->state == 0 && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL; // 没有找到合适的分区
}

// 查找合适的分区（最佳适配策略）
SubAreaNode *findBestFit(int size) {
    SubAreaNode *current = head;
    SubAreaNode *best = NULL;
    while (current) {
        if (current->state == 0 && current->size >= size) {
            if (best == NULL || current->size < best->size) {
                best = current;
            }
        }
        current = current->next;
    }
    return best;
}

// 查找合适的分区（最差适配策略）
SubAreaNode *findWorstFit(int size) {
    SubAreaNode *current = head;
    SubAreaNode *worst = NULL;
    while (current) {
        if (current->state == 0 && current->size >= size) {
            if (worst == NULL || current->size > worst->size) {
                worst = current;
            }
        }
        current = current->next;
    }
    return worst;
}

// 分配内存
int allocate(SubAreaNode *(*findFit)(int), int taskNo, int size) {
    SubAreaNode *fit = findFit(size);
    if (!fit) {
        printf("内存分配失败: 没有足够的空间为作业%d分配%dKB内存。\n", taskNo, size);
        return -1; // 内存分配失败
    }
    // 判断是否需要分割
    if (fit->size > size) {
        SubAreaNode *newNode = (SubAreaNode *)malloc(sizeof(SubAreaNode));
        newNode->address = fit->address + size;
        newNode->size = fit->size - size;
        newNode->state = 0;
        newNode->taskNo = 0;
        newNode->next = fit->next;
        newNode->prior = fit;
        if (fit->next) {
            fit->next->prior = newNode;
        }
        fit->next = newNode;
        fit->size = size;
    }
    // 分配内存
    fit->state = 1;
    fit->taskNo = taskNo;
    printf("已为作业%d分配%dKB内存。\n", taskNo, size);
    return 0; // 内存分配成功
}

// 回收内存
int deallocate(int taskNo) {
    SubAreaNode *current = head;
    while (current) {
        if (current->state == 1 && current->taskNo == taskNo) {
            current->state = 0;
            current->taskNo = 0;

            // 合并与前一个空闲块
            if (current->prior && current->prior->state == 0) {
                current->prior->size += current->size;
                current->prior->next = current->next;
                if (current->next) {
                    current->next->prior = current->prior;
                }
                free(current);
                current = current->prior;
            }
            // 合并与后一个空闲块
            if (current->next && current->next->state == 0) {
                current->size += current->next->size;
                SubAreaNode *temp = current->next;
                current->next = temp->next;
                if (temp->next) {
                    temp->next->prior = current;
                }
                free(temp);
            }
            printf("已释放作业%d占用的内存。\n", taskNo);
            return 0; // 内存回收成功
        }
        current = current->next;
    }
    printf("内存回收失败: 未找到作业%d的内存分区。\n", taskNo);
    return -1; // 内存回收失败
}

int main() {
    printf("\n模拟首次适应算法：\n");
    initializeMemory();
    displayMemory();
    allocate(findFirstFit, 1, 8000);  // 分配 8000KB 给作业1
    allocate(findFirstFit, 2, 12000); // 分配 12000KB 给作业2
    allocate(findFirstFit, 3, 6000);  // 分配 6000KB 给作业3
    allocate(findFirstFit, 4, 20000); // 分配 20000KB 给作业4
    allocate(findFirstFit, 5, 4000);  // 分配 4000KB 给作业5
    deallocate(3);                    // 释放作业3，前空后占
    allocate(findFirstFit, 6,
             5000);                   // 分配 5000KB 给作业6（使用作业3释放的部分）
    deallocate(2);                    // 释放作业2，前占后空
    allocate(findFirstFit, 7, 15000); // 分配 15000KB 给作业7
    deallocate(4);                    // 释放作业4，前占后占
    deallocate(6);                    // 释放作业6，前空后空
    displayMemory();
    printf("\n模拟最佳适应算法：\n");
    initializeMemory();
    displayMemory();
    allocate(findBestFit, 1, 5000);  // 分配 5000KB 给作业1
    allocate(findBestFit, 2, 15000); // 分配 15000KB 给作业2
    allocate(findBestFit, 3, 10000); // 分配 10000KB 给作业3
    allocate(findBestFit, 4, 25000); // 分配 25000KB 给作业4
    allocate(findBestFit, 5, 8000);  // 分配 8000KB 给作业5
    deallocate(2);                   // 释放作业2，前占后空
    allocate(findBestFit, 6, 12000); // 分配 12000KB 给作业6
    deallocate(5);                   // 释放作业5，前空后占
    allocate(findBestFit, 7, 8000);  // 分配 8000KB 给作业7
    deallocate(4);                   // 释放作业4，前占后占
    deallocate(6);                   // 释放作业6，前空后空
    displayMemory();
    printf("\n模拟最坏适应算法：\n");
    initializeMemory();
    displayMemory();
    allocate(findWorstFit, 1, 10000); // 分配 10000KB 给作业1
    allocate(findWorstFit, 2, 20000); // 分配 20000KB 给作业2
    allocate(findWorstFit, 3, 5000);  // 分配 5000KB 给作业3
    allocate(findWorstFit, 4, 30000); // 分配 30000KB 给作业4
    allocate(findWorstFit, 5, 6000);  // 分配 6000KB 给作业5
    deallocate(4);                    // 释放作业4，前占后占
    allocate(findWorstFit, 6, 20000); // 分配 20000KB 给作业6
    deallocate(2);                    // 释放作业2，前占后空
    allocate(findWorstFit, 7, 15000); // 分配 15000KB 给作业7
    deallocate(5);                    // 释放作业5，前空后占
    deallocate(6);                    // 释放作业6，前空后空
    displayMemory();
    return 0;
}
