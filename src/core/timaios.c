#include "timaios.h"

/* global configuration setting */

/* program configuration */
struct tm_configuration configuration;

/* @TODO should replace with Hash */
tm_action_t* action_map[10];

/**
 * Main function.
 *
 *
 */
int main(int argc, char *argv[])
{
  /* initialize configuration files */
  if (tm_init_configuration(argc, argv) != 0) {
    return 0;
  }
  
  TM_SERVER_SOCKET server_socket;
  struct sockaddr_in client;
  socklen_t len;
  int client_socket;
  int n, i;
  struct epoll_event ev, ev_ret[configuration.nevents];
  int epfd;
  int nfds;
  pid_t process_id;
  
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
    "========================================\n",
    configuration.pid_file,
    configuration.port,
    configuration.nevents,
    process_id,
    configuration.log_file
  );
  
  /* @TODO close log file */
  configuration.log_fd = tm_logger_init();
  if (configuration.log_fd < 0) {
    tm_perror("open");
    return 1;
  }
  
  /* initialize action mapping */
  tm_acton_regiser_init();
  
  /* deamonize process */
  if (configuration.is_daemon) {
    daemon(1, 0);
  }
  
  server_socket = tm_initialize_socket();
  
  epfd = tm_epoll_create(configuration.nevents);

  if (epfd < 0) {
    tm_perror("epoll_create");
    return 1;
  }
  
  tm_memory_reset(&ev, sizeof(ev));
  
  ev.events = EPOLLIN;
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
    nfds = epoll_wait(epfd, ev_ret, configuration.nevents, -1);
    if (nfds < 0) {
      tm_perror("epoll_wait");
      return 1;
    }
    
    if (nfds > 1) {
      tm_debug("after epoll_wait : nfds=%d\n", nfds);
    }
    
    for (i = 0; i < nfds; i++) {
      tm_connection_t *tm_connection = ev_ret[i].data.ptr;
      
      if (tm_connection->fd == server_socket) {
        /* handling event that accept client'request */
        len = sizeof(client);
        client_socket = accept(server_socket, (struct sockaddr *)&client, &len);
        tm_setnonblocking(client_socket);
        
        if (client_socket < 0) {
          if (errno == EAGAIN) {
            tm_debug("MADA KONAI");
            continue;
          } else {
            tm_perror("accept");
            return 1;
          }
        }
        
        tm_memory_reset(&ev, sizeof(ev));
        
        ev.events = EPOLLIN | EPOLLET;
        ev.data.ptr = tm_create_connection(client_socket);
        if (ev.data.ptr == NULL) {
          tm_debug("failed to create connection");
          return 1;
        }
        
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &ev) != 0) {
          tm_perror("epoll_ctl:(EPOLL_CTL_ADD)");
          return 1;
        }
      } else {
        if (ev_ret[i].events & EPOLLIN) {
          /* event that read data from client's request */
          tm_connection->n = tm_readv(tm_connection->fd, tm_connection->raw_data);
          
          /* @TODO manage EAGAIN status */
          if (tm_connection->n < 0) {
            if (errno == EAGAIN) {
              tm_debug("waiting data ...");
              continue;
            } else {
              tm_perror("read");
              return 1;
            }
          }
          
          tm_connection->status = TM_CONNECTION_READ;
          tm_parse_request(tm_connection);
          ev_ret[i].events = EPOLLOUT;
          
          if (epoll_ctl(epfd, EPOLL_CTL_MOD, tm_connection->fd, &ev_ret[i]) != 0) {
            tm_perror("epoll_ctl(EPOLL_CTL_MOD)");
            return 1;
          }
        } else if (ev_ret[i].events & EPOLLOUT) {
          /* event that write response data to the client */
          
          /* @TODO pluggable response data */
          tm_action_t *action = tm_action_find(tm_connection->request->path);
          if (action) {
            action->func(tm_connection);
          } else {
            tm_action_not_found(tm_connection);
          }
          
          /* write header data */
          tm_http_write_header(tm_connection);
          
          /* write data into socket */
          n = tm_write_response_data(tm_connection);
          
          if (n < 0) {
            tm_perror("write");
            return 1;
          }
          
          tm_connection->status = TM_CONNECTION_WRITE;
          
          if (epoll_ctl(epfd, EPOLL_CTL_DEL, tm_connection->fd, &ev_ret[i]) != 0) {
            tm_perror("epoll_ctl(EPOLL_CTL_DEL)");
            return 1;
          }
          
          close(tm_connection->fd);
          
          tm_destroy_connection(ev_ret[i].data.ptr);
        }
      }
    }
  }
  
  close(server_socket);
  
  return 0;
}
