#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h> // kmalloc ve kfree için
#include <linux/seq_file.h> // proc dosyası için

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SEFO");
MODULE_DESCRIPTION("Memory Allocation Monitor with Tracepoints");

#define LOG_BUFFER_SIZE 4096

static char log_buffer[LOG_BUFFER_SIZE];
static int log_index = 0;

static void log_memory_event(const char *format, ...) {
    va_list args;
    int remaining_space = LOG_BUFFER_SIZE - log_index - 1; // -1 for null terminator
    if (remaining_space > 0) {
        va_start(args, format);
        int bytes_written = vsnprintf(log_buffer + log_index, remaining_space, format, args);
        va_end(args);
        if (bytes_written > 0) {
            log_index += bytes_written;
            if (log_index >= LOG_BUFFER_SIZE) {
                log_index = LOG_BUFFER_SIZE - 1; // Prevent overflow
            }
        }
    }
}

static ssize_t read_proc(struct file *file, char __user *buffer, size_t length, loff_t *offset) {
    return simple_read_from_buffer(buffer, length, offset, log_buffer, strlen(log_buffer));
}

static const struct proc_ops proc_fops = {
    .proc_read = read_proc,
};

static int __init mem_monitor_init(void) {
    struct proc_dir_entry *entry = proc_create("mem_monitor", 0666, NULL, &proc_fops);
    if (!entry) {
        return -ENOMEM;
    }

    log_memory_event("Memory Monitor Loaded\n");
    printk(KERN_INFO "Memory Monitor Loaded\n");

    // Bellek tahsisi örneği
    void *ptr = kmalloc(512, GFP_KERNEL); // 512 byte tahsis et
    if (ptr) {
        log_memory_event("Memory Allocated: %zu bytes at address %p\n", 512, ptr);
        // Belleği serbest bırak
        kfree(ptr);
        log_memory_event("Memory Freed at address %p\n", ptr);
    } else {
        log_memory_event("Memory Allocation Failed for size: %zu bytes\n", 512);
    }

    return 0;
}

static void __exit mem_monitor_exit(void) {
    log_memory_event("Memory Monitor Unloaded\n");
    remove_proc_entry("mem_monitor", NULL);
    printk(KERN_INFO "Memory Monitor Unloaded\n");
}

module_init(mem_monitor_init);
module_exit(mem_monitor_exit);

