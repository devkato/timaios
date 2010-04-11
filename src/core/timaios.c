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
  
  epfd = tm_epoll_create(configuration.nevents);

  if (epfd < 0) {
    tm_perror("epoll_create");
    return 1;
  }
  
  tm_memory_reset(&ev, sizeof(ev));
  
  ev.events = EPOLLIN | EPOLLHUP;
  ev.data.ptr = tm_create_connection(server_socket);
  if (ev.data.ptr == NULL) {
    tm_debug("failed to create connection");
    return 1;
  }
  
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_socket, &ev) != 0) {
    tm_perror("epoll_ctl");
    return 1;
  }
  
  for (;;) {
    // nfds = epoll_wait(epfd, ev_ret, configuration.nevents, -1);
    nfds = epoll_wait(epfd, ev_ret, configuration.nevents, 10 * 1000);
    if (nfds < 0) {
      tm_perror("epoll_wait");
      return 1;
    }
    
    // if (nfds > 1) {
    //   tm_debug("after epoll_wait : nfds=%d\n", nfds);
    // }
    
    for (i = 0; i < nfds; i++) {
      tm_connection_t *tm_connection = ev_ret[i].data.ptr;
      
      if (tm_connection->fd == server_socket) {
        tm_handle_accept(server_socket, epfd, ev_ret[i]);
      } else if (ev_ret[i].events & EPOLLIN) {
        tm_handle_read(epfd, tm_connection, ev_ret[i]);
      } else if (ev_ret[i].events & EPOLLOUT) {
        tm_handle_write(epfd, tm_connection, ev_ret[i]);
      } else if (ev_ret[i].events & EPOLLHUP) {
        tm_handle_close(epfd, tm_connection);
      }
    }
  }
  
  close(server_socket);
  
  return 0;
}
