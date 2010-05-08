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
  TM_SERVER_SOCKET server_socket;
  pid_t process_id;
  int num_of_process;
  int max_fd = tm_get_max_fd_num();
  
  /* get number of logical cpus */
  num_of_process = tm_sys_get_cpu_num() - 1;
  
  /* initialize configuration files */
  if (tm_init_configuration(argc, argv) != 0) {
    return 0;
  }
  
  /* @TODO close log file */
  configuration.log_fd = tm_logger_init();
  if (configuration.log_fd < 0) {
    tm_perror("open");
    return 1;
  }
  
  /* initialize action mapping */
  tm_acton_regiser_init();
  
  /* register signal handler */
  signal(SIGINT, tm_handle_signal_SIGINT);
  
  /* create pid file */
  process_id = getpid();
  tm_create_pid_file(process_id, configuration.pid_file);
  
  server_socket = tm_initialize_socket();
  tm_debug("server_socket : %d", server_socket);
  
  /* Start-up message */
  printf(
    "========================================\n"
    " Boot timaios\n"
    " \n"
    " Pid File      : %s\n"
    " Server Port   : %d\n"
    " NEVENTS       : %d\n"
    " Pid           : %d\n"
    " Log File      : %s\n"
    " Child Process : %d\n"
    " Descriptor Limit : %d\n"
    "========================================\n",
    configuration.pid_file,
    configuration.port,
    configuration.nevents,
    process_id,
    configuration.log_file,
    num_of_process,
    max_fd
  );
  
  // tm_debug("SSIZE_MAX : %ld", SSIZE_MAX);
  
  configuration.nevents = max_fd;
  
  /* deamonize process */
  if (configuration.is_daemon) {
    daemon(1, 0);
  }
  
  /* single process */
  create_single_process(server_socket);
  
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
  
  tm_debug("send_data_to_worker : to %d", _socket_pair_with_child);
  if (sendmsg(_socket_pair_with_child, &mh, 0) < 0) {
    tm_perror("sendmsg");
    return -1;
  }
  return 0;
}


int receive_data_from_parent(int _socket_pair_with_parent)
{
  int rv;
  struct msghdr mh;
  struct tm_socket_message msg;
  
  initialize_msg(&mh, &msg);
  
  tm_debug("receive_data_from_parent : from %d", _socket_pair_with_parent);
  
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


void create_single_process(TM_SERVER_SOCKET _server_socket)
{
  int i;    /* increment number */
  int epfd; /* event descriptor */
  int nfds; /* number of descriptors that get events */
  
  /* event structure */
  struct epoll_event ev;
  
  tm_debug("create_single_process : %d", _server_socket);
  
  epfd = tm_epoll_create(configuration.nevents);

  if (epfd < 0) {
    tm_perror("epoll_create");
    return;
  }
  
  tm_memory_reset(&ev, sizeof(ev));
  
  ev.events = EPOLLIN | EPOLLHUP;
  ev.data.ptr = tm_create_connection(_server_socket);
  if (ev.data.ptr == NULL) {
    tm_debug("failed to create connection");
    return;
  }
  
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, _server_socket, &ev) != 0) {
    tm_perror("epoll_ctl");
    return;
  }
  
  // tm_debug("create_worker_process : 2");
  
  while (1) {
    struct epoll_event ev_ret[configuration.nevents];
    
    // nfds = epoll_wait(epfd, ev_ret, configuration.nevents, 3 * 1000);
    nfds = epoll_wait(epfd, ev_ret, configuration.nevents, -1);
    
    tm_debug("create_worker_process : nfds => %d", nfds);
    
    if (nfds < 0) {
      tm_perror("epoll_wait");
      return;
    }
    
    for (i = 0; i < nfds; i++) {
      tm_connection_t *tm_connection = ev_ret[i].data.ptr;
      
      if (tm_connection->fd == _server_socket) {
        
        tm_debug("accept data");
        tm_handle_accept(tm_connection->fd, epfd, ev_ret[i]);
        
      } else if (ev_ret[i].events & EPOLLIN) {
        
        tm_debug("read data");
        tm_handle_read(epfd, tm_connection, ev_ret[i]);
        
      } else if (ev_ret[i].events & EPOLLOUT) {
        
        tm_debug("write data");
        tm_handle_write(epfd, tm_connection, ev_ret[i]);
        
      } else if (ev_ret[i].events & EPOLLHUP) {
        
        tm_debug("error occurred on handling event.");
        tm_handle_close(epfd, tm_connection);
        
      }
    }
  }
  
  tm_debug("end of create_single_process");
}


void create_worker_process(int pair_socket)
{
  int i;    /* increment number */
  int epfd; /* event descriptor */
  int nfds; /* number of descriptors that get events */
  
  /* event structure */
  struct epoll_event ev;
  
  tm_debug("create_worker_process : %d", pair_socket);
  
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
  
  // tm_debug("create_worker_process : 2");
  
  while (1) {
    struct epoll_event ev_ret[configuration.nevents];
    
    // nfds = epoll_wait(epfd, ev_ret, configuration.nevents, -1);
    nfds = epoll_wait(epfd, ev_ret, configuration.nevents, 3 * 1000);
    
    tm_debug("create_worker_process : nfds => %d", nfds);
    
    if (nfds < 0) {
      tm_perror("epoll_wait");
      return;
    }
    
    for (i = 0; i < nfds; i++) {
      tm_connection_t *tm_connection = ev_ret[i].data.ptr;
      
      if (tm_connection->fd == pair_socket) {
        
        tm_debug("accept data through pair-socket");
        tm_handle_accept(tm_connection->fd, epfd, ev_ret[i]);
        
      } else if (ev_ret[i].events & EPOLLIN) {
        
        tm_debug("read data through pair-socket");
        tm_handle_read(epfd, tm_connection, ev_ret[i]);
        
      } else if (ev_ret[i].events & EPOLLOUT) {
        
        tm_debug("write data to pair-socket");
        tm_handle_write(epfd, tm_connection, ev_ret[i]);
        
      } else if (ev_ret[i].events & EPOLLHUP) {
        
        tm_debug("error occurred on handling event.");
        tm_handle_close(epfd, tm_connection);
        
      }
    }
  }
  
  tm_debug("end of create_worker_process");
  // close(pair_socket);
}
