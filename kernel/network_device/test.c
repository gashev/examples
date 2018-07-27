#include <linux/module.h>
#include <linux/etherdevice.h>
#include <net/rtnetlink.h>

#define DRV_NAME	"test"
#define DRV_VERSION	"0.1"

struct pcpu_dstats {
	u64			tx_packets;
	u64			tx_bytes;
	struct u64_stats_sync	syncp;
};

static int dummy_dev_init(struct net_device *dev)
{
	dev->dstats = netdev_alloc_pcpu_stats(struct pcpu_dstats);
	if (!dev->dstats)
		return -ENOMEM;

	return 0;
}

static void dummy_dev_uninit(struct net_device *dev)
{
	free_percpu(dev->dstats);
}

static const struct net_device_ops dummy_netdev_ops = {
	.ndo_init	= dummy_dev_init,
	.ndo_uninit	= dummy_dev_uninit
};

static void setup(struct net_device *dev)
{
	ether_setup(dev);

	/* Initialize the device structure. */
	dev->netdev_ops = &dummy_netdev_ops;
	//dev->ethtool_ops = &dummy_ethtool_ops;
	dev->needs_free_netdev = true;

	/* Fill in device structure with ethernet-generic values. */
	dev->flags = IFF_UP | IFF_LOWER_UP | IFF_MULTICAST | IFF_BROADCAST;
	dev->priv_flags |= IFF_LIVE_ADDR_CHANGE | IFF_NO_QUEUE;
	dev->features	|= NETIF_F_SG | NETIF_F_FRAGLIST;
	dev->features	|= NETIF_F_ALL_TSO;
	dev->features	|= NETIF_F_HW_CSUM | NETIF_F_HIGHDMA | NETIF_F_LLTX;
	dev->features	|= NETIF_F_GSO_ENCAP_ALL;
	dev->hw_features |= dev->features;
	dev->hw_enc_features |= dev->features;
	eth_hw_addr_random(dev);

	dev->min_mtu = 0;
	dev->max_mtu = 0;
}


static struct rtnl_link_ops link_ops __read_mostly = {
	.kind		= DRV_NAME,
	.setup		= setup,
};

static int __init init_one(void)
{
	struct net_device *dev_dummy;
	int err;

	dev_dummy = alloc_netdev(0, "test%d", NET_NAME_ENUM, setup);
	if (!dev_dummy)
		return -ENOMEM;

	dev_dummy->rtnl_link_ops = &link_ops;
	err = register_netdevice(dev_dummy);
	if (err < 0)
		goto err;
	return 0;

err:
	free_netdev(dev_dummy);
	return err;
}

static int __init test_init_module(void)
{
	printk(KERN_ERR "init_module");
	int i, err = 0;

	down_write(&pernet_ops_rwsem);
	rtnl_lock();
	err = __rtnl_link_register(&link_ops);
	if (err < 0)
		goto out;

	err = init_one();
	printk(KERN_ERR "init_one result: %d", err);
	cond_resched();
	if (err < 0)
		__rtnl_link_unregister(&link_ops);

out:
	rtnl_unlock();
	up_write(&pernet_ops_rwsem);

	return err;
}

static void __exit test_cleanup_module(void)
{
	printk(KERN_ERR "cleanup_module");
	rtnl_link_unregister(&link_ops);
}

module_init(test_init_module);
module_exit(test_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_ALIAS_RTNL_LINK(DRV_NAME);
MODULE_VERSION(DRV_VERSION);
