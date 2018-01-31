This package will install two patches:
- One to /EFI/BOOT/BOOTX64.efi that will run before Clover to shutdown the computer outside macOS environment
- One to /sbin/shutdown that will run before the original shutdown so that it tells the first patch to shutdown in the EFI environment or to restart

With these patches installed you’ll be able to:
- Restart as usual, both from Terminal and from macOS user interface
- Shutdown as usual, both from Terminal and from macOS user interface
- Sleep as usual (this patch has nothing to do with sleep, so no changes here)

This patch can be installed multiple times safely (if already installed, it’ll do nothing).

Every time after you update your macOS installation, prior to shutting your computer down, you MUST install the patch again by running this installer.
If you try to shutdown after an update without running this installer, you might have to do an EC reset or CMOS reset (as you had to before this patch).
That’s because some macOS updates might overwrite this patch.

Also, you should run this patch again if you update Clover, as it’s going to overwrite this patch.

To install this patch you MUST have SIP (System Integrity Protection) off. If you don’t have it, just take a look at tonymacx86. There are plenty of guides there.

Even if this patch works great, you should notice that this is some sort of a workaround. This isn’t a real fix. But, don’t worry. You should only pay attention to two things:
- Don’t force shutdown your computer (by pressing the power button)
- Don’t let the battery drain completely (you should plug your computer to AC when it gets to around 7%. If you can’t do it, shutdown your computer safely and don’t let the battery drain below 7%).
Why should you follow the above piece of advice? Because force shutdown and battery drain power off are both called “unsafe shutdown”. By unsafe I mean that your computer will not execute
this patch prior to shutting down. That means that you’ll have to do an EC reset or CMOS reset (as you had to before this patch).

This patch is intended to Sierra (macOS 10.12) and above. I don’t recommend trying this patch on OSX versions prior to Sierra.