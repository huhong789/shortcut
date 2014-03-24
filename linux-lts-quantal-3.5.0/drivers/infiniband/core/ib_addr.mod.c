#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

MODULE_INFO(intree, "Y");

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xb128b138, "module_layout" },
	{ 0xff95a544, "kmalloc_caches" },
	{ 0xea5d48dc, "ipv6_chk_addr" },
	{ 0x43a53735, "__alloc_workqueue_key" },
	{ 0xe50ad69b, "dst_release" },
	{ 0xd35e9659, "mutex_unlock" },
	{ 0xdd0e969f, "neigh_destroy" },
	{ 0x7d11c268, "jiffies" },
	{ 0xcf7d4870, "__neigh_event_send" },
	{ 0x68dfc59f, "__init_waitqueue_head" },
	{ 0x3fa58ef8, "wait_for_completion" },
	{ 0xd5f2172f, "del_timer_sync" },
	{ 0xb4390f9a, "mcount" },
	{ 0x1598dc9d, "unregister_netevent_notifier" },
	{ 0xd36d011b, "mutex_lock" },
	{ 0x8c03d20c, "destroy_workqueue" },
	{ 0x730951df, "dev_get_by_index" },
	{ 0xb5f636b1, "init_net" },
	{ 0x4571214b, "ip6_route_output" },
	{ 0xfd6293c2, "boot_tvec_bases" },
	{ 0x59d220ed, "ipv6_dev_get_saddr" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x3bd1b1f6, "msecs_to_jiffies" },
	{ 0x496d7988, "kmem_cache_alloc_trace" },
	{ 0x693f3681, "ip_route_output_flow" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0x4f5ba237, "__ip_dev_find" },
	{ 0x19a9e62b, "complete" },
	{ 0x5dd67618, "register_netevent_notifier" },
	{ 0x47c149ab, "queue_delayed_work" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "2D4142BFBD30A4D7B87ABF1");