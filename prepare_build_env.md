# Prepare Dev Environment

{: .important}
> These can be used in the "Dockerfile.alpine" to create Docker images.


## Install Debian Packages
`sudo apt update`

`sudo apt install git cmake pkg-config curl g++ zip`


## Repo Access
- Setup keys for GitLab (see Keys profile settings on GitLab)
- [Generate](https://docs.gitlab.com/ee/user/ssh.html#generate-an-ssh-key-pair) a SSH key pair
- [Add](https://docs.gitlab.com/ee/user/ssh.html#add-an-ssh-key-to-your-gitlab-account) to GitLab


## Setup Repo

- Clone fusion repo
- cd into repo
- `git submodule update --init --recursive`


## Install vcpkg pacakges
- `./prepare_vcpkg_run_install.sh`


## Additional
- install vscode, postman
- open vscode
- select working folder as fusion repo
- install extensions: C/C++, C++ extension pack, cmake-tool, git-rename
- scan for kits, select GCC
- build fusionserver