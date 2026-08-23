#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#ifndef KBD_PATH
    #define KBD_PATH "/sys/devices/platform/tuxedo_keyboard/leds/rgb:kbd_backlight"
#endif

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

int read_key(int *mouse_x) {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return '\0';

    if (c == '\033') {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\033';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\033';
        
        if (seq[0] == '[') {
            if (seq[1] == 'A') return 1000;
            if (seq[1] == 'B') return 1001;
            if (seq[1] == 'C') return 1002;
            if (seq[1] == 'D') return 1003;
            if (seq[1] == '<') {
                char buf[32];
                int i = 0;
                char type = 0;
                while (i < 31) {
                    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
                    if (buf[i] == 'M' || buf[i] == 'm') {
                        type = buf[i];
                        buf[i] = '\0';
                        break;
                    }
                    i++;
                }
                int btn, x, y;
                if (sscanf(buf, "%d;%d;%d", &btn, &x, &y) == 3) {
                    if (mouse_x) *mouse_x = x;
                    return (type == 'M') ? 2000 : 2001;
                }
                return '\033';
            }
        }
        return '\033';
    }
    return c;
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

int read_sysfs(const char *name, char *buffer, size_t size) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", KBD_PATH, name);
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    if (fgets(buffer, size, f)) {
        buffer[strcspn(buffer, "\n")] = 0;
        fclose(f);
        return 1;
    }
    fclose(f);
    return 0;
}

void set_backlight(int brightness, int r, int g, int b) {
    char b_str[16], rgb_str[64];
    snprintf(b_str, sizeof(b_str), "%d", brightness);
    snprintf(rgb_str, sizeof(rgb_str), "%d %d %d", r, g, b);

    write_sysfs("brightness", b_str);
    write_sysfs("multi_intensity", rgb_str);
}

void calculate_tint(int val, int rb, int gb, int bb, int *r, int *g, int *b) {
    if (val <= 127) {
        float f = val / 127.0f;
        *r = (int)(255 - (255 - rb) * f);
        *g = (int)(255 - (255 - gb) * f);
        *b = (int)(255 - (255 - bb) * f);
    } else {
        float f = 1.0f - 0.8f * ((val - 127) / 128.0f);
        *r = (int)(rb * f);
        *g = (int)(gb * f);
        *b = (int)(bb * f);
    }
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

        int key = read_key(NULL);
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
    printf("\033[K\033[1;32m \033[1;37m%s\033[0m \033[90m› %s\033[0m\n", title, options[selected]);
    
    return selected;
}

int run_slider(const char *title, int current_val) {
    int val = current_val;
    int first_draw = 1;
    int bar_len = 40;

    enable_raw_mode();
    printf("\033[?1000h\033[?1002h\033[?1015h\033[?1006h");

    while (1) {
        if (!first_draw) {
            printf("\033[2A");
        }
        
        printf("\033[K\033[1;36m? \033[1;37m%s\033[0m\n", title);
        
        int pos = (val * (bar_len - 1)) / 255;
        
        printf("\033[K   ");
        for (int i = 0; i < bar_len; i++) {
            int step = (i * 255) / (bar_len - 1);
            int tc = step > 127 ? 0 : 255;
            
            if (i == pos) {
                printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm|", step, step, step, tc, tc, tc);
            } else {
                printf("\033[48;2;%d;%d;%dm ", step, step, step);
            }
        }
        printf("\033[0m %d/255\n", val);
        
        first_draw = 0;
        fflush(stdout);

        int mouse_x = 0;
        int key = read_key(&mouse_x);

        if (key == 1002 || key == '>' || key == '.') {
            val += 5;
            if (val > 255) val = 255;
        } else if (key == 1003 || key == '<' || key == ',') {
            val -= 5;
            if (val < 0) val = 0;
        } else if (key == 2000) {
            if (mouse_x >= 4 && mouse_x < 4 + bar_len) {
                pos = mouse_x - 4;
                val = (pos * 255) / (bar_len - 1);
                if (val > 255) val = 255;
                if (val < 0) val = 0;
            }
        } else if (key == '\n' || key == '\r') {
            break;
        }
    }
    
    printf("\033[?1000l\033[?1002l\033[?1015l\033[?1006l");
    disable_raw_mode();
    
    printf("\033[2A\033[K\n\033[2A");
    printf("\033[K\033[1;32m \033[1;37m%s\033[0m \033[90m› %d\033[0m\n", title, val);
    
    return val;
}

