#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <asm/uaccess.h>

#include <linux/configfs.h>
#include <linux/init.h>
#include <linux/tty.h>          /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>           /* For KDSETLED */
#include <linux/vt.h>
#include <linux/console_struct.h>       /* For vc_cons */
#include <linux/vt_kern.h>

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Oleg Tsiliuric <olej@front.ru>" );

#define NAME_NODE "flashing_leds"
#define LEN_MSG 160                       // длина буфера и сам буфер обмена
static char buf_msg[ LEN_MSG + 1 ] = "on";

struct timer_list my_timer;
struct tty_driver *my_driver;
char kbledstatus = 0;
#define BLINK_DOT     HZ/5
#define BLINK_PAUSE	  HZ/2
#define	BLINK_LONG	  HZ
#define ALL_LEDS_ON   0x07
#define RESTORE_LEDS  0xFF

char symb[257][5];

void SetSymb( unsigned char index, char val1, char val2, char val3, char val4, char val5 ) {

	symb[index][0] = val1;
	symb[index][1] = val2;
	symb[index][2] = val3;
	symb[index][3] = val4;
	symb[index][4] = val5;

}

static void my_timer_func(unsigned long ptr)
{
        int *pstatus = (int *)ptr;
		static int charNumber = 0;
		static int posInChar = 0;
		int pause;

		if ( *pstatus == RESTORE_LEDS ) {
			*pstatus = 0x07;

			if ( symb[buf_msg[charNumber]][posInChar] == 0 || posInChar == 5 ) {
				posInChar = 0;
				charNumber++;
			}
			if ( buf_msg[charNumber] == '\n' || buf_msg[charNumber] == '\0' ) {
				charNumber = 0;
			}

			if ( symb[buf_msg[charNumber]][posInChar] == 1 ) {
				pause = BLINK_DOT;
			} else if ( symb[buf_msg[charNumber]][posInChar] == 2 ) {
				pause = BLINK_LONG;
			}

			posInChar++;
			printk( KERN_INFO "Char now [%d]=(%c), pos on symb (%d)\n", charNumber, buf_msg[charNumber], posInChar );
		} else {
			*pstatus = RESTORE_LEDS;
			pause = BLINK_PAUSE;
			//printk( KERN_INFO "Pause\n" );
		}

		(my_driver->ops->ioctl) (vc_cons[fg_console].d->vc_tty, NULL,  KDSETLED,
                            *pstatus);

		my_timer.expires = jiffies + pause;

        add_timer(&my_timer);
}

static ssize_t node_read( struct file *file, char *buf, size_t count, loff_t *ppos ) {
   static int odd = 0;
   printk( KERN_INFO "read: %d\n", count );
   if( 0 == odd ) {
      int res = copy_to_user( (void*)buf, &buf_msg, strlen( buf_msg ) );
      odd = 1;
      //put_user( '\n', buf + strlen( buf_msg ) );   // buf — это адресное пространство пользователя
      res = strlen( buf_msg ) + 1;
      printk( KERN_INFO "return bytes :  %d\n", res );
      return res;
   }
   odd = 0;
   printk( KERN_INFO "EOF\n" );
   return 0;
}

static ssize_t node_write( struct file *file, const char *buf, size_t count, loff_t *ppos ) {
	int res, len = count < LEN_MSG ? count : LEN_MSG;
	printk( KERN_INFO "write: %d\n", count );
	res = copy_from_user( &buf_msg, (void*)buf, len );
	if( '\n' == buf_msg[ len -1 ] ) {
	    buf_msg[ len -1 ] = '\0';
	} else {
		buf_msg[ len ] = '\0';
	}
	printk( KERN_INFO "put bytes = %d\n", len );
	return len;
}

static const struct file_operations node_fops = {
   .owner = THIS_MODULE,
   .read  = node_read,
   .write  = node_write
};

