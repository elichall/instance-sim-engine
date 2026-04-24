import pandas as pd
import matplotlib.pyplot as plt
import os


script_dir = os.path.dirname(os.path.abspath(__file__))

repo_root = os.path.abspath(os.path.join(script_dir, "..", ".."))
csv_path = os.path.join(repo_root, "build", "orbital_performance.csv")
output_img = os.path.join(repo_root, "build", "orbital_report.png")

# Define the path to the CSV (assuming we run this script from the project root)

def main():
    if not os.path.exists(csv_path):
        print(f"Error: Could not find {csv_path}. Run the C++ simulation first!")
        return

    # 1. Load the data using Pandas
    df = pd.read_csv(csv_path)

    # Add a "Time (Seconds)" column based on the row index (since you log once per second)
    df['Time_s'] = df.index

    # 2. Setup the Matplotlib figure with 2 subplots
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 10))

    # --- Plot 1: Performance (O(N^2) Physics vs O(N) Rendering) ---
    ax1.plot(df['Time_s'], df['PhysicsTimeMS'], label='Physics Time (ms)', color='red', linewidth=2)
    ax1.plot(df['Time_s'], df['RenderTimeMS'], label='Render Time (ms)', color='blue', linewidth=2)
    
    # Optional: Plot FPS on a secondary Y-axis
    ax1_fps = ax1.twinx()
    ax1_fps.plot(df['Time_s'], df['TotalFPS'], label='Total FPS', color='green', linestyle='--')
    ax1_fps.set_ylabel('Frames Per Second', color='green')

    ax1.set_title('Engine Performance Profiling')
    ax1.set_xlabel('Time (seconds)')
    ax1.set_ylabel('Compute Time (milliseconds)')
    ax1.grid(True, linestyle=':', alpha=0.6)
    ax1.legend(loc='upper left')

    # --- Plot 2: Energy Conservation (Symplectic Euler Validation) ---
    ax2.plot(df['Time_s'], df['EnergyDrift'], label='Absolute Energy Drift (|ΔE|)', color='purple', linewidth=2)
    
    ax2.set_title('Symplectic Euler Integration: Energy Conservation')
    ax2.set_xlabel('Time (seconds)')
    ax2.set_ylabel('Energy Error (Joules)')
    ax2.grid(True, linestyle=':', alpha=0.6)
    ax2.legend(loc='upper left')

    # 3. Display the graphs!
    plt.tight_layout()
    
    output_img = os.path.join("build", "performance_report.png")
    plt.savefig(output_img, dpi=300, bbox_inches='tight')
    print(f"Success! Graphs saved to: {output_img}")

if __name__ == "__main__":
    main()