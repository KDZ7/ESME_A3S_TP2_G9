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

// Inclusion du fichier header ou est défini la structure de mon protocole de message
#include "protocol_messag.h"

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
   struct protocol_message *protocol_msg;

   // Vérifier que le message a une taille suffisante pour remplir la structure du protocole de message
   if (nbc < sizeof(struct protocol_message))
   {
      printk(KERN_ERR "Message trop court pour être valide\n");
      return -EINVAL;
   }

   // Allouer de la mémoire pour le protocole de message
   protocol_msg = kmalloc(nbc, GFP_KERNEL);
   if (!protocol_msg)
   {
      printk(KERN_ERR "Erreur d'allocation de mémoire\n");
      return -ENOMEM;
   }

   // Copier les données du message de l'utilisateur dans la mémoire dynamique protocole_msg
   if (copy_from_user(protocol_msg, buf, nbc))
   {
      printk(KERN_ERR "Erreur de copie des données\n");
      kfree(protocol_msg);
      return -EFAULT;
   }

   // Obtenir la taille des données du message
   size_t data_size = protocol_msg->length - sizeof(struct protocol_message);

   printk(KERN_INFO "Message reçu: length=%u, type=%u, pid=%u, data=%s\n", protocol_msg->length, protocol_msg->type, protocol_msg->pid, protocol_msg->data);

   // Traiter le message en fonction de son type
   switch (protocol_msg->type)
   {
   case MSG_TYPE_WRITE:
      if (data_size > MEM_SIZE)
      {
         printk(KERN_ERR "Message trop grand pour la mémoire MEM1\n");
         kfree(protocol_msg);
         return -EINVAL;
      }
      // Copier les données du message dans MEM1
      memcpy(MEM1, protocol_msg->data, data_size);
      printk(KERN_INFO "Données copiées dans MEM1 : %s\n", (char *)MEM1);
      break;

   default:
      printk(KERN_ERR "Type de message inconnu\n");
      kfree(protocol_msg);
      return -EINVAL;
   }

   // S'assurer de libérer la mémoire allouée pour le protocole de message
   kfree(protocol_msg);
   return nbc;
}

/* pour la fct read */
ssize_t messag_read1(struct file *fp, char __user *buf, size_t nbc, loff_t *pos)
{
   struct protocol_message *protocol_msg;
   ssize_t data_size = MEM_SIZE;
   size_t length = sizeof(struct protocol_message) + data_size;

   // Vérifier que l'espace Mémoire est suffisant pour contenir le protocole de message et les données
   if (nbc < length)
   {
      printk(KERN_ERR "Espace insuffisant pour le message\n");
      return -EINVAL;
   }

   // Allouer de la mémoire pour le protocole de message
   protocol_msg = kmalloc(length, GFP_KERNEL);
   if (!protocol_msg)
   {
      printk(KERN_ERR "Erreur d'allocation de mémoire\n");
      return -ENOMEM;
   }

   // Préparation du protocole de message
   protocol_msg->length = length;
   protocol_msg->type = MSG_TYPE_READ;
   protocol_msg->pid = current->pid; // PID du processus qui a envoyé le message
   memcpy(protocol_msg->data, MEM1, data_size);

   // Copier le protocole de message dans l'espace mémoire de l'utilisateur
   if (copy_to_user(buf, protocol_msg, length))
   {
      printk(KERN_ERR "Erreur de copie des données\n");
      kfree(protocol_msg);
      return -EFAULT;
   }

   // S'assurer de libérer la mémoire allouée pour le protocole de message
   kfree(protocol_msg);

   return length;
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
