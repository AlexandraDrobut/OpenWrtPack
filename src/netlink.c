#include <stdio.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

struct sockaddr_nl local;
struct sockaddr_nl kernel;
struct msghdr msg;
struct iovec iov;

int rtn;
int fd;
char buf[8192];

struct {
struct nlmsghdr nl;

struct rtmsg rt;
char buf[8000];
}req;

void send_request()
{
	printf("Sending the request");
	memset(&local, 0 , sizeof(local));
	memset(&kernel, 0 , sizeof(kernel));
	memset(&msg, 0, sizeof(msg));
 
	req.nl.nlmsg_len = NLMSG_LENGTH(sizeof(req.rt));
	req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nl.nlmsg_type = RTM_GETROUTE;
	req.rt.rtm_family = AF_INET;
	req.rt.rtm_table = RT_TABLE_MAIN ;

	kernel.nl_family = AF_NETLINK;

	msg.msg_name = (void*)&kernel;
	msg.msg_namelen = sizeof(kernel);

	iov.iov_base = (void*)&req.nl;
	iov.iov_len = req.nl.nlmsg_len;

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	rtn = sendmsg(fd, &msg, 0);
}
int nll;
int rtn, rtl;
struct nlmsghdr *nlp;
struct rtmsg *rtp;
struct rtattr *rtap;

void parse_netlink()
{
    char *p;
 
    memset(buf, 0, sizeof(buf));
 
    p = buf;
    nll = 0;
 
    while(1) {
        rtn = recv(fd, p, sizeof(buf) - nll, 0);
 
        nlp = (struct nlmsghdr *)p;
 
        if (nlp->nlmsg_type == NLMSG_DONE)
          break;
 
        p += rtn;
        nll += rtn;
 
        if ((local.nl_groups & RTMGRP_IPV4_ROUTE) == RTMGRP_IPV4_ROUTE)
          break;
    }
}
 
void read_reply()
{
    char dst[INET6_ADDRSTRLEN] = {};
    char gw[INET6_ADDRSTRLEN] = {};
    int dst_len = 0;
    int ifindex = 0;
    int priority = 0;
 
    nlp = (struct nlmsghdr *) buf;
    for (; NLMSG_OK(nlp, nll); nlp=NLMSG_NEXT(nlp, nll)) {
 
        rtp = (struct rtmsg *) NLMSG_DATA(nlp);
 
        dst[0] = '\0';
        gw[0] = '\0';
        dst_len = 0;
        ifindex = 0;
        priority = 0;
 
        rtap = (struct rtattr *) RTM_RTA(rtp);
        rtl = RTM_PAYLOAD(nlp);
 
        for (; RTA_OK(rtap, rtl); rtap=RTA_NEXT(rtap,rtl)) {
            switch(rtap->rta_type) {
                case RTA_DST:
                    inet_ntop(AF_INET, RTA_DATA(rtap), dst, sizeof(dst));
                    break;
 
                case RTA_GATEWAY:
                    inet_ntop(AF_INET, RTA_DATA(rtap), gw, sizeof(gw));
                    break;
 
                case RTA_OIF:
                    ifindex = *(int *)RTA_DATA(rtap);
                    break;
 
                case RTA_PRIORITY:
                    priority = *(int *)RTA_DATA(rtap);
                    break;
 
                default:
                  break;
            }
        }
 
        dst_len = rtp->rtm_dst_len;
 
        if (!gw[0] || !ifindex)
            continue;
 
        if (dst_len == 0 && dst[0] == 0)
            printf("Found a default gateway! (priority %d)\n", priority);
 
        printf("destination: %s/%d gw_addr: %s ifindex %d\n",
               dst[0] ? dst : "0.0.0.0",
               dst_len, gw, ifindex);
    }
}
	
int main(int argc,char *argv[])
{

int rc;

printf("Creating socket\n");

fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
local.nl_family = AF_NETLINK;
local.nl_pad =0;
local.nl_pid = getpid();
local.nl_groups = 0;

rc = bind(fd, (struct sockaddr*)&local, sizeof(local));
printf("Sending request after binding the socket\n");
send_request();
parse_netlink();
read_reply();
close(fd);

return 0;
}
