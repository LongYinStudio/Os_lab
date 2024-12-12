//**********************************/
// name: 敬培全
// data: 2024.12.11
// description: 进程调度 SJF
//**********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void display_banner() {
    printf("%s\n", "██████╗ ██████╗  ██████╗  ██████╗███████╗███████╗███████╗    ███╗   ███╗ █████╗ ███╗   ██╗ █████╗  ██████╗ ███████╗██████╗ ");
    printf("%s\n", "██╔══██╗██╔══██╗██╔═══██╗██╔════╝██╔════╝██╔════╝██╔════╝    ████╗ ████║██╔══██╗████╗  ██║██╔══██╗██╔════╝ ██╔════╝██╔══██╗");
    printf("%s\n", "██████╔╝██████╔╝██║   ██║██║     █████╗  ███████╗███████╗    ██╔████╔██║███████║██╔██╗ ██║███████║██║  ███╗█████╗  ██████╔╝");
    printf("%s\n", "██╔═══╝ ██╔══██╗██║   ██║██║     ██╔══╝  ╚════██║╚════██║    ██║╚██╔╝██║██╔══██║██║╚██╗██║██╔══██║██║   ██║██╔══╝  ██╔══██╗");
    printf("%s\n", "██║     ██║  ██║╚██████╔╝╚██████╗███████╗███████║███████║    ██║ ╚═╝ ██║██║  ██║██║ ╚████║██║  ██║╚██████╔╝███████╗██║  ██║");
    printf("%s\n", "╚═╝     ╚═╝  ╚═╝ ╚═════╝  ╚═════╝╚══════╝╚══════╝╚══════╝    ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝");
    printf("\n");
}

#define getpch(type) (type *)malloc(sizeof(type))
#define NOTHING NULL

typedef struct pcd {
    char name[10];
    char state;
    int ntime;
    int rtime;
    int pid;
    int ppid;
    struct pcd *link;
} PCB;

typedef struct process_tree_node {
    PCB process;
    struct process_tree_node *left;
    struct process_tree_node *right;
} processtreenode;

PCB *ready = NOTHING, *pfend = NOTHING, *p = NOTHING;
processtreenode *root = NOTHING;

int geti() {
    char ch;
    int i = 0;
    fflush(stdin);
    ch = getchar();
    while (ch == '\n') {
        fflush(stdin);
        ch = getchar();
    }

    while (ch != '\n') {
        if (ch > '9' || ch < '0') {
            printf("\t输入错误，请输入一个整数\n");
            fflush(stdin);
            i = 0;
            ch = getchar();
        } else {
            i = i * 10 + (ch - '0');
            ch = getchar();
        }
    }
    return i;
}

processtreenode *create_process_tree_node(PCB process) {
    processtreenode *new_node =
        (processtreenode *)malloc(sizeof(processtreenode));
    new_node->process = process;
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}
processtreenode *insert_process_tree(processtreenode *root, PCB process) {
    if (root == NULL) {
        return create_process_tree_node(process);
    }

    if (process.pid < root->process.pid) {
        root->left = insert_process_tree(root->left, process);
    } else if (process.pid > root->process.pid) {
        root->right = insert_process_tree(root->right, process);
    }

    return root;
}

void SJF() {
    PCB *tp, *tempp;
    // 如果就绪队列为空
    if (!ready) {
        // 将当前进程设置为就绪队列的第一个进程
        ready = p;
        // 更新就绪队列的最后一个进程
        pfend = p;
    } else {
        // 初始化指针以遍历就绪队列
        tp = ready;
        // 初始化指针以跟踪遍历过程中的前一个节点
        tempp = NULL;
        // 遍历就绪队列，直到到达末尾或找到执行时间更短的进程
        while (tp && p->ntime >= tp->ntime) {
            // 更新前一个节点指针
            tempp = tp;
            // 移动到就绪队列中的下一个节点
            tp = tp->link;
        }
        // 如果插入位置在就绪队列的开头
        if (!tempp) {
            // 将当前进程连接到就绪队列的其余部分
            p->link = ready;
            // 更新就绪队列指针为当前进程
            ready = p;
        } else {
            // 将前一个进程连接到当前进程
            tempp->link = p;
            // 将当前进程连接到就绪队列的其余部分
            p->link = tp;
        }
    }
}

