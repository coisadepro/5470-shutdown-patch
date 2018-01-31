clear
cd `dirname $0`
VBoxManage startvm "GNUEFI" --type headless 
clear
sshpass -p toor ssh -o StrictHostKeyChecking=no toor@192.168.56.10 'cd "/media/sf_EFI Patch" && sh build.sh'