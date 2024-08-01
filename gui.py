import tkinter as tk
from tkinter import filedialog, messagebox
import subprocess
import os

class App:
    def __init__(self, root):
        self.root = root
        self.root.title("Lattice Configuration")

        self.initial_file = ""
        self.final_file = ""
        self.scen_file = ""

        self.create_widgets()

    def create_widgets(self):
        self.select_initial_button = tk.Button(self.root, text="Select Initial File", command=self.select_initial_file)
        self.select_initial_button.pack()

        self.select_final_button = tk.Button(self.root, text="Select Final File", command=self.select_final_file)
        self.select_final_button.pack()

        self.run_bfs_button = tk.Button(self.root, text="Run BFS", command=self.run_bfs)
        self.run_bfs_button.pack()

        self.select_scen_button = tk.Button(self.root, text="Select Scen File", command=self.select_scen_file)
        self.select_scen_button.pack()

        self.run_visualization_button = tk.Button(self.root, text="Run Visualization", command=self.run_visualization)
        self.run_visualization_button.pack()

        self.result_text = tk.Text(self.root, height=20, width=80)
        self.result_text.pack()

    def select_initial_file(self):
        self.initial_file = filedialog.askopenfilename(title="Select Initial JSON File", filetypes=[("JSON files", "*.json")])
        if self.initial_file:
            self.result_text.insert(tk.END, f"Selected initial file: {self.initial_file}\n")

    def select_final_file(self):
        self.final_file = filedialog.askopenfilename(title="Select Final JSON File", filetypes=[("JSON files", "*.json")])
        if self.final_file:
            self.result_text.insert(tk.END, f"Selected final file: {self.final_file}\n")

    def select_scen_file(self):
        self.scen_file = filedialog.askopenfilename(title="Select SCEN File", filetypes=[("SCEN files", "*.scen")])
        if self.scen_file:
            self.result_text.insert(tk.END, f"Selected SCEN file: {self.scen_file}\n")

    def run_bfs(self):
        if not self.initial_file or not self.final_file:
            messagebox.showerror("Error", "Please select both initial and final files.")
            return

        try:
            result = subprocess.run([
                './main.exe',
                '--initial-file', self.initial_file,
                '--final-file', self.final_file
            ], capture_output=True, text=True)
            self.result_text.insert(tk.END, result.stdout + "\n")
            if result.stderr:
                self.result_text.insert(tk.END, "Errors:\n" + result.stderr + "\n")
        except Exception as e:
            self.result_text.insert(tk.END, str(e) + "\n")

    def run_visualization(self):
        if not self.scen_file:
            messagebox.showerror("Error", "Please select .scen file.")
            return

        try:
            root_dir = os.getcwd()
            visualization_dir = os.path.join(root_dir, "Visualization")
            os.chdir(visualization_dir)
            scen_file_base = os.path.splitext(os.path.basename(self.scen_file))[0]
            print(scen_file_base)
            result = subprocess.run(['./main.exe', scen_file_base], capture_output=True, text=True)
            self.result_text.insert(tk.END, result.stdout + "\n")
            if result.stderr:
                self.result_text.insert(tk.END, "Errors:\n" + result.stderr + "\n")
            os.chdir(root_dir)
        except Exception as e:
            self.result_text.insert(tk.END, str(e) + "\n")

if __name__ == "__main__":
    root = tk.Tk()
    app = App(root)
    root.mainloop()