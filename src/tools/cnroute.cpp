//
//  main.cpp
//  cnroute
//
//  Created by 周海洋 on 15/10/14.
//  Copyright © 2015年 周海洋. All rights reserved.
//

#include <stdio.h>

#include <stdlib.h>

#include <iostream>
#include <vector>
#include <fstream>

#include <getopt.h>
#include <err.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

/* Darwin doesn't define this for some very odd reason */
#ifndef SA_SIZE
# define SA_SIZE(sa)                        \
(  (!(sa) || ((struct sockaddr *)(sa))->sa_len == 0) ?  \
sizeof(long)     :               \
1 + ( (((struct sockaddr *)(sa))->sa_len - 1) | (sizeof(long) - 1) ) )
#endif


in_addr_t default_gateway(void)
{
  int mib[6];
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;
  mib[3] = 0;
  mib[4] = NET_RT_DUMP;
  mib[5] = 0;
  size_t needed = 0;
  if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
    err(1, "sysctl: net.route.0.0.dump estimate");
  }
  char *buf, *next, *lim;
  if ((buf = (char *)malloc(needed)) == NULL) {
    errx(2, "malloc(%lu)", (unsigned long)needed);
  }
  if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
    err(1, "sysctl: net.route.0.0.dump");
  }
  lim  = buf + needed;
  struct rt_msghdr *rtm;
  for (next = buf; next < lim; next += rtm->rtm_msglen) {
    rtm = (struct rt_msghdr *)next;
    struct sockaddr *sa = (struct sockaddr *)(rtm + 1);
    struct sockaddr_in *dest = (struct sockaddr_in *)sa;
    if (dest->sin_addr.s_addr != 0) {
      break;
    }
    struct sockaddr_in *gateway = (struct sockaddr_in *)(SA_SIZE(sa) + (char *)sa);
    if (gateway->sin_addr.s_addr != 0) {
      free(buf);
      return gateway->sin_addr.s_addr;
    }
  }
  
  free(buf);
  return 0;
}

int change_route(int mode,
                 const std::vector<std::pair<struct sockaddr_in, struct sockaddr_in> > &route_entries,
                 struct sockaddr_in &gateway)
{
  if (route_entries.empty()) {
    return 0;
  }
  
  int socket_fd = socket(PF_ROUTE, SOCK_RAW, AF_INET);
  if (socket_fd < 0) {
    std::cerr<<"failed to open route socket. exiting..."<<std::endl;
    return -1;
  }
  typedef struct {
    struct rt_msghdr msg_header;
    struct sockaddr_in sin[3];// Destination, gateway, netmask,
  } RouteMessage;
  RouteMessage route_message;
  memset(&route_message, 0, sizeof(route_message));
  
  route_message.msg_header.rtm_msglen = sizeof(route_message);
  route_message.msg_header.rtm_version = RTM_VERSION;
  route_message.msg_header.rtm_index = 0;
  route_message.msg_header.rtm_type = mode;
  route_message.msg_header.rtm_addrs = RTA_DST | RTA_GATEWAY | RTA_NETMASK;
  route_message.msg_header.rtm_pid = 0;
  route_message.msg_header.rtm_seq = 0;
  route_message.msg_header.rtm_errno = 0;
  route_message.msg_header.rtm_flags = RTF_UP | RTF_GATEWAY;
  
  
  for (std::vector<std::pair<struct sockaddr_in, struct sockaddr_in> >::const_iterator iter = route_entries.begin();
       iter != route_entries.end(); ++iter) {
    route_message.sin[0] = iter->first; //destination
    route_message.sin[1] = gateway; //gateway
    route_message.sin[2] = iter->second; //mask
    if (write(socket_fd, &route_message, sizeof(route_message)) < 0 ) {
      std::string dest = inet_ntoa(route_message.sin[0].sin_addr);
      std::string gate = inet_ntoa(route_message.sin[1].sin_addr);
      std::string mask = inet_ntoa(route_message.sin[2].sin_addr);
      std::cerr<<"failed to write route message \""<<dest<<"\t"<<mask<<"\t"<<gate<<"\""<<std::endl;
      continue;
    }
  }
  if (close(socket_fd) < 0) {
    std::cerr<<"failed to close route socket. exiting..."<<std::endl;
  }
  
  return 0;
  
}
void print_usage(char *command)
{
  printf("Usage(MUST HAVE ROOT PRIVILEGE!): \n");
  printf("To Add Route Entries:   \t%s -f [ip file] -a\n",command);
  printf("To Change Route Entries:\t%s -f [ip file] -c\n",command);
  printf("To Delete Route Entries:\t%s -f [ip file] -d\n",command);
}

int main(int argc, char *argv[])
{
  int mode = 0;
  char ch = 0;
  std::string file;
  while ((ch = getopt(argc, argv, "acdhf:"))!=-1) {
    switch (ch) {
      case 'a':
        mode = RTM_ADD;
        break;
      case 'c':
        mode = RTM_CHANGE;
        break;
      case 'd':
        mode = RTM_DELETE;
        break;
      case 'h':
        print_usage(argv[0]);
        return 0;
      case 'f':
        file = optarg;
      default:
        break;
    }
  }
  if (mode == 0 || file == "") {
    print_usage(argv[0]);
    return -1;
  }
  std::string line;
  std::ifstream ifs(file);
  if (!ifs.is_open()) {
    std::cerr<<"failed to open file \""<<file<<"\""<<std::endl;
    return -1;
  }
  std::vector<std::pair<struct sockaddr_in, struct sockaddr_in> > route_entries;
  struct sockaddr_in ip, mask, gateway;
  memset(&ip, 0, sizeof(struct sockaddr_in));
  ip.sin_len = sizeof(struct sockaddr_in);
  ip.sin_family = AF_INET;
  memset(&mask, 0, sizeof(struct sockaddr_in));
  mask.sin_len = sizeof(struct sockaddr_in);
  mask.sin_family = AF_INET;
  memset(&gateway, 0, sizeof(struct sockaddr_in));
  gateway.sin_len = sizeof(struct sockaddr_in);
  gateway.sin_family = AF_INET;
  while (true) {
    std::getline(ifs, line);
    if (ifs.eof()) {
      break;
    }
    std::vector<std::string> route_params;
    boost::split(route_params, line, boost::is_any_of("/"));
    if (route_params.size()!=2) {
      std::cerr<<"wrong file format: \""<<file<<"\""<<std::endl;
      return -1;
    }
    
    ip.sin_addr.s_addr = inet_addr(route_params[0].c_str());
    mask.sin_addr.s_addr = htonl(~((1 << (32 - boost::lexical_cast<unsigned int>(route_params[1]))) - 1));
    route_entries.push_back(std::make_pair(ip, mask));
  }
  gateway.sin_addr.s_addr = default_gateway();
  if (gateway.sin_addr.s_addr == 0) {
    std::cerr<<"failed to get default gateway. exiting..."<<std::endl;
    return -1;
  }
  if (change_route(mode, route_entries, gateway)!=0) {
    std::cerr<<"exiting with errors..."<<std::endl;
  }
  return 0;
}