
import time
import sys
import random
import shutil

OVERLAY_TEXT = [
    r"overlay"
]

LINUX_TEXT = [
    r" linux "
]

COLORS_GLITCH = [
    "\033[38;5;39m",
    "\033[38;5;51m",
    "\033[38;5;13m",
    "\033[38;5;208m",
    "\033[38;5;196m",
    "\033[38;5;46m",
]

GRADIENT = [
    "\033[38;5;93m",
    "\033[38;5;135m",
    "\033[38;5;141m",
    "\033[38;5;255m",
    "\033[38;5;250m",
    "\033[38;5;244m",
]

RESET = "\033[0m"

def clear():
    sys.stdout.write("\033[2J\033[H")

def glitch_chars(line):
    if not line: return line
    if random.random() < 0.15:
        chars = list(line)
        for _ in range(random.randint(1, max(1, len(chars)//3))):
            idx = random.randint(0, len(chars)-1)
            if chars[idx] != " ":
                chars[idx] = random.choice(["█", "▓", "▒", "░", "╬", "§", "?", "#", "@", "%", "&", "X"])
        return "".join(chars)
    return line

def render(seconds_left, total_seconds):
    cols, rows = shutil.get_terminal_size((80, 24))

    if seconds_left < 2.0 and random.random() < 0.75:
        sys.stdout.write("\033[2J\033[H")
        sys.stdout.flush()
        return

    sys.stdout.write("\033[H")
    
    padding_top = (rows // 2) - 2
    if padding_top < 2:
        padding_top = 2
        
    is_glitching_frame = random.random() < 0.05
    if is_glitching_frame and random.random() < 0.3:
        padding_top += 1
        
    sys.stdout.write("\n" * padding_top)
    
    for line in OVERLAY_TEXT:
        glitched = glitch_chars(line) if is_glitching_frame else line

        pad_left = max(0, (cols - len(line)) // 2)
        
        colored_line = " " * pad_left
        for i, char in enumerate(glitched):
            grad_idx = int((i / max(1, len(glitched))) * len(GRADIENT))
            grad_idx = min(max(grad_idx, 0), len(GRADIENT)-1)
            
            color = GRADIENT[grad_idx]
            if is_glitching_frame and random.random() < 0.1:
                color = random.choice(COLORS_GLITCH)
                
            colored_line += f"{color}{char}"
            
        offset = " " * random.randint(1, 4) if (is_glitching_frame and random.random() < 0.4) else ""
        sys.stdout.write(f"{offset}{colored_line}{RESET}\n")
        
    for line in LINUX_TEXT:
        glitched = glitch_chars(line) if is_glitching_frame else line
        pad_left = max(0, (cols - len(line)) // 2)
        
        c = "\033[38;5;244m"
        if is_glitching_frame and random.random() < 0.2:
            c = random.choice(COLORS_GLITCH)
            
        offset = " " * random.randint(1, 3) if (is_glitching_frame and random.random() < 0.3) else ""
        sys.stdout.write(f"{offset}{' ' * pad_left}{c}{glitched}{RESET}\n")

    sys.stdout.write("\n\n")

    percent = min(100.0, max(0.0, ((total_seconds - seconds_left) / total_seconds) * 100))
    
    countdown_text = f"T-{max(0, int(seconds_left))}s"
    percent_text = f"{int(percent)}%"
    
    color_sec = "\033[38;5;39m"
    color_perc = "\033[38;5;208m"
    
    progress_line = f"{color_sec}{countdown_text}{RESET}   |   {color_perc}{percent_text}{RESET}"
    
    raw_len = len(countdown_text) + 7 + len(percent_text)
    pad_left = max(0, (cols - raw_len) // 2)
    
    if is_glitching_frame and random.random() < 0.2:
        progress_line = glitch_chars(progress_line)
        
    sys.stdout.write(f"{' ' * pad_left}{progress_line}\n")

    sys.stdout.write("\033[J")
    sys.stdout.flush()

def main():
    seconds = 100.0
    if len(sys.argv) > 1:
        try:
            seconds = float(sys.argv[1])
        except ValueError:
            pass

    clear()
    total_seconds = seconds
    end_time = time.time() + total_seconds
    
    try:
        while True:
            now = time.time()
            if now >= end_time:
                clear()
                sys.stdout.write("\nDone and done.\n")
                sys.stdout.flush()
                break
                
            left = end_time - now
            render(left, total_seconds)
            time.sleep(random.uniform(0.02, 0.08))
    except KeyboardInterrupt:
        pass
    finally:
        sys.stdout.write(f"{RESET}\n")

if __name__ == "__main__":
    main()
