import subprocess
import os

root_dir = os.getcwd()
executable = "main"
command = "make"
scenario = "metamodule"

# Run make in the root directory
make_result = subprocess.run([command], capture_output=True, text=True)
print(command + ": ", make_result.stdout)
if make_result.returncode != 0:
    print(command + ": ", make_result.stderr)
    exit(make_result.returncode)

# Run the executable in the root folder
main_result = subprocess.run(["./" + executable], capture_output=True, text=True)
print(executable + ": ", main_result.stdout)
if main_result.returncode != 0:
    print(executable + ":", main_result.stderr)

# Change to the Visualization directory
visualization_dir = os.path.join(root_dir, "Visualization")
os.chdir(visualization_dir)

# Run make in the Visualization directory
make_result = subprocess.run([command], capture_output=True, text=True)
print(command + ": ", make_result.stdout)
if make_result.returncode != 0:
    print(command + ": ", make_result.stderr)
    exit(make_result.returncode)

# Run the executable with the scenario file as an argument
main_result = subprocess.run(["./" + executable, scenario], capture_output=True, text=True)
print(executable + ": ", main_result.stdout)
if main_result.returncode != 0:
    print(executable + ":", main_result.stderr)

# Change back to the root directory
os.chdir(root_dir)