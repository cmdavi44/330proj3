#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x27ce139e, "module_layout" },
	{ 0x9862696, "param_ops_int" },
	{ 0x46a4b118, "hrtimer_cancel" },
	{ 0x3c5d543a, "hrtimer_start_range_ns" },
	{ 0x2d0684a9, "hrtimer_init" },
	{ 0x92997ed8, "_printk" },
	{ 0x4b249969, "init_task" },
	{ 0x828e22f4, "hrtimer_forward" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x8a57a7f4, "pv_ops" },
	{ 0xdad13544, "ptrs_per_p4d" },
	{ 0x8a35b432, "sme_me_mask" },
	{ 0xa92ec74, "boot_cpu_data" },
	{ 0x72d79d83, "pgdir_shift" },
	{ 0x1d19f77b, "physical_mask" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "014691AB3C7E08BCA844412");
