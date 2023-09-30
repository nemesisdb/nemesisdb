These should be similar to the "Dockerfile.alpine".

apt update
apt install git cmake pkg-config curl g++ zip

- setup keys for GitLab (see Keys profile settings on GitLab)
- clone fusion repo
- cd into repo

git submodule add https://github.com/martinus/unordered_dense.git
git submodule add https://github.com/Microsoft/vcpkg.git

cd vcpkg && ./bootstrap-vcpkg.sh
cd ../ && ./prepare_vcpkg_run_install.sh

- install vscode, postman
- open vscode
- select working folder as fusion repo
- install extensions: C/C++, git-rename, cmake(?)
- select C++ kit
