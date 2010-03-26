#include "timaios.h"

/**
 *
 *
 *
 */
int main(int argc, char *argv[])
{
  TM_SERVER_SOCKET server_socket;
  struct sockaddr_in client;
  socklen_t len;
  int client_socket;
  int n, i;
  struct epoll_event ev, ev_ret[TM_NEVENTS];
  int epfd;
  int nfds;
  pid_t process_id;
  
  /*
   *
   */
  signal(SIGINT, tm_handle_signal_SIGINT);
  
  
  /* create pid file */
  process_id = getpid();
  tm_create_pid_file(process_id, TM_PID_FILE_NAME);
  
  printf(
    "========================================\n"
    " Boot timaios\n"
    " \n"
    " Pid File    : %s\n"
    " Server Port : %d\n"
    " NEVENTS     : %d\n"
    " Pid         : %d\n"
    "========================================\n",
    TM_PID_FILE_NAME,
    TM_SERVER_PORT,
    TM_NEVENTS,
    process_id
  );
  
  // deamonize process
  // int ret = daemon(1, 0);
  
  server_socket = tm_initialize_socket();
  
  epfd = tm_epoll_create(TM_NEVENTS);

  if (epfd < 0) {
    tm_perror("epoll_create");
    return 1;
  }
  
  tm_memory_reset(&ev, sizeof(ev));
  
  ev.events = EPOLLIN | EPOLLET;
  // ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
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
    nfds = epoll_wait(epfd, ev_ret, TM_NEVENTS, -1);
    if (nfds < 0) {
      tm_perror("epoll_wait");
      return 1;
    }
    
    if (nfds > 1) {
      tm_debug("after epoll_wait : nfds=%d\n", nfds);
    }
    
    for (i = 0; i < nfds; i++) {
      tm_connection_t *tm_connection = ev_ret[i].data.ptr;
      // tm_debug("fd=%d\n", ci->fd);
      
      if (tm_connection->fd == server_socket) {
        /* handling event that accept client'request */
        len = sizeof(client);
        client_socket = accept(server_socket, (struct sockaddr *)&client, &len);
        if (client_socket < 0) {
          if (errno == EAGAIN) {
            tm_debug("MADA KONAI");
            continue;
          } else {
            tm_perror("accept");
            return 1;
          }
        }
        
        // tm_debug("accept sock=%d\n", sock);
        
        tm_memory_reset(&ev, sizeof(ev));
        
        ev.events = EPOLLIN | EPOLLONESHOT | EPOLLET;
        ev.data.ptr = tm_create_connection(client_socket);
        if (ev.data.ptr == NULL) {
          tm_debug("failed to create connection");
          return 1;
        }
        
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &ev) != 0) {
          tm_perror("epoll_ctl");
          return 1;
        }
      } else {
        if (ev_ret[i].events & EPOLLIN) {
          /* event that read data from client's request */
          tm_connection->n = read(tm_connection->fd, tm_connection->raw_data, TM_REQUEST_MAX_READ_SIZE);
          
          if (tm_connection->n < 0) {
            tm_perror("read");
            return 1;
          }
          
          tm_connection->status = TM_CONNECTION_READ;
          tm_parse_request(tm_connection);
          ev_ret[i].events = EPOLLOUT;
          
          if (epoll_ctl(epfd, EPOLL_CTL_MOD, tm_connection->fd, &ev_ret[i]) != 0) {
            tm_perror("epoll_ctl");
            return 1;
          }
        } else if (ev_ret[i].events & EPOLLOUT) {
          /* event that write response data to the client */
          n = write(tm_connection->fd, tm_connection->raw_data, tm_connection->n);
          if (n < 0) {
            tm_perror("write");
            return 1;
          }
          
          tm_connection->status = TM_CONNECTION_WRITE;
          
          if (epoll_ctl(epfd, EPOLL_CTL_DEL, tm_connection->fd, &ev_ret[i]) != 0) {
            tm_perror("epoll_ctl");
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
