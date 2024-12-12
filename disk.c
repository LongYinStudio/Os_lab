//**********************************/
// name: 敬培全
// data: 2024.12.11
// description: 外存管理
//**********************************/
#include <stdio.h>
#include <stdlib.h>

// 记录每种算法中都需要的数据
int num;          //磁道数
int request[100]; //请求磁道序列
int begin;        //开始磁道位置
int cross;        //横跨的总数
int k;            //每次横跨的磁道数
int re[100];      //复制初始序列
int r[100];       //记录每个算法执行后序列

void FCFS() { //先来先服务调度算法
    cross = abs(begin - request[0]);
    printf("\n先来先服务调度（FCFS）算法：\n    访问顺序：      %3d", begin);
    for (int i = 0; i < num; i++)
        printf(" %3d", request[i]);
    printf("\n    横跨磁道数为：      %3d", abs(begin - request[0]));
    for (int i = 1; i < num; i++) {
        k = abs(request[i - 1] - request[i]);
        printf(" %3d", k);
        cross += k;
    }
    printf("\n    横跨的总磁道数：    %3d", cross);
    printf("\n    平均寻道长度：      %.5f\n", 1.0 * cross / num);
}

int Smin(int b, int re[]) { //返回离开始磁道b最近的磁道下标
    int min = abs(b - re[0]);
    int j = 0;
    for (int i = 1; i < num; i++)
        if (abs(b - re[i]) < min) {
            min = abs(b - re[i]);
            j = i;
        }
    return j;
}

void SSTF() { //最短寻道时间优先调度算法
    int c = 0, b = begin;
    printf("\n最短寻道时间优先（SSTF）算法：\n    访问顺序：      %3d", begin);
    for (int i = 0; i < num; i++) {
        c = Smin(b, re); //返回最近的磁道下标
        b = re[c];       //将最近的磁道作为开始
        re[c] = 9999999; //将已经访问过的磁道设为很大值
        printf(" %3d", b);
        r[i] = b;
    }
    cross = abs(begin - r[0]);
    printf("\n    横跨磁道数为：      %3d", abs(begin - r[0]));
    for (int i = 1; i < num; i++) {
        k = abs(r[i - 1] - r[i]);
        printf(" %3d", k);
        cross += k;
    }
    printf("\n    横跨的总磁道数：    %3d", cross);
    printf("\n    平均寻道长度：      %.5f\n", 1.0 * cross / num);
}

void SCAN() { //电梯调度算法
    int c = 0, b = begin;
    for (int i = 0; i < num; i++) // SSTF时re[]已改变
        re[i] = request[i];
    printf("\n电梯调度（SCAN）算法：\n    访问顺序：      %3d", begin);
    for (int i = 0; i < num - 1; i++) {
        for (int j = 0; j < num - i - 1; j++) {
            if (re[j] > re[j + 1]) {
                re[j] = re[j] + re[j + 1];
                re[j + 1] = re[j] - re[j + 1];
                re[j] = re[j] - re[j + 1];
            }
        }
    }
    for (int i = 0; i < num; i++)
        if (re[i] > b) {
            printf(" %3d", re[i]);
            r[c++] = re[i];
        }
    for (int i = num - 1; i >= 0; i--)
        if (re[i] < b) {
            printf(" %3d", re[i]);
            r[c++] = re[i];
        }
    cross = abs(begin - r[0]);
    printf("\n    横跨磁道数为：      %3d", abs(begin - r[0]));
    for (int i = 1; i < num; i++) {
        k = abs(r[i - 1] - r[i]);
        printf(" %3d", k);
        cross += k;
    }
    printf("\n    横跨的总磁道数：    %3d", cross);
    printf("\n    平均寻道长度：      %.5f\n", 1.0 * cross / num);
}

void CSCAN() { //循环式单向电梯调度算法
    int c = 0, b = begin;
    printf("\n循环式单向电梯调度（CSCAN）算法：\n    访问顺序：      %3d", begin);
    for (int i = 0; i < num; i++)
        if (re[i] > b) {
            printf(" %3d", re[i]);
            r[c++] = re[i];
        }
    for (int i = 0; i < num; i++)
        if (re[i] < b) {
            printf(" %3d", re[i]);
            r[c++] = re[i];
        }
    cross = abs(begin - r[0]);
    printf("\n    横跨磁道数为：      %3d", abs(begin - r[0]));
    for (int i = 1; i < num; i++) {
        k = abs(r[i - 1] - r[i]);
        printf(" %3d", k);
        cross += k;
    }
    printf("\n    横跨的总磁道数：    %3d", cross);
    printf("\n    平均寻道长度：      %.5f\n", 1.0 * cross / num);
}

int main() {
    printf("磁道调度模拟实现\n\n请输入调度磁道数量:    ");
    scanf("%d", &num);
    for (int i = 0; i < num; i++) {
        request[i] = rand() % 200; // 生成0-199以内的随机数作为磁道号
        re[i] = request[i];
    }
    printf("请输入当前磁道号：     ");
    scanf("%d", &begin);
    FCFS();
    SSTF();
    SCAN();
    CSCAN();
    return 0;
}
