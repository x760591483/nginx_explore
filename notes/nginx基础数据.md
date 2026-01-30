# nginx基础数据结构详解

nginx作为高性能的Web服务器，其内部实现了很多高效的数据结构。本文档详细介绍nginx中常用的基础数据结构及其相关函数。

## 1. ngx_str_t - 字符串结构

### 结构定义
```c
typedef struct {
    size_t      len;    // 字符串长度
    u_char     *data;   // 字符串数据指针
} ngx_str_t;
```

### 主要特点
- 不包含结束符'\0'，通过len字段表示长度
- 支持二进制数据，data指针指向原始数据
- 内存管理由外部负责

### 常用宏定义
```c
#define ngx_string(str)     { sizeof(str) - 1, (u_char *) str }
#define ngx_null_string     { 0, NULL }
#define ngx_str_set(str, text)                                               \
    (str)->len = sizeof(text) - 1; (str)->data = (u_char *) text
#define ngx_str_null(str)   (str)->len = 0; (str)->data = NULL
```

### 相关函数
```c
// 字符串复制
u_char *ngx_pstrdup(ngx_pool_t *pool, ngx_str_t *src);

// 字符串比较
#define ngx_strcmp(s1, s2)  strcmp((const char *) s1, (const char *) s2)
#define ngx_strncmp(s1, s2, n)  strncmp((const char *) s1, (const char *) s2, n)

// 字符串查找
#define ngx_strstr(s1, s2)  strstr((const char *) s1, (const char *) s2)
#define ngx_strchr(s1, c)   strchr((const char *) s1, (int) c)

// 字符串转换
void ngx_strlow(u_char *dst, u_char *src, size_t n);
#define ngx_tolower(c)      (u_char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define ngx_toupper(c)      (u_char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

// 字符串长度
#define ngx_strlen(s)       strlen((const char *) s)
size_t ngx_strnlen(u_char *p, size_t n);
```

## 2. ngx_list_t - 链表结构

### 结构定义
```c
typedef struct ngx_list_part_s  ngx_list_part_t;

struct ngx_list_part_s {
    void             *elts;      // 元素数组
    ngx_uint_t        nelts;     // 当前部分已使用元素个数
    ngx_list_part_t  *next;      // 指向下一个部分
};

typedef struct {
    ngx_list_part_t  *last;      // 指向最后一个部分
    ngx_list_part_t   part;      // 第一个部分
    size_t            size;      // 单个元素大小
    ngx_uint_t        nalloc;    // 每个部分可容纳的元素个数
    ngx_pool_t       *pool;      // 内存池
} ngx_list_t;
```

### 主要特点
- 链表由多个数组组成，每个数组称为一个part
- 支持动态扩容，当当前part满时自动创建新的part
- 内存分配由内存池管理

### 相关函数
```c
// 创建链表
ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);

// 初始化链表
ngx_int_t ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size);

// 添加元素
void *ngx_list_push(ngx_list_t *l);
```

### 使用示例
```c
// 创建链表
ngx_list_t *list = ngx_list_create(pool, 10, sizeof(ngx_str_t));

// 添加元素
ngx_str_t *str = ngx_list_push(list);
str->len = 5;
str->data = (u_char *) "hello";

// 遍历链表
ngx_list_part_t *part = &list->part;
ngx_str_t *item = part->elts;

for (ngx_uint_t i = 0; ; i++) {
    if (i >= part->nelts) {
        if (part->next == NULL) {
            break;
        }
        part = part->next;
        item = part->elts;
        i = 0;
    }
    
    // 处理 item[i]
    ngx_log_error(NGX_LOG_INFO, log, 0, "item: %V", &item[i]);
}
```

## 3. ngx_array_t - 数组结构

### 结构定义
```c
typedef struct {
    void        *elts;      // 元素数组
    ngx_uint_t   nelts;     // 实际元素个数
    size_t       size;      // 单个元素大小
    ngx_uint_t   nalloc;    // 数组容量
    ngx_pool_t  *pool;      // 内存池
} ngx_array_t;
```

### 主要特点
- 动态数组，支持自动扩容
- 内存连续分配，访问效率高
- 扩容时会重新分配内存并复制数据

### 相关函数
```c
// 创建数组
ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);

// 初始化数组
ngx_int_t ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size);

// 添加元素
void *ngx_array_push(ngx_array_t *a);
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);

// 销毁数组
void ngx_array_destroy(ngx_array_t *a);
```

