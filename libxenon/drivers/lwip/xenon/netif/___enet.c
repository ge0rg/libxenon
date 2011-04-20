#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "netif/etharp.h"
#include "cache.h"
#include "processor.h"

#define TX_DESCRIPTOR_NUM 0x10
#define RX_DESCRIPTOR_NUM 0x10
#define MTU 1528
//#define MEM(x) (0x8000000000000000ULL|((long)x))
#define MEM(x) (0x80000000ULL|((long)x))

#define dcache_flush memdcbst
#define dcache_inv memdcbf
#define	flush_code memicbi

struct enet_context
{
	volatile uint32_t *rx_descriptor_base;
	void *rx_receive_base;
	int rx_descriptor_rptr;

	volatile uint32_t *tx_descriptor_base;
	int tx_descriptor_wptr;
	void *tx_buffer_base;

	struct eth_addr *ethaddr;
};

static int enet_open(struct netif *ctx);
static struct pbuf *enet_linkinput(struct enet_context *context);
static err_t enet_linkoutput(struct netif *netif, struct pbuf *p);
static err_t enet_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);

static inline unsigned int swap32(unsigned int v)
{
	return ((v&0xFF) << 24) | ((v&0xFF00)<<8) | ((v&0xFF0000)>>8) | ((v&0xFF000000)>>24);
}

static inline void write32(unsigned int addr, unsigned int value)
{
	*(unsigned int*)(addr) = value;
}

static inline unsigned int read32(unsigned int addr)
{
	return *(unsigned int*)(addr);
}

static inline void write32mem(unsigned int addr, unsigned int value)
{
	*(unsigned int*)(addr) = value;
}

static inline unsigned int read32mem(unsigned int addr)
{
	return *(unsigned int*)(addr);
}

static unsigned short phy_read(int addr)
{
	write32(0xea001444, swap32((addr << 11) | 0x50));
	while (swap32(read32(0xea001444)) & 0x10);
	return swap32(read32(0xea001444)) >> 16;
}

static void phy_write(int addr, unsigned short data)
{
	write32(0xea001444, swap32((addr << 11) | 0x70 | (data << 16)));
	while (swap32(read32(0xea001444)) & 0x10);
}

static void gpio_control(int reg, int clear, int set)
{
	int r[]={0x20,0x24,0x28,0x30,0x34,0x38,0x40,0x44,0x48};
	int b = swap32(read32(0xea001000 | r[reg]));
	b &= ~clear;
	b |= set;
	write32(0xea001000 | r[reg], swap32(b));
}


static void wait_empty_tx(struct enet_context *context)
{
//	printf(">> CURRENT TX PTR: %08x\n", swap32(read32(0xea00140c)));
	while (swap32(context->tx_descriptor_base[context->tx_descriptor_wptr * 4 + 1]) & 0x80000000); // busy
}

static inline int virt_to_phys(volatile void *ptr)
{
	return ((long)ptr) & 0x7fffffff;
}

static void tx_data(struct enet_context *context, unsigned char *data, int len)
{
	wait_empty_tx(context);
	int descnr = context->tx_descriptor_wptr;
	volatile uint32_t *descr = context->tx_descriptor_base + descnr * 4;
	
//	printf("using tx descriptor %d\n", descnr);

//	printf("really before %08x %08x %08x %08x\n", descr[0], descr[1], descr[2], descr[3]);
	
	descr[0] = swap32(len);
	descr[2] = swap32(virt_to_phys(data));
	descr[3] = swap32(len | ((descnr == TX_DESCRIPTOR_NUM - 1) ? 0x80000000 : 0));
	descr[1] = swap32(0xc0230000); // ownership, ...
	
//	printf("data at %08x\n", virt_to_phys(data));

	dcache_flush(descr, 0x10);

//	printf("before %08x %08x %08x %08x\n", descr[0], descr[1], descr[2], descr[3]);
	
	write32(0xea001400, read32(0xea001400) | swap32(0x10)); // Q0_START
	
//	printf("after %08x %08x %08x %08x\n", descr[0], descr[1], descr[2], descr[3]);
	
	context->tx_descriptor_wptr++;
	context->tx_descriptor_wptr %= TX_DESCRIPTOR_NUM;

	write32(0xea001410, swap32(0x00101c11));	// enable RX
	
//	printf("TX\n");
}

