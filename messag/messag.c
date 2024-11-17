/*
 * exemple d'un module pour multi memory mapping
 * avec un nb de devices limite
 * et memory-mapping des deux types de memoires
 * code complet avec read et lseek !!
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <asm/page.h>
#include <asm/io.h>

static int messag_major = 0;

MODULE_AUTHOR("P. Foubet");
MODULE_LICENSE("Dual BSD/GPL");

#define MEM_SIZE PAGE_SIZE
void *MEM1;

static int messag_open(struct inode *inode, struct file *fp)
{
   return 0;
}

static int messag_release(struct inode *inode, struct file *fp)
{
   return 0;
}

static int messag_cas1_mmap(struct file *fp, struct vm_area_struct *vma)
{
   if (remap_pfn_range(vma, vma->vm_start,
                       vmalloc_to_pfn(MEM1),
                       MEM_SIZE,
                       vma->vm_page_prot))
      return -EAGAIN;
   return 0;
}

/* pour la fct lseek */
loff_t messag_llseek(struct file *fp, loff_t off, int whence)
{
   loff_t newpos;

   switch (whence)
   {
   case 0: /* SEEK_SET */
      newpos = off;
      break;
   case 1: /* SEEK_CUR */
      newpos = fp->f_pos + off;
      break;
   case 2: /* SEEK_END */
      newpos = MEM_SIZE + off;
      break;
   default: /* code inconnu */
      return -EINVAL;
   }
   if (newpos < 0)
      return -EINVAL;
   fp->f_pos = newpos;
   return newpos;
}

/* pour la fct write */
ssize_t messag_write1(struct file *fp, const char __user *buf, size_t nbc, loff_t *pos)
{
   size_t nbe = nbc, err;
   loff_t p = *pos;
   printk(KERN_DEBUG "messag1 : write avec pos=%Ld !\n", p);
   if ((p < 0) || (p > MEM_SIZE - 1))
      return 0; /* idem EOF */
   if (p + nbe > MEM_SIZE)
      nbe = MEM_SIZE - p;
   if ((err = copy_from_user((void *)(MEM1 + p), buf, nbe)) != 0)
      nbe -= err;
   return nbe; /* retourne le nb de car recus du user */
}

/* pour la fct read */
ssize_t messag_read1(struct file *fp, char __user *buf, size_t nbc, loff_t *pos)
{
   size_t nbe = nbc, err;
   loff_t p = *pos;
   printk(KERN_DEBUG "messag1 : read avec pos=%Ld !\n", p);
   if ((p < 0) || (p > MEM_SIZE - 1))
      return 0; /* idem EOF */
   if (p + nbe > MEM_SIZE)
      nbe = MEM_SIZE - p;
   if ((err = copy_to_user(buf, (void *)(MEM1 + p), nbe)) != 0)
      nbe -= err;
   return nbe; /* retourne le nb de car envoyes */
}

/* petite fonction d'initialisation des devices */
static void messag_setup_cdev(struct cdev *dev, int minor,
                              struct file_operations *fops)
{
   int err;
   dev_t devno = MKDEV(messag_major, minor);
   cdev_init(dev, fops);
   dev->owner = THIS_MODULE;
   dev->ops = fops;
   err = cdev_add(dev, devno, 1);
   if (err)
      printk(KERN_NOTICE "Erreur %d ajout messag %d\n", err, minor);
}

static struct file_operations messag_cas1_ops = {
    .owner = THIS_MODULE,
    .open = messag_open,
    .release = messag_release,
    .llseek = messag_llseek,
    .write = messag_write1,
    .read = messag_read1,
    .mmap = messag_cas1_mmap,
};

/* on initialise 2 devices */
static struct cdev MessagDev;

static int messag_init(void)
{
   int i, res;
   char *CMEM1;
   dev_t dev = MKDEV(messag_major, 0);

   if (messag_major)
      res = register_chrdev_region(dev, 1, "messag");
   else
   {
      res = alloc_chrdev_region(&dev, 0, 1, "messag");
      messag_major = MAJOR(dev);
   }
   if (res < 0)
   {
      printk(KERN_WARNING "messag : erreur sur major %d\n", messag_major);
      return res;
   }
   if (messag_major == 0)
      messag_major = res;

   /* initialisation des memoires */
   MEM1 = vmalloc(MEM_SIZE);
   CMEM1 = (char *)MEM1;
   for (i = 0; i < 50; i++)
   {
      CMEM1[i] = 'a' + i;
   }

   /* initialisation du device caractere */
   messag_setup_cdev(&MessagDev, 0, &messag_cas1_ops);
   return 0;
}

static void messag_exit(void)
{
   cdev_del(&MessagDev);
   vfree(MEM1);
   unregister_chrdev_region(MKDEV(messag_major, 0), 1);
}

module_init(messag_init);
module_exit(messag_exit);