### 使用示例
```c
// 创建数组
ngx_array_t *array = ngx_array_create(pool, 10, sizeof(ngx_str_t));

// 添加元素
ngx_str_t *str = ngx_array_push(array);
str->len = 5;
str->data = (u_char *) "hello";

// 遍历数组
ngx_str_t *items = array->elts;
for (ngx_uint_t i = 0; i < array->nelts; i++) {
    ngx_log_error(NGX_LOG_INFO, log, 0, "item: %V", &items[i]);
}
```

## 4. ngx_table_elt_t - 键值对结构

### 结构定义
```c
typedef struct {
    ngx_uint_t        hash;       // 键的哈希值
    ngx_str_t         key;        // 键
    ngx_str_t         value;      // 值
    u_char           *lowcase_key; // 小写的键（用于HTTP头部）
} ngx_table_elt_t;
```

### 主要特点
- 用于表示HTTP头部等键值对数据
- 支持哈希查找
- 键会被转换为小写以支持HTTP头部的不区分大小写特性

### 使用场景
- HTTP请求/响应头部
- 配置指令参数
- 环境变量

## 5. ngx_buf_t - 缓冲区结构

### 结构定义
```c
struct ngx_buf_s {
    u_char          *pos;         // 当前读取位置
    u_char          *last;        // 当前写入位置
    off_t            file_pos;    // 文件读取位置
    off_t            file_last;   // 文件结束位置

    u_char          *start;       // 缓冲区开始位置
    u_char          *end;         // 缓冲区结束位置
    ngx_buf_tag_t    tag;         // 缓冲区标签
    ngx_file_t      *file;        // 关联的文件
    ngx_buf_t       *shadow;      // 影子缓冲区

    /* 标志位 */
    unsigned         temporary:1;  // 临时缓冲区
    unsigned         memory:1;     // 内存缓冲区
    unsigned         mmap:1;       // mmap缓冲区
    unsigned         recycled:1;   // 可回收
    unsigned         in_file:1;    // 文件缓冲区
    unsigned         flush:1;      // 需要刷新
    unsigned         sync:1;       // 同步标志
    unsigned         last_buf:1;   // 最后一个缓冲区
    unsigned         last_in_chain:1; // 链中最后一个
    unsigned         last_shadow:1;   // 最后一个影子缓冲区
    unsigned         temp_file:1;      // 临时文件
};
```

### 主要特点
- 支持内存和文件两种数据源
- 通过标志位控制缓冲区行为
- 支持缓冲区链式操作

### 相关函数
```c
// 创建临时缓冲区
ngx_buf_t *ngx_create_temp_buf(ngx_pool_t *pool, size_t size);

// 分配缓冲区
#define ngx_alloc_buf(pool)  ngx_palloc(pool, sizeof(ngx_buf_t))
#define ngx_calloc_buf(pool) ngx_pcalloc(pool, sizeof(ngx_buf_t))

// 缓冲区大小
#define ngx_buf_size(b)                                                      \
    (ngx_buf_in_memory(b) ? (off_t) ((b)->last - (b)->pos):                  \
                            ((b)->file_last - (b)->file_pos))
```

## 6. ngx_chain_t - 缓冲区链结构

### 结构定义
```c
struct ngx_chain_s {
    ngx_buf_t    *buf;    // 缓冲区
    ngx_chain_t  *next;   // 下一个链节点
};
```

### 主要特点
- 将多个ngx_buf_t连接成链表
- 支持大数据的分块传输
- 常用于HTTP响应体、文件传输等场景

### 相关函数
```c
// 分配链节点
ngx_chain_t *ngx_alloc_chain_link(ngx_pool_t *pool);

// 创建缓冲区链
ngx_chain_t *ngx_create_chain_of_bufs(ngx_pool_t *pool, ngx_bufs_t *bufs);

// 释放链节点
#define ngx_free_chain(pool, cl)                                             \
    (cl)->next = (pool)->chain;                                              \
    (pool)->chain = (cl)

// 链操作
ngx_int_t ngx_chain_add_copy(ngx_pool_t *pool, ngx_chain_t **chain, ngx_chain_t *in);
ngx_chain_t *ngx_chain_get_free_buf(ngx_pool_t *p, ngx_chain_t **free);
void ngx_chain_update_chains(ngx_pool_t *p, ngx_chain_t **free, ngx_chain_t **busy, ngx_chain_t **out, ngx_buf_tag_t tag);
```

### 使用示例
```c
// 创建缓冲区链
ngx_chain_t *chain = NULL;
ngx_buf_t *buf = ngx_create_temp_buf(pool, 1024);
ngx_chain_t *cl = ngx_alloc_chain_link(pool);

cl->buf = buf;
cl->next = NULL;
chain = cl;

// 添加数据到缓冲区
ngx_memcpy(buf->pos, "Hello World", 11);
buf->last = buf->pos + 11;

// 遍历缓冲区链
ngx_chain_t *link = chain;
while (link) {
    ngx_buf_t *b = link->buf;
    // 处理缓冲区 b
    link = link->next;
}
```

