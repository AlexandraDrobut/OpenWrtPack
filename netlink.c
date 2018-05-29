
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <libxsocket.h>
#include <linux/if.h>
#include <libxutils/netlink.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>

struct sockaddr_nl local;
struct sockaddr_nl kernel;
struct msghdr msg;
struct iovec iov;
struct nlmsghdr *nlp;
struct rtmsg *rtp;
struct rtattr *rtap;
int rtl;
int nll;
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
	memset(&local, 0 , sizeof(local));
	memset(&kernel, 0 , sizeof(kernel));
	memset(&msg, 0, sizeof(msg));
	req.nl.nlmsg_len = sizeof(req.rt);
	req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_MULTI | NLM_F_DUMP;
	req.nl.nlmsg_type = RTM_GETROUTE;
	req.nl.nlmsg_seq = 1;
	req.rt.rtm_family = AF_INET;
	req.rt.rtm_table = RT_TABLE_MAIN | RT_TABLE_DEFAULT;

	kernel.nl_family = AF_NETLINK;

	msg.msg_name = (void*)&kernel;
	msg.msg_namelen = sizeof(kernel);
	iov.iov_base = (void*)&req.nl;
	iov.iov_len = req.nl.nlmsg_len;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	rtn = sendmsg(fd, &msg, 0);
}

void parse_nlmsghdr(struct nlmsghdr *h)
{
struct ifinfomsg *iface;
struct rtattr *attribute;
int len;

iface = NLMSG_DATA(h);
len = h->nlmsg_len - NLMSG_LENGTH(sizeof(*iface));

for (attribute = IFLA_RTA(iface); RTA_OK(attribute, len); attribute = RTA_NEXT(attribute, len))
{
   switch(attribute->rta_type)
   {
	   case IFLA_IFNAME:
	   printf("Interface %d : %s\n", iface->ifi_index, (char *) RTA_DATA(attribute));
	   break;
 	}
}
}
void read_reply()
{
	int len ;
	char buffer[8192];
	struct nlmsghdr *nh;
	struct iovec iov = { buffer, sizeof(buffer)};
	struct msghdr msg;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	len = recvmsg(fd, &msg, 0);
	for( nh = (struct nlmsghdr*)buffer; NLMSG_OK(nh,len); nh = NLMSG_NEXT(nh, len))
	{
		if( nh->nlmsg_type == NLMSG_DONE)
			return;

		parse_nlmsghdr(hdr);
	}
}

int main(int argc, int argv[])
{
int rc;
fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
local.nl_family = AF_NETLINK;
local.nl_pad =0;
local.nl_pid = getpid();
local.nl_groups = 0;

rc = bind(fd, (struct sockaddr*)&local, sizeof(local));

send_request();
read_reply();
close(fd);
return 0;
}