void input() {
    int i, num;
    printf("\n 输入进程数量: ");
    num = geti();
    for (i = 0; i < num; i++) {
        printf("\nprocess %d\n", i + 1);
        p = getpch(PCB);
        printf(" 输入进程名称: ");
        scanf("%s", p->name);
        printf(" 输入进程执行的时间: ");
        p->ntime = geti();
        p->rtime = 0;
        p->state = 'w';
        // p->pid = getpid();   // 设置当前进程的 pid
        // p->ppid = getppid(); // 设置当前进程的 ppid
        printf(" 输入进程pid: ");
        scanf("%d", &p->pid);
        printf(" 输入进程ppid: ");
        scanf("%d", &p->ppid);
        p->link = NOTHING;
        SJF();
        // 将进程插入到二叉树中
        root = insert_process_tree(root, *p);
    }
}

void tableHeader() {
    printf("\n+---------------+---------------+---------------+---------------+\n");
    printf("|  进程名称     |  状态         |  总执行时间   |  已执行时间   |\n");
    printf("+---------------+---------------+---------------+---------------+\n");
}
void disp(PCB *pr) {
    printf("|  %-12s\t", pr->name);
    // printf("|%c\t", pr->state);
    if (pr->state == 'w') {
        printf("|  \033[34m%-12s\033[0m\t", "等待中");
    } else if (pr->state == 'r') {
        printf("|  \033[32m%-12s\033[0m\t", "运行中");
    }
    printf("|  %-12d\t", pr->ntime);
    printf("|  %-12d\t", pr->rtime);
    printf("|");
    printf("\n+---------------+---------------+---------------+---------------+\n");
}

void check() {
    PCB *pr;
    printf("\n当前正在运行的进程: %s", ready->name);
    tableHeader();
    disp(ready);
    pr = ready->link;
    printf("\n在队列里的进程:");
    tableHeader();
    while (pr != NOTHING) {
        disp(pr);
        pr = pr->link;
    }
}

void destroy() {
    printf("\n进程 [%s] 已完成\n", ready->name);
    p = ready;
    ready = ready->link;
    free(p);
}

void running() {
    ready->state = 'r';
    (ready->rtime)++;
    check();
    if (ready->rtime == ready->ntime) {
        destroy();
    }
}

// 手动销毁进程
void destroyByHand() {
    if (ready == NOTHING) {
        printf("\n就绪队列为空，没有可销毁的进程\n");
        return;
    }
    printf("\n输入要销毁的进程名称: ");
    char target_name[10];
    scanf("%s", target_name);
    PCB *current = ready;
    PCB *previous = NOTHING;
    while (current != NOTHING) {
        if (strcmp(current->name, target_name) == 0) {
            // 找到目标进程
            printf("\n进程 [%s] 已销毁\n", current->name);
            if (previous == NOTHING) {
                // 目标进程是队列的第一个进程
                ready = current->link;
            } else {
                // 从队列中跳过目标进程
                previous->link = current->link;
            }
            free(current);
            return;
        }
        // 继续遍历
        previous = current;
        current = current->link;
    }
    // 如果未找到目标进程
    printf("\n未找到名称为 [%s] 的进程\n", target_name);
}

// 显示进程树
// 查找并显示以指定PPID为父进程的子进程
void inorderTraversal(processtreenode *node) {
    if (node == NULL) {
        return;
    }
    inorderTraversal(node->left);
    printf("|  %-12s |  %-12d |  %-12d |\n", node->process.name,
           node->process.pid, node->process.ppid);
    printf("+---------------+---------------+---------------+\n");

    inorderTraversal(node->right);
}
void showProcessTree() {
    if (root == NULL) {
        printf("\n进程树为空\n");
        return;
    }
    printf("\n显示进程树󱘎 \n");
    printf("\n+---------------+---------------+---------------+\n");
    printf("|  进程名称     |  进程PID      |  父进程PID    |\n");
    printf("+---------------+---------------+---------------+\n");
    inorderTraversal(root);
}
int main() {
    display_banner();
    char ch;
    input();
    while (ready != NOTHING) {
        printf("\n按\033[34mi\033[0m加入新的进程, 按\033[31md\033[0m销毁进程, "
               "按\033[36ms\033[0m显示当前进程树, 按\033[31mr\033[0m键继续...");
        fflush(stdin);
        // ch = getchar();
        scanf("%s", &ch);
        if (ch == 'i' || ch == 'I') {
            input();
        }
        if (ch == 'd' || ch == 'D') {
            destroyByHand();
        }
        if (ch == 's' || ch == 'S') {
            showProcessTree();
        }
        if (ch == 'r' || ch == 'R') {
            running();
        }
    }
    printf("\n\n全部进程执行完毕\n");
    getchar();
    return 0;
}
