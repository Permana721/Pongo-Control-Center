#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define KBD_PATH "/sys/devices/platform/tuxedo_keyboard/leds/rgb:kbd_backlight"

struct termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

int read_key() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return '\0';

    if (c == '\033') {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\033';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\033';
        
        if (seq[0] == '[') {
            if (seq[1] == 'A') return 1000;
            if (seq[1] == 'B') return 1001;
        }
        return '\033';
    }
    return c;
}

int run_menu(const char *title, const char *options[], int count) {
    int selected = 0;
    int first_draw = 1;
    
    enable_raw_mode();
    while (1) {
        if (!first_draw) {
            printf("\033[%dA", count + 1);
        }
        
        printf("\033[K\033[1;36m? \033[1;37m%s\033[0m\n", title);
        
        for (int i = 0; i < count; i++) {
            if (i == selected) {
                printf("\033[K\033[1;32m ❯ \033[1;37m%s\033[0m\n", options[i]);
            } else {
                printf("\033[K   \033[90m%s\033[0m\n", options[i]);
            }
        }
        first_draw = 0;
        
        fflush(stdout);

        int key = read_key();
        if (key == 1000) {
            selected--;
            if (selected < 0) selected = count - 1;
        } else if (key == 1001) {
            selected++;
            if (selected >= count) selected = 0;
        } else if (key == '\n' || key == '\r') {
            break;
        }
    }
    disable_raw_mode();
    
    printf("\033[%dA", count + 1);
    for (int i = 0; i <= count; i++) printf("\033[K\n"); 
    printf("\033[%dA", count + 1);
    printf("\033[K\033[1;32m✔ \033[1;37m%s\033[0m \033[90m› %s\033[0m\n", title, options[selected]);
    
    return selected;
}

int write_sysfs(const char *name, const char *value) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", KBD_PATH, name);
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "%s", value);
    fclose(f);
    return 1;
}

void set_backlight(int brightness, int r, int g, int b) {
    char b_str[16], rgb_str[64];
    snprintf(b_str, sizeof(b_str), "%d", brightness);
    snprintf(rgb_str, sizeof(rgb_str), "%d %d %d", r, g, b);

    write_sysfs("brightness", b_str);
    write_sysfs("multi_intensity", rgb_str);
}

int main() {
    int brightness = 255;
    
    const char *main_menu[] = {
        "Preset Colors",
        "Manual RGB Code",
        "Brightness Level",
        "Exit"
    };
    
    const char *color_menu[] = {
        "Red", "Yellow", "Green", "Cyan", "Blue", "White", "Turn Off", "Back"
    };

    printf("\n");
    while (1) {
        int main_sel = run_menu("Control Center - Main Menu:", main_menu, 4);

        if (main_sel == 0) {
            int color_sel = run_menu("Select a preset color:", color_menu, 8);
            switch (color_sel) {
                case 0: set_backlight(brightness, 255, 0, 0); break;
                case 1: set_backlight(brightness, 255, 255, 0); break;
                case 2: set_backlight(brightness, 0, 255, 0); break;
                case 3: set_backlight(brightness, 0, 255, 255); break;
                case 4: set_backlight(brightness, 0, 0, 255); break;
                case 5: set_backlight(brightness, 255, 255, 255); break;
                case 6: set_backlight(0, 0, 0, 0); break;
                case 7: break;
            }
        } 
        else if (main_sel == 1) {
            int r, g, b;
            printf("\033[1;36m? \033[1;37mEnter RGB values (0-255) separated by spaces (e.g., 128 0 255): \033[0m");
            if (scanf("%d %d %d", &r, &g, &b) == 3) {
                set_backlight(brightness, r, g, b);
                printf("\033[1;32m✔ \033[1;37mColor applied successfully!\033[0m\n");
            } else {
                printf("\033[1;31m✖ Invalid format!\033[0m\n");
            }
            while (getchar() != '\n');
        } 
        else if (main_sel == 2) {
            int b_val;
            printf("\033[1;36m? \033[1;37mEnter brightness level (0-255): \033[0m");
            if (scanf("%d", &b_val) == 1) {
                brightness = (b_val > 255) ? 255 : (b_val < 0 ? 0 : b_val);
                printf("\033[1;32m✔ \033[1;37mBrightness saved to %d!\033[0m\n", brightness);
            }
            while (getchar() != '\n');
        } 
        else if (main_sel == 3) {
            break;
        }
        printf("\n");
    }
    
    return 0;
}