static void tx_init(struct enet_context *context, void *base)
{
	int i;
	
	context->tx_descriptor_base = base;
	context->tx_descriptor_wptr = 0;
	
	for (i=0; i < TX_DESCRIPTOR_NUM; ++i)
	{
				/* tx descriptors */
		context->tx_descriptor_base[i * 4 + 0] = 0;
		context->tx_descriptor_base[i * 4 + 1] = 0;
		context->tx_descriptor_base[i * 4 + 2] = 0;
		if (i != TX_DESCRIPTOR_NUM - 1)
			context->tx_descriptor_base[i * 4 + 3] = 0;
		else
			context->tx_descriptor_base[i * 4 + 3] = swap32(0x80000000);
	}

	memdcbst(context->tx_descriptor_base, TX_DESCRIPTOR_NUM * 0x10);

	context->tx_buffer_base = base + TX_DESCRIPTOR_NUM * 0x10;
	
	write32(0xea001400, swap32(0x00001c00));
	write32(0xea001404, swap32(virt_to_phys(context->tx_descriptor_base)));
	write32(0xea001400, swap32(0x00011c00));
	write32(0xea001404, swap32(virt_to_phys(context->tx_descriptor_base)));
	write32(0xea001400, swap32(0x00001c00));
}


static void rx_init(struct enet_context *context, void *base)
{
	int i;
	
	context->rx_descriptor_base = base;
	context->rx_receive_base = base + RX_DESCRIPTOR_NUM * 0x10;
	context->rx_descriptor_rptr = 0;
	
			/* setup rx descriptors */
	for (i=0; i < RX_DESCRIPTOR_NUM; ++i)
	{
		context->rx_descriptor_base[i * 4 + 0] = swap32(0);
		context->rx_descriptor_base[i * 4 + 1] = swap32(0xc0000000);
		context->rx_descriptor_base[i * 4 + 2] = swap32(virt_to_phys(context->rx_receive_base + i * MTU));
		context->rx_descriptor_base[i * 4 + 3] = swap32(MTU | ((i == RX_DESCRIPTOR_NUM - 1) ? 0x80000000 : 0));
	}
	dcache_flush(context->rx_descriptor_base, RX_DESCRIPTOR_NUM * 0x10);
	
	write32(0xea001414, swap32(virt_to_phys(context->rx_descriptor_base))); // RX queue start
	write32(0xea001440, 0x1100004); 
	write32(0xea001450, 0);
}


static void
arp_timer(void *arg)
{
	etharp_tmr();
	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}


err_t enet_init(struct netif *netif)
{
	struct enet_context * context;
	
	context = (struct enet_context *)mem_malloc(sizeof(struct enet_context));
	if(context == NULL) {
		printf("enet: Failed to allocate context memory.\n");
		return ERR_MEM;
	} else {
		memset(context, 0, sizeof(struct enet_context));

		// enet_write_mac_address(context, m);
		memcpy(netif->hwaddr, "\x00\x01\x30\x44\x55\x66", 6);
	}
	
	netif->state = context;
	netif->name[0] = 'e';
	netif->name[1] = 'n';
	netif->output = enet_output;
	netif->linkoutput = enet_linkoutput;
	context->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
	
	netif->hwaddr_len = 6;
	netif->mtu = 1500;
	netif->flags = NETIF_FLAG_BROADCAST;
	
	printf("NETIF at %p\n", netif);
	
	enet_open(netif);
	etharp_init();
	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
	return ERR_OK;
}


static int enet_open(struct netif *netif)
{
	struct enet_context *context = (struct enet_context *) netif->state;
	
	printf("NIC reset\n");
	write32(0xea001424, 0); // disable interrupts
	write32(0xea001428, 0x01805508);
	printf("reset: %08x\n", read32(0xea001428));
	write32(0xea001428, 0x01005508);

	write32(0xea001444, swap32(4));
	write32(0xea001444, swap32(0));
	printf("1478 before: %08x\n", read32(0xea001478));
		// Set MTU
	write32(0xea001478, 0xf2050000); printf("1478: %08x\n", read32(0xea001478));
	
		// important
	write32(0xea001450, 0x60230000);

		// mac 1+2
	write32(0xea001478, 0xF2050000 | (netif->hwaddr[0] << 8) | (netif->hwaddr[1] << 0));
	write32(0xea00147c, (netif->hwaddr[2] << 24) | (netif->hwaddr[3] << 16) | (netif->hwaddr[4] << 8) | (netif->hwaddr[5] << 0) );

	write32(0xea001460, 0x380e0000 | (netif->hwaddr[0] << 8) | (netif->hwaddr[1] << 0));
	write32(0xea001464, (netif->hwaddr[2] << 24) | (netif->hwaddr[3] << 16) | (netif->hwaddr[4] << 8) | (netif->hwaddr[5] << 0) );

		// multicast hash
	write32(0xea001468, 0);
	write32(0xea00146c, 0);

	write32(0xea001400, 0x001c0000);
	write32(0xea001410, swap32(0x00101c00));
	
	write32(0xea001440, 0x01190004);

	void *base = (void*)0x2000000;
	
	printf("init tx\n");
	tx_init(context, (void*)MEM(base));
	printf("init rx\n");
	rx_init(context, (void*)MEM(base + 0x8000));
#if 1
	printf("reinit phy..\n");
	phy_write(0, 0x9000);
	printf("%08x\n", phy_read(0));
	while (phy_read(0) & 0x8000);

	phy_write(0x10, 0x8058);
	phy_write(4, 0x5e1);
	phy_write(0x14, 0x4048);

	printf("phy init complete.\n");

	int linkstate = phy_read(1);
	if (!(linkstate & 4))
	{
		printf("waiting for link..\n");
		while (!(phy_read(1) & 4));
	}

	linkstate = phy_read(0x11);

	printf("link up at %dMBit, %s-duplex\n", linkstate & 0x8000 ? 100 : 10, linkstate & 0x4000 ? "full" : "half");
#endif
	
	
//	write32(0xea001428, 0x01805508);
	write32(0xea001428, 0x01005508);
	write32(0xea001410, swap32(0x00101c11));	// enable RX
	write32(0xea001400, swap32(0x00001c01));	// enable TX

	return 0;
}