void run_color_slider(const char *title, int rb, int gb, int bb, int *r_out, int *g_out, int *b_out, int brightness) {
    int val = 127;
    int first_draw = 1;
    int bar_len = 40;

    enable_raw_mode();
    printf("\033[?1000h\033[?1002h\033[?1015h\033[?1006h");

    while (1) {
        if (!first_draw) {
            printf("\033[2A");
        }
        
        printf("\033[K\033[1;36m? \033[1;37m%s\033[0m\n", title);
        
        int pos = (val * (bar_len - 1)) / 255;
        
        printf("\033[K   ");
        for (int i = 0; i < bar_len; i++) {
            int step = (i * 255) / (bar_len - 1);
            int r, g, b;
            calculate_tint(step, rb, gb, bb, &r, &g, &b);
            
            int tc = ((r * 299) + (g * 587) + (b * 114)) > 128000 ? 0 : 255;
            
            if (i == pos) {
                printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm|", r, g, b, tc, tc, tc);
            } else {
                printf("\033[48;2;%d;%d;%dm ", r, g, b);
            }
        }
        printf("\033[0m\n");
        
        first_draw = 0;
        fflush(stdout);

        int mouse_x = 0;
        int key = read_key(&mouse_x);

        if (key == 1002 || key == '>' || key == '.') {
            val += 5;
            if (val > 255) val = 255;
        } else if (key == 1003 || key == '<' || key == ',') {
            val -= 5;
            if (val < 0) val = 0;
        } else if (key == 2000) {
            if (mouse_x >= 4 && mouse_x < 4 + bar_len) {
                pos = mouse_x - 4;
                val = (pos * 255) / (bar_len - 1);
                if (val > 255) val = 255;
                if (val < 0) val = 0;
            }
        } else if (key == '\n' || key == '\r') {
            break;
        }

        calculate_tint(val, rb, gb, bb, r_out, g_out, b_out);
        set_backlight(brightness, *r_out, *g_out, *b_out);
    }
    
    printf("\033[?1000l\033[?1002l\033[?1015l\033[?1006l");
    disable_raw_mode();
    
    printf("\033[2A\033[K\n\033[2A");
    printf("\033[K\033[1;32m \033[1;37m%s\033[0m \033[90m› Applied\033[0m\n", title);
}

int main() {
    int brightness = 255;
    int curr_r = 255, curr_g = 255, curr_b = 255;
    char buf[64];
    
    if (read_sysfs("brightness", buf, sizeof(buf))) {
        brightness = atoi(buf);
    }
    if (read_sysfs("multi_intensity", buf, sizeof(buf))) {
        sscanf(buf, "%d %d %d", &curr_r, &curr_g, &curr_b);
    }
    
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
            if (color_sel >= 0 && color_sel <= 5) {
                int rb = 0, gb = 0, bb = 0;
                switch (color_sel) {
                    case 0: rb=255; gb=0; bb=0; break;
                    case 1: rb=255; gb=255; bb=0; break;
                    case 2: rb=0; gb=255; bb=0; break;
                    case 3: rb=0; gb=255; bb=255; break;
                    case 4: rb=0; gb=0; bb=255; break;
                    case 5: rb=255; gb=255; bb=255; break;
                }
                curr_r = rb; curr_g = gb; curr_b = bb;
                set_backlight(brightness, curr_r, curr_g, curr_b);
                run_color_slider("Adjust color tone (Light <-> Dark):", rb, gb, bb, &curr_r, &curr_g, &curr_b, brightness);
            } else if (color_sel == 6) {
                curr_r = 0; curr_g = 0; curr_b = 0;
                set_backlight(brightness, curr_r, curr_g, curr_b);
            } else if (color_sel == 7) {
                continue;
            }
        } 
        else if (main_sel == 1) {
            printf("\033[1;36m? \033[1;37mEnter RGB values (0-255) separated by spaces (e.g., 128 0 255): \033[0m");
            if (scanf("%d %d %d", &curr_r, &curr_g, &curr_b) == 3) {
                set_backlight(brightness, curr_r, curr_g, curr_b);
                printf("\033[1;32m \033[1;37mColor applied successfully!\033[0m\n");
            } else {
                printf("\033[1;31m Invalid format!\033[0m\n");
            }
            while (getchar() != '\n');
        } 
        else if (main_sel == 2) {
            brightness = run_slider("Adjust brightness level:", brightness);
            set_backlight(brightness, curr_r, curr_g, curr_b);
        } 
        else if (main_sel == 3) {
            break;
        }
        printf("\n");
    }
    
    return 0;
}