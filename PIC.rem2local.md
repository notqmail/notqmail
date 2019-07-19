qmail-smtpd     Receive message by SMTP from another host:
                   
     |             MAIL FROM:<bill@irs.gov>
     |             RCPT TO:<joe@heaven.af.mil>
     |          
     |          Is $RELAYCLIENT set? No.
     |          Is heaven.af.mil in rcpthosts? Yes.
     |          Accept RCPT.  
     V             
                   
qmail-queue     Store message safely on disk.
                Trigger qmail-send.
     |             
     V             
                   
qmail-send      Look at envelope recipient, joe@heaven.af.mil.
     |          Is heaven.af.mil in locals? Yes.
     |          Deliver locally to joe@heaven.af.mil.
     V          
                   
qmail-lspawn ./Mailbox
                   
     |          Look at mailbox name, joe.
     |          Is joe listed in qmail-users? No.
     |          Is there a joe account? Yes.
     |          Is joe's uid nonzero? Yes.
     |          Is ~joe visible to the qmailp user? Yes.
     |          Is ~joe owned by joe? Yes.
     |          Give control of the message to joe.
     |          Run qmail-local.
     V          
                   
qmail-local joe ~joe joe '' '' heaven.af.mil bill@irs.gov ./Mailbox
                   
                Does ~joe/.qmail exist? No.
                Write message to ./Mailbox in mbox format.
