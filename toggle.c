extern int execvp(const char *, char *const[]);
extern int fork(void);
extern int waitpid(int, int *, int);
extern void _exit(int);
extern int strcmp(const char *, const char *);
extern int strncmp(const char *, const char *, unsigned long);
extern unsigned long strlen(const char *);
extern char *strstr(const char *, const char *);
extern int snprintf(char *, unsigned long, const char *, ...);
extern long write(int, const void *, unsigned long);
extern long read(int, void *, unsigned long);
extern long readlink(const char *, char *, unsigned long);
extern int open(const char *, int);
extern int close(int);
extern char *getenv(const char *);
extern int system(const char *);

static char BASE[1024]   = "/var/lib/kali-overlay/base";
static char UPPER[1024]  = "/var/lib/kali-overlay/upper";
static char WORK[1024]   = "/var/lib/kali-overlay/work";
static char MERGED[1024] = "/var/lib/kali-overlay/merged";
static char SKULL[1024]  = "";
static char SELF_DIR[1024] = "";

static int run(char *const argv[]) {
    int pid = fork();
    if (pid == 0) { execvp(argv[0], argv); _exit(127); }
    int st = 0;
    waitpid(pid, &st, 0);
    return st;
}

static void say(const char *s) {
    write(2, s, strlen(s));
    write(2, "\n", 1);
}

static long slurp(const char *path, char *buf, long max) {
    int fd = open(path, 0);
    if (fd < 0) return -1;
    long n = read(fd, buf, max - 1);
    close(fd);
    if (n < 0) return -1;
    buf[n] = 0;
    return n;
}

static void detect_self_dir(void) {
    char buf[2048];
    long n = readlink("/proc/self/exe", buf, sizeof buf - 1);
    if (n <= 0) return;
    buf[n] = 0;
    while (n > 0 && buf[n] != '/') n--;
    if (n > 0) buf[n] = 0;
    long i = 0;
    while (buf[i] && i < (long)sizeof SELF_DIR - 1) { SELF_DIR[i] = buf[i]; i++; }
    SELF_DIR[i] = 0;
}

static int conf_get(const char *buf, const char *key, char *out, long outsz) {
    unsigned long klen = strlen(key);
    const char *p = buf;
    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == ';' || *p == '\n' || *p == 0) {
            while (*p && *p != '\n') p++;
            if (*p) p++;
            continue;
        }
        if (strncmp(p, key, klen) == 0) {
            const char *q = p + klen;
            while (*q == ' ' || *q == '\t') q++;
            if (*q == '=') {
                q++;
                while (*q == ' ' || *q == '\t') q++;
                long i = 0;
                while (*q && *q != '\n' && *q != '\r' && i < outsz - 1) {
                    out[i++] = *q++;
                }
                while (i > 0 && (out[i-1] == ' ' || out[i-1] == '\t')) i--;
                out[i] = 0;
                return (int)i;
            }
        }
        while (*p && *p != '\n') p++;
        if (*p) p++;
    }
    return -1;
}

static void load_config(void) {
    detect_self_dir();
    snprintf(SKULL, sizeof SKULL, "%s/skull.bash", SELF_DIR);

    char path[2048];
    const char *env = getenv("OVERLAY_CONFIG");
    if (env && *env) {
        long i = 0;
        while (env[i] && i < (long)sizeof path - 1) { path[i] = env[i]; i++; }
        path[i] = 0;
    } else {
        snprintf(path, sizeof path, "%s/config.conf", SELF_DIR);
    }
    static char buf[16384];
    long n = slurp(path, buf, sizeof buf);
    if (n <= 0) return;
    conf_get(buf, "base",   BASE,   sizeof BASE);
    conf_get(buf, "upper",  UPPER,  sizeof UPPER);
    conf_get(buf, "work",   WORK,   sizeof WORK);
    conf_get(buf, "merged", MERGED, sizeof MERGED);
    conf_get(buf, "skull",  SKULL,  sizeof SKULL);
}

static void show_banner(void) {
    char cmd[4096];
    snprintf(cmd, sizeof cmd,
        "if command -v lolcat >/dev/null 2>&1; then bash %s | lolcat; else bash %s; fi",
        SKULL, SKULL);
    system(cmd);
}

static int is_mounted(const char *target) {
    static char buf[131072];
    long n = slurp("/proc/mounts", buf, sizeof buf);
    if (n <= 0) return 0;
    char needle[1200];
    snprintf(needle, sizeof needle, " %s ", target);
    return strstr(buf, needle) != 0;
}

