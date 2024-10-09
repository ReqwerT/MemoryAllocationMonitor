#include <stdio.h>

int main() {
    FILE *file = fopen("/proc/mem_monitor", "r");
    if (file) {
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), file)) {
            printf("%s", buffer);
        }
        fclose(file);
    } else {
        perror("Failed to open proc file");
    }
    return 0;
}
