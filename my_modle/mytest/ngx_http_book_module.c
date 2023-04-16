#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct
{
       ngx_flag_t iset;
}ngx_http_book_loc_conf_t;

static ngx_int_t ngx_http_book_init(ngx_conf_t *cf);

static void *ngx_http_book_create_loc_conf(ngx_conf_t *cf);

static ngx_int_t ngx_http_book_handler(ngx_http_request_t *r);


/*
static char *ngx_http_book_string(ngx_conf_t *cf, ngx_command_t *cmd,
        void *conf);
static char *ngx_http_book_counter(ngx_conf_t *cf, ngx_command_t *cmd,
        void *conf);
*/

// 由 ngx_module_t ->commands指定该成员
static ngx_command_t ngx_http_book_commands[] = {
   	{
                ngx_string("mybook"),// 配置项名称
                NGX_HTTP_LOC_CONF|NGX_CONF_FLAG, // 配置项类型 指定出现的位置和携带的参数个数
                ngx_conf_set_flag_slot, // 处理配置参数  自己的参数也只有自己知道怎么处理
                NGX_HTTP_LOC_CONF_OFFSET, // 配置文件中偏移量
                offsetof(ngx_http_book_loc_conf_t, iset), // 配合上面参数使用
                NULL // 配置项读取后的处理方法，必须为ngx_conf_post_t指针
                },

        ngx_null_command
};


/*
static u_char ngx_book_default_string[] = "Default String: Hello, world!";
*/
//static int ngx_book_visited_times = 0;

// 由 ngx_module_t ->ctx指定该成员
static ngx_http_module_t ngx_http_book_module_ctx = {
        NULL,                          /* preconfiguration */
        ngx_http_book_init,           /* postconfiguration 完成配置文件后被调用 */

        NULL,                          /* create main configuration */
        NULL,                          /* init main configuration */

        NULL,                          /* create server configuration */
        NULL,                          /* merge server configuration */

        ngx_http_book_create_loc_conf, /* create location configuration 需要创建loc级别存储时被调用*/
        NULL                            /* merge location configuration */
};


ngx_module_t ngx_http_book_module = {
        NGX_MODULE_V1, // 0，0，0，0，0，0，1
        &ngx_http_book_module_ctx,    /* module context   ctx变量*/
        ngx_http_book_commands,       /* module directives commands变量 */
        NGX_HTTP_MODULE,               /* module type  类别*/
        NULL,                          /* init master */
        NULL,                          /* init module */
        NULL,                          /* init process */
        NULL,                          /* init thread */
        NULL,                          /* exit thread */
        NULL,                          /* exit process */
        NULL,                          /* exit master */
        NGX_MODULE_V1_PADDING
};