static int ensure_dirs(void) {
    char *a[] = {"sudo","mkdir","-p",UPPER,WORK,MERGED,0};
    return run(a);
}

static int mount_overlay(void) {
    char opts[4096];
    snprintf(opts, sizeof opts, "lowerdir=%s,upperdir=%s,workdir=%s", BASE, UPPER, WORK);
    char *a[] = {"sudo","mount","-t","overlay","kali-overlay","-o",opts,MERGED,0};
    return run(a);
}

static int bind_one(const char *src, const char *subpath) {
    char dst[2048];
    snprintf(dst, sizeof dst, "%s%s", MERGED, subpath);
    char *mk[] = {"sudo","mkdir","-p",dst,0};
    run(mk);
    char *a[] = {"sudo","mount","--bind",(char*)src,dst,0};
    return run(a);
}

static int umount_one(const char *subpath) {
    char dst[2048];
    snprintf(dst, sizeof dst, "%s%s", MERGED, subpath);
    char *a[] = {"sudo","umount","-l",dst,0};
    return run(a);
}

static int copy_resolv(void) {
    char dst[2048];
    snprintf(dst, sizeof dst, "%s/etc/resolv.conf", MERGED);
    char *a[] = {"sudo","cp","--remove-destination","/etc/resolv.conf",dst,0};
    return run(a);
}

static int cmd_on(void) {
    if (is_mounted(MERGED)) { say("already on"); return 0; }
    ensure_dirs();
    if (mount_overlay()) { say("overlay mount failed"); return 1; }
    bind_one("/proc", "/proc");
    bind_one("/sys", "/sys");
    bind_one("/dev", "/dev");
    bind_one("/dev/pts", "/dev/pts");
    copy_resolv();
    say("overlay on");
    return 0;
}

static int cmd_off(void) {
    umount_one("/dev/pts");
    umount_one("/dev");
    umount_one("/sys");
    umount_one("/proc");
    char *a[] = {"sudo","umount",MERGED,0};
    if (run(a)) {
        char *a2[] = {"sudo","umount","-l",MERGED,0};
        run(a2);
    }
    say("overlay off");
    return 0;
}

static int cmd_status(void) {
    if (is_mounted(MERGED)) { say("on"); return 0; }
    say("off");
    return 1;
}

static int cmd_enter(void) {
    if (!is_mounted(MERGED)) { say("overlay not mounted, run: toggle on"); return 1; }
    char *a[] = {"sudo","chroot",MERGED,"/bin/bash","-l",0};
    return run(a);
}

static int cmd_reset(void) {
    if (is_mounted(MERGED)) { say("turn off first: toggle off"); return 1; }
    char *a[] = {"sudo","rm","-rf",UPPER,WORK,0};
    run(a);
    ensure_dirs();
    say("upper layer wiped");
    return 0;
}

static int cmd_show(void) {
    write(1, "self=",   5); write(1, SELF_DIR, strlen(SELF_DIR)); write(1, "\n", 1);
    write(1, "base=",   5); write(1, BASE,   strlen(BASE));   write(1, "\n", 1);
    write(1, "upper=",  6); write(1, UPPER,  strlen(UPPER));  write(1, "\n", 1);
    write(1, "work=",   5); write(1, WORK,   strlen(WORK));   write(1, "\n", 1);
    write(1, "merged=", 7); write(1, MERGED, strlen(MERGED)); write(1, "\n", 1);
    write(1, "skull=",  6); write(1, SKULL,  strlen(SKULL));  write(1, "\n", 1);
    return 0;
}

int main(int argc, char **argv) {
    load_config();
    if (argc < 2) {
        show_banner();
        say("usage: toggle on|off|status|enter|reset|show");
        return 2;
    }
    if (strcmp(argv[1], "on") == 0)     { show_banner(); return cmd_on(); }
    if (strcmp(argv[1], "off") == 0)    return cmd_off();
    if (strcmp(argv[1], "status") == 0) return cmd_status();
    if (strcmp(argv[1], "enter") == 0)  { show_banner(); return cmd_enter(); }
    if (strcmp(argv[1], "reset") == 0)  return cmd_reset();
    if (strcmp(argv[1], "show") == 0)   return cmd_show();
    say("unknown command");
    return 2;
}