static struct pbuf *enet_linkinput(struct enet_context *context)
{
	volatile uint32_t *d = context->rx_descriptor_base + context->rx_descriptor_rptr * 4;
	dcache_inv(d, 0x10);
	if (swap32(d[1]) & 0x80000000) /* check ownership */
	{
//		printf("no data!\n");
		return 0;
	}

//	printf("RX descriptor %d contains data!\n", context->rx_descriptor_rptr);
//	printf("%08x %08x %08x %08x\n", 
//		swap32(d[0]), 
//		swap32(d[1]), 
//		swap32(d[2]), 
//		swap32(d[3]));

	int size = swap32(d[0]) & 0xFFFF;
	void *phys_addr = context->rx_receive_base + context->rx_descriptor_rptr * MTU;
	dcache_inv(phys_addr, size);
		
	// hexdump(phys_addr, size);

	struct pbuf *p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL), *q;

	if (p != NULL) {
		int rptr = 0;
		/* We iterate over the pbuf chain until we have read the entire
		 packet into the pbuf. */
		for(q = p; q != NULL; q = q->next) {
				/* Read enough bytes to fill this pbuf in the chain. The
					 available data in the pbuf is given by the q->len
					 variable. */
			memcpy(q->payload, phys_addr + rptr, q->len);
			rptr += q->len;
		}
#ifdef LINK_STATS
		lwip_stats.link.recv++;
#endif /* LINK_STATS */			
	} else {
#ifdef LINK_STATS
		lwip_stats.link.memerr++;
		lwip_stats.link.drop++;
#endif /* LINK_STATS */			
	}

		/* reclaim descriptor */	
	d[2] = swap32(virt_to_phys(context->rx_receive_base + context->rx_descriptor_rptr * MTU));
	d[3] = swap32(MTU | ((context->rx_descriptor_rptr == RX_DESCRIPTOR_NUM - 1) ? 0x80000000 : 0));
	d[0] = swap32(0);
	d[1] = swap32(0xc0000000);
	dcache_flush(d, 0x10);

	context->rx_descriptor_rptr++;
	context->rx_descriptor_rptr %= RX_DESCRIPTOR_NUM;	

	return p;
}

static err_t enet_linkoutput(struct netif *netif, struct pbuf *p)
{
	struct pbuf *q;
	struct enet_context *context = (struct enet_context *) netif->state;
	
//	printf("enet linkoutput\n");
	
	void *dstptr = context->tx_buffer_base + context->tx_descriptor_wptr * MTU;
	int offset = 0;
	
	for(q = p; q != NULL; q = q->next) {
		memcpy(dstptr + offset, q->payload, q->len);
		offset += q->len;
	}
	
	dcache_flush(dstptr, p->tot_len);
	tx_data(context, dstptr, p->tot_len);
	
//	printf("ok, data transmitted!\n");

	return 0;
}


static err_t
enet_output(struct netif *netif, struct pbuf *p,
			struct ip_addr *ipaddr)
{
//	printf("ENET OUTPUT\n");
  return etharp_output(netif, ipaddr, p);
}

static void
enet_input(struct netif *netif)
{
	struct enet_context *context = (struct enet_context *) netif->state;
	struct eth_hdr *ethhdr;
	struct pbuf *p;

	p = enet_linkinput(context);

	if (p == NULL)
		return;

#if LINK_STATS
	lwip_stats.link.recv++;
#endif /* LINK_STATS */

	ethhdr = p->payload;

	switch (htons(ethhdr->type))
	{
		/* IP packet? */
	case ETHTYPE_IP:
		/* update ARP table */
		etharp_ip_input(netif, p);
		/* skip Ethernet header */
		pbuf_header(p, -sizeof(struct eth_hdr));
		/* pass to network layer */
		netif->input(p, netif);
		break;
			
	case ETHTYPE_ARP:
		/* pass p to ARP module	*/
		etharp_arp_input(netif, context->ethaddr, p);
		break;
	default:
		pbuf_free(p);
		p = NULL;
		break;
	}
}

int cnt;

void
enet_poll(struct netif *netif)
{
//	printf("\r%08x", mfdec());
	enet_input(netif);
}


void enet_quiesce(void)
{
	write32(0xea001400, 0);
	write32(0xea001410, 0);
}
