cd `dirname $0`
cp dependencies/startup.nsh build/
clear
qemu-system-x86_64 -L . -no-reboot -display none -nographic -net none --bios dependencies/uefi.bin -hda fat:rw:build
rm build/NvVars >/dev/null
rm build/startup.nsh >/dev/null