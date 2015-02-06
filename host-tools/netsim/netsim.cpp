/**
 * (C) 2009, 2010 by Holger Hans Peter Freyther <zecke@selfish.org>
 *
 * Based on nfqnl_test.c
 *
 * (C) 2005 by Harald Welte <laforge@gnumonks.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>		/* for NF_ACCEPT */

#include <libnetfilter_queue/libnetfilter_queue.h>

#define MICRO_SECONDS  1000000LL
#define TO_MICRO(tv) (tv->tv_sec * MICRO_SECONDS + tv->tv_usec)

#define MSEC(X) (X*1000)
static int sleep_for_mode = MSEC(70);

/* the maximum usleep can sleep */
#define WAIT_MAX 1000000

/* convert the time to microseconds and return the diff */
static long long diff_time(struct timeval *packet, struct timeval *now)
{
    unsigned long long packet_time = TO_MICRO(packet);
    unsigned long long now_time = TO_MICRO(now);

    return now_time - packet_time;
}

static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
	      struct nfq_data *nfa, void *data)
{
	int id = 0;
	struct nfqnl_msg_packet_hdr *ph;
	struct timeval tv, now;
	long long time_passed;
	long long wait_time;

	ph = nfq_get_msg_packet_hdr(nfa);
	if (ph)
		id = ntohl(ph->packet_id);

	/* figure out how long to delay this single packet */
	nfq_get_timestamp(nfa, &tv);
	gettimeofday(&now, NULL);
	time_passed = diff_time(&tv, &now);

	/* see how much more we want to wait */
	wait_time = sleep_for_mode - time_passed;
	if (wait_time < 0)
	    goto accept;

        /* for multi second intervals we need to do this */
	while (wait_time > 0) {
		long long sleep = wait_time;
		if (sleep > WAIT_MAX)
		    sleep = WAIT_MAX;

		usleep(sleep);
		wait_time -= WAIT_MAX;
	}

accept:
	return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

static void print_usage(void)
{
	printf("Usage: netsim [options]\n");
        printf("Also use a rule like \"iptables -I INPUT -i lo -j NFQUEUE\" to "
               "activate filtering on the device.\n");
}

static void print_help(void)
{
	printf("  -m --mode=umts|gsm|lan\t\n");
}

static void handle_options(int argc, char **argv)
{
	while (1) {
		int c, option_index;
		static struct option long_options[] = {
			{ "mode", 1, 0, 'm' },
			{ "help", 0, 0, 'h' },
			{ 0, 0, 0, 0},
		};

		c = getopt_long(argc, argv, "m:h", long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 'm':
			if (strcasecmp(optarg, "umts") == 0) {
				sleep_for_mode = MSEC(200);
			} else if (strcasecmp(optarg, "gsm") == 0) {
				sleep_for_mode = MSEC(900);
			} else if (strcasecmp(optarg, "lan") == 0) {
				sleep_for_mode = MSEC(70);
			} else {
				fprintf(stderr, "Unknown mode: '%s'\n", optarg);
				exit(-1);
			}
			    break;
		case 'h':
			print_usage();
			print_help();
			exit(0);
		}
	}
}

int main(int argc, char **argv)
{
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	struct nfnl_handle *nh;
	int fd;
	int rv;
	char buf[4096] __attribute__ ((aligned));

	printf("Simple latency simulator\n");
	printf("(C) 2009, 2010 by Holger Freyther\n");
	printf("(C) 2005 by Harald Welte\n");

	handle_options(argc, argv);

	h = nfq_open();
	if (!h) {
		fprintf(stderr, "error during nfq_open()\n");
		exit(1);
	}

	printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
	if (nfq_unbind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nfq_unbind_pf()\n");
		exit(1);
	}

	printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
	if (nfq_bind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nfq_bind_pf()\n");
		exit(1);
	}

	printf("binding this socket to queue '0'\n");
	qh = nfq_create_queue(h,  0, &cb, NULL);
	if (!qh) {
		fprintf(stderr, "error during nfq_create_queue()\n");
		exit(1);
	}

	printf("setting copy_packet mode\n");
	if (nfq_set_mode(qh, NFQNL_COPY_META, 0xffff) < 0) {
		fprintf(stderr, "can't set packet_copy mode\n");
		exit(1);
	}

	fd = nfq_fd(h);

	while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
		nfq_handle_packet(h, buf, rv);
	}

	printf("unbinding from queue 0\n");
	nfq_destroy_queue(qh);

	printf("closing library handle\n");
	nfq_close(h);

	exit(0);
}
