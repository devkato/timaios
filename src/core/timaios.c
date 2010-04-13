#include "timaios.h"

/* global configuration setting */

/* program configuration */
struct tm_configuration configuration;

/* @TODO should replace with Hash */
tm_action_t* action_map[10];

/**
 * Main function.
 *
 * @TODO make main routine with uulti process.
 *
 */
int main(int argc, char *argv[])
{
  /* initialize configuration files */
  if (tm_init_configuration(argc, argv) != 0) {
    return 0;
  }
  
  /* deamonize process */
  if (configuration.is_daemon) {
    daemon(1, 0);
  }
  
  TM_SERVER_SOCKET server_socket;
  int i;
  struct epoll_event ev, ev_ret[configuration.nevents];
  int epfd;
  int nfds;
  pid_t process_id;
  int num_of_process;
  
  num_of_process = tm_sys_get_cpu_num();
  
  /* register signal handler */
  signal(SIGINT, tm_handle_signal_SIGINT);
  
  /* create pid file */
  process_id = getpid();
  tm_create_pid_file(process_id, configuration.pid_file);
  
  printf(
    "========================================\n"
    " Boot timaios\n"
    " \n"
    " Pid File    : %s\n"
    " Server Port : %d\n"
    " NEVENTS     : %d\n"
    " Pid         : %d\n"
    " Log File    : %s\n"
    " Processor   : %d\n"
    "========================================\n",
    configuration.pid_file,
    configuration.port,
    configuration.nevents,
    process_id,
    configuration.log_file,
    num_of_process
  );
  
  /* @TODO close log file */
  configuration.log_fd = tm_logger_init();
  if (configuration.log_fd < 0) {
    tm_perror("open");
    return 1;
  }
  
  /* initialize action mapping */
  tm_acton_regiser_init();
  
  server_socket = tm_initialize_socket();
  
#define TM_MULTI_PROCESS

# ifdef TM_MULTI_PROCESS

  int socks[num_of_process];

  for (i = 0; i < num_of_process; i++) {
    int sockpair[2];
    
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sockpair) != 0) {
      tm_perror("pipe");
      return 1;
    }
    
    socks[i] = sockpair[0];
    pid_t pid = fork();
    
    if (pid < 0) {
      tm_perror("fork");
      return 1;
    }
    
    if (pid == 0) {
      // child process
      
      // close sockets that are already created.
      int j;
      for (j = 0; j < i; j++) {
        close(socks[j]);
      }
      // close one socket that is currently created with parent process
      close(sockpair[0]);
      create_worker_process(sockpair[1]);
    } else {
      // parent process
      close(sockpair[1]);
    }
  }

/*
  int sockpair[2];
  
  // if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair) != 0) {
  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sockpair) != 0) {
    tm_perror("pipe");
    return 1;
  }
  
  // tm_setnonblocking(sockpair[0]);
  // tm_setnonblocking(sockpair[1]);
  // 
  // int yes = 1;
  // ioctl(sockpair[0], FIOASYNC, &yes);
  // ioctl(sockpair[0], F_SETOWN, process_id);
  // fcntl(sockpair[0], F_SETFD, FD_CLOEXEC);
  // fcntl(sockpair[1], F_SETFD, FD_CLOEXEC);
  
  pid_t pid = fork();
  if (pid < 0) {
    tm_perror("fork");
    return 1;
  }
  
  if (pid == 0) {
    // child
    close(sockpair[0]);
    create_worker_process(sockpair[1]);
  }
  
  // parent
  close(sockpair[1]);
*/
  
  int sock_index;
  struct sockaddr_in client;
  int ret;
  int client_socket;
  
  i = 0;
  while (1) {
    socklen_t len = sizeof(client);
    client_socket = accept(server_socket, (struct sockaddr *)&client, &len);
    
    // tm_debug("client_socket : %d\n", client_socket);
    
    if (client_socket < 0) {
      tm_perror("accept");
      return 1;
    }
    // ret = send_data_to_worker(sockpair[0], client_socket);
    ret = send_data_to_worker(socks[i++], client_socket);
    i = i % num_of_process;
    close(client_socket);
  }

# else

  // create single worker process
  create_worker_process(server_socket);
  
# endif
  
  return 0;
}

