#!/usr/bin/python3

from pathlib import Path
import argparse
import sys
import json
import random
import string
import shutil


gpu_host = "qal-workshop~host"


def parse_ssh_config(config: str) -> list[str]:
    ret = []

    for line in config.splitlines():
        if line.startswith("Host") or line.startswith("host"):
            ret.extend(line.split(" ")[1:])

    return ret


def add_ssh_config_hosts(config_path: Path, jump_host_name: str, gpu_host_name: str, user: str, teacher_dir: Path) -> None:
    ssh_config = config_path.read_text() if config_path.is_file() else ""
    hosts = parse_ssh_config(ssh_config)
    
    jump_host = "qal-workshop-jump-host"
    apptainer_image_host = "qal-workshop~*"
    qal_workshop_image_path = teacher_dir / "JHS_data" / "qal-workshop-fakeroot.img"
    if jump_host in hosts or apptainer_image_host in hosts or gpu_host in hosts:
        raise Exception("Some of the new hosts already exist in the config")

    jump_host_config = f"""
Host {jump_host}
  HostName {jump_host_name}
  User {user}
"""

    apptainer_image_config = f"""
Host {apptainer_image_host}
  RemoteCommand kill -9 $(pgrep -U $UID -f vscode | grep -v  ^$$\$); apptainer shell --nv --no-mount hostfs --writable-tmpfs {qal_workshop_image_path}
  RequestTTY yes
"""

    gpu_host_config = f"""
Host {gpu_host}
  HostName {gpu_host_name}
  ProxyCommand ssh -W %h:%p {jump_host}
  User {user}
"""

    with config_path.open("a") as f:
        f.write(jump_host_config)
        f.write(apptainer_image_config)
        f.write(gpu_host_config)


def get_vscode_settings_path() -> Path:
    if sys.platform == 'win32':
        # Windows
        appdata = os.getenv('APPDATA')
        if appdata:
            vscode_settings_path = Path(appdata) / 'Code' / 'User' / 'settings.json'
        else:
            raise OSError("APPDATA environment variable not found.")
    elif sys.platform == 'darwin':
        # macOS
        home = Path.home()
        vscode_settings_path = home / 'Library' / 'Application Support' / 'Code' / 'User' / 'settings.json'
    elif sys.platform.startswith('linux'):
        # Linux
        home = Path.home()
        vscode_settings_path = home / '.config' / 'Code' / 'User' / 'settings.json'
    else:
        raise OSError("Unsupported OS")

    return vscode_settings_path


def add_vscode_settings() -> None:
    settings_path = get_vscode_settings_path()

    random_file_suffix = ''.join(random.choice(string.ascii_lowercase) for i in range(16))
    backup_path = f"{settings_path.stem}.{random_file_suffix}"
    shutil.copy2(settings_path, backup_path)

    settings = json.loads(settings_path.read_text())
    
    settings["remote.SSH.enableRemoteCommand"] = True

    if "remote.SSH.serverInstallPath" not in settings:
        settings["remote.SSH.serverInstallPath"] = dict()
    
    remote_vscode_path =  "~/.vscode-container/qal-workshop"
    if gpu_host in settings["remote.SSH.serverInstallPath"] and settings["remote.SSH.serverInstallPath"][gpu_host] != remote_vscode_path:
        raise Exception(f"The {gpu_host} already exists in your VSCode settings with unexpected path")
    else:
         settings["remote.SSH.serverInstallPath"][gpu_host] = remote_vscode_path        

    settings_path.write_text(json.dumps(settings, indent=4))


def main() -> None:
    parser = argparse.ArgumentParser(description='Setup ssh-config file for QAL workshop')
    
    parser.add_argument("gpu_host_name", help="The hostname of the GPU server")
    parser.add_argument("user", help="The username in the remote server of the participant")
    parser.add_argument("teacher_dir", nargs='?', default=Path("/projects") / "0" / "jhssrf003", type=Path, help="The path of the teacher directory (saved in TEACHER_DIR)")
    parser.add_argument("jump_host_name", nargs='?', help="The hostname of the login server", default="snellius.surf.nl")
    parser.add_argument("config_path", nargs='?', default=Path.home() / ".ssh" / "config", type=Path, help="Path to local ssh_config file")

    args = parser.parse_args()

    add_ssh_config_hosts(args.config_path, args.jump_host_name, args.gpu_host_name, args.user, args.teacher_dir)
    
    add_vscode_settings()


if __name__ == "__main__":
    main()