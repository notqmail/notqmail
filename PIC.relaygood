qmail-smtpd     Receive message by SMTP from another host:
                   
     |             MAIL FROM:<joe@heaven.af.mil>
     |             RCPT TO:<bill@irs.gov>
     |          
     |          Is $RELAYCLIENT set? Yes: "".
     |          Accept RCPT.  
     V             
                   
qmail-queue     Store message safely on disk.
                Trigger qmail-send.
     |             
     V             
                   
qmail-send      Look at envelope recipient, bill@irs.gov.
     |          Is irs.gov in locals? No.
     |          Is bill@irs.gov in virtualdomains? No.
     |          Is irs.gov in virtualdomains? No.
     |          Is .gov in virtualdomains? No.
     |          Deliver remotely to bill@irs.gov.
     V          
                   
qmail-rspawn    Run qmail-remote.

     |             
     V             

qmail-remote    Look at host name, irs.gov.
                Is irs.gov listed in smtproutes? No.
		Look up DNS MX/A for irs.gov and connect to it by SMTP:
                   
                   MAIL FROM:<joe@heaven.af.mil>
                   RCPT TO:<bill@irs.gov>
