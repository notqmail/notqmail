The qmail daemons have to be restarted whenever your system reboots.
Meanwhile, sendmail doesn't have to be started any more. Here's what you
should do.

Find sendmail in your boot scripts. It's usually in either /etc/rc or
/etc/init.d/sendmail. It looks like

        sendmail -bd -q15m

-q15m means it should run the queue every 15 minutes; you may see a
different number. Comment out this line, and replace it with

        csh -cf '/var/qmail/rc &'

That's it. (Make sure you include the &.)
