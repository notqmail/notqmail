Here's how to remove sendmail from your system.

1. Find sendmail in your boot scripts. It's usually in either /etc/rc or
   /etc/init.d/sendmail. It looks like
           sendmail -bd -q15m
   -q15m means that it should run the queue every 15 minutes; you may
   see a different number. Comment out this line.

2. Kill the sendmail daemon. You should first kill -STOP the daemon; if
   any children are running, you should kill -CONT, wait, kill -STOP
   again, and repeat ad nauseam. If there aren't any children, kill
   -TERM and then kill -CONT.

3. Check whether you have any messages in the sendmail queue,
   /var/spool/mqueue. If you do, you will have to try flushing them with
   sendmail.bak -q. If necessary, wait a while and run sendmail.bak -q
   again. Repeat until the queue is empty. This may take several days.

4. Remove the setuid bit on the sendmail binary, to prevent local users
   from gaining extra privileges through sendmail's security holes. The
   binary may be at several different locations:
      # chmod 0 /usr/lib/sendmail
      # chmod 0 /usr/sbin/sendmail
      # chmod 0 /usr/lib/sendmail.mx

5. Move the sendmail binary out of the way:
      # mv /usr/lib/sendmail /usr/lib/sendmail.bak
      # mv /usr/sbin/sendmail /usr/sbin/sendmail.bak