// 实际处理请求的回调函数
static ngx_int_t
ngx_http_book_handler(ngx_http_request_t *r)
{
        ngx_int_t    rc;
        ngx_buf_t   *b;
        ngx_buf_t   *b2;
        ngx_chain_t  *out;
        ngx_chain_t  *out2;
        
       // ngx_http_book_loc_conf_t* my_conf;
       // u_char ngx_book_string[1024] = {0};
        //ngx_uint_t content_length = 0;
		


		if(!(r->method & NGX_HTTP_GET))
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "bookweb HTTP method is GET!");
			return NGX_HTTP_NOT_ALLOWED;
		}
        // 丢弃请求包体
		rc = ngx_http_discard_request_body(r);
		if(rc != NGX_OK)
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "bookweb ngx_http_discard_request_body is not NGX_OK:%d",rc);
			return rc;
		}

		/***set args of out***/
		/***set content_type**/
        ngx_str_t type = ngx_string("text/plain");
		r->headers_out.content_type = type;
		//r->headers_out.content_type.len = sizeof("text/plain") - 1;

		/**set status**/
		r->headers_out.status = NGX_HTTP_OK;

		/*set content_length_n**/
	//	r->headers_out.content_length_n = 100;

        ngx_str_t body_out = ngx_string("hello world");
        ngx_str_t body_out2 = ngx_string("abcdefg");

        r->headers_out.content_length_n = body_out.len + body_out2.len;

		/**send head ***/
		rc = ngx_http_send_header(r);
		if(rc == NGX_ERROR || rc > NGX_OK || r->header_only)
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "bookweb ngx_http_send_header is not NGX_OK:%d",rc);
			return rc;
		}

		/***set body***/
        //生成buf结构体
		b = ngx_create_temp_buf(r->pool,sizeof(ngx_buf_t));
		if(b == NULL)
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "bookweb ngx_create_temp_buf is get NULL");
			return NGX_HTTP_INTERNAL_SERVER_ERROR;
		}

        b2 = ngx_create_temp_buf(r->pool,sizeof(ngx_buf_t));
		if(b2 == NULL)
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "bookweb ngx_create_temp_buf is get NULL");
			return NGX_HTTP_INTERNAL_SERVER_ERROR;
		}

        out = ngx_pcalloc(r->pool,sizeof(ngx_chain_t));
		if(out == NULL)
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "bookweb ngx_create_temp_buf is get NULL");
			return NGX_HTTP_INTERNAL_SERVER_ERROR;
		}

        out2 = ngx_pcalloc(r->pool,sizeof(ngx_chain_t));
		if(out2 == NULL)
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "bookweb ngx_create_temp_buf is get NULL");
			return NGX_HTTP_INTERNAL_SERVER_ERROR;
		}

		out->buf = b;
		out->next = out2;
        out2->buf = b2;
        out2->next = NULL;

        // 在内存池中申请内存
        //b->pos = ngx_pcalloc(r->pool, body_out.len);
        ngx_memcpy(b->pos, body_out.data, body_out.len);
        // 指定buf中有效内容的结束位置
        b->last = b->pos + body_out.len;
        // 指定该buf为内容最后一个buf
        b->last_buf = 0;

        // 在内存池中申请内存
        // b2->pos = ngx_pcalloc(r->pool, body_out2.len);
        ngx_memcpy(b2->pos, body_out2.data, body_out2.len);
        // 指定buf中有效内容的结束位置
        b2->last = b2->pos + body_out2.len;
        // 指定该buf为内容最后一个buf
        b2->last_buf = 1;

        return ngx_http_output_filter(r, out);



		// /***set send body is file**/
		// b->in_file = 1;
		// u_char* filename = (u_char*)"/home/xxh/bookfile/test.txt";
		// b->file = ngx_pcalloc(r->pool, sizeof(ngx_file_t));
		// b->file->fd=ngx_open_file(filename,NGX_FILE_RDONLY | NGX_FILE_NONBLOCK, NGX_FILE_OPEN,0);
		// b->file->log = r->connection->log;
		// b->file->name.data = filename;
		// b->file->name.len = strlen(filename);
		// if(b->file->fd <= 0)
		// {
		// 	ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "bookweb file open is err");
		// 	return NGX_HTTP_NOT_FOUND;
		// }

		// if(ngx_file_info(filename,&b->file->info)==NGX_FILE_ERROR)
		// {
		// 	ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "bookweb ngx_file_info is err");
		// 	return NGX_HTTP_INTERNAL_SERVER_ERROR;
		// }
		
		// r->headers_out.content_length_n = b->file->info.st_size;
		// b->file_pos = 0;
		// b->file_last = b->file->info.st_size;
		
		// /***clean fd***/
		// ngx_pool_cleanup_t* cln = ngx_pool_cleanup_add(r->pool,sizeof(ngx_pool_cleanup_file_t));
		// if(cln == NULL)
		// {
		// 	ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "bookweb ngx_pool_cleanup_add is err");
		// 	return NGX_ERROR;
		// }
		// cln->handler = ngx_pool_cleanup_file;
		// ngx_pool_cleanup_file_t *clnf = cln->data;
		// clnf->fd = b->file->fd;
		// clnf->name = b->file->name.data;
		// clnf->log = r->pool->log;

		//return ngx_http_output_filter(r,out);


}

/***create ngx_http_book_loc_conf_t struct 创建loc级别内存时执行****/
static void *ngx_http_book_create_loc_conf(ngx_conf_t *cf)
{
        ngx_http_book_loc_conf_t* local_conf = NULL; // 自定义结构体
        local_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_book_loc_conf_t)); // 创建出内存空间
        if (local_conf == NULL)
        {
                return NULL;
        }
        // 重置结构体中参数
        //ngx_str_null(&local_conf->book_string);
	// 必须得重置
        local_conf->iset = NGX_CONF_UNSET;
        // 返回结构体指针
        return local_conf;
}

/****set moudle handler  完成配置文件后被调用*****/
static ngx_int_t
ngx_http_book_init(ngx_conf_t *cf) // ngx_conf_t 是啥
{
        ngx_http_handler_pt        *h;
        ngx_http_core_main_conf_t  *cmcf;
        // 获取对应main级别的配置项，相应的也有 srv和loc级别的配置项获取方式
        cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
        
        //涉及到存储问题，之后解决
        h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
        if (h == NULL) {
                return NGX_ERROR;
        }

        *h = ngx_http_book_handler; //指定 实际处理请求的回调函数

        return NGX_OK;
}





