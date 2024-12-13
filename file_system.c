//**********************************/
// name: 敬培全
// data: 2024.12.11
// description: 文件系统
//**********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define DISK_SIZE 8192 // 磁盘大小 8MB
#define BLOCK_SIZE 512 // 块大小 512 bytes
#define BLOCK_COUNT (DISK_SIZE / BLOCK_SIZE)
#define MAX_FILES 100
#define MAX_DIRS 10

typedef enum {
    FILE_NORMAL,
    FILE_READONLY,
    FILE_HIDDEN,
    FILE_SYSTEM
} FileAttribute;

char disk_memory[DISK_SIZE];

typedef struct {
    char bitmap[BLOCK_COUNT / 8]; // 位示图
} DiskSpaceManager;

typedef struct FileControlBlock {
    char filename[36]; // 包含扩展名的文件名
    int start_block;
    int size;
    int is_open;
    time_t creation_time;
    FileAttribute attribute;
    struct FileControlBlock *next;
} FileControlBlock;

typedef struct Directory {
    char dirname[32];
    FileControlBlock *fcb[MAX_FILES];
    int file_count;
    struct Directory *subdirs[MAX_DIRS];
    int dir_count;
} Directory;

void init_disk_space_manager(DiskSpaceManager *manager) {
    memset(manager->bitmap, 0, sizeof(manager->bitmap));
}

int allocate_block(DiskSpaceManager *manager) {
    for (int i = 0; i < BLOCK_COUNT; ++i) {
        if (!(manager->bitmap[i / 8] & (1 << (i % 8)))) {
            manager->bitmap[i / 8] |= (1 << (i % 8));
            return i;
        }
    }
    return -1;
}

void free_block(DiskSpaceManager *manager, int block_index) {
    manager->bitmap[block_index / 8] &= ~(1 << (block_index % 8));
}

int create_file(Directory *dir, DiskSpaceManager *manager, const char *filename, const char *data, int size, FileAttribute attribute) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("创建文件失败");
        return -1;
    }
    if (fwrite(data, 1, size, file) != size) {
        perror("写入文件失败");
        fclose(file);
        return -1;
    }
    fclose(file);
    if (dir->file_count >= MAX_FILES)
        return -1;
    int block_index = allocate_block(manager);
    if (block_index == -1)
        return -1;
    FileControlBlock *fcb = (FileControlBlock *)malloc(sizeof(FileControlBlock));
    if (!fcb)
        return -1; // 检查内存分配
    strcpy(fcb->filename, filename);
    fcb->start_block = block_index;
    fcb->size = size;
    fcb->is_open = 0;
    fcb->creation_time = time(NULL);
    fcb->attribute = attribute;
    fcb->next = NULL;
    memcpy(disk_memory + block_index * BLOCK_SIZE, data, size);
    dir->fcb[dir->file_count++] = fcb;
    return 0;
}

int read_file(Directory *dir, const char *filename, char *buffer, int size) {
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->fcb[i]->filename, filename) == 0) {
            int block_index = dir->fcb[i]->start_block;
            memcpy(buffer, disk_memory + block_index * BLOCK_SIZE, size);
            return dir->fcb[i]->size;
        }
    }
    return -1;
}

int delete_file(Directory *dir, DiskSpaceManager *manager, const char *filename) {
    if (remove(filename) != 0) {
        perror("删除文件失败");
        return -1;
    }
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->fcb[i]->filename, filename) == 0) {
            free_block(manager, dir->fcb[i]->start_block);
            free(dir->fcb[i]);
            for (int j = i; j < dir->file_count - 1; ++j) {
                dir->fcb[j] = dir->fcb[j + 1];
            }
            dir->file_count--;
            return 0;
        }
    }
    return -1;
}

int rename_file(Directory *dir, const char *old_name, const char *new_name) {
    if (rename(old_name, new_name) != 0) {
        perror("重命名失败");
        return -1;
    }
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->fcb[i]->filename, old_name) == 0) {
            strcpy(dir->fcb[i]->filename, new_name);
            return 0;
        }
    }
    return -1;
}

int create_directory(Directory *parent, const char *dirname) {
    if (mkdir(dirname, 0755) != 0) {
        perror("创建目录失败");
        return -1;
    }
    if (parent->dir_count >= MAX_DIRS)
        return -1;
    Directory *new_dir = (Directory *)malloc(sizeof(Directory));
    if (!new_dir)
        return -1; // 检查内存分配
    strcpy(new_dir->dirname, dirname);
    new_dir->file_count = 0;
    new_dir->dir_count = 0;
    parent->subdirs[parent->dir_count++] = new_dir;
    return 0;
}

