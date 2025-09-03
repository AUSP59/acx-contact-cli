# SELinux Policy (Template)
A minimal module is provided in `security/selinux/acx.te`. Build and load with:
```
checkmodule -M -m -o acx.mod security/selinux/acx.te
semodule_package -o acx.pp -m acx.mod
sudo semodule -i acx.pp
```
Adapt to your distribution policies and paths.