void initialize_msg(struct msghdr *mh, struct tm_socket_message *msg)
{
  tm_memory_reset(msg, sizeof(*msg));
  msg->hdr.cmsg_level = SOL_SOCKET;
  msg->hdr.cmsg_type = SCM_RIGHTS;
  msg->hdr.cmsg_len = sizeof(*msg);
  
  tm_memory_reset(mh, sizeof(*mh));
  mh->msg_control = msg;
  mh->msg_controllen = msg->hdr.cmsg_len;
}

int send_data_to_worker(int _socket_pair_with_child, int _client_socket)
{
  struct msghdr mh;
  struct tm_socket_message msg;
  
  initialize_msg(&mh, &msg);
  msg.fd = _client_socket;
  
  // tm_debug("send_data_to_worker : from %d to %d\n", _client_socket, _socket_pair_with_child);
  
  return sendmsg(_socket_pair_with_child, &mh, 0);
}


// int send_data_to_worker2(int _socket_pair_with_child, int _client_socket)
// {
//   struct msghdr mh;
//   struct iovec iov[1];
//   
//   mh.msg_accrights = (caddr_t)&_client_socket;
//   mh.msg_accrightslen = sizeof(int);
//   
//   iov[0].iov_base = (char *) ch;
//   iov[0].iov_len = size;
//   
//   tm_debug("send_data_to_worker : from %d to %d\n", _client_socket, _socket_pair_with_child);
//   
//   return sendmsg(_socket_pair_with_child, &mh, 0);
// }


int receive_data_from_parent(int _socket_pair_with_parent)
{
  int rv;
  struct msghdr mh;
  struct tm_socket_message msg;
  
  initialize_msg(&mh, &msg);
  
  // tm_debug("receive_data_from_parent : from %d\n", _socket_pair_with_parent);
  
  rv = recvmsg(_socket_pair_with_parent, &mh, sizeof(mh));
  
  /* @TODO research what happen. */
  if (rv == 0 &&
    mh.msg_control &&
    mh.msg_controllen == sizeof(msg) &&
    msg.hdr.cmsg_level == SOL_SOCKET &&
    msg.hdr.cmsg_type == SCM_RIGHTS
  ) {
    return msg.fd;
  }
  
  return 1;
}


void create_worker_process(int pair_socket)
{
  int i;    /* increment number */
  int epfd; /* event descriptor */
  int nfds; /* number of descriptors that get events */
  
  /* event structure */
  struct epoll_event ev;
  
  tm_debug("create_worker_process : %d\n", pair_socket);
  
  epfd = tm_epoll_create(configuration.nevents);

  if (epfd < 0) {
    tm_perror("epoll_create");
    return;
  }
  
  tm_memory_reset(&ev, sizeof(ev));
  
  ev.events = EPOLLIN | EPOLLHUP;
  ev.data.ptr = tm_create_connection(pair_socket);
  if (ev.data.ptr == NULL) {
    tm_debug("failed to create connection");
    return;
  }
  
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, pair_socket, &ev) != 0) {
    tm_perror("epoll_ctl");
    return;
  }
  
  // tm_debug("create_worker_process : 2\n");
  
  while (1) {
    struct epoll_event ev_ret[configuration.nevents];
    
    nfds = epoll_wait(epfd, ev_ret, configuration.nevents, -1);
    // nfds = epoll_wait(epfd, ev_ret, configuration.nevents, 10 * 1000);
    
    // tm_debug("create_worker_process : nfds => %d\n", nfds);
    
    if (nfds < 0) {
      tm_perror("epoll_wait");
      return;
    }
    
    for (i = 0; i < nfds; i++) {
      tm_connection_t *tm_connection = ev_ret[i].data.ptr;
      
      if (tm_connection->fd == pair_socket) {
        
        // tm_debug("accept data through pair-socket\n");
        tm_handle_accept(tm_connection->fd, epfd, ev_ret[i]);
        
      } else if (ev_ret[i].events & EPOLLIN) {
        
        // tm_debug("read data through pair-socket\n");
        tm_handle_read(epfd, tm_connection, ev_ret[i]);
        
      } else if (ev_ret[i].events & EPOLLOUT) {
        
        // tm_debug("write data to pair-socket\n");
        tm_handle_write(epfd, tm_connection, ev_ret[i]);
        
      } else if (ev_ret[i].events & EPOLLHUP) {
        
        // tm_debug("error occurred on handling event.\n");
        tm_handle_close(epfd, tm_connection);
        
      }
    }
  }
  
  tm_debug("end of create_worker_process\n");
  // close(pair_socket);
}