static int __init proc_init( void ) {
   int ret;
   struct proc_dir_entry *own_proc_node;
   int i;
		// 1 .  2 -
	SetSymb( 'a', 1, 2, 0, 0, 0 );
	SetSymb( 'b', 2, 1, 1, 1, 0 );
	SetSymb( 'c', 2, 1, 2, 1, 0 );
	SetSymb( 'd', 2, 1, 1, 0, 0 );
	SetSymb( 'e', 1, 0, 0, 0, 0 );
	SetSymb( 'f', 1, 1, 2, 1, 0 );
	SetSymb( 'g', 2, 2, 1, 0, 0 );
	SetSymb( 'h', 1, 1, 1, 1, 0 );
	SetSymb( 'i', 1, 1, 0, 0, 0 );
	SetSymb( 'j', 1, 2, 2, 2, 0 );
	SetSymb( 'k', 2, 1, 2, 0, 0 );
	SetSymb( 'l', 1, 2, 1, 1, 0 );
	SetSymb( 'm', 2, 2, 0, 0, 0 );
	SetSymb( 'n', 2, 1, 0, 0, 0 );
	SetSymb( 'o', 2, 2, 2, 0, 0 );
	SetSymb( 'p', 1, 2, 2, 1, 0 );
	SetSymb( 'q', 2, 2, 1, 2, 0 );
	SetSymb( 'r', 1, 2, 1, 0, 0 );
	SetSymb( 's', 1, 1, 1, 0, 0 );
	SetSymb( 't', 2, 0, 0, 0, 0 );
	SetSymb( 'u', 1, 1, 2, 0, 0 );
	SetSymb( 'v', 1, 1, 1, 2, 0 );
	SetSymb( 'w', 1, 2, 2, 0, 0 );
	SetSymb( 'x', 2, 1, 1, 2, 0 );
	SetSymb( 'y', 2, 1, 2, 2, 0 );
	SetSymb( 'z', 2, 2, 1, 1, 0 );

	SetSymb( '1', 1, 2, 2, 2, 2 );
	SetSymb( '2', 1, 1, 2, 2, 2 );
	SetSymb( '3', 1, 1, 1, 2, 2 );
	SetSymb( '4', 1, 1, 1, 1, 2 );
	SetSymb( '5', 1, 1, 1, 1, 1 );
	SetSymb( '6', 2, 1, 1, 1, 1 );
	SetSymb( '7', 2, 2, 1, 1, 1 );
	SetSymb( '8', 2, 2, 2, 1, 1 );
	SetSymb( '9', 2, 2, 2, 2, 1 );
	SetSymb( '0', 2, 2, 2, 2, 2 );

   /*
	*	Инициализация proc
   	*/
   own_proc_node = create_proc_entry( NAME_NODE, S_IFREG | S_IRUGO | S_IWUGO, NULL );
   if( NULL == own_proc_node ) {
      ret = -ENOMEM;
      printk( KERN_ERR "can't create /proc/%s\n", NAME_NODE );
      return ret;
   }
   own_proc_node->uid = 0;
   own_proc_node->gid = 0;
   own_proc_node->proc_fops = &node_fops;
   printk( KERN_INFO "module : success!\n");

	/*
	 *	Инициализация leds
	 */
   printk(KERN_INFO "kbleds: loading\n");
   printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);
   for (i = 0; i < MAX_NR_CONSOLES; i++) {
		   if (!vc_cons[i].d)
				   break;
		   printk(KERN_INFO "poet_atkm: console[%i/%i] #%i, tty %lx\n", i,
				  MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
				  (unsigned long)vc_cons[i].d->vc_tty);
   }
   printk(KERN_INFO "kbleds: finished scanning consoles\n");
   my_driver = vc_cons[fg_console].d->vc_tty->driver;
   printk(KERN_INFO "kbleds: tty driver magic %x\n", my_driver->magic);
   printk( KERN_INFO "fg_console: %d\n", fg_console );
   /*
	* Set up the LED blink timer the first time
	*/
   init_timer(&my_timer);
   my_timer.function = my_timer_func;
   my_timer.data = (unsigned long)&kbledstatus;
   add_timer(&my_timer);

   return 0;
}

static void __exit proc_exit( void ) {
	printk(KERN_INFO "kbleds: unloading...\n");
	del_timer(&my_timer);
	(my_driver->ops->ioctl) (vc_cons[fg_console].d->vc_tty, NULL, KDSETLED,
						RESTORE_LEDS);
   remove_proc_entry( NAME_NODE, NULL );
   printk(KERN_INFO "/proc/%s removed\n", NAME_NODE );
}

module_init( proc_init );
module_exit( proc_exit );
