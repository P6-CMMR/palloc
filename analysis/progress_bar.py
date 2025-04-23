import sys
import time

def print_progress_bar(completed, total, start_time=None, final=False):
    """Print a progress bar to the console"""
    width = 40
    percentage = int(100 * completed / total) if total > 0 else 0
    completed_bar = int(width * percentage / 100)
    
    time_info = ""
    if start_time:
        current_time = time.time()
        elapsed = int(current_time - start_time)
        
        elapsed_min = elapsed // 60
        elapsed_sec = elapsed % 60
        
        if completed > 0:
            per_job = elapsed / completed
            remaining = int(per_job * (total - completed))
            remaining_min = remaining // 60
            remaining_sec = remaining % 60
            time_info = f" | {elapsed_min:02d}:{elapsed_sec:02d} elapsed | ~{remaining_min:02d}:{remaining_sec:02d} remaining"
        else:
            time_info = " | calculating time..."
    
    bar = "█" * completed_bar + " " * (width - completed_bar)
    
    sys.stdout.write(f"\r\033[KProgress: [{bar}] {percentage}% ({completed}/{total}){time_info}")
    if final:
        sys.stdout.write("\n")
    sys.stdout.flush()