int move(const char *filename, const char *dir) {
    char new_path[1024];                                          // 假设新路径不会超过1024个字符
    snprintf(new_path, sizeof(new_path), "%s/%s", dir, filename); // 构造新路径
    // 使用rename函数移动文件
    if (rename(filename, new_path) == 0) {
        return 0; // 成功
    } else {
        perror("移动文件失败");
        return -1; // 失败
    }
}

void display_file_list(Directory *dir) {
    printf("文件列表:\n");
    printf("+---------------+---------------+---------------+-----------------------+---------------+\n");
    printf("|  序号         |  名称         |  大小         |  创建时间             |  属性         |\n");
    printf("+---------------+---------------+---------------+-----------------------+---------------+\n");
    for (int i = 0; i < dir->file_count; ++i) {
        char creation_date[20];
        struct tm *tm_info = gmtime(&dir->fcb[i]->creation_time); // 使用 gmtime 而不是 localtime
        if (tm_info != NULL) {
            int size = strftime(creation_date, sizeof(creation_date), "%Y-%m-%d %H:%M:%S", tm_info);
            if (size < 0) {
                strcpy(creation_date, "未知时间"); // 如果获取时间失败，显示未知时间
            } else {
                creation_date[size] = '\0'; // 确保字符串以空字符结尾
            }
        } else {
            strcpy(creation_date, "未知时间");
        }
        const char *attributes[] = {"普通文件", "只读", "隐藏", "系统文件"};
        printf("|  %-12d	", i + 1);
        printf("|  %-12s	", dir->fcb[i]->filename);
        printf("|  %-12d	", dir->fcb[i]->size);
        printf("|  %-12s	", creation_date);
        printf("|  %-12s	", attributes[dir->fcb[i]->attribute]);
        printf("|\n");
    }
    printf("+---------------+---------------+---------------+-----------------------+---------------+\n");
}

int main() {
    DiskSpaceManager manager;
    Directory root = {"根目录", {0}, 0, {0}, 0};
    init_disk_space_manager(&manager);
    char choice;
    char filename[36], data[BLOCK_SIZE], buffer[BLOCK_SIZE];
    FileAttribute attribute;
    while (1) {
        printf(" 文件系统命令菜单:\033[32mc\033[0m:创建文件 \033[36mr\033[0m:读取文件 \033[35md\033[0m:删除文件 "
               "\033[34mR\033[0m:重命名文件 \033[32mC\033[0m:创建目录 \033[36ms\033[0m:显示文件列表 \033[36mm\033[0m:移动文件\n");
        printf("请输入您的选择: ");
        scanf("%s", &choice);
        switch (choice) {
        case 'c': {
            printf("请输入文件名 (格式: name.ext): ");
            scanf("%s", filename);
            printf("请输入文件内容: ");
            scanf("%s", data);
            printf("选择文件属性 (0: 普通文件, 1: 只读文件, 2: 隐藏文件, 3: 系统文件): ");
            int attr;
            scanf("%d", &attr);
            attribute = (FileAttribute)attr;
            if (create_file(&root, &manager, filename, data, strlen(data), attribute) == 0) {
                printf("文件创建成功!\n");
            } else {
                printf("文件创建失败!\n");
            }
            break;
        }
        case 'r': {
            printf("请输入文件名: ");
            scanf("%s", filename);
            if (read_file(&root, filename, buffer, BLOCK_SIZE) >= 0) {
                printf("文件内容: %s\n", buffer);
            } else {
                printf("文件未找到!\n");
            }
            break;
        }
        case 'd': {
            printf("请输入文件名: ");
            scanf("%s", filename);
            if (delete_file(&root, &manager, filename) == 0) {
                printf("文件删除成功!\n");
            } else {
                printf("文件未找到!\n");
            }
            break;
        }
        case 'R': {
            printf("请输入旧文件名: ");
            scanf("%s", filename);
            char new_name[36];
            printf("请输入新文件名: ");
            scanf("%s", new_name);
            if (rename_file(&root, filename, new_name) == 0) {
                printf("文件重命名成功!\n");
            } else {
                printf("文件未找到!\n");
            }
            break;
        }
        case 'C': {
            printf("请输入目录名: ");
            char dirname[32];
            scanf("%s", dirname);
            if (create_directory(&root, dirname) == 0) {
                printf("目录创建成功!\n");
            } else {
                printf("目录创建失败!\n");
            }
            break;
        }
        case 's': {
            display_file_list(&root);
            break;
        }
        case 'm': {
            char new_path[1024];
            printf("请输入文件名: ");
            scanf("%s", filename);
            char dir_name[36];
            printf("请输入目录名: ");
            scanf("%s", dir_name);
            if (move(filename, dir_name) == 0) {
                printf("文件移动成功!\n");
            } else {
                printf("文件移动失败!\n");
            }
            break;
        }
        default:
            printf("无效选择，请重新输入.\n");
        }
    }
    return 0;
}
