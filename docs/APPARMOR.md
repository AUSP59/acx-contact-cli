# AppArmor Profile
Use `security/apparmor/acx` as a starting profile. Load with:
```
sudo apparmor_parser -r -W security/apparmor/acx
sudo aa-enforce acx
```
Adjust paths based on your installation prefix.
