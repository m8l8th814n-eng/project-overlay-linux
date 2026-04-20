extern int execvp(const char *, char *const[]);
extern int fork(void);
extern int waitpid(int, int *, int);
extern int kill(int, int);
extern int setsid(void);
extern void _exit(int);
extern int open(const char *, int);
extern int close(int);
extern long read(int, void *, unsigned long);
extern long write(int, const void *, unsigned long);
extern long readlink(const char *, char *, unsigned long);
extern unsigned long strlen(const char *);
extern int strncmp(const char *, const char *, unsigned long);
extern int snprintf(char *, unsigned long, const char *, ...);
extern char *getenv(const char *);
extern int system(const char *);

static char MERGED[1024] = "/var/lib/kali-overlay/merged";
static char TOGGLE[1024] = "";
static char ANIM[1024]   = "";
static char LOG[1024]    = "/var/log/kali-overlay.log";
static char SKULL[1024]  = "";
static char LIST[1024]   = "";
static char SELF_DIR[1024] = "";

static void say(const char *s) {
    write(1, s, strlen(s));
    write(1, "\n", 1);
}

static int run_wait(char *const argv[]) {
    int pid = fork();
    if (pid == 0) { execvp(argv[0], argv); _exit(127); }
    int st = 0;
    waitpid(pid, &st, 0);
    return st;
}

static int spawn_session(char *const argv[]) {
    int pid = fork();
    if (pid == 0) {
        setsid();
        execvp(argv[0], argv);
        _exit(127);
    }
    return pid;
}

static void terminate_group(int pid) {
    kill(-pid, 15);
    int st;
    waitpid(pid, &st, 0);
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
    snprintf(TOGGLE, sizeof TOGGLE, "%s/toggle",           SELF_DIR);
    snprintf(ANIM,   sizeof ANIM,   "%s/overlaylinux.sh",  SELF_DIR);
    snprintf(SKULL,  sizeof SKULL,  "%s/skull.bash",       SELF_DIR);
    snprintf(LIST,   sizeof LIST,   "%s/packages.list",    SELF_DIR);

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
    conf_get(buf, "merged", MERGED, sizeof MERGED);
    conf_get(buf, "toggle", TOGGLE, sizeof TOGGLE);
    conf_get(buf, "anim",   ANIM,   sizeof ANIM);
    conf_get(buf, "log",    LOG,    sizeof LOG);
    conf_get(buf, "skull",  SKULL,  sizeof SKULL);
    conf_get(buf, "list",   LIST,   sizeof LIST);
}

static void show_banner(void) {
    char cmd[4096];
    snprintf(cmd, sizeof cmd,
        "if command -v lolcat >/dev/null 2>&1; then bash %s | lolcat; else bash %s; fi",
        SKULL, SKULL);
    system(cmd);
}

static int chroot_exec(const char *sh) {
    char *a[] = {"sudo","chroot",MERGED,"/bin/bash","-lc",(char*)sh,0};
    return run_wait(a);
}

static int apt_update(void) {
    char cmd[2048];
    snprintf(cmd, sizeof cmd, "apt-get update >>%s 2>&1", LOG);
    return chroot_exec(cmd);
}

static int apt_install(const char *pkg) {
    char cmd[4096];
    snprintf(cmd, sizeof cmd,
        "DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends %s >>%s 2>&1",
        pkg, LOG);
    return chroot_exec(cmd);
}

int main(int argc, char **argv) {
    load_config();
    show_banner();
    const char *list = (argc >= 2) ? argv[1] : LIST;

    char *t_on[] = {TOGGLE,"on",0};
    if (run_wait(t_on)) { say("failed to enable overlay"); return 1; }

    static char buf[131072];
    long n = slurp(list, buf, sizeof buf);
    if (n < 0) { say("cannot read package list"); return 1; }

    char anim_cmd[2048];
    snprintf(anim_cmd, sizeof anim_cmd, "while :; do %s; done", ANIM);
    char *anim_argv[] = {"/bin/bash","-c",anim_cmd,0};
    int anim = spawn_session(anim_argv);

    apt_update();

    char *p = buf;
    char *end = buf + n;
    while (p < end) {
        char *line = p;
        while (p < end && *p != '\n') p++;
        if (p < end) { *p = 0; p++; }
        while (*line == ' ' || *line == '\t') line++;
        if (*line == 0) continue;
        apt_install(line);
    }

    terminate_group(anim);
    say("");
    char done[2048];
    snprintf(done, sizeof done, "install finished, log at %s (inside chroot)", LOG);
    say(done);
    return 0;
}
