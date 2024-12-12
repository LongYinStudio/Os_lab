//**********************************/
// name: 敬培全
// data: 2024.12.11
// description: 文件系统
//**********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DISK_SIZE 8192 // 8MB
#define BLOCK_SIZE 512 // 512 bytes
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

int create_file(Directory *dir, DiskSpaceManager *manager, const char *filename,
                const char *data, int size, FileAttribute attribute) {
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

int write_file(Directory *dir, const char *filename, const char *data,
               int size) {
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->fcb[i]->filename, filename) == 0) {
            int block_index = dir->fcb[i]->start_block;
            memcpy(disk_memory + block_index * BLOCK_SIZE, data, size);
            dir->fcb[i]->size = size;
            return 0;
        }
    }
    return -1;
}

int delete_file(Directory *dir, DiskSpaceManager *manager,
                const char *filename) {
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
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->fcb[i]->filename, old_name) == 0) {
            strcpy(dir->fcb[i]->filename, new_name);
            return 0;
        }
    }
    return -1;
}

int search_file(Directory *dir, const char *filename) {
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->fcb[i]->filename, filename) == 0) {
            return i;
        }
    }
    return -1;
}

int open_file(Directory *dir, const char *filename) {
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->fcb[i]->filename, filename) == 0) {
            dir->fcb[i]->is_open = 1;
            return 0;
        }
    }
    return -1;
}

int close_file(Directory *dir, const char *filename) {
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->fcb[i]->filename, filename) == 0) {
            dir->fcb[i]->is_open = 0;
            return 0;
        }
    }
    return -1;
}

int create_directory(Directory *parent, const char *dirname) {
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

void display_file_list(Directory *dir) {
    printf("文件列表:\n");
    printf("序号\t名称\t\t大小\t\t创建日期\t属性\n");
    for (int i = 0; i < dir->file_count; ++i) {
        char creation_date[20];
        struct tm tm_info;
        localtime(&dir->fcb[i]->creation_time); // 使用 localtime
        strftime(creation_date, sizeof(creation_date), "%Y-%m-%d %H:%M:%S", &tm_info);

        const char *attributes[] = {"普通文件", "只读", "隐藏", "系统文件"};
        printf("%d\t%s\t\t%d\t\t%s\t%s\n", i + 1, dir->fcb[i]->filename, dir->fcb[i]->size, creation_date, attributes[dir->fcb[i]->attribute]);
    }
}

void display_menu() {
    printf("文件系统功能菜单:\n");
    printf("1. 创建文件\n");
    printf("2. 读取文件\n");
    printf("3. 写入文件\n");
    printf("4. 删除文件\n");
    printf("5. 重命名文件\n");
    printf("6. 搜索文件\n");
    printf("7. 创建目录\n");
    printf("8. 显示文件列表\n");
    printf("9. 退出\n");
}

int main() {
    DiskSpaceManager manager;
    Directory root = {"根目录", {0}, 0, {0}, 0};
    init_disk_space_manager(&manager);
    int choice;
    char filename[36], data[BLOCK_SIZE], buffer[BLOCK_SIZE];
    FileAttribute attribute;

    while (1) {
        display_menu();
        printf("请输入您的选择: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1: {
            printf("请输入文件名 (格式: name.ext): ");
            scanf("%s", filename);
            printf("请输入文件内容: ");
            scanf("%s", data);
            printf("选择文件属性 (0: 普通文件, 1: 只读, 2: 隐藏, 3: 系统文件): ");
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

        case 2: {
            printf("请输入文件名: ");
            scanf("%s", filename);
            if (read_file(&root, filename, buffer, BLOCK_SIZE) >= 0) {
                printf("文件内容: %s\n", buffer);
            } else {
                printf("文件未找到!\n");
            }
            break;
        }

        case 3: {
            printf("请输入文件名: ");
            scanf("%s", filename);
            printf("请输入新内容: ");
            scanf("%s", data);
            if (write_file(&root, filename, data, strlen(data)) == 0) {
                printf("文件写入成功!\n");
            } else {
                printf("文件未找到!\n");
            }
            break;
        }

        case 4: {
            printf("请输入文件名: ");
            scanf("%s", filename);
            if (delete_file(&root, &manager, filename) == 0) {
                printf("文件删除成功!\n");
            } else {
                printf("文件未找到!\n");
            }
            break;
        }

        case 5: {
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

        case 6: {
            printf("请输入文件名: ");
            scanf("%s", filename);
            int index = search_file(&root, filename);
            if (index != -1) {
                printf("文件 \"%s\" 找到，文件索引为 %d\n", filename, index + 1);
            } else {
                printf("文件未找到!\n");
            }
            break;
        }

        case 7: {
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

        case 8: {
            display_file_list(&root);
            break;
        }

        case 9:
            printf("退出系统...\n");
            exit(0);
            break;

        default:
            printf("无效选择，请重新输入.\n");
        }
    }
    return 0;
}
