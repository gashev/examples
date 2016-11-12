#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>

struct Item {
    int value;
    struct list_head list;
};

struct Item itemList;

static void print_list(void) {
    struct Item *tmpItem;

    printk(KERN_INFO "Print list.");
    list_for_each_entry(tmpItem, &itemList.list, list) {
        printk(
            KERN_INFO "List=%pK Prev=%pk Next=%pK Value=%d",
            (void*) &tmpItem->list,
            (void*) (&tmpItem->list)->prev, 
            (void*) (&tmpItem->list)->next, 
            tmpItem->value
        );
    } 
}

static int init_linked_list(void) {
    int i;
    struct Item *newItem;

    printk(KERN_INFO "initialize linked_list kernel module.");
    INIT_LIST_HEAD(&itemList.list); 

    for(i = 0; i < 5; i++) {
        newItem = kmalloc(sizeof(*newItem), GFP_KERNEL); 
        newItem->value = i;
        INIT_LIST_HEAD(&newItem->list);
        list_add_tail(&(newItem->list), &(itemList.list));
        print_list();
    }

    return 0;
}

void exit_linked_list(void) {
    struct Item *currentItem, *tmpItem;

    printk(KERN_INFO "kernel module unloaded.");

    list_for_each_entry_safe(currentItem, tmpItem, &itemList.list, list){
        printk(KERN_INFO "freeing node %d", currentItem->value);
        list_del(&currentItem->list);
        kfree(currentItem);
    }
}

module_init(init_linked_list);
module_exit(exit_linked_list);

MODULE_AUTHOR("Oleg Gashev");
MODULE_LICENSE("GPL");

