/*
该文件负责接受终端用户的请求逻辑
*/


/*init*/
extern void ipc_init(void);
/*accept handler*/
extern void ipc_accept_handler();
/*recv and send handler*/
extern void ipc_recv_send_handler();