## 7. ngx_queue_t - 双向链表

### 结构定义
```c
typedef struct ngx_queue_s  ngx_queue_t;

struct ngx_queue_s {
    ngx_queue_t  *prev;
    ngx_queue_t  *next;
};
```

### 主要特点
- 双向链表，支持快速插入和删除
- 通过嵌入结构体实现，类似Linux内核的list_head
- 内存效率高，不需要额外的指针

### 相关函数
```c
// 初始化队列
#define ngx_queue_init(q)                                                     \
    (q)->prev = q;                                                            \
    (q)->next = q

// 判断队列是否为空
#define ngx_queue_empty(h)                                                    \
    (h == (h)->prev)

// 插入节点
#define ngx_queue_insert_head(h, x)                                           \
    (x)->next = (h)->next;                                                    \
    (x)->next->prev = x;                                                      \
    (x)->prev = h;                                                            \
    (h)->next = x

#define ngx_queue_insert_tail(h, x)                                           \
    (x)->prev = (h)->prev;                                                    \
    (x)->prev->next = x;                                                      \
    (x)->next = h;                                                            \
    (h)->prev = x

// 删除节点
#define ngx_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next

// 获取数据
#define ngx_queue_data(q, type, link)                                         \
    (type *) ((u_char *) q - offsetof(type, link))
```

### 使用示例
```c
// 定义包含队列的结构体
typedef struct {
    ngx_str_t name;
    ngx_queue_t queue;
} my_item_t;

// 初始化队列头
ngx_queue_t queue;
ngx_queue_init(&queue);

// 创建并插入项目
my_item_t *item = ngx_palloc(pool, sizeof(my_item_t));
item->name.len = 5;
item->name.data = (u_char *) "hello";
ngx_queue_insert_tail(&queue, &item->queue);

// 遍历队列
ngx_queue_t *q;
for (q = ngx_queue_head(&queue); q != ngx_queue_sentinel(&queue); q = ngx_queue_next(q)) {
    my_item_t *item = ngx_queue_data(q, my_item_t, queue);
    ngx_log_error(NGX_LOG_INFO, log, 0, "item: %V", &item->name);
}
```

## 8. ngx_hash_t - 哈希表

### 结构定义
```c
typedef struct {
    ngx_hash_elt_t  **buckets;    // 哈希桶数组
    ngx_uint_t        size;       // 哈希表大小
} ngx_hash_t;

typedef struct {
    void             *value;      // 值
    u_short           len;        // 键长度
    u_char            name[1];    // 键（变长数组）
} ngx_hash_elt_t;
```

### 主要特点
- 静态哈希表，创建后不能修改
- 支持通配符匹配
- 内存效率高，查找速度快

### 相关函数
```c
// 查找元素
void *ngx_hash_find(ngx_hash_t *hash, ngx_uint_t key, u_char *name, size_t len);

// 通配符查找
void *ngx_hash_find_wc_head(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);
void *ngx_hash_find_wc_tail(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);

// 组合哈希表查找
void *ngx_hash_find_combined(ngx_hash_combined_t *hash, ngx_uint_t key, u_char *name, size_t len);
```

## 9. ngx_pool_t - 内存池

### 结构定义
```c
struct ngx_pool_s {
    ngx_pool_data_t   d;          // 内存池数据
    size_t            max;        // 最大分配大小
    ngx_pool_t       *current;    // 当前内存池
    ngx_chain_t      *chain;      // 链节点缓存
    ngx_pool_large_t *large;      // 大内存块链表
    ngx_pool_cleanup_t *cleanup;  // 清理函数链表
    ngx_log_t        *log;        // 日志对象
};
```

### 主要特点
- 内存分配效率高，减少内存碎片
- 支持自动清理，避免内存泄漏
- 支持大内存块和小内存块的不同处理策略

### 相关函数
```c
// 创建内存池
ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);

// 销毁内存池
void ngx_destroy_pool(ngx_pool_t *pool);

// 内存分配
void *ngx_palloc(ngx_pool_t *pool, size_t size);
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);

// 添加清理函数
ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);
```

## 总结

nginx的基础数据结构设计体现了以下特点：

1. **高效性**：针对Web服务器的高并发场景优化
2. **内存友好**：通过内存池管理，减少内存碎片
3. **类型安全**：通过结构体封装，提供类型安全
4. **扩展性**：支持动态扩容和链式操作
5. **易用性**：提供丰富的宏定义和函数接口

这些数据结构为nginx的高性能提供了坚实的基础，也是学习nginx源码的重要